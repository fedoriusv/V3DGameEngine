#include "RenderPipelineTAA.h"
#include "Utils/Logger.h"

#include "Resource/ResourceManager.h"

#include "Resource/Loader/AssetSourceFileLoader.h"
#include "Resource/Loader/ShaderSourceFileLoader.h"
#include "Resource/Loader/ModelFileLoader.h"

namespace v3d
{
namespace renderer
{

RenderPipelineTAAStage::RenderPipelineTAAStage(RenderTechnique* technique) noexcept
    : RenderPipelineStage(technique, "taa")

    , m_renderTarget(nullptr)
    , m_pipeline(nullptr)

    , m_samplerLinear(nullptr)
    , m_samplerPoint(nullptr)

    , m_resolved(nullptr)
    , m_history(nullptr)
{
}

RenderPipelineTAAStage::~RenderPipelineTAAStage()
{
}

void RenderPipelineTAAStage::create(Device* device, scene::SceneData& state)
{
    createRenderTarget(device, state);

    const renderer::VertexShader* vertShader = resource::ResourceManager::getInstance()->loadShader<renderer::VertexShader, resource::ShaderSourceFileLoader>(device,
        "offscreen.hlsl", "offscreen_vs", {}, {}/*, resource::ShaderCompileFlag::ShaderCompile_UseDXCompilerForSpirV*/);
    const renderer::FragmentShader* fragShader = resource::ResourceManager::getInstance()->loadShader<renderer::FragmentShader, resource::ShaderSourceFileLoader>(device,
        "taa.hlsl", "main_ps", {}, {}/*, resource::ShaderCompileFlag::ShaderCompile_UseDXCompilerForSpirV*/);

    RenderPassDesc desc{};
    desc._countColorAttachments = 1;
    desc._attachmentsDesc[0]._format = Format_R32G32B32A32_SFloat;

    m_pipeline = new renderer::GraphicsPipelineState(device, renderer::VertexInputAttributeDesc(), desc, new renderer::ShaderProgram(device, vertShader, fragShader), "taa");

    m_pipeline->setPrimitiveTopology(renderer::PrimitiveTopology::PrimitiveTopology_TriangleList);
    m_pipeline->setFrontFace(renderer::FrontFace::FrontFace_Clockwise);
    m_pipeline->setCullMode(renderer::CullMode::CullMode_Back);
    m_pipeline->setDepthCompareOp(renderer::CompareOperation::CompareOp_Always);
    m_pipeline->setDepthWrite(false);
    m_pipeline->setDepthTest(false);
    m_pipeline->setColorMask(0, renderer::ColorMask::ColorMask_All);

    m_samplerLinear = new renderer::SamplerState(device, renderer::SamplerFilter::SamplerFilter_Bilinear, renderer::SamplerAnisotropic::SamplerAnisotropic_None);
    m_samplerLinear->setWrap(renderer::SamplerWrap::TextureWrap_ClampToBorder);

    m_samplerPoint = new renderer::SamplerState(device, renderer::SamplerFilter::SamplerFilter_Nearest, renderer::SamplerAnisotropic::SamplerAnisotropic_None);
    m_samplerPoint->setWrap(renderer::SamplerWrap::TextureWrap_ClampToBorder);
}

void RenderPipelineTAAStage::destroy(Device* device, scene::SceneData& state)
{
    destroyRenderTarget(device, state);

    delete m_samplerLinear;
    m_samplerLinear = nullptr;

    delete m_samplerPoint;
    m_samplerPoint = nullptr;

    delete m_pipeline;
    m_pipeline = nullptr;
}

void RenderPipelineTAAStage::prepare(Device* device, scene::SceneData& state)
{
    if (!m_renderTarget)
    {
        createRenderTarget(device, state);
    }
    else if (m_renderTarget->getRenderArea() != state.m_viewportState._viewpotSize)
    {
        destroyRenderTarget(device, state);
        createRenderTarget(device, state);
    }
}

void RenderPipelineTAAStage::execute(Device* device, scene::SceneData& state)
{
    DEBUG_MARKER_SCOPE(state.m_renderState.m_cmdList, "TAA", color::colorrgbaf::GREEN);

    state.m_renderState.m_cmdList->beginRenderTarget(*m_renderTarget);
    state.m_renderState.m_cmdList->setViewport({ 0.f, 0.f, (f32)state.m_viewportState._viewpotSize._width, (f32)state.m_viewportState._viewpotSize._height });
    state.m_renderState.m_cmdList->setScissor({ 0.f, 0.f, (f32)state.m_viewportState._viewpotSize._width, (f32)state.m_viewportState._viewpotSize._height });
    state.m_renderState.m_cmdList->setPipelineState(*m_pipeline);

    ObjectHandle render_target = state.m_globalResources.get("render_target");
    ASSERT(render_target.isValid(), "must be valid");
    renderer::Texture2D* render_targetTexture = objectFromHandle<renderer::Texture2D>(render_target);

    ObjectHandle velocity_target = state.m_globalResources.get("gbuffer_velocity");
    ASSERT(render_target.isValid(), "must be valid");
    renderer::Texture2D* velocity_targetTexture = objectFromHandle<renderer::Texture2D>(velocity_target);

    state.m_renderState.m_cmdList->bindDescriptorSet(0,
        {
            renderer::Descriptor(renderer::Descriptor::ConstantBuffer{ &state.m_viewportState._viewportBuffer, 0, sizeof(state.m_viewportState._viewportBuffer)}, 0)
        });

    state.m_renderState.m_cmdList->bindDescriptorSet(1,
        {
            renderer::Descriptor(m_samplerLinear, 1),
            renderer::Descriptor(m_samplerPoint, 2),
            renderer::Descriptor(renderer::TextureView(render_targetTexture, 0, 0), 3),
            renderer::Descriptor(renderer::TextureView(m_history, 0, 0), 4),
            renderer::Descriptor(renderer::TextureView(velocity_targetTexture, 0, 0), 5)
        });

    state.m_renderState.m_cmdList->draw(renderer::GeometryBufferDesc(), 0, 3, 0, 1);
    state.m_renderState.m_cmdList->endRenderTarget();

    state.m_globalResources.bind("render_target", m_renderTarget->getColorTexture<renderer::Texture2D>(0));

    state.m_renderState.m_cmdList->copy(m_resolved, m_history, m_resolved->getDimension());
}

void RenderPipelineTAAStage::createRenderTarget(Device* device, scene::SceneData& data)
{
    ASSERT(m_resolved == nullptr, "must be nullptr");
    m_resolved = new renderer::Texture2D(device, renderer::TextureUsage::TextureUsage_Attachment | renderer::TextureUsage::TextureUsage_Sampled | renderer::TextureUsage::TextureUsage_Read,
        renderer::Format::Format_R32G32B32A32_SFloat, data.m_viewportState._viewpotSize, renderer::TextureSamples::TextureSamples_x1, "resolved_taa");

    ASSERT(m_history == nullptr, "must be nullptr");
    m_history = new renderer::Texture2D(device, renderer::TextureUsage::TextureUsage_Attachment | renderer::TextureUsage::TextureUsage_Sampled | renderer::TextureUsage::TextureUsage_Write,
        renderer::Format::Format_R32G32B32A32_SFloat, data.m_viewportState._viewpotSize, renderer::TextureSamples::TextureSamples_x1, "history_taa");

    ASSERT(m_renderTarget == nullptr, "must be nullptr");
    m_renderTarget = new renderer::RenderTargetState(device, data.m_viewportState._viewpotSize, 1);
    m_renderTarget->setColorTexture(0, m_resolved,
        {
            renderer::RenderTargetLoadOp::LoadOp_DontCare, renderer::RenderTargetStoreOp::StoreOp_Store, color::Color(0.0f)
        },
        {
            renderer::TransitionOp::TransitionOp_Undefined, renderer::TransitionOp::TransitionOp_ColorAttachment
        });
}

void RenderPipelineTAAStage::destroyRenderTarget(Device* device, scene::SceneData& data)
{
    ASSERT(m_resolved, "must be valid");
    delete m_resolved;
    m_resolved = nullptr;

    ASSERT(m_history, "must be valid");
    delete m_history;
    m_history = nullptr;

    ASSERT(m_renderTarget, "must be valid");
    delete m_renderTarget;
    m_renderTarget = nullptr;
}

} //namespace renderer
} //namespace v3d
