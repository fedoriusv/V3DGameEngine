#include "RenderPipelineTonemapStage.h"
#include "Utils/Logger.h"

#include "Resource/ResourceManager.h"
#include "Resource/Loader/AssetSourceFileLoader.h"
#include "Resource/Loader/ShaderSourceFileLoader.h"
#include "Resource/Loader/ModelFileLoader.h"
#include "Scene/SceneNode.h"

#include "Renderer/ShaderProgram.h"

#include "FrameProfiler.h"

namespace v3d
{
namespace scene
{

RenderPipelineTonemapStage::RenderPipelineTonemapStage(RenderTechnique* technique) noexcept
    : RenderPipelineStage(technique, "Tonemap")
    , m_tonemapRenderTarget(nullptr)
    , m_pipeline(nullptr)
{
}

RenderPipelineTonemapStage::~RenderPipelineTonemapStage()
{
}

void RenderPipelineTonemapStage::create(renderer::Device* device, scene::SceneData& scene, scene::FrameData& frame)
{
    createRenderTarget(device, scene, frame);

    const renderer::VertexShader* vertShader = resource::ResourceManager::getInstance()->loadShader<renderer::VertexShader, resource::ShaderSourceFileLoader>("offscreen.hlsl", "offscreen_vs",
        {}, {}, resource::ShaderCompileFlag::ShaderCompile_UseDXCompilerForSpirV);
    const renderer::FragmentShader* fragShader = resource::ResourceManager::getInstance()->loadShader<renderer::FragmentShader, resource::ShaderSourceFileLoader>("tonemapping.hlsl", "tonemapping_ps",
        {}, {}, resource::ShaderCompileFlag::ShaderCompile_UseDXCompilerForSpirV);

    m_pipeline = V3D_NEW(renderer::GraphicsPipelineState, memory::MemoryLabel::MemoryGame)(device, renderer::VertexInputAttributeDesc(), m_tonemapRenderTarget->getRenderPassDesc(),
        V3D_NEW(renderer::ShaderProgram, memory::MemoryLabel::MemoryGame)(device, vertShader, fragShader), "tonemap_pipeline");

    m_pipeline->setPrimitiveTopology(renderer::PrimitiveTopology::PrimitiveTopology_TriangleList);
    m_pipeline->setFrontFace(renderer::FrontFace::FrontFace_Clockwise);
    m_pipeline->setCullMode(renderer::CullMode::CullMode_Back);
    m_pipeline->setDepthCompareOp(renderer::CompareOperation::Always);
    m_pipeline->setDepthWrite(false);
    m_pipeline->setDepthTest(false);
    m_pipeline->setColorMask(0, renderer::ColorMask::ColorMask_All);

    BIND_SHADER_PARAMETER(m_pipeline, m_parameters, cb_Viewport);
    BIND_SHADER_PARAMETER(m_pipeline, m_parameters, cb_Tonemapper);
    BIND_SHADER_PARAMETER(m_pipeline, m_parameters, s_LinearMirrorSampler);
    BIND_SHADER_PARAMETER(m_pipeline, m_parameters, s_LinearClampSampler);
    BIND_SHADER_PARAMETER(m_pipeline, m_parameters, t_ColorTexture);
    BIND_SHADER_PARAMETER(m_pipeline, m_parameters, t_LUTTexture);
}

void RenderPipelineTonemapStage::destroy(renderer::Device* device, scene::SceneData& scene, scene::FrameData& frame)
{
    destroyRenderTarget(device, scene, frame);

    if (m_pipeline)
    {
        const renderer::ShaderProgram* program = m_pipeline->getShaderProgram();
        V3D_DELETE(program, memory::MemoryLabel::MemoryGame);

        V3D_DELETE(m_pipeline, memory::MemoryLabel::MemoryGame);
        m_pipeline = nullptr;
    }
}

void RenderPipelineTonemapStage::prepare(renderer::Device* device, scene::SceneData& scene, scene::FrameData& frame)
{
    if (!m_tonemapRenderTarget)
    {
        createRenderTarget(device, scene, frame);
    }
    else if (m_tonemapRenderTarget->getRenderArea() != scene.m_viewportSize)
    {
        destroyRenderTarget(device, scene, frame);
        createRenderTarget(device, scene, frame);
    }

    ObjectHandle inputTarget_handle = frame.m_frameResources.get("render_target");
    if (!inputTarget_handle.isValid())
    {
        inputTarget_handle = scene.m_globalResources.get("color_target");
    }

    renderer::Texture2D* inputTargetTexture = inputTarget_handle.as<renderer::Texture2D>();
    frame.m_frameResources.bind("input_target_tonemap", inputTarget_handle);
    frame.m_frameResources.bind("render_target", m_tonemapRenderTarget->getColorTexture<renderer::Texture2D>(0));
}

void RenderPipelineTonemapStage::execute(renderer::Device* device, scene::SceneData& scene, scene::FrameData& frame)
{
    auto renderJob = [this](renderer::Device* device, renderer::CmdListRender* cmdList, const scene::SceneData& scene, const scene::FrameData& frame) -> void
        {
            TRACE_PROFILER_SCOPE("Tonemap", color::rgba8::GREEN);
            DEBUG_MARKER_SCOPE(cmdList, "Tonemap", color::rgbaf::GREEN);

            ObjectHandle viewportState_handle = frame.m_frameResources.get("viewport_state");
            ASSERT(viewportState_handle.isValid(), "must be valid");
            scene::ViewportState* viewportState = viewportState_handle.as<scene::ViewportState>();

            cmdList->beginRenderTarget(*m_tonemapRenderTarget);
            cmdList->setViewport({ 0.f, 0.f, (f32)scene.m_viewportSize._width, (f32)scene.m_viewportSize._height });
            cmdList->setScissor({ 0.f, 0.f, (f32)scene.m_viewportSize._width, (f32)scene.m_viewportSize._height });
            cmdList->setPipelineState(*m_pipeline);
            cmdList->bindDescriptorSet(m_pipeline->getShaderProgram(), 0,
                {
                    renderer::Descriptor(renderer::Descriptor::ConstantBuffer{ viewportState, 0, sizeof(scene::ViewportState)}, m_parameters.cb_Viewport)
                });

            struct Tonemapper
            {
                u32     tonemapper;
                u32     lut;
                f32     ev100;
                f32     gamma;
            } tonemapper;

            tonemapper.tonemapper = scene.m_settings._tonemapParams._tonemapper;
            tonemapper.lut = scene.m_settings._tonemapParams._lut;
            tonemapper.ev100 = scene.m_settings._tonemapParams._ev100; //@see: https://en.wikipedia.org/wiki/Exposure_value#Tabulated_exposure_values
            tonemapper.gamma = scene.m_settings._tonemapParams._gamma;

            ObjectHandle inputTarget_handle = frame.m_frameResources.get("input_target_tonemap");
            ASSERT(inputTarget_handle.isValid(), "must be valid");
            renderer::Texture2D* inputTargetTexture = inputTarget_handle.as<renderer::Texture2D>();

            ObjectHandle lut_handle = scene.m_globalResources.get("current_lut");
            ASSERT(lut_handle.isValid(), "must be valid");
            renderer::Texture3D* LUT = lut_handle.as<renderer::Texture3D>();

            ObjectHandle linearSamplerMirror_handle = scene.m_globalResources.get("linear_sampler_mirror");
            ASSERT(linearSamplerMirror_handle.isValid(), "must be valid");
            renderer::SamplerState* linearSamplerMirrorState = linearSamplerMirror_handle.as<renderer::SamplerState>();

            ObjectHandle linearSamplerRepeat_handle = scene.m_globalResources.get("linear_sampler_repeat");
            ASSERT(linearSamplerRepeat_handle.isValid(), "must be valid");
            renderer::SamplerState* linearSamplerRepeatState = linearSamplerRepeat_handle.as<renderer::SamplerState>();

            cmdList->bindDescriptorSet(m_pipeline->getShaderProgram(), 1,
                {
                    renderer::Descriptor(renderer::Descriptor::ConstantBuffer{ &tonemapper, 0, sizeof(Tonemapper)}, m_parameters.cb_Tonemapper),
                    renderer::Descriptor(linearSamplerMirrorState, m_parameters.s_LinearMirrorSampler),
                    renderer::Descriptor(linearSamplerRepeatState, m_parameters.s_LinearClampSampler),
                    renderer::Descriptor(renderer::TextureView(inputTargetTexture, 0, 0), m_parameters.t_ColorTexture),
                    renderer::Descriptor(renderer::TextureView(LUT, 0, 0), m_parameters.t_LUTTexture),
                });

            cmdList->draw(renderer::GeometryBufferDesc(), 0, 3, 0, 1);
            cmdList->endRenderTarget();
        };

    addRenderJob("Tonemap Job", renderJob, device, scene);
}

void RenderPipelineTonemapStage::createRenderTarget(renderer::Device* device, scene::SceneData& data, scene::FrameData& frame)
{
    ASSERT(m_tonemapRenderTarget == nullptr, "must be nullptr");
    m_tonemapRenderTarget = V3D_NEW(renderer::RenderTargetState, memory::MemoryLabel::MemoryGame)(device, data.m_viewportSize, 1, 0);

    renderer::Texture2D* tonemap = V3D_NEW(renderer::Texture2D, memory::MemoryLabel::MemoryGame)(device, renderer::TextureUsage::TextureUsage_Attachment | renderer::TextureUsage::TextureUsage_Sampled,
        renderer::Format::Format_R8G8B8A8_UNorm, data.m_viewportSize, renderer::TextureSamples::TextureSamples_x1, "tonemap");
    data.m_globalResources.bind("final_target", tonemap);

    m_tonemapRenderTarget->setColorTexture(0, tonemap,
        {
            renderer::RenderTargetLoadOp::LoadOp_DontCare, renderer::RenderTargetStoreOp::StoreOp_Store, color::Color(0.0f)
        },
        {
            renderer::TransitionOp::TransitionOp_ColorAttachment, renderer::TransitionOp::TransitionOp_ShaderRead
        });

}

void RenderPipelineTonemapStage::destroyRenderTarget(renderer::Device* device, scene::SceneData& data, scene::FrameData& frame)
{
    ASSERT(m_tonemapRenderTarget, "must be valid");
    renderer::Texture2D* tonemap = m_tonemapRenderTarget->getColorTexture<renderer::Texture2D>(0);
    V3D_DELETE(tonemap, memory::MemoryLabel::MemoryGame);

    V3D_DELETE(m_tonemapRenderTarget, memory::MemoryLabel::MemoryGame);
    m_tonemapRenderTarget = nullptr;
}

} //namespace scene
} //namespace v3d