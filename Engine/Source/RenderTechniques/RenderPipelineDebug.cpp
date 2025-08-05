#include "RenderPipelineDebug.h"

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

RenderPipelineDebugStage::RenderPipelineDebugStage(RenderTechnique* technique, scene::ModelHandler* modelHandler) noexcept
    : RenderPipelineStage(technique, "Debug")
    , m_modelHandler(modelHandler)
    , m_renderTarget(nullptr)
{
}

RenderPipelineDebugStage::~RenderPipelineDebugStage()
{
}

void RenderPipelineDebugStage::create(Device* device, scene::SceneData& scene, scene::FrameData& frame)
{
    createRenderTarget(device, scene);

    {
        const renderer::VertexShader* vertShader = resource::ResourceManager::getInstance()->loadShader<renderer::VertexShader, resource::ShaderSourceFileLoader>(device,
            "simple.hlsl", "main_vs", {}, {}, resource::ShaderCompileFlag::ShaderCompile_UseDXCompilerForSpirV);
        const renderer::FragmentShader* fragShader = resource::ResourceManager::getInstance()->loadShader<renderer::FragmentShader, resource::ShaderSourceFileLoader>(device,
            "simple.hlsl", "unlit_ps", {}, {}, resource::ShaderCompileFlag::ShaderCompile_UseDXCompilerForSpirV);

        RenderPassDesc desc;
        desc._countColorAttachments = 1;
        desc._attachmentsDesc[0]._format = Format::Format_R16G16B16A16_SFloat;
        desc._attachmentsDesc.back()._format = Format::Format_D32_SFloat;
        desc._hasDepthStencilAttahment = true;

        {
            renderer::GraphicsPipelineState* pipeline = V3D_NEW(renderer::GraphicsPipelineState, memory::MemoryLabel::MemoryGame)(device, VertexFormatSimpleLitDesc, desc,
                V3D_NEW(renderer::ShaderProgram, memory::MemoryLabel::MemoryGame)(device, vertShader, fragShader), "debug");

            pipeline->setPrimitiveTopology(renderer::PrimitiveTopology::PrimitiveTopology_TriangleList);
            pipeline->setFrontFace(renderer::FrontFace::FrontFace_Clockwise);
            pipeline->setCullMode(renderer::CullMode::CullMode_None);
            pipeline->setPolygonMode(renderer::PolygonMode::PolygonMode_Line);
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

        {
            renderer::GraphicsPipelineState* pipeline = V3D_NEW(renderer::GraphicsPipelineState, memory::MemoryLabel::MemoryGame)(device, VertexFormatSimpleLitDesc, desc,
                V3D_NEW(renderer::ShaderProgram, memory::MemoryLabel::MemoryGame)(device, vertShader, fragShader), "debug_line");

            pipeline->setPrimitiveTopology(renderer::PrimitiveTopology::PrimitiveTopology_LineList);
            pipeline->setFrontFace(renderer::FrontFace::FrontFace_Clockwise);
            pipeline->setCullMode(renderer::CullMode::CullMode_None);
            pipeline->setPolygonMode(renderer::PolygonMode::PolygonMode_Line);
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
}

void RenderPipelineDebugStage::destroy(Device* device, scene::SceneData& scene, scene::FrameData& frame)
{
    destroyRenderTarget(device, scene);

    for (auto pipeline : m_pipelines)
    {
        const renderer::ShaderProgram* program = pipeline->getShaderProgram();
        V3D_DELETE(program, memory::MemoryLabel::MemoryGame);

        V3D_DELETE(pipeline, memory::MemoryLabel::MemoryGame);
        pipeline = nullptr;
    }
    m_pipelines.clear();
}

void RenderPipelineDebugStage::prepare(Device* device, scene::SceneData& scene, scene::FrameData& frame)
{
    if (!m_renderTarget)
    {
        createRenderTarget(device, scene);
    }
    else if (m_renderTarget->getRenderArea() != scene.m_viewportState._viewpotSize)
    {
        destroyRenderTarget(device, scene);
        createRenderTarget(device, scene);
    }
}

void RenderPipelineDebugStage::execute(Device* device, scene::SceneData& scene, scene::FrameData& frame)
{
    renderer::CmdListRender* cmdList = scene.m_renderState.m_cmdList;
    scene::ViewportState& viewportState = scene.m_viewportState;

    DEBUG_MARKER_SCOPE(cmdList, "Debug", color::rgbaf::GREEN);

    ObjectHandle render_target = scene.m_globalResources.get("render_target");
    ASSERT(render_target.isValid(), "must be valid");
    renderer::Texture2D* renderTargetTexture = objectFromHandle<renderer::Texture2D>(render_target);

    ObjectHandle depth_stencil = scene.m_globalResources.get("depth_stencil");
    ASSERT(depth_stencil.isValid(), "must be valid");
    renderer::Texture2D* depthStencilTexture = objectFromHandle<renderer::Texture2D>(depth_stencil);

    m_renderTarget->setColorTexture(0, renderTargetTexture, 
        {
            renderer::RenderTargetLoadOp::LoadOp_Load, renderer::RenderTargetStoreOp::StoreOp_Store, color::Color(0.0f, 0.0f, 0.0f, 1.0f)
        },
        {
            renderer::TransitionOp::TransitionOp_ColorAttachment, renderer::TransitionOp::TransitionOp_ColorAttachment
        }
    );
    m_renderTarget->setDepthStencilTexture(depthStencilTexture,
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


    cmdList->beginRenderTarget(*m_renderTarget);
    cmdList->setViewport({ 0.f, 0.f, (f32)viewportState._viewpotSize._width, (f32)viewportState._viewpotSize._height });
    cmdList->setScissor({ 0.f, 0.f, (f32)viewportState._viewpotSize._width, (f32)viewportState._viewpotSize._height });

    for (auto& item : scene.m_lists[toEnumType(scene::MaterialType::Debug)])
    {
        const scene::Mesh& mesh = *static_cast<scene::Mesh*>(item->_object);
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
        constantBuffer.modelMatrix = mesh.getTransform();
        constantBuffer.prevModelMatrix = mesh.getPrevTransform();
        constantBuffer.normalMatrix = constantBuffer.modelMatrix.getTransposed();
        constantBuffer.tint = item->_material._tint;
        constantBuffer.objectID = 0;
        if (item->_parent)
        {
            constantBuffer.modelMatrix = item->_parent->_object->getTransform() * mesh.getTransform();
            constantBuffer.prevModelMatrix = item->_parent->_object->getPrevTransform() * mesh.getPrevTransform();
        }

        cmdList->bindDescriptorSet(1,
            {
                renderer::Descriptor(renderer::Descriptor::ConstantBuffer{ &constantBuffer, 0, sizeof(constantBuffer)}, 1),
            });

        DEBUG_MARKER_SCOPE(cmdList, std::format("Object {}, pipeline {}", item->_objectID, m_pipelines[item->_pipelineID]->getName()), color::rgbaf::LTGREY);
        renderer::GeometryBufferDesc desc(mesh.m_indexBuffer, 0, mesh.m_vertexBuffer[0], 0, sizeof(VertexFormatSimpleLit), 0);
        cmdList->drawIndexed(desc, 0, mesh.m_indexBuffer->getIndicesCount(), 0, 0, 1);
    }

    cmdList->endRenderTarget();
}

void RenderPipelineDebugStage::createRenderTarget(Device* device, scene::SceneData& data)
{
    ASSERT(m_renderTarget == nullptr, "must be nullptr");
    m_renderTarget = V3D_NEW(renderer::RenderTargetState, memory::MemoryLabel::MemoryGame)(device, data.m_viewportState._viewpotSize, 1);
}

void RenderPipelineDebugStage::destroyRenderTarget(Device* device, scene::SceneData& data)
{
    ASSERT(m_renderTarget, "must be valid");
    V3D_DELETE(m_renderTarget, memory::MemoryLabel::MemoryGame);
    m_renderTarget = nullptr;
}

} //namespace renderer
} //namespace v3d