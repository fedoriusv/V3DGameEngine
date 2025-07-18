#include "RenderPipelineComposite.h"

#include "Resource/ResourceManager.h"

#include "Resource/Loader/AssetSourceFileLoader.h"
#include "Resource/Loader/ShaderSourceFileLoader.h"
#include "Resource/Loader/ModelFileLoader.h"

namespace v3d
{
namespace renderer
{

RenderPipelineCompositionStage::RenderPipelineCompositionStage(RenderTechnique* technique) noexcept
    : RenderPipelineStage(technique, "composition")
    , m_compositionRenderTarget(nullptr)
    , m_sampler(nullptr)
{
}

RenderPipelineCompositionStage::~RenderPipelineCompositionStage()
{
}

void RenderPipelineCompositionStage::create(Device* device, scene::SceneData& state)
{
    createRenderTarget(device, state);

    const renderer::VertexShader* vertShader = resource::ResourceManager::getInstance()->loadShader<renderer::VertexShader, resource::ShaderSourceFileLoader>(device,
        "offscreen.hlsl", "offscreen_vs", {}, {}/*, resource::ShaderCompileFlag::ShaderCompile_UseDXCompilerForSpirV*/);
    const renderer::FragmentShader* fragShader = resource::ResourceManager::getInstance()->loadShader<renderer::FragmentShader, resource::ShaderSourceFileLoader>(device,
        "composition.hlsl", "main_ps", {}, {}/*, resource::ShaderCompileFlag::ShaderCompile_UseDXCompilerForSpirV*/);

    renderer::GraphicsPipelineState* pipeline = new renderer::GraphicsPipelineState(
        device, renderer::VertexInputAttributeDesc(), m_compositionRenderTarget->getRenderPassDesc(), new renderer::ShaderProgram(device, vertShader, fragShader), "composition_pipeline");

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

void RenderPipelineCompositionStage::destroy(Device* device, scene::SceneData& state)
{
    destroyRenderTarget(device, state);

    delete m_sampler;
    m_sampler = nullptr;
}

void RenderPipelineCompositionStage::prepare(Device* device, scene::SceneData& state)
{
    if (!m_compositionRenderTarget)
    {
        createRenderTarget(device, state);
    }
    else if (m_compositionRenderTarget->getRenderArea() != state.m_viewportState._viewpotSize)
    {
        destroyRenderTarget(device, state);
        createRenderTarget(device, state);
    }
}

void RenderPipelineCompositionStage::execute(Device* device, scene::SceneData& state)
{
    DEBUG_MARKER_SCOPE(state.m_renderState.m_cmdList, "Composition", color::colorrgbaf::GREEN);

    state.m_renderState.m_cmdList->beginRenderTarget(*m_compositionRenderTarget);
    state.m_renderState.m_cmdList->setViewport({ 0.f, 0.f, (f32)state.m_viewportState._viewpotSize._width, (f32)state.m_viewportState._viewpotSize._height });
    state.m_renderState.m_cmdList->setScissor({ 0.f, 0.f, (f32)state.m_viewportState._viewpotSize._width, (f32)state.m_viewportState._viewpotSize._height });
    state.m_renderState.m_cmdList->setPipelineState(*m_pipeline[0]);

    state.m_renderState.m_cmdList->bindDescriptorSet(0,
        {
            renderer::Descriptor(renderer::Descriptor::ConstantBuffer{ &state.m_viewportState._viewportBuffer, 0, sizeof(state.m_viewportState._viewportBuffer)}, 0)
        });

    ObjectHandle gbuffer_albedo = state.m_globalResources.get("gbuffer_albedo");
    ASSERT(gbuffer_albedo.isValid(), "must be valid");
    renderer::Texture2D* gbuffer_albedo_texture = objectFromHandle<renderer::Texture2D>(gbuffer_albedo);

    ObjectHandle gbuffer_normals = state.m_globalResources.get("gbuffer_normals");
    ASSERT(gbuffer_normals.isValid(), "must be valid");
    renderer::Texture2D* gbuffer_normals_texture = objectFromHandle<renderer::Texture2D>(gbuffer_normals);

    ObjectHandle gbuffer_material = state.m_globalResources.get("gbuffer_material");
    ASSERT(gbuffer_material.isValid(), "must be valid");
    renderer::Texture2D* gbuffer_material_texture = objectFromHandle<renderer::Texture2D>(gbuffer_material);

    state.m_renderState.m_cmdList->bindDescriptorSet(1,
        {
            renderer::Descriptor(m_sampler, 1),
            renderer::Descriptor(renderer::TextureView(gbuffer_albedo_texture, 0, 0), 2)
            //renderer::Descriptor(gbuffer_normals_texture, 3),
            //renderer::Descriptor(gbuffer_material_texture, 4),
        });

    state.m_renderState.m_cmdList->draw(renderer::GeometryBufferDesc(), 0, 3, 0, 1);
    state.m_renderState.m_cmdList->endRenderTarget();

    state.m_globalResources.bind("render_target", m_compositionRenderTarget->getColorTexture<renderer::Texture2D>(0));
}

void RenderPipelineCompositionStage::createRenderTarget(Device* device, scene::SceneData& data)
{
    ASSERT(m_compositionRenderTarget == nullptr, "must be nullptr");
    m_compositionRenderTarget = new renderer::RenderTargetState(device, data.m_viewportState._viewpotSize, 1);

    renderer::Texture2D* composition = new renderer::Texture2D(device, renderer::TextureUsage::TextureUsage_Attachment | renderer::TextureUsage::TextureUsage_Sampled,
        renderer::Format::Format_R16G16B16A16_SFloat, data.m_viewportState._viewpotSize, renderer::TextureSamples::TextureSamples_x1, "composition");

    m_compositionRenderTarget->setColorTexture(0, composition,
        {
            renderer::RenderTargetLoadOp::LoadOp_DontCare, renderer::RenderTargetStoreOp::StoreOp_Store, color::Color(0.0f)
        },
        {
            renderer::TransitionOp::TransitionOp_Undefined, renderer::TransitionOp::TransitionOp_ColorAttachment
        });

    data.m_globalResources.bind("composition_target", composition);
}

void RenderPipelineCompositionStage::destroyRenderTarget(Device* device, scene::SceneData& data)
{
    ASSERT(m_compositionRenderTarget, "must be valid");
    renderer::Texture2D* composition = m_compositionRenderTarget->getColorTexture<renderer::Texture2D>(0);
    delete composition;

    delete m_compositionRenderTarget;
    m_compositionRenderTarget = nullptr;
}

} //namespace renderer
} //namespace v3d