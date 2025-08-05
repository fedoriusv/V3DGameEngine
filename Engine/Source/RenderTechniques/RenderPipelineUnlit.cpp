#include "RenderPipelineUnlit.h"

#include "Resource/ResourceManager.h"

#include "Resource/Loader/AssetSourceFileLoader.h"
#include "Resource/Loader/ShaderSourceFileLoader.h"
#include "Resource/Loader/ModelFileLoader.h"

namespace v3d
{
namespace renderer
{

RenderPipelineUnlitStage::RenderPipelineUnlitStage(RenderTechnique* technique, scene::ModelHandler* modelHandler) noexcept
    : RenderPipelineStage(technique, "Unlit")
    , m_modelHandler(modelHandler)
    , m_renderTarget(nullptr)
{
}

RenderPipelineUnlitStage::~RenderPipelineUnlitStage()
{
}

void RenderPipelineUnlitStage::create(Device* device, scene::SceneData& scene, scene::FrameData& frame)
{
    //Material 0
    {
        const renderer::VertexShader* vertShader = resource::ResourceManager::getInstance()->loadShader<renderer::VertexShader, resource::ShaderSourceFileLoader>(device,
            "simple.hlsl", "billboard_vs", {}, {}, resource::ShaderCompileFlag::ShaderCompile_UseDXCompilerForSpirV);
        const renderer::FragmentShader* fragShader = resource::ResourceManager::getInstance()->loadShader<renderer::FragmentShader, resource::ShaderSourceFileLoader>(device,
            "simple.hlsl", "unlit_selectable_ps", {}, {}, resource::ShaderCompileFlag::ShaderCompile_UseDXCompilerForSpirV);

        RenderPassDesc desc{};
        desc._countColorAttachments = 2;
        desc._attachmentsDesc[0]._format = Format_R16G16B16A16_SFloat;
        desc._attachmentsDesc[1]._format = Format_R16G16B16A16_SFloat;
        desc._hasDepthStencilAttahment = true;
        desc._attachmentsDesc.back()._format = Format_D32_SFloat;

        renderer::GraphicsPipelineState* pipeline = V3D_NEW(renderer::GraphicsPipelineState, memory::MemoryLabel::MemoryGame)(device, renderer::VertexInputAttributeDesc(), desc,
            V3D_NEW(renderer::ShaderProgram, memory::MemoryLabel::MemoryGame)(device, vertShader, fragShader), "unlit_billboard");

        pipeline->setPrimitiveTopology(renderer::PrimitiveTopology::PrimitiveTopology_TriangleStrip);
        pipeline->setFrontFace(renderer::FrontFace::FrontFace_Clockwise);
        pipeline->setCullMode(renderer::CullMode::CullMode_None);
        pipeline->setPolygonMode(renderer::PolygonMode::PolygonMode_Fill);
#if ENABLE_REVERSED_Z
        pipeline->setDepthCompareOp(renderer::CompareOperation::CompareOp_GreaterOrEqual);
#else
        pipeline->setDepthCompareOp(renderer::CompareOperation::CompareOp_LessOrEqual);
#endif
        pipeline->setDepthTest(true);
        pipeline->setDepthWrite(true);
        pipeline->setColorMask(0, renderer::ColorMask::ColorMask_All);

        m_pipelines.push_back(pipeline);
    }
}

void RenderPipelineUnlitStage::destroy(Device* device, scene::SceneData& scene, scene::FrameData& frame)
{
    for (auto& pipeline : m_pipelines)
    {
        const renderer::ShaderProgram* program = pipeline->getShaderProgram();
        V3D_DELETE(program, memory::MemoryLabel::MemoryGame);

        V3D_DELETE(pipeline, memory::MemoryLabel::MemoryGame);
        pipeline = nullptr;
    }
    m_pipelines.clear();
}

void RenderPipelineUnlitStage::prepare(Device* device, scene::SceneData& scene, scene::FrameData& frame)
{
    if (!m_renderTarget)
    {
        m_renderTarget = V3D_NEW(renderer::RenderTargetState, memory::MemoryLabel::MemoryGame)(device, scene.m_viewportState._viewpotSize, 2);
    }
    else if (m_renderTarget->getRenderArea() != scene.m_viewportState._viewpotSize)
    {
        V3D_DELETE(m_renderTarget, memory::MemoryLabel::MemoryGame);
        m_renderTarget = V3D_NEW(renderer::RenderTargetState, memory::MemoryLabel::MemoryGame)(device, scene.m_viewportState._viewpotSize, 2);
    }
}

void RenderPipelineUnlitStage::execute(Device* device, scene::SceneData& scene, scene::FrameData& frame)
{
    renderer::CmdListRender* cmdList = scene.m_renderState.m_cmdList;
    scene::ViewportState& viewportState = scene.m_viewportState;

    if (!scene.m_lists[toEnumType(scene::MaterialType::Billboard)].empty())
    {
        DEBUG_MARKER_SCOPE(cmdList, "Billboard", color::rgbaf::GREEN);

        ObjectHandle rt_h = scene.m_globalResources.get("render_target");
        ASSERT(rt_h.isValid(), "must be valid");
        renderer::Texture2D* renderTargetTexture = objectFromHandle<renderer::Texture2D>(rt_h);

        ObjectHandle gbuffer_material_h = scene.m_globalResources.get("gbuffer_material");
        ASSERT(gbuffer_material_h.isValid(), "must be valid");
        renderer::Texture2D* gbufferMaterialTexture = objectFromHandle<renderer::Texture2D>(gbuffer_material_h);

        ObjectHandle depth_stencil_h = scene.m_globalResources.get("depth_stencil");
        ASSERT(depth_stencil_h.isValid(), "must be valid");
        renderer::Texture2D* depthStencilTexture = objectFromHandle<renderer::Texture2D>(depth_stencil_h);

        m_renderTarget->setColorTexture(0, renderTargetTexture,
            {
                renderer::RenderTargetLoadOp::LoadOp_Load, renderer::RenderTargetStoreOp::StoreOp_Store, color::Color(0.0f)
            },
            {
                renderer::TransitionOp::TransitionOp_ColorAttachment, renderer::TransitionOp::TransitionOp_ColorAttachment
            });

        m_renderTarget->setColorTexture(1, gbufferMaterialTexture,
                {
                    renderer::RenderTargetLoadOp::LoadOp_Load, renderer::RenderTargetStoreOp::StoreOp_Store, color::Color(0.0f)
                },
            {
                renderer::TransitionOp::TransitionOp_ShaderRead, renderer::TransitionOp::TransitionOp_ColorAttachment
            });

        m_renderTarget->setDepthStencilTexture(depthStencilTexture,
            {
                renderer::RenderTargetLoadOp::LoadOp_Load, renderer::RenderTargetStoreOp::StoreOp_Store, 0.f
            },
            {
                renderer::RenderTargetLoadOp::LoadOp_Load, renderer::RenderTargetStoreOp::StoreOp_Store, 0U
            },
            {
                renderer::TransitionOp::TransitionOp_DepthStencilAttachment, renderer::TransitionOp::TransitionOp_DepthStencilAttachment
            });

        cmdList->beginRenderTarget(*m_renderTarget);
        cmdList->setViewport({ 0.f, 0.f, (f32)viewportState._viewpotSize._width, (f32)viewportState._viewpotSize._height });
        cmdList->setScissor({ 0.f, 0.f, (f32)viewportState._viewpotSize._width, (f32)viewportState._viewpotSize._height });

        cmdList->bindDescriptorSet(0,
            {
                renderer::Descriptor(renderer::Descriptor::ConstantBuffer{ &viewportState._viewportBuffer, 0, sizeof(viewportState._viewportBuffer)}, 0)
            });

        for (auto& item : scene.m_lists[toEnumType(scene::MaterialType::Billboard)])
        {
            cmdList->setStencilRef(0x0);
            cmdList->setPipelineState(*m_pipelines[item->_pipelineID]);

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
                    renderer::Descriptor(item->_material._sampler, 2),
                    renderer::Descriptor(renderer::TextureView(item->_material._baseColor), 3),
                });

            DEBUG_MARKER_SCOPE(cmdList, std::format("Object {}, pipeline {}", item->_objectID, m_pipelines[item->_pipelineID]->getName()), color::rgbaf::LTGREY);
            cmdList->draw(renderer::GeometryBufferDesc(), 0, 4, 0, 1);
        }

        cmdList->endRenderTarget();
    }
}

} //namespace renderer
} //namespace v3d