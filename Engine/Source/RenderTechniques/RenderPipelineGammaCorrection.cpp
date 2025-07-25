#include "RenderPipelineGammaCorrection.h"

#include "Resource/ResourceManager.h"

#include "Resource/Loader/AssetSourceFileLoader.h"
#include "Resource/Loader/ShaderSourceFileLoader.h"
#include "Resource/Loader/ModelFileLoader.h"

namespace v3d
{
namespace renderer
{

RenderPipelineGammaCorrectionStage::RenderPipelineGammaCorrectionStage(RenderTechnique* technique) noexcept
    : RenderPipelineStage(technique, "Gamma")
    , m_gammaRenderTarget(nullptr)
{
}

RenderPipelineGammaCorrectionStage::~RenderPipelineGammaCorrectionStage()
{
}

void RenderPipelineGammaCorrectionStage::create(Device* device, scene::SceneData& scene, scene::FrameData& frame)
{
    createRenderTarget(device, scene);

    const renderer::VertexShader* vertShader = resource::ResourceManager::getInstance()->loadShader<renderer::VertexShader, resource::ShaderSourceFileLoader>(device,
        "offscreen.hlsl", "offscreen_vs", {}, {}, resource::ShaderCompileFlag::ShaderCompile_UseDXCompilerForSpirV);
    const renderer::FragmentShader* fragShader = resource::ResourceManager::getInstance()->loadShader<renderer::FragmentShader, resource::ShaderSourceFileLoader>(device,
        "gamma.hlsl", "gamma_ps", {}, {}, resource::ShaderCompileFlag::ShaderCompile_UseDXCompilerForSpirV);

    renderer::GraphicsPipelineState* pipeline = new renderer::GraphicsPipelineState(
        device, renderer::VertexInputAttributeDesc(), m_gammaRenderTarget->getRenderPassDesc(), new renderer::ShaderProgram(device, vertShader, fragShader), "gamma_pipeline");

    pipeline->setPrimitiveTopology(renderer::PrimitiveTopology::PrimitiveTopology_TriangleList);
    pipeline->setFrontFace(renderer::FrontFace::FrontFace_Clockwise);
    pipeline->setCullMode(renderer::CullMode::CullMode_Back);
    pipeline->setDepthCompareOp(renderer::CompareOperation::CompareOp_Always);
    pipeline->setDepthWrite(false);
    pipeline->setDepthTest(false);
    pipeline->setColorMask(0, renderer::ColorMask::ColorMask_All);

    m_pipeline.push_back(pipeline);
}

void RenderPipelineGammaCorrectionStage::destroy(Device* device, scene::SceneData& scene, scene::FrameData& frame)
{
    destroyRenderTarget(device, scene);

    for (auto pipeline : m_pipeline)
    {
        delete pipeline;
    }
    m_pipeline.clear();
}

void RenderPipelineGammaCorrectionStage::prepare(Device* device, scene::SceneData& scene, scene::FrameData& frame)
{
    if (!m_gammaRenderTarget)
    {
        createRenderTarget(device, scene);
    }
    else if (m_gammaRenderTarget->getRenderArea() != scene.m_viewportState._viewpotSize)
    {
        destroyRenderTarget(device, scene);
        createRenderTarget(device, scene);
    }
}

void RenderPipelineGammaCorrectionStage::execute(Device* device, scene::SceneData& scene, scene::FrameData& frame)
{
    renderer::CmdListRender* cmdList = scene.m_renderState.m_cmdList;
    scene::ViewportState& viewportState = scene.m_viewportState;

    DEBUG_MARKER_SCOPE(cmdList, "Gamma", color::colorrgbaf::GREEN);

    cmdList->beginRenderTarget(*m_gammaRenderTarget);
    cmdList->setViewport({ 0.f, 0.f, (f32)viewportState._viewpotSize._width, (f32)viewportState._viewpotSize._height });
    cmdList->setScissor({ 0.f, 0.f, (f32)viewportState._viewpotSize._width, (f32)viewportState._viewpotSize._height });
    cmdList->setPipelineState(*m_pipeline[0]);

    cmdList->bindDescriptorSet(0,
        {
            renderer::Descriptor(renderer::Descriptor::ConstantBuffer{ &viewportState._viewportBuffer, 0, sizeof(viewportState._viewportBuffer)}, 0)
        });

    ObjectHandle composite_attachment = scene.m_globalResources.get("render_target");
    ASSERT(composite_attachment.isValid(), "must be valid");
    renderer::Texture2D* texture = objectFromHandle<renderer::Texture2D>(composite_attachment);

    ObjectHandle sampler_state_h = scene.m_globalResources.get("linear_sampler_mirror");
    ASSERT(sampler_state_h.isValid(), "must be valid");
    renderer::SamplerState* sampler_state = objectFromHandle<renderer::SamplerState>(sampler_state_h);

    cmdList->bindDescriptorSet(1,
        {
            renderer::Descriptor(sampler_state, 0),
            renderer::Descriptor(renderer::TextureView(texture, 0, 0), 1),
        });

    cmdList->draw(renderer::GeometryBufferDesc(), 0, 3, 0, 1);
    cmdList->endRenderTarget();
}

void RenderPipelineGammaCorrectionStage::createRenderTarget(Device* device, scene::SceneData& data)
{
    ASSERT(m_gammaRenderTarget == nullptr, "must be nullptr");
    m_gammaRenderTarget = new renderer::RenderTargetState(device, data.m_viewportState._viewpotSize, 1, 0, "gamma_pass");

    renderer::Texture2D* gamma = new renderer::Texture2D(device, renderer::TextureUsage::TextureUsage_Attachment | renderer::TextureUsage::TextureUsage_Sampled,
        renderer::Format::Format_R8G8B8A8_UNorm, data.m_viewportState._viewpotSize, renderer::TextureSamples::TextureSamples_x1, "gamma");

    m_gammaRenderTarget->setColorTexture(0, gamma,
        {
            renderer::RenderTargetLoadOp::LoadOp_DontCare, renderer::RenderTargetStoreOp::StoreOp_Store, color::Color(0.0f)
        },
        {
            renderer::TransitionOp::TransitionOp_Undefined, renderer::TransitionOp::TransitionOp_ColorAttachment
        }
    );

    data.m_globalResources.bind("final", gamma);
}

void RenderPipelineGammaCorrectionStage::destroyRenderTarget(Device* device, scene::SceneData& data)
{
    ASSERT(m_gammaRenderTarget, "must be valid");
    renderer::Texture2D* gamma = m_gammaRenderTarget->getColorTexture<renderer::Texture2D>(0);
    delete gamma;

    delete m_gammaRenderTarget;
    m_gammaRenderTarget = nullptr;
}

} //namespace renderer
} //namespace v3d