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
    : RenderPipelineStage(technique, "gamma")
    , m_gammaRenderTarget(nullptr)
{
}

RenderPipelineGammaCorrectionStage::~RenderPipelineGammaCorrectionStage()
{
}

void RenderPipelineGammaCorrectionStage::create(Device* device, scene::Scene::SceneData& state)
{
    createRenderTarget(device, state);

    const renderer::VertexShader* vertShader = resource::ResourceManager::getInstance()->loadShader<renderer::VertexShader, resource::ShaderSourceFileLoader>(device,
        "offscreen.hlsl", "offscreen_vs", {}, {}/*, resource::ShaderCompileFlag::ShaderCompile_UseDXCompilerForSpirV*/);
    const renderer::FragmentShader* fragShader = resource::ResourceManager::getInstance()->loadShader<renderer::FragmentShader, resource::ShaderSourceFileLoader>(device,
        "gamma.hlsl", "gamma_ps", {}, {}/*, resource::ShaderCompileFlag::ShaderCompile_UseDXCompilerForSpirV*/);

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

    m_sampler = new renderer::SamplerState(device, renderer::SamplerFilter::SamplerFilter_Trilinear, renderer::SamplerAnisotropic::SamplerAnisotropic_4x);
    m_sampler->setWrap(renderer::SamplerWrap::TextureWrap_MirroredRepeat);
}

void RenderPipelineGammaCorrectionStage::destroy(Device* device, scene::Scene::SceneData& state)
{
}

void RenderPipelineGammaCorrectionStage::prepare(Device* device, scene::Scene::SceneData& state)
{
}

void RenderPipelineGammaCorrectionStage::execute(Device* device, scene::Scene::SceneData& state)
{
    DEBUG_MARKER_SCOPE(state.m_renderState.m_cmdList, "Gamma", color::colorrgbaf::GREEN);

    state.m_renderState.m_cmdList->beginRenderTarget(*m_gammaRenderTarget);
    state.m_renderState.m_cmdList->setViewport({ 0.f, 0.f, (f32)state.m_viewportState.m_viewpotSize._width, (f32)state.m_viewportState.m_viewpotSize._height });
    state.m_renderState.m_cmdList->setScissor({ 0.f, 0.f, (f32)state.m_viewportState.m_viewpotSize._width, (f32)state.m_viewportState.m_viewpotSize._height });
    state.m_renderState.m_cmdList->setPipelineState(*m_pipeline[0]);

    state.m_renderState.m_cmdList->bindDescriptorSet(0,
        {
            renderer::Descriptor(renderer::Descriptor::ConstantBuffer{ &state.m_viewportState._viewportBuffer, 0, sizeof(state.m_viewportState._viewportBuffer)}, 0)
        });

    struct GammaBuffer
    {
        math::float4 params;
    };
    GammaBuffer constantBuffer;
    constantBuffer.params._x = 0; //lod

    ObjectHandle composite_attachment = state.m_globalResources.get("render_target");
    ASSERT(composite_attachment.isValid(), "must be valid");
    renderer::Texture2D* texture = objectFromHandle<renderer::Texture2D>(composite_attachment);

    state.m_renderState.m_cmdList->bindDescriptorSet(1,
        {
            renderer::Descriptor(renderer::Descriptor::ConstantBuffer{ &constantBuffer, 0, sizeof(constantBuffer)}, 0),
            renderer::Descriptor(m_sampler, 1),
            renderer::Descriptor(renderer::TextureView(texture, 0, 0), 2),
        });

    state.m_renderState.m_cmdList->draw(renderer::GeometryBufferDesc(), 0, 3, 0, 1);
    state.m_renderState.m_cmdList->endRenderTarget();
}

void RenderPipelineGammaCorrectionStage::changed(Device* device, scene::Scene::SceneData& data)
{
    if (!m_gammaRenderTarget)
    {
        createRenderTarget(device, data);
    }
    else if (m_gammaRenderTarget->getRenderArea() != data.m_viewportState.m_viewpotSize)
    {
        destroyRenderTarget(device, data);
        createRenderTarget(device, data);
    }
}

void RenderPipelineGammaCorrectionStage::createRenderTarget(Device* device, scene::Scene::SceneData& data)
{
    ASSERT(m_gammaRenderTarget == nullptr, "must be nullptr");
    m_gammaRenderTarget = new renderer::RenderTargetState(device, data.m_viewportState.m_viewpotSize, 1, 0, "gamma_pass");

    renderer::Texture2D* gamma = new renderer::Texture2D(device, renderer::TextureUsage::TextureUsage_Attachment | renderer::TextureUsage::TextureUsage_Sampled,
        renderer::Format::Format_R8G8B8A8_UNorm, data.m_viewportState.m_viewpotSize, renderer::TextureSamples::TextureSamples_x1, "gamma");

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

void RenderPipelineGammaCorrectionStage::destroyRenderTarget(Device* device, scene::Scene::SceneData& data)
{
    ASSERT(m_gammaRenderTarget, "must be valid");
    renderer::Texture2D* gamma = m_gammaRenderTarget->getColorTexture<renderer::Texture2D>(0);
    delete gamma;

    delete m_gammaRenderTarget;
    m_gammaRenderTarget = nullptr;
}

} //namespace renderer
} //namespace v3d