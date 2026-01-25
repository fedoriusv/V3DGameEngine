#include "RenderPipelineGammaCorrection.h"
#include "Utils/Logger.h"

#include "Resource/ResourceManager.h"
#include "Resource/Loader/AssetSourceFileLoader.h"
#include "Resource/Loader/ShaderSourceFileLoader.h"
#include "Resource/Loader/ModelFileLoader.h"

#include "Renderer/ShaderProgram.h"

#include "FrameProfiler.h"

namespace v3d
{
namespace scene
{

RenderPipelineGammaCorrectionStage::RenderPipelineGammaCorrectionStage(RenderTechnique* technique) noexcept
    : RenderPipelineStage(technique, "Gamma")
    , m_gammaRenderTarget(nullptr)
    , m_pipeline(nullptr)
{
}

RenderPipelineGammaCorrectionStage::~RenderPipelineGammaCorrectionStage()
{
}

void RenderPipelineGammaCorrectionStage::create(renderer::Device* device, scene::SceneData& scene, scene::FrameData& frame)
{
    createRenderTarget(device, scene);

    const renderer::VertexShader* vertShader = resource::ResourceManager::getInstance()->loadShader<renderer::VertexShader, resource::ShaderSourceFileLoader>(device,
        "offscreen.hlsl", "offscreen_vs", {}, {}, resource::ShaderCompileFlag::ShaderCompile_UseDXCompilerForSpirV);
    const renderer::FragmentShader* fragShader = resource::ResourceManager::getInstance()->loadShader<renderer::FragmentShader, resource::ShaderSourceFileLoader>(device,
        "tonemapping.hlsl", "tonemapping_ps", {}, {}, resource::ShaderCompileFlag::ShaderCompile_UseDXCompilerForSpirV);

    m_pipeline = V3D_NEW(renderer::GraphicsPipelineState, memory::MemoryLabel::MemoryGame)(device, renderer::VertexInputAttributeDesc(), m_gammaRenderTarget->getRenderPassDesc(),
        V3D_NEW(renderer::ShaderProgram, memory::MemoryLabel::MemoryGame)(device, vertShader, fragShader), "gamma_pipeline");

    m_pipeline->setPrimitiveTopology(renderer::PrimitiveTopology::PrimitiveTopology_TriangleList);
    m_pipeline->setFrontFace(renderer::FrontFace::FrontFace_Clockwise);
    m_pipeline->setCullMode(renderer::CullMode::CullMode_Back);
    m_pipeline->setDepthCompareOp(renderer::CompareOperation::Always);
    m_pipeline->setDepthWrite(false);
    m_pipeline->setDepthTest(false);
    m_pipeline->setColorMask(0, renderer::ColorMask::ColorMask_All);

    BIND_SHADER_PARAMETER(m_pipeline, m_parameters, cb_Viewport);
    BIND_SHADER_PARAMETER(m_pipeline, m_parameters, cb_Tonemapper);
    BIND_SHADER_PARAMETER(m_pipeline, m_parameters, s_ColorSampler);
    BIND_SHADER_PARAMETER(m_pipeline, m_parameters, t_ColorTexture);
}

void RenderPipelineGammaCorrectionStage::destroy(renderer::Device* device, scene::SceneData& scene, scene::FrameData& frame)
{
    destroyRenderTarget(device, scene);

    if (m_pipeline)
    {
        const renderer::ShaderProgram* program = m_pipeline->getShaderProgram();
        V3D_DELETE(program, memory::MemoryLabel::MemoryGame);

        V3D_DELETE(m_pipeline, memory::MemoryLabel::MemoryGame);
        m_pipeline = nullptr;
    }
}

void RenderPipelineGammaCorrectionStage::prepare(renderer::Device* device, scene::SceneData& scene, scene::FrameData& frame)
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

void RenderPipelineGammaCorrectionStage::execute(renderer::Device* device, scene::SceneData& scene, scene::FrameData& frame)
{
    renderer::CmdListRender* cmdList = frame.m_cmdList;
    scene::ViewportState& viewportState = scene.m_viewportState;

    TRACE_PROFILER_SCOPE("Gamma", color::rgba8::GREEN);
    DEBUG_MARKER_SCOPE(cmdList, "Gamma", color::rgbaf::GREEN);

    cmdList->beginRenderTarget(*m_gammaRenderTarget);
    cmdList->setViewport({ 0.f, 0.f, (f32)viewportState._viewpotSize._width, (f32)viewportState._viewpotSize._height });
    cmdList->setScissor({ 0.f, 0.f, (f32)viewportState._viewpotSize._width, (f32)viewportState._viewpotSize._height });
    cmdList->setPipelineState(*m_pipeline);

    cmdList->bindDescriptorSet(m_pipeline->getShaderProgram(), 0,
        {
            renderer::Descriptor(renderer::Descriptor::ConstantBuffer{ &viewportState._viewportBuffer, 0, sizeof(viewportState._viewportBuffer)}, m_parameters.cb_Viewport)
        });

    struct Tonemapper
    {
        u32     tonemapper;
        u32     lut;
        f32     exposure;
        f32     gamma;
    } tonemapper;

    tonemapper.tonemapper = scene.m_settings._tonemapParams._tonemapper;
    tonemapper.lut = 0u;
    tonemapper.exposure = 1.0f;
    tonemapper.gamma = scene.m_settings._tonemapParams._gamma;

    ObjectHandle composite_attachment = scene.m_globalResources.get("render_target");
    ASSERT(composite_attachment.isValid(), "must be valid");
    renderer::Texture2D* texture = objectFromHandle<renderer::Texture2D>(composite_attachment);

    ObjectHandle sampler_state_h = scene.m_globalResources.get("linear_sampler_mirror");
    ASSERT(sampler_state_h.isValid(), "must be valid");
    renderer::SamplerState* sampler_state = objectFromHandle<renderer::SamplerState>(sampler_state_h);

    cmdList->bindDescriptorSet(m_pipeline->getShaderProgram(), 1,
        {
            renderer::Descriptor(renderer::Descriptor::ConstantBuffer{ &tonemapper, 0, sizeof(Tonemapper)}, m_parameters.cb_Tonemapper),
            renderer::Descriptor(sampler_state, m_parameters.s_ColorSampler),
            renderer::Descriptor(renderer::TextureView(texture, 0, 0), m_parameters.t_ColorTexture),
        });

    cmdList->draw(renderer::GeometryBufferDesc(), 0, 3, 0, 1);
    cmdList->endRenderTarget();
}

void RenderPipelineGammaCorrectionStage::createRenderTarget(renderer::Device* device, scene::SceneData& data)
{
    ASSERT(m_gammaRenderTarget == nullptr, "must be nullptr");
    m_gammaRenderTarget = V3D_NEW(renderer::RenderTargetState, memory::MemoryLabel::MemoryGame)(device, data.m_viewportState._viewpotSize, 1, 0);

    renderer::Texture2D* gamma = V3D_NEW(renderer::Texture2D, memory::MemoryLabel::MemoryGame)(device, renderer::TextureUsage::TextureUsage_Attachment | renderer::TextureUsage::TextureUsage_Sampled,
        renderer::Format::Format_R8G8B8A8_UNorm, data.m_viewportState._viewpotSize, renderer::TextureSamples::TextureSamples_x1, "gamma");

    m_gammaRenderTarget->setColorTexture(0, gamma,
        {
            renderer::RenderTargetLoadOp::LoadOp_DontCare, renderer::RenderTargetStoreOp::StoreOp_Store, color::Color(0.0f)
        },
        {
            renderer::TransitionOp::TransitionOp_ColorAttachment, renderer::TransitionOp::TransitionOp_ShaderRead
        });

    data.m_globalResources.bind("final", gamma);
}

void RenderPipelineGammaCorrectionStage::destroyRenderTarget(renderer::Device* device, scene::SceneData& data)
{
    ASSERT(m_gammaRenderTarget, "must be valid");
    renderer::Texture2D* gamma = m_gammaRenderTarget->getColorTexture<renderer::Texture2D>(0);
    V3D_DELETE(gamma, memory::MemoryLabel::MemoryGame);

    V3D_DELETE(m_gammaRenderTarget, memory::MemoryLabel::MemoryGame);
    m_gammaRenderTarget = nullptr;
}

} //namespace scene
} //namespace v3d