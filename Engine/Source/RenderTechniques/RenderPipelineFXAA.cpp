#include "RenderTechniques/RenderPipelineFXAA.h"
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

RenderPipelineFXAAStage::RenderPipelineFXAAStage(RenderTechnique* technique) noexcept
    : RenderPipelineStage(technique, "FXAA")
    , m_renderTarget(nullptr)
    , m_pipeline(nullptr)
{
}

RenderPipelineFXAAStage::~RenderPipelineFXAAStage()
{
}

void RenderPipelineFXAAStage::create(renderer::Device* device, scene::SceneData& scene, scene::FrameData& frame)
{
    createRenderTarget(device, scene, frame);

    const renderer::VertexShader* vertShader = resource::ResourceManager::getInstance()->loadShader<renderer::VertexShader, resource::ShaderSourceFileLoader>(device,
        "offscreen.hlsl", "offscreen_vs", {}, {}, resource::ShaderCompileFlag::ShaderCompile_UseDXCompilerForSpirV);
    const renderer::FragmentShader* fragShader = resource::ResourceManager::getInstance()->loadShader<renderer::FragmentShader, resource::ShaderSourceFileLoader>(device,
        "fxaa.hlsl", "fxaa_ps", {}, {}, resource::ShaderCompileFlag::ShaderCompile_UseDXCompilerForSpirV);

    m_pipeline = V3D_NEW(renderer::GraphicsPipelineState, memory::MemoryLabel::MemoryGame)(device, renderer::VertexInputAttributeDesc(), m_renderTarget->getRenderPassDesc(),
        V3D_NEW(renderer::ShaderProgram, memory::MemoryLabel::MemoryGame)(device, vertShader, fragShader), "fxaa_pipeline");

    m_pipeline->setPrimitiveTopology(renderer::PrimitiveTopology::PrimitiveTopology_TriangleList);
    m_pipeline->setFrontFace(renderer::FrontFace::FrontFace_Clockwise);
    m_pipeline->setCullMode(renderer::CullMode::CullMode_Back);
    m_pipeline->setDepthCompareOp(renderer::CompareOperation::Always);
    m_pipeline->setDepthWrite(false);
    m_pipeline->setDepthTest(false);
    m_pipeline->setColorMask(0, renderer::ColorMask::ColorMask_All);

    BIND_SHADER_PARAMETER(m_pipeline, m_parameters, cb_Viewport);
    BIND_SHADER_PARAMETER(m_pipeline, m_parameters, s_SamplerState);
    BIND_SHADER_PARAMETER(m_pipeline, m_parameters, t_TextureColor);
}

void RenderPipelineFXAAStage::destroy(renderer::Device* device, scene::SceneData& scene, scene::FrameData& frame)
{
    destroyRenderTarget(device, scene, frame);

    const renderer::ShaderProgram* program = m_pipeline->getShaderProgram();
    V3D_DELETE(program, memory::MemoryLabel::MemoryGame);

    V3D_DELETE(m_pipeline, memory::MemoryLabel::MemoryGame);
    m_pipeline = nullptr;
}

void RenderPipelineFXAAStage::prepare(renderer::Device* device, scene::SceneData& scene, scene::FrameData& frame)
{
    if (!m_renderTarget)
    {
        createRenderTarget(device, scene, frame);
    }
    else if (m_renderTarget->getRenderArea() != scene.m_viewportSize)
    {
        destroyRenderTarget(device, scene, frame);
        createRenderTarget(device, scene, frame);
    }
}

void RenderPipelineFXAAStage::execute(renderer::Device* device, scene::SceneData& scene, scene::FrameData& frame)
{
    ObjectHandle inputTarget_handle = frame.m_frameResources.get("render_target");
    if (!inputTarget_handle.isValid())
    {
        inputTarget_handle = scene.m_globalResources.get("color_target");
    }

    frame.m_frameResources.bind("input_target_taa", inputTarget_handle);
    frame.m_frameResources.bind("render_target", m_renderTarget->getColorTexture<renderer::Texture2D>(0));

    auto renderJob = [this](renderer::Device* device, renderer::CmdListRender* cmdList, const scene::SceneData& scene, const scene::FrameData& frame) -> void
        {
            TRACE_PROFILER_SCOPE("FXAA", color::rgba8::GREEN);
            DEBUG_MARKER_SCOPE(cmdList, "FXAA", color::rgbaf::GREEN);

            ObjectHandle viewportState_handle = frame.m_frameResources.get("viewport_state");
            ASSERT(viewportState_handle.isValid(), "must be valid");
            scene::ViewportState* viewportState = viewportState_handle.as<scene::ViewportState>();

            cmdList->beginRenderTarget(*m_renderTarget);
            cmdList->setViewport({ 0.f, 0.f, (f32)viewportState->viewportSize._x, (f32)viewportState->viewportSize._y });
            cmdList->setScissor({ 0.f, 0.f, (f32)viewportState->viewportSize._x, (f32)viewportState->viewportSize._y });
            cmdList->setPipelineState(*m_pipeline);
            cmdList->bindDescriptorSet(m_pipeline->getShaderProgram(), 0,
                {
                    renderer::Descriptor(renderer::Descriptor::ConstantBuffer{ viewportState, 0, sizeof(scene::ViewportState) }, m_parameters.cb_Viewport)
                });

            ObjectHandle renderTarget_handle = scene.m_globalResources.get("render_target");
            ASSERT(renderTarget_handle.isValid(), "must be valid");
            renderer::Texture2D* texture = renderTarget_handle.as<renderer::Texture2D>();

            ObjectHandle samplerState_handle = scene.m_globalResources.get("linear_sampler_mirror");
            ASSERT(samplerState_handle.isValid(), "must be valid");
            renderer::SamplerState* sampler_state = samplerState_handle.as<renderer::SamplerState>();

            cmdList->bindDescriptorSet(m_pipeline->getShaderProgram(), 1,
                {
                    renderer::Descriptor(sampler_state, m_parameters.s_SamplerState),
                    renderer::Descriptor(texture, m_parameters.t_TextureColor)
                });

            cmdList->draw(renderer::GeometryBufferDesc(), 0, 3, 0, 1);
            cmdList->endRenderTarget();

        };

    addRenderJob("FXAA Job", renderJob, device, scene);
}

void RenderPipelineFXAAStage::createRenderTarget(renderer::Device* device, scene::SceneData& scene, scene::FrameData& frame)
{
    ASSERT(m_renderTarget == nullptr, "must be nullptr");
    m_renderTarget = V3D_NEW(renderer::RenderTargetState, memory::MemoryLabel::MemoryGame)(device, scene.m_viewportSize, 1, 0);

    renderer::Texture2D* texture = V3D_NEW(renderer::Texture2D, memory::MemoryLabel::MemoryGame)(device, renderer::TextureUsage::TextureUsage_Attachment | renderer::TextureUsage::TextureUsage_Sampled,
        renderer::Format::Format_R8G8B8A8_UNorm, scene.m_viewportSize, renderer::TextureSamples::TextureSamples_x1, "fxaa");

    m_renderTarget->setColorTexture(0, texture,
        {
            renderer::RenderTargetLoadOp::LoadOp_DontCare, renderer::RenderTargetStoreOp::StoreOp_Store, color::Color(0.0f)
        },
        {
            renderer::TransitionOp::TransitionOp_ColorAttachment, renderer::TransitionOp::TransitionOp_ColorAttachment
        });

    scene.m_globalResources.bind("fxaa", texture);
}

void RenderPipelineFXAAStage::destroyRenderTarget(renderer::Device* device, scene::SceneData& scene, scene::FrameData& frame)
{
    ASSERT(m_renderTarget, "must be valid");
    renderer::Texture2D* composition = m_renderTarget->getColorTexture<renderer::Texture2D>(0);
    V3D_DELETE(composition, memory::MemoryLabel::MemoryGame);

    V3D_DELETE(m_renderTarget, memory::MemoryLabel::MemoryGame);
    m_renderTarget = nullptr;
}

} //namespace scene
} //namespace v3d