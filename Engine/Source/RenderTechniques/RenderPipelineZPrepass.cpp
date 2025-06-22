#include "RenderPipelineZPrepass.h"

#include "Resource/ResourceManager.h"

#include "Resource/Loader/AssetSourceFileLoader.h"
#include "Resource/Loader/ShaderSourceFileLoader.h"
#include "Resource/Loader/ModelFileLoader.h"

#include "Task/RenderTask/RenderTask.h"

namespace v3d
{
namespace renderer
{

RenderPipelineZPrepassStage::RenderPipelineZPrepassStage(RenderTechnique* technique, scene::ModelHandler* modelHandler) noexcept
    : RenderPipelineStage(technique, "zprepass")

    , m_depthRenderTarget(nullptr)
    , m_depthPipeline(nullptr)
{
}

RenderPipelineZPrepassStage::~RenderPipelineZPrepassStage()
{
}

void RenderPipelineZPrepassStage::create(Device* device, scene::SceneData& state)
{
    createRenderTarget(device, state);

    const renderer::VertexShader* vertShader = resource::ResourceManager::getInstance()->loadShader<renderer::VertexShader, resource::ShaderSourceFileLoader>(device,
        "gbuffer.hlsl", "gbuffer_standard_vs", {}, {}/*, resource::ShaderCompileFlag::ShaderCompile_UseDXCompilerForSpirV*/);
    const renderer::FragmentShader* fragShader = resource::ResourceManager::getInstance()->loadShader<renderer::FragmentShader, resource::ShaderSourceFileLoader>(device,
        "gbuffer.hlsl", "gbuffer_depth_ps", {}, {}/*, resource::ShaderCompileFlag::ShaderCompile_UseDXCompilerForSpirV*/);

    m_depthPipeline = new renderer::GraphicsPipelineState(
        device, VertexFormatStandardDesc, m_depthRenderTarget->getRenderPassDesc(), new renderer::ShaderProgram(device, vertShader, fragShader), "prepass_pipeline");

   m_depthPipeline->setPrimitiveTopology(renderer::PrimitiveTopology::PrimitiveTopology_TriangleList);
   m_depthPipeline->setFrontFace(renderer::FrontFace::FrontFace_Clockwise);
   m_depthPipeline->setCullMode(renderer::CullMode::CullMode_Back);
#if ENABLE_REVERSED_Z
   m_depthPipeline->setDepthCompareOp(renderer::CompareOperation::CompareOp_GreaterOrEqual);
#else
   m_depthPipeline->setDepthCompareOp(renderer::CompareOperation::CompareOp_LessOrEqual);
#endif
    m_depthPipeline->setDepthWrite(true);
    m_depthPipeline->setDepthTest(true);
   m_depthPipeline->setColorMask(0, renderer::ColorMask::ColorMask_All);
}

void RenderPipelineZPrepassStage::destroy(Device* device, scene::SceneData& state)
{
    destroyRenderTarget(device, state);

    const renderer::ShaderProgram* program = m_depthPipeline->getShaderProgram();
    delete program;

    delete m_depthPipeline;
    m_depthPipeline = nullptr;
}

void RenderPipelineZPrepassStage::prepare(Device* device, scene::SceneData& state)
{
    if (!m_depthRenderTarget)
    {
        createRenderTarget(device, state);
    }
    else if (m_depthRenderTarget->getRenderArea() != state.m_viewportState._viewpotSize)
    {
        destroyRenderTarget(device, state);
        createRenderTarget(device, state);
    }
}

void RenderPipelineZPrepassStage::execute(Device* device, scene::SceneData& state)
{
    static auto renderTask = [this, &state](Device* device, CmdListRender* cmdList) -> void
        {
            DEBUG_MARKER_SCOPE(cmdList, "ZPrepass", color::colorrgbaf::GREEN);

            cmdList->beginRenderTarget(*m_depthRenderTarget);
            cmdList->setViewport({ 0.f, 0.f, (f32)state.m_viewportState._viewpotSize._width, (f32)state.m_viewportState._viewpotSize._height });
            cmdList->setScissor({ 0.f, 0.f, (f32)state.m_viewportState._viewpotSize._width, (f32)state.m_viewportState._viewpotSize._height });
            cmdList->setStencilRef(0);

            cmdList->setPipelineState(*m_depthPipeline);

            cmdList->bindDescriptorSet(0,
                {
                    renderer::Descriptor(renderer::Descriptor::ConstantBuffer{ &state.m_viewportState._viewportBuffer, 0, sizeof(state.m_viewportState._viewportBuffer)}, 0)
                });

            for (auto& draw : state.m_data)
            {
                if (draw.m_stageID == "gbuffer")
                {
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
                    constantBuffer.modelMatrix = draw.m_transform.getTransform();
                    constantBuffer.prevModelMatrix = draw.m_prevTransform.getTransform();
                    constantBuffer.normalMatrix = constantBuffer.modelMatrix.getTransposed();
                    constantBuffer.tint = draw.m_tint;
                    constantBuffer.objectID = draw.m_objectID;

                    cmdList->bindDescriptorSet(1,
                        {
                            renderer::Descriptor(renderer::Descriptor::ConstantBuffer{ &constantBuffer, 0, sizeof(constantBuffer)}, 1),
                        });

                    DEBUG_MARKER_SCOPE(cmdList, std::format("Object {}, pipeline {}", draw.m_objectID, m_depthPipeline->getName()), color::colorrgbaf::LTGREY);
                    renderer::GeometryBufferDesc desc(draw.m_IdxBuffer, 0, draw.m_VtxBuffer, 0, sizeof(VertexFormatStandard), 0);
                    cmdList->drawIndexed(desc, 0, draw.m_IdxBuffer->getIndicesCount(), 0, 0, 1);
                }
            }

            cmdList->endRenderTarget();
        };

    //TODO move to another thread
    renderTask(device, state.m_renderState.m_cmdList);
}

void RenderPipelineZPrepassStage::createRenderTarget(Device* device, scene::SceneData& data)
{
#if ENABLE_REVERSED_Z
    f32 clearValue = 0.0f;
#else
    f32 clearValue = 1.0f;
#endif

    ASSERT(m_depthRenderTarget == nullptr, "must be nullptr");
    m_depthRenderTarget = new renderer::RenderTargetState(device, data.m_viewportState._viewpotSize, 0, 0, "zprepass");

    renderer::Texture2D* depthStencilAttachment = new renderer::Texture2D(device, renderer::TextureUsage::TextureUsage_Attachment | renderer::TextureUsage::TextureUsage_Sampled,
        renderer::Format::Format_R32_SFloat, data.m_viewportState._viewpotSize, renderer::TextureSamples::TextureSamples_x1, "depth_stencil");
    m_depthRenderTarget->setDepthStencilTexture(depthStencilAttachment,
        {
            renderer::RenderTargetLoadOp::LoadOp_Clear, renderer::RenderTargetStoreOp::StoreOp_Store, clearValue
        },
        {
            renderer::RenderTargetLoadOp::LoadOp_Clear, renderer::RenderTargetStoreOp::StoreOp_Store, 0U
        },
        {
            renderer::TransitionOp::TransitionOp_Undefined, renderer::TransitionOp::TransitionOp_DepthStencilAttachment
        }
    );

    data.m_globalResources.bind("depth_stencil", depthStencilAttachment);
}

void RenderPipelineZPrepassStage::destroyRenderTarget(Device* device, scene::SceneData& data)
{
    ASSERT(m_depthRenderTarget, "must be valid");
    renderer::Texture2D* depthStencilAttachment = m_depthRenderTarget->getDepthStencilTexture<renderer::Texture2D>();
    delete depthStencilAttachment;

    delete m_depthRenderTarget;
    m_depthRenderTarget = nullptr;
}

} //namespace renderer
} //namespace v3d