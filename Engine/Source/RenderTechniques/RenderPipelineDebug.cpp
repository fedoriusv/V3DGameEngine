#include "RenderPipelineDebug.h"

#include "Resource/ResourceManager.h"

#include "Resource/Loader/AssetSourceFileLoader.h"
#include "Resource/Loader/ShaderSourceFileLoader.h"
#include "Resource/Loader/ModelFileLoader.h"

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
            "debug.hlsl", "simple_vs", {}, {}, resource::ShaderCompileFlag::ShaderCompile_UseDXCompilerForSpirV);
        const renderer::FragmentShader* fragShader = resource::ResourceManager::getInstance()->loadShader<renderer::FragmentShader, resource::ShaderSourceFileLoader>(device,
            "debug.hlsl", "simple_unlit_ps", {}, {}, resource::ShaderCompileFlag::ShaderCompile_UseDXCompilerForSpirV);

        RenderPassDesc desc;
        desc._countColorAttachments = 1;
        desc._attachmentsDesc[0]._format = Format::Format_R16G16B16A16_SFloat;
        desc._attachmentsDesc.back()._format = Format::Format_D32_SFloat;
        desc._hasDepthStencilAttahment = true;

        {
            renderer::GraphicsPipelineState* pipeline = new renderer::GraphicsPipelineState(
                device, VertexFormatSimpleDesc, desc, new renderer::ShaderProgram(device, vertShader, fragShader), "debug");

            pipeline->setPrimitiveTopology(renderer::PrimitiveTopology::PrimitiveTopology_TriangleList);
            pipeline->setFrontFace(renderer::FrontFace::FrontFace_Clockwise);
            pipeline->setCullMode(renderer::CullMode::CullMode_Back);
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
            renderer::GraphicsPipelineState* pipeline = new renderer::GraphicsPipelineState(
                device, VertexFormatSimpleDesc, desc, new renderer::ShaderProgram(device, vertShader, fragShader), "debug_line");

            pipeline->setPrimitiveTopology(renderer::PrimitiveTopology::PrimitiveTopology_LineList);
            pipeline->setFrontFace(renderer::FrontFace::FrontFace_Clockwise);
            pipeline->setCullMode(renderer::CullMode::CullMode_Back);
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
        delete program;

        delete pipeline;
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

    DEBUG_MARKER_SCOPE(cmdList, "Debug", color::colorrgbaf::GREEN);

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

    for (auto& list : scene.m_lists[toEnumType(scene::MaterialType::Debug)])
    {
        cmdList->setStencilRef(0);
        cmdList->setPipelineState(*m_pipelines[list->_pipelineID]);

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
        constantBuffer.modelMatrix = list->_transform.getTransform();
        constantBuffer.prevModelMatrix = list->_prevTransform.getTransform();
        constantBuffer.normalMatrix = constantBuffer.modelMatrix.getTransposed();
        constantBuffer.tint = list->_material._tint;
        constantBuffer.objectID = 0;

        cmdList->bindDescriptorSet(1,
            {
                renderer::Descriptor(renderer::Descriptor::ConstantBuffer{ &constantBuffer, 0, sizeof(constantBuffer)}, 1),
            });

        DEBUG_MARKER_SCOPE(cmdList, std::format("Object {}, pipeline {}", list->_objectID, m_pipelines[list->_pipelineID]->getName()), color::colorrgbaf::LTGREY);
        renderer::GeometryBufferDesc desc(list->_geometry._idxBuffer, 0, list->_geometry._vtxBuffer, 0, sizeof(VertexFormatSimple), 0);
        cmdList->drawIndexed(desc, 0, list->_geometry._idxBuffer->getIndicesCount(), 0, 0, 1);
    }

    cmdList->endRenderTarget();
}

void RenderPipelineDebugStage::createRenderTarget(Device* device, scene::SceneData& data)
{
    ASSERT(m_renderTarget == nullptr, "must be nullptr");
    m_renderTarget = new renderer::RenderTargetState(device, data.m_viewportState._viewpotSize, 1);
}

void RenderPipelineDebugStage::destroyRenderTarget(Device* device, scene::SceneData& data)
{
    ASSERT(m_renderTarget, "must be valid");
    delete m_renderTarget;
    m_renderTarget = nullptr;
}

} //namespace renderer
} //namespace v3d