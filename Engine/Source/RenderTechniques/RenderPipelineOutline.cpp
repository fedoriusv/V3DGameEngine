#include "RenderPipelineOutline.h"
#include "Utils/Logger.h"

#include "Resource/ResourceManager.h"

#include "Resource/Loader/AssetSourceFileLoader.h"
#include "Resource/Loader/ShaderSourceFileLoader.h"
#include "Resource/Loader/ModelFileLoader.h"

namespace v3d
{
namespace renderer
{

RenderPipelineOutlineStage::RenderPipelineOutlineStage(RenderTechnique* technique) noexcept
    : RenderPipelineStage(technique, "outline")
    , m_readbackObjectID(nullptr)
    , m_renderTarget(nullptr)
{
}

RenderPipelineOutlineStage::~RenderPipelineOutlineStage()
{
}

void RenderPipelineOutlineStage::create(Device* device, scene::Scene::SceneData& state)
{
    createRenderTarget(device, state);

    const renderer::VertexShader* vertShader = resource::ResourceManager::getInstance()->loadShader<renderer::VertexShader, resource::ShaderSourceFileLoader>(device,
        "offscreen.hlsl", "offscreen_vs", {}, {}/*, resource::ShaderCompileFlag::ShaderCompile_UseDXCompilerForSpirV*/);
    const renderer::FragmentShader* fragShader = resource::ResourceManager::getInstance()->loadShader<renderer::FragmentShader, resource::ShaderSourceFileLoader>(device,
        "outline.hlsl", "main_ps", {}, {}/*, resource::ShaderCompileFlag::ShaderCompile_UseDXCompilerForSpirV*/);

    m_pipeline = new renderer::GraphicsPipelineState(
        device, renderer::VertexInputAttributeDesc(), m_renderTarget->getRenderPassDesc(), new renderer::ShaderProgram(device, vertShader, fragShader), "outline_pipeline");

   m_pipeline->setPrimitiveTopology(renderer::PrimitiveTopology::PrimitiveTopology_TriangleList);
   m_pipeline->setFrontFace(renderer::FrontFace::FrontFace_Clockwise);
   m_pipeline->setCullMode(renderer::CullMode::CullMode_Back);
   m_pipeline->setDepthCompareOp(renderer::CompareOperation::CompareOp_Always);
   m_pipeline->setDepthWrite(false);
   m_pipeline->setDepthTest(false);
   m_pipeline->setColorMask(0, renderer::ColorMask::ColorMask_All);

    m_sampler = new renderer::SamplerState(device, renderer::SamplerFilter::SamplerFilter_Trilinear, renderer::SamplerAnisotropic::SamplerAnisotropic_4x);
    m_sampler->setWrap(renderer::SamplerWrap::TextureWrap_ClampToBorder);

    m_readbackObjectID = new renderer::UnorderedAccessBuffer(device, renderer::BufferUsage::Buffer_GPURead, sizeof(u64) * 2, "objectID");
    m_mappedData._ptr = m_readbackObjectID->map<u32>();
    state.m_globalResources.bind("readback_objectIDData", &m_mappedData);
}

void RenderPipelineOutlineStage::destroy(Device* device, scene::Scene::SceneData& state)
{
    m_readbackObjectID->unmap();
    delete m_readbackObjectID;
    m_readbackObjectID = nullptr;
}

void RenderPipelineOutlineStage::prepare(Device* device, scene::Scene::SceneData& state)
{
}

void RenderPipelineOutlineStage::execute(Device* device, scene::Scene::SceneData& state)
{
    DEBUG_MARKER_SCOPE(state.m_renderState.m_cmdList, "Outline", color::colorrgbaf::GREEN);

    state.m_renderState.m_cmdList->beginRenderTarget(*m_renderTarget);
    state.m_renderState.m_cmdList->setViewport({ 0.f, 0.f, (f32)state.m_viewportState.m_viewpotSize._width, (f32)state.m_viewportState.m_viewpotSize._height });
    state.m_renderState.m_cmdList->setScissor({ 0.f, 0.f, (f32)state.m_viewportState.m_viewpotSize._width, (f32)state.m_viewportState.m_viewpotSize._height });
    state.m_renderState.m_cmdList->setPipelineState(*m_pipeline);

    state.m_renderState.m_cmdList->bindDescriptorSet(0,
        {
            renderer::Descriptor(renderer::Descriptor::ConstantBuffer{ &state.m_viewportState._viewportBuffer, 0, sizeof(state.m_viewportState._viewportBuffer)}, 0)
        });

    ObjectHandle composite = state.m_globalResources.get("render_target");
    ASSERT(composite.isValid(), "must be valid");
    renderer::Texture2D* compositeTexture = objectFromHandle<renderer::Texture2D>(composite);

    ObjectHandle gbuffer_material = state.m_globalResources.get("gbuffer_material");
    ASSERT(gbuffer_material.isValid(), "must be valid");
    renderer::Texture2D* gbuffer_materialTexture = objectFromHandle<renderer::Texture2D>(gbuffer_material);

    struct OutlineBuffer
    {
        math::float4 lineColor;
        f32          lineThickness;
        u32          selectedID;
    };
    OutlineBuffer constantBuffer;
    constantBuffer.lineColor = { 1.f, 1.f, 0.f, 1.f };
    constantBuffer.lineThickness = 2.f;
    constantBuffer.selectedID = state.m_editorState.selectedObjectID;

    state.m_renderState.m_cmdList->bindDescriptorSet(1,
        {
            renderer::Descriptor(renderer::Descriptor::ConstantBuffer{ &constantBuffer, 0, sizeof(constantBuffer)}, 1),
            renderer::Descriptor(m_sampler, 2),
            renderer::Descriptor(renderer::TextureView(compositeTexture, 0, 0), 3),
            renderer::Descriptor(renderer::TextureView(gbuffer_materialTexture, 0, 0), 4),
            renderer::Descriptor(m_readbackObjectID, 5),
        });

    state.m_renderState.m_cmdList->draw(renderer::GeometryBufferDesc(), 0, 3, 0, 1);
    state.m_renderState.m_cmdList->endRenderTarget();

    state.m_globalResources.bind("render_target", m_renderTarget->getColorTexture<renderer::Texture2D>(0));
}

void RenderPipelineOutlineStage::changed(Device* device, scene::Scene::SceneData& data)
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

void RenderPipelineOutlineStage::createRenderTarget(Device* device, scene::Scene::SceneData& data)
{
    ASSERT(m_renderTarget == nullptr, "must be nullptr");
    m_renderTarget = new renderer::RenderTargetState(device, data.m_viewportState.m_viewpotSize, 1, 0, "outline_pass");

    renderer::Texture2D* outline = new renderer::Texture2D(device, renderer::TextureUsage::TextureUsage_Attachment | renderer::TextureUsage::TextureUsage_Sampled,
        renderer::Format::Format_R8G8B8A8_UNorm, data.m_viewportState.m_viewpotSize, renderer::TextureSamples::TextureSamples_x1, "outline");

    m_renderTarget->setColorTexture(0, outline,
        {
            renderer::RenderTargetLoadOp::LoadOp_DontCare, renderer::RenderTargetStoreOp::StoreOp_Store, color::Color(0.0f)
        },
        {
            renderer::TransitionOp::TransitionOp_Undefined, renderer::TransitionOp::TransitionOp_ColorAttachment
        }
    );
}

void RenderPipelineOutlineStage::destroyRenderTarget(Device* device, scene::Scene::SceneData& data)
{
    ASSERT(m_renderTarget, "must be valid");
    renderer::Texture2D* outline = m_renderTarget->getColorTexture<renderer::Texture2D>(0);
    delete outline;

    delete m_renderTarget;
    m_renderTarget = nullptr;
}

} //namespace renderer
} //namespace v3d