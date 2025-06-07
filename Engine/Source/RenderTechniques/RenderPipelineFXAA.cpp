#include "RenderTechniques/RenderPipelineFXAA.h"

#include "RenderPipelineComposite.h"

#include "Resource/ResourceManager.h"

#include "Resource/Loader/AssetSourceFileLoader.h"
#include "Resource/Loader/ShaderSourceFileLoader.h"
#include "Resource/Loader/ModelFileLoader.h"

namespace v3d
{
namespace renderer
{

RenderPipelineFXAAStage::RenderPipelineFXAAStage(RenderTechnique* technique) noexcept
    : RenderPipelineStage(technique, "fxaa")
    , m_renderTarget(nullptr)
{
}

RenderPipelineFXAAStage::~RenderPipelineFXAAStage()
{
}

void RenderPipelineFXAAStage::create(Device* device, scene::Scene::SceneData& state)
{
    createRenderTarget(device, state);

    const renderer::VertexShader* vertShader = resource::ResourceManager::getInstance()->loadShader<renderer::VertexShader, resource::ShaderSourceFileLoader>(device,
        "offscreen.hlsl", "offscreen_vs", {}, {}/*, resource::ShaderCompileFlag::ShaderCompile_UseDXCompilerForSpirV*/);
    const renderer::FragmentShader* fragShader = resource::ResourceManager::getInstance()->loadShader<renderer::FragmentShader, resource::ShaderSourceFileLoader>(device,
        "fxaa.hlsl", "fxaa_ps", {}, {}/*, resource::ShaderCompileFlag::ShaderCompile_UseDXCompilerForSpirV*/);

    renderer::GraphicsPipelineState* pipeline = new renderer::GraphicsPipelineState(
        device, renderer::VertexInputAttributeDesc(), m_renderTarget->getRenderPassDesc(), new renderer::ShaderProgram(device, vertShader, fragShader), "fxaa_pipeline");

    pipeline->setPrimitiveTopology(renderer::PrimitiveTopology::PrimitiveTopology_TriangleList);
    pipeline->setFrontFace(renderer::FrontFace::FrontFace_Clockwise);
    pipeline->setCullMode(renderer::CullMode::CullMode_Back);
    pipeline->setColorMask(renderer::ColorMask::ColorMask_All);
    pipeline->setDepthCompareOp(renderer::CompareOperation::CompareOp_Always);
    pipeline->setDepthWrite(false);
    pipeline->setDepthTest(false);

    m_pipeline.push_back(pipeline);

    m_sampler = new renderer::SamplerState(device, renderer::SamplerFilter::SamplerFilter_Trilinear, renderer::SamplerAnisotropic::SamplerAnisotropic_4x);
    m_sampler->setWrap(renderer::SamplerWrap::TextureWrap_MirroredRepeat);
}

void RenderPipelineFXAAStage::destroy(Device* device, scene::Scene::SceneData& state)
{
}

void RenderPipelineFXAAStage::prepare(Device* device, scene::Scene::SceneData& state)
{
}

void RenderPipelineFXAAStage::execute(Device* device, scene::Scene::SceneData& state)
{
    state.m_renderState.m_cmdList->beginRenderTarget(*m_renderTarget);
    state.m_renderState.m_cmdList->setViewport({ 0.f, 0.f, (f32)state.m_viewportState.m_viewpotSize._width, (f32)state.m_viewportState.m_viewpotSize._height });
    state.m_renderState.m_cmdList->setScissor({ 0.f, 0.f, (f32)state.m_viewportState.m_viewpotSize._width, (f32)state.m_viewportState.m_viewpotSize._height });
    state.m_renderState.m_cmdList->setPipelineState(*m_pipeline[0]);

    state.m_renderState.m_cmdList->bindDescriptorSet(0,
        {
            renderer::Descriptor(renderer::Descriptor::ConstantBuffer{ &state.m_viewportState._viewportBuffer, 0, sizeof(state.m_viewportState._viewportBuffer)}, 0)
        });

    ObjectHandle render_target = state.m_globalResources.get("full_composite");
    ASSERT(render_target.isValid(), "must be valid");
    renderer::Texture2D* texture = objectFromHandle<renderer::Texture2D>(render_target);

    state.m_renderState.m_cmdList->bindDescriptorSet(1,
        {
            renderer::Descriptor(m_sampler, 1),
            renderer::Descriptor(texture, 2)
        });

    state.m_renderState.m_cmdList->draw(renderer::GeometryBufferDesc(), 0, 3, 0, 1);

    state.m_renderState.m_cmdList->endRenderTarget();

    state.m_globalResources.bind("render_target", m_renderTarget->getColorTexture<renderer::Texture2D>(0));
}

void RenderPipelineFXAAStage::changed(Device* device, scene::Scene::SceneData& data)
{
    if (!m_renderTarget)
    {
        createRenderTarget(device, data);
    }
    else if (m_renderTarget->getRenderArea() != data.m_viewportState.m_viewpotSize)
    {
        destroyRenderTarget(device, data);
        createRenderTarget(device, data);
    }
}

void RenderPipelineFXAAStage::createRenderTarget(Device* device, scene::Scene::SceneData& data)
{
    ASSERT(m_renderTarget == nullptr, "must be nullptr");
    m_renderTarget = new renderer::RenderTargetState(device, data.m_viewportState.m_viewpotSize, 1, 0, "fxaa_pass");

    renderer::Texture2D* texture = new renderer::Texture2D(device, renderer::TextureUsage::TextureUsage_Attachment | renderer::TextureUsage::TextureUsage_Sampled,
        renderer::Format::Format_R8G8B8A8_UNorm, data.m_viewportState.m_viewpotSize, renderer::TextureSamples::TextureSamples_x1, "fxaa");

    m_renderTarget->setColorTexture(0, texture,
        {
            renderer::RenderTargetLoadOp::LoadOp_DontCare, renderer::RenderTargetStoreOp::StoreOp_Store, color::Color(0.0f)
        },
        {
            renderer::TransitionOp::TransitionOp_Undefined, renderer::TransitionOp::TransitionOp_ColorAttachment
        }
    );

    data.m_globalResources.bind("fxaa", texture);
}

void RenderPipelineFXAAStage::destroyRenderTarget(Device* device, scene::Scene::SceneData& data)
{
    ASSERT(m_renderTarget, "must be valid");
    renderer::Texture2D* composition = m_renderTarget->getColorTexture<renderer::Texture2D>(0);
    delete composition;

    delete m_renderTarget;
    m_renderTarget = nullptr;
}

} //namespace renderer
} //namespace v3d