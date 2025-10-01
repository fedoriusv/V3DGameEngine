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

    , m_resolved(nullptr)
    , m_history(nullptr)
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

        ObjectHandle render_target = scene.m_globalResources.get("render_target");
        ASSERT(render_target.isValid(), "must be valid");
        renderer::Texture2D* render_targetTexture = objectFromHandle<renderer::Texture2D>(render_target);

        ObjectHandle velocity_target = scene.m_globalResources.get("gbuffer_velocity");
        ASSERT(render_target.isValid(), "must be valid");
        renderer::Texture2D* velocity_targetTexture = objectFromHandle<renderer::Texture2D>(velocity_target);

        ObjectHandle sampler_state_linear_h = scene.m_globalResources.get("linear_sampler_clamp");
        ASSERT(sampler_state_linear_h.isValid(), "must be valid");
        renderer::SamplerState* sampler_state_linear = objectFromHandle<renderer::SamplerState>(sampler_state_linear_h);

        ObjectHandle linear_sampler_clamp_h = scene.m_globalResources.get("linear_sampler_clamp");
        ASSERT(linear_sampler_clamp_h.isValid(), "must be valid");
        renderer::SamplerState* sampler_state_point = objectFromHandle<renderer::SamplerState>(linear_sampler_clamp_h);

        cmdList->bindDescriptorSet(0,
            {
                renderer::Descriptor(renderer::Descriptor::ConstantBuffer{ &viewportState._viewportBuffer, 0, sizeof(viewportState._viewportBuffer)}, 0)
            });

        cmdList->bindDescriptorSet(1,
            {
                renderer::Descriptor(sampler_state_linear, 1),
                renderer::Descriptor(sampler_state_point, 2),
                renderer::Descriptor(renderer::TextureView(render_targetTexture, 0, 0), 3),
                renderer::Descriptor(renderer::TextureView(m_history, 0, 0), 4),
                renderer::Descriptor(renderer::TextureView(velocity_targetTexture, 0, 0), 5)
            });

        cmdList->draw(renderer::GeometryBufferDesc(), 0, 3, 0, 1);
        cmdList->endRenderTarget();
    }

    {
        TRACE_PROFILER_SCOPE("TAA Copy", color::rgba8::GREEN);
        cmdList->copy(m_resolved, m_history, { m_resolved->getWidth(), m_resolved->getHeight(), 1 });
    }

    scene.m_globalResources.bind("render_target", m_renderTarget->getColorTexture<renderer::Texture2D>(0));
}

void RenderPipelineTAAStage::createRenderTarget(renderer::Device* device, scene::SceneData& data)
{
    ASSERT(m_resolved == nullptr, "must be nullptr");
    m_resolved = V3D_NEW(renderer::Texture2D, memory::MemoryLabel::MemoryGame)(device, renderer::TextureUsage::TextureUsage_Attachment | renderer::TextureUsage::TextureUsage_Sampled | renderer::TextureUsage::TextureUsage_Read,
        renderer::Format::Format_R16G16B16A16_SFloat, data.m_viewportState._viewpotSize, renderer::TextureSamples::TextureSamples_x1, "resolved_taa");

    ASSERT(m_history == nullptr, "must be nullptr");
    m_history = V3D_NEW(renderer::Texture2D, memory::MemoryLabel::MemoryGame)(device, renderer::TextureUsage::TextureUsage_Attachment | renderer::TextureUsage::TextureUsage_Sampled | renderer::TextureUsage::TextureUsage_Write,
        renderer::Format::Format_R16G16B16A16_SFloat, data.m_viewportState._viewpotSize, renderer::TextureSamples::TextureSamples_x1, "history_taa");

    ASSERT(m_renderTarget == nullptr, "must be nullptr");
    m_renderTarget = V3D_NEW(renderer::RenderTargetState, memory::MemoryLabel::MemoryGame)(device, data.m_viewportState._viewpotSize, 1);
    m_renderTarget->setColorTexture(0, m_resolved,
        {
            renderer::RenderTargetLoadOp::LoadOp_DontCare, renderer::RenderTargetStoreOp::StoreOp_Store, color::Color(0.0f)
        },
        {
            renderer::TransitionOp::TransitionOp_Undefined, renderer::TransitionOp::TransitionOp_ColorAttachment
        });
}

void RenderPipelineTAAStage::destroyRenderTarget(renderer::Device* device, scene::SceneData& data)
{
    ASSERT(m_resolved, "must be valid");
    V3D_DELETE(m_resolved, memory::MemoryLabel::MemoryGame);
    m_resolved = nullptr;

    ASSERT(m_history, "must be valid");
    V3D_DELETE(m_history, memory::MemoryLabel::MemoryGame);
    m_history = nullptr;

    ASSERT(m_renderTarget, "must be valid");
    V3D_DELETE(m_renderTarget, memory::MemoryLabel::MemoryGame);
    m_renderTarget = nullptr;
}

} //namespace scene
} //namespace v3d
