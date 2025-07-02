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
    : RenderPipelineStage(technique, "Composition")
    , m_compositionRenderTarget(nullptr)
    , m_pipeline(nullptr)
{
}

RenderPipelineCompositionStage::~RenderPipelineCompositionStage()
{
}

void RenderPipelineCompositionStage::create(Device* device, scene::SceneData& scene, scene::FrameData& frame)
{
    createRenderTarget(device, scene);

    const renderer::VertexShader* vertShader = resource::ResourceManager::getInstance()->loadShader<renderer::VertexShader, resource::ShaderSourceFileLoader>(device,
        "offscreen.hlsl", "offscreen_vs", {}, {}/*, resource::ShaderCompileFlag::ShaderCompile_UseDXCompilerForSpirV*/);
    const renderer::FragmentShader* fragShader = resource::ResourceManager::getInstance()->loadShader<renderer::FragmentShader, resource::ShaderSourceFileLoader>(device,
        "composition.hlsl", "main_ps", {}, {}/*, resource::ShaderCompileFlag::ShaderCompile_UseDXCompilerForSpirV*/);

    m_pipeline = new renderer::GraphicsPipelineState(
        device, renderer::VertexInputAttributeDesc(), m_compositionRenderTarget->getRenderPassDesc(), new renderer::ShaderProgram(device, vertShader, fragShader), "composition_pipeline");

    m_pipeline->setPrimitiveTopology(renderer::PrimitiveTopology::PrimitiveTopology_TriangleList);
    m_pipeline->setFrontFace(renderer::FrontFace::FrontFace_Clockwise);
    m_pipeline->setCullMode(renderer::CullMode::CullMode_Back);
    m_pipeline->setDepthCompareOp(renderer::CompareOperation::CompareOp_Always);
    m_pipeline->setDepthWrite(false);
    m_pipeline->setDepthTest(false);
    m_pipeline->setColorMask(0, renderer::ColorMask::ColorMask_All);
}

void RenderPipelineCompositionStage::destroy(Device* device, scene::SceneData& scene, scene::FrameData& frame)
{
    destroyRenderTarget(device, scene);

    delete m_pipeline;
    m_pipeline = nullptr;
}

void RenderPipelineCompositionStage::prepare(Device* device, scene::SceneData& scene, scene::FrameData& frame)
{
    if (!m_compositionRenderTarget)
    {
        createRenderTarget(device, scene);
    }
    else if (m_compositionRenderTarget->getRenderArea() != scene.m_viewportState._viewpotSize)
    {
        destroyRenderTarget(device, scene);
        createRenderTarget(device, scene);
    }
}

void RenderPipelineCompositionStage::execute(Device* device, scene::SceneData& scene, scene::FrameData& frame)
{
    renderer::CmdListRender* cmdList = scene.m_renderState.m_cmdList;
    scene::ViewportState& viewportState = scene.m_viewportState;

    DEBUG_MARKER_SCOPE(cmdList, "Composition", color::colorrgbaf::GREEN);

    cmdList->beginRenderTarget(*m_compositionRenderTarget);
    cmdList->setViewport({ 0.f, 0.f, (f32)viewportState._viewpotSize._width, (f32)viewportState._viewpotSize._height });
    cmdList->setScissor({ 0.f, 0.f, (f32)viewportState._viewpotSize._width, (f32)viewportState._viewpotSize._height });
    cmdList->setPipelineState(*m_pipeline);

    cmdList->bindDescriptorSet(0,
        {
            renderer::Descriptor(renderer::Descriptor::ConstantBuffer{ &viewportState._viewportBuffer, 0, sizeof(viewportState._viewportBuffer)}, 0)
        });

    ObjectHandle gbuffer_albedo_h = scene.m_globalResources.get("gbuffer_albedo");
    ASSERT(gbuffer_albedo_h.isValid(), "must be valid");
    renderer::Texture2D* gbuffer_albedo_texture = objectFromHandle<renderer::Texture2D>(gbuffer_albedo_h);

    ObjectHandle gbuffer_normals_h = scene.m_globalResources.get("gbuffer_normals");
    ASSERT(gbuffer_normals_h.isValid(), "must be valid");
    renderer::Texture2D* gbuffer_normals_texture = objectFromHandle<renderer::Texture2D>(gbuffer_normals_h);

    ObjectHandle gbuffer_material_h = scene.m_globalResources.get("gbuffer_material");
    ASSERT(gbuffer_material_h.isValid(), "must be valid");
    renderer::Texture2D* gbuffer_material_texture = objectFromHandle<renderer::Texture2D>(gbuffer_material_h);

    ObjectHandle sampler_state_h = scene.m_globalResources.get("linear_sampler_mirror");
    ASSERT(sampler_state_h.isValid(), "must be valid");
    renderer::SamplerState* sampler_state = objectFromHandle<renderer::SamplerState>(sampler_state_h);

    cmdList->bindDescriptorSet(1,
        {
            renderer::Descriptor(sampler_state, 1),
            renderer::Descriptor(renderer::TextureView(gbuffer_albedo_texture, 0, 0), 2),
            renderer::Descriptor(gbuffer_normals_texture, 3),
            renderer::Descriptor(gbuffer_material_texture, 4),
        });

    cmdList->draw(renderer::GeometryBufferDesc(), 0, 3, 0, 1);
    cmdList->endRenderTarget();

    scene.m_globalResources.bind("render_target", m_compositionRenderTarget->getColorTexture<renderer::Texture2D>(0));
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