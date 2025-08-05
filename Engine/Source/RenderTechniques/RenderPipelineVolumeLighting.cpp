#include "RenderPipelineVolumeLighting.h"

#include "Resource/ResourceManager.h"

#include "Resource/Loader/AssetSourceFileLoader.h"
#include "Resource/Loader/ShaderSourceFileLoader.h"
#include "Resource/Loader/ModelFileLoader.h"

#include "Scene/ModelHandler.h"
#include "Scene/Geometry/Mesh.h"

namespace v3d
{
namespace renderer
{

RenderPipelineVolumeLightingStage::RenderPipelineVolumeLightingStage(RenderTechnique* technique, scene::ModelHandler* modelHandler) noexcept
    : RenderPipelineStage(technique, "VolumeLight")
    , m_modelHandler(modelHandler)
    , m_lightRenderTarget(nullptr)
{
}

RenderPipelineVolumeLightingStage::~RenderPipelineVolumeLightingStage()
{
}

void RenderPipelineVolumeLightingStage::create(Device* device, scene::SceneData& scene, scene::FrameData& frame)
{
    createRenderTarget(device, scene);

    {
        const renderer::VertexShader* vertShader = resource::ResourceManager::getInstance()->loadShader<renderer::VertexShader, resource::ShaderSourceFileLoader>(device,
            "simple.hlsl", "main_vs", {}, {}, resource::ShaderCompileFlag::ShaderCompile_UseDXCompilerForSpirV);
        const renderer::FragmentShader* fragShader = resource::ResourceManager::getInstance()->loadShader<renderer::FragmentShader, resource::ShaderSourceFileLoader>(device,
            "light.hlsl", "light_primitive_ps", {}, {}, resource::ShaderCompileFlag::ShaderCompile_UseDXCompilerForSpirV);

        RenderPassDesc desc{};
        desc._countColorAttachments = 1;
        desc._attachmentsDesc[0]._format = Format_R16G16B16A16_SFloat;
        desc._hasDepthStencilAttahment = true;
        desc._attachmentsDesc.back()._format = Format_D32_SFloat;

        renderer::GraphicsPipelineState* pipeline = V3D_NEW(renderer::GraphicsPipelineState, memory::MemoryLabel::MemoryGame)(device, VertexFormatSimpleLitDesc, desc,
            V3D_NEW(renderer::ShaderProgram, memory::MemoryLabel::MemoryGame)(device, vertShader, fragShader), "lighting");

        pipeline->setPrimitiveTopology(renderer::PrimitiveTopology::PrimitiveTopology_TriangleList);
        pipeline->setFrontFace(renderer::FrontFace::FrontFace_Clockwise);
        pipeline->setCullMode(renderer::CullMode::CullMode_Back);
        pipeline->setPolygonMode(renderer::PolygonMode::PolygonMode_Fill);
#if ENABLE_REVERSED_Z
        pipeline->setDepthCompareOp(renderer::CompareOperation::CompareOp_GreaterOrEqual);
#else
        pipeline->setDepthCompareOp(renderer::CompareOperation::CompareOp_LessOrEqual);
#endif
        pipeline->setDepthTest(true);
        pipeline->setDepthWrite(false);
        pipeline->setColorMask(0, renderer::ColorMask::ColorMask_All);
        pipeline->setColorBlendFactor(0, renderer::BlendFactor::BlendFactor_One, renderer::BlendFactor::BlendFactor_One);
        pipeline->setColorBlendOp(0, renderer::BlendOperation::BlendOp_Add);

        m_pipelines.push_back(pipeline);
    }
}

void RenderPipelineVolumeLightingStage::destroy(Device* device, scene::SceneData& scene, scene::FrameData& frame)
{
    destroyRenderTarget(device, scene);

    for (auto& pipeline : m_pipelines)
    {
        const renderer::ShaderProgram* program = pipeline->getShaderProgram();
        V3D_DELETE(program, memory::MemoryLabel::MemoryGame);

        V3D_DELETE(pipeline, memory::MemoryLabel::MemoryGame);
        pipeline = nullptr;
    }
    m_pipelines.clear();
}

void RenderPipelineVolumeLightingStage::prepare(Device* device, scene::SceneData& scene, scene::FrameData& frame)
{
    if (!m_lightRenderTarget)
    {
        createRenderTarget(device, scene);
    }
    else if (m_lightRenderTarget->getRenderArea() != scene.m_viewportState._viewpotSize)
    {
        destroyRenderTarget(device, scene);
        createRenderTarget(device, scene);
    }
}

void RenderPipelineVolumeLightingStage::execute(Device* device, scene::SceneData& scene, scene::FrameData& frame)
{
    renderer::CmdListRender* cmdList = scene.m_renderState.m_cmdList;
    scene::ViewportState& viewportState = scene.m_viewportState;

    DEBUG_MARKER_SCOPE(cmdList, "VolumeLight", color::rgbaf::GREEN);

    if (!scene.m_lists[toEnumType(scene::MaterialType::Lights)].empty())
    {
        ObjectHandle rt_h = scene.m_globalResources.get("render_target");
        ASSERT(rt_h.isValid(), "must be valid");
        renderer::Texture2D* renderTargetTexture = objectFromHandle<renderer::Texture2D>(rt_h);

        ObjectHandle depth_stencil_h = scene.m_globalResources.get("depth_stencil");
        ASSERT(depth_stencil_h.isValid(), "must be valid");
        renderer::Texture2D* depthStencilTexture = objectFromHandle<renderer::Texture2D>(depth_stencil_h);

        m_lightRenderTarget->setColorTexture(0, renderTargetTexture,
            {
                renderer::RenderTargetLoadOp::LoadOp_Load, renderer::RenderTargetStoreOp::StoreOp_Store, color::Color(0.0f)
            },
            {
                renderer::TransitionOp::TransitionOp_ColorAttachment, renderer::TransitionOp::TransitionOp_ColorAttachment
            }
        );

        m_lightRenderTarget->setDepthStencilTexture(depthStencilTexture,
            {
                renderer::RenderTargetLoadOp::LoadOp_Load, renderer::RenderTargetStoreOp::StoreOp_Store, 0.f
            },
            {
                renderer::RenderTargetLoadOp::LoadOp_Load, renderer::RenderTargetStoreOp::StoreOp_Store, 0U
            },
            {
                renderer::TransitionOp::TransitionOp_DepthStencilAttachment, renderer::TransitionOp::TransitionOp_DepthStencilAttachment
            }
        );

        cmdList->beginRenderTarget(*m_lightRenderTarget);
        cmdList->setViewport({ 0.f, 0.f, (f32)viewportState._viewpotSize._width, (f32)viewportState._viewpotSize._height });
        cmdList->setScissor({ 0.f, 0.f, (f32)viewportState._viewpotSize._width, (f32)viewportState._viewpotSize._height });

        for (auto& item : scene.m_lists[toEnumType(scene::MaterialType::Lights)])
        {
            const scene::Light& light = *static_cast<scene::Light*>(item->_object);
            const scene::Mesh* volume = light.getVolumeMesh();
            ASSERT(volume, "missing volume");

            cmdList->setPipelineState(*m_pipelines[item->_pipelineID]);

            cmdList->bindDescriptorSet(0,
                {
                    renderer::Descriptor(renderer::Descriptor::ConstantBuffer{ &viewportState._viewportBuffer, 0, sizeof(viewportState._viewportBuffer)}, 0)
                });

            struct ModelBuffer
            {
                math::Matrix4D modelMatrix;
                math::Matrix4D prevModelMatrix;
                math::Matrix4D normalMatrix;
                math::float4   tint;
                u64            objectID;
                u64           _pad = 0;
            };

            ModelBuffer constantBuffer;
            constantBuffer.modelMatrix = item->_object->getTransform();
            constantBuffer.prevModelMatrix = item->_object->getPrevTransform();
            constantBuffer.normalMatrix = constantBuffer.modelMatrix.getInversed();
            constantBuffer.normalMatrix.makeTransposed();
            constantBuffer.tint = item->_material._tint;
            constantBuffer.objectID = item->_objectID;

            cmdList->bindDescriptorSet(1,
                {
                    renderer::Descriptor(renderer::Descriptor::ConstantBuffer{ &constantBuffer, 0, sizeof(constantBuffer)}, 1),
                });

            DEBUG_MARKER_SCOPE(cmdList, std::format("Object {}, pipeline {}", item->_objectID, m_pipelines[item->_pipelineID]->getName()), color::rgbaf::LTGREY);
            renderer::GeometryBufferDesc desc(volume->m_indexBuffer, 0, volume->m_vertexBuffer[0], 0, sizeof(VertexFormatSimpleLit), 0);
            cmdList->drawIndexed(desc, 0, volume->m_indexBuffer->getIndicesCount(), 0, 0, 1);
        }

        cmdList->endRenderTarget();
    }
}

void RenderPipelineVolumeLightingStage::createRenderTarget(Device* device, scene::SceneData& data)
{
    ASSERT(m_lightRenderTarget == nullptr, "must be nullptr");
    m_lightRenderTarget = V3D_NEW(renderer::RenderTargetState, memory::MemoryLabel::MemoryGame)(device, data.m_viewportState._viewpotSize, 1);
}

void RenderPipelineVolumeLightingStage::destroyRenderTarget(Device* device, scene::SceneData& data)
{
    ASSERT(m_lightRenderTarget, "must be valid");
    V3D_DELETE(m_lightRenderTarget, memory::MemoryLabel::MemoryGame);
    m_lightRenderTarget = nullptr;
}

} // namespace renderer
} // namespace v3d