#include "RenderPipelineTAA.h"
#include "Utils/Logger.h"

#include "Resource/ResourceManager.h"

#include "Resource/Loader/AssetSourceFileLoader.h"
#include "Resource/Loader/ShaderSourceFileLoader.h"
#include "Resource/Loader/ModelFileLoader.h"

#include "FrameProfiler.h"

namespace v3d
{
namespace scene
{

RenderPipelineTAAStage::RenderPipelineTAAStage(RenderTechnique* technique) noexcept
    : RenderPipelineStage(technique, "TAA")

    , m_renderTarget(nullptr)
    , m_pipeline(nullptr)

    , m_resolvedTexture(nullptr)
    , m_historyTexture(nullptr)
{
}

RenderPipelineTAAStage::~RenderPipelineTAAStage()
{
}

void RenderPipelineTAAStage::create(renderer::Device* device, scene::SceneData& scene, scene::FrameData& frame)
{
    createRenderTarget(device, scene);

    const renderer::VertexShader* vertShader = resource::ResourceManager::getInstance()->loadShader<renderer::VertexShader, resource::ShaderSourceFileLoader>(device,
        "offscreen.hlsl", "offscreen_vs", {}, {}, resource::ShaderCompileFlag::ShaderCompile_UseDXCompilerForSpirV);
    const renderer::FragmentShader* fragShader = resource::ResourceManager::getInstance()->loadShader<renderer::FragmentShader, resource::ShaderSourceFileLoader>(device,
        "taa.hlsl", "main_ps", {}, {}, resource::ShaderCompileFlag::ShaderCompile_UseDXCompilerForSpirV);

    renderer::RenderPassDesc desc{};
    desc._countColorAttachments = 1;
    desc._attachmentsDesc[0]._format = scene.m_settings._colorFormat;

    m_pipeline = V3D_NEW(renderer::GraphicsPipelineState, memory::MemoryLabel::MemoryGame)(device, renderer::VertexInputAttributeDesc(), desc, 
        V3D_NEW(renderer::ShaderProgram, memory::MemoryLabel::MemoryGame)(device, vertShader, fragShader), "taa");

    m_pipeline->setPrimitiveTopology(renderer::PrimitiveTopology::PrimitiveTopology_TriangleList);
    m_pipeline->setFrontFace(renderer::FrontFace::FrontFace_Clockwise);
    m_pipeline->setCullMode(renderer::CullMode::CullMode_Back);
    m_pipeline->setDepthCompareOp(renderer::CompareOperation::Always);
    m_pipeline->setDepthWrite(false);
    m_pipeline->setDepthTest(false);
    m_pipeline->setColorMask(0, renderer::ColorMask::ColorMask_All);

    BIND_SHADER_PARAMETER(m_pipeline, m_parameters, cb_Viewport);
    BIND_SHADER_PARAMETER(m_pipeline, m_parameters, s_SamplerLinear);
    BIND_SHADER_PARAMETER(m_pipeline, m_parameters, s_SamplerPoint);
    BIND_SHADER_PARAMETER(m_pipeline, m_parameters, t_TextureBaseColor);
    BIND_SHADER_PARAMETER(m_pipeline, m_parameters, t_TextureHistory);
    BIND_SHADER_PARAMETER(m_pipeline, m_parameters, t_TextureVelocity);
}

void RenderPipelineTAAStage::destroy(renderer::Device* device, scene::SceneData& scene, scene::FrameData& frame)
{
    destroyRenderTarget(device, scene);

    const renderer::ShaderProgram* program = m_pipeline->getShaderProgram();
    V3D_DELETE(program, memory::MemoryLabel::MemoryGame);

    V3D_DELETE(m_pipeline, memory::MemoryLabel::MemoryGame);
    m_pipeline = nullptr;
}

void RenderPipelineTAAStage::prepare(renderer::Device* device, scene::SceneData& scene, scene::FrameData& frame)
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

void RenderPipelineTAAStage::execute(renderer::Device* device, scene::SceneData& scene, scene::FrameData& frame)
{
    renderer::CmdListRender* cmdList = frame.m_cmdList;
    scene::ViewportState& viewportState = scene.m_viewportState;

    {
        TRACE_PROFILER_SCOPE("TAA", color::rgba8::GREEN);
        DEBUG_MARKER_SCOPE(cmdList, "TAA", color::rgbaf::GREEN);

        cmdList->beginRenderTarget(*m_renderTarget);
        cmdList->setViewport({ 0.f, 0.f, (f32)viewportState._viewpotSize._width, (f32)viewportState._viewpotSize._height });
        cmdList->setScissor({ 0.f, 0.f, (f32)viewportState._viewpotSize._width, (f32)viewportState._viewpotSize._height });
        cmdList->setPipelineState(*m_pipeline);

        ObjectHandle render_target_h = scene.m_globalResources.get("render_target");
        ASSERT(render_target_h.isValid(), "must be valid");
        renderer::Texture2D* renderRenderTexture = objectFromHandle<renderer::Texture2D>(render_target_h);

        ObjectHandle velocity_target_h = scene.m_globalResources.get("gbuffer_velocity");
        ASSERT(velocity_target_h.isValid(), "must be valid");
        renderer::Texture2D* velocityRenderTarget = objectFromHandle<renderer::Texture2D>(velocity_target_h);

        ObjectHandle sampler_state_linear_h = scene.m_globalResources.get("linear_sampler_clamp");
        ASSERT(sampler_state_linear_h.isValid(), "must be valid");
        renderer::SamplerState* samplerStateLinear = objectFromHandle<renderer::SamplerState>(sampler_state_linear_h);

        ObjectHandle linear_sampler_clamp_h = scene.m_globalResources.get("linear_sampler_clamp");
        ASSERT(linear_sampler_clamp_h.isValid(), "must be valid");
        renderer::SamplerState* samplerStatePoint = objectFromHandle<renderer::SamplerState>(linear_sampler_clamp_h);

        cmdList->bindDescriptorSet(m_pipeline->getShaderProgram(), 0,
            {
                renderer::Descriptor(renderer::Descriptor::ConstantBuffer{ &viewportState._viewportBuffer, 0, sizeof(viewportState._viewportBuffer)}, m_parameters.cb_Viewport)
            });

        cmdList->bindDescriptorSet(m_pipeline->getShaderProgram(), 1,
            {
                renderer::Descriptor(samplerStateLinear, m_parameters.s_SamplerLinear),
                renderer::Descriptor(samplerStatePoint, m_parameters.s_SamplerPoint),
                renderer::Descriptor(renderer::TextureView(renderRenderTexture, 0, 0), m_parameters.t_TextureBaseColor),
                renderer::Descriptor(renderer::TextureView(m_historyTexture, 0, 0), m_parameters.t_TextureHistory),
                renderer::Descriptor(renderer::TextureView(velocityRenderTarget, 0, 0), m_parameters.t_TextureVelocity)
            });

        cmdList->draw(renderer::GeometryBufferDesc(), 0, 3, 0, 1);
        cmdList->endRenderTarget();
    }

    {
        TRACE_PROFILER_SCOPE("TAA Copy", color::rgba8::GREEN);
        cmdList->copy(m_resolvedTexture, m_historyTexture, { m_resolvedTexture->getWidth(), m_resolvedTexture->getHeight(), 1 });
    }

    scene.m_globalResources.bind("render_target", m_renderTarget->getColorTexture<renderer::Texture2D>(0));
}

void RenderPipelineTAAStage::createRenderTarget(renderer::Device* device, scene::SceneData& data)
{
    ASSERT(m_resolvedTexture == nullptr, "must be nullptr");
    m_resolvedTexture = V3D_NEW(renderer::Texture2D, memory::MemoryLabel::MemoryGame)(device, renderer::TextureUsage::TextureUsage_Attachment | renderer::TextureUsage::TextureUsage_Sampled | renderer::TextureUsage::TextureUsage_Read,
        renderer::Format::Format_R16G16B16A16_SFloat, data.m_viewportState._viewpotSize, renderer::TextureSamples::TextureSamples_x1, "resolved_taa");

    ASSERT(m_historyTexture == nullptr, "must be nullptr");
    m_historyTexture = V3D_NEW(renderer::Texture2D, memory::MemoryLabel::MemoryGame)(device, renderer::TextureUsage::TextureUsage_Attachment | renderer::TextureUsage::TextureUsage_Sampled | renderer::TextureUsage::TextureUsage_Write,
        renderer::Format::Format_R16G16B16A16_SFloat, data.m_viewportState._viewpotSize, renderer::TextureSamples::TextureSamples_x1, "history_taa");

    ASSERT(m_renderTarget == nullptr, "must be nullptr");
    m_renderTarget = V3D_NEW(renderer::RenderTargetState, memory::MemoryLabel::MemoryGame)(device, data.m_viewportState._viewpotSize, 1);
    m_renderTarget->setColorTexture(0, m_resolvedTexture,
        {
            renderer::RenderTargetLoadOp::LoadOp_DontCare, renderer::RenderTargetStoreOp::StoreOp_Store, color::Color(0.0f)
        },
        {
            renderer::TransitionOp::TransitionOp_Undefined, renderer::TransitionOp::TransitionOp_ColorAttachment
        });
}

void RenderPipelineTAAStage::destroyRenderTarget(renderer::Device* device, scene::SceneData& data)
{
    ASSERT(m_resolvedTexture, "must be valid");
    V3D_DELETE(m_resolvedTexture, memory::MemoryLabel::MemoryGame);
    m_resolvedTexture = nullptr;

    ASSERT(m_historyTexture, "must be valid");
    V3D_DELETE(m_historyTexture, memory::MemoryLabel::MemoryGame);
    m_historyTexture = nullptr;

    ASSERT(m_renderTarget, "must be valid");
    V3D_DELETE(m_renderTarget, memory::MemoryLabel::MemoryGame);
    m_renderTarget = nullptr;
}

} //namespace scene
} //namespace v3d
