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
    : RenderPipelineStage(technique, "Outline")
    , m_readbackObjectID(nullptr)
    , m_renderTarget(nullptr)
    , m_pipeline(nullptr)
{
}

RenderPipelineOutlineStage::~RenderPipelineOutlineStage()
{
}

void RenderPipelineOutlineStage::create(Device* device, scene::SceneData& scene, scene::FrameData& frame)
{
    createRenderTarget(device, scene);

    const renderer::VertexShader* vertShader = resource::ResourceManager::getInstance()->loadShader<renderer::VertexShader, resource::ShaderSourceFileLoader>(device,
        "offscreen.hlsl", "offscreen_vs", {}, {}, resource::ShaderCompileFlag::ShaderCompile_UseDXCompilerForSpirV);
    const renderer::FragmentShader* fragShader = resource::ResourceManager::getInstance()->loadShader<renderer::FragmentShader, resource::ShaderSourceFileLoader>(device,
        "outline.hlsl", "main_ps", {}, {}, resource::ShaderCompileFlag::ShaderCompile_UseDXCompilerForSpirV);

    m_pipeline = new renderer::GraphicsPipelineState(
        device, renderer::VertexInputAttributeDesc(), m_renderTarget->getRenderPassDesc(), new renderer::ShaderProgram(device, vertShader, fragShader), "outline_pipeline");

   m_pipeline->setPrimitiveTopology(renderer::PrimitiveTopology::PrimitiveTopology_TriangleList);
   m_pipeline->setFrontFace(renderer::FrontFace::FrontFace_Clockwise);
   m_pipeline->setCullMode(renderer::CullMode::CullMode_Back);
   m_pipeline->setDepthCompareOp(renderer::CompareOperation::CompareOp_Always);
   m_pipeline->setDepthWrite(false);
   m_pipeline->setDepthTest(false);
   m_pipeline->setColorMask(0, renderer::ColorMask::ColorMask_All);

    m_readbackObjectID = new renderer::UnorderedAccessBuffer(device, renderer::BufferUsage::Buffer_GPURead, sizeof(u64) * 2, "objectID");
    m_mappedData._ptr = m_readbackObjectID->map<u32>();
    scene.m_globalResources.bind("readback_objectIDData", &m_mappedData);
}

void RenderPipelineOutlineStage::destroy(Device* device, scene::SceneData& scene, scene::FrameData& frame)
{
    destroyRenderTarget(device, scene);

    m_readbackObjectID->unmap();
    delete m_readbackObjectID;
    m_readbackObjectID = nullptr;

    delete m_pipeline;
    m_pipeline = nullptr;
}

void RenderPipelineOutlineStage::prepare(Device* device, scene::SceneData& scene, scene::FrameData& frame)
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

void RenderPipelineOutlineStage::execute(Device* device, scene::SceneData& scene, scene::FrameData& frame)
{
    renderer::CmdListRender* cmdList = scene.m_renderState.m_cmdList;
    scene::ViewportState& viewportState = scene.m_viewportState;

    DEBUG_MARKER_SCOPE(cmdList, "Outline", color::colorrgbaf::GREEN);

    cmdList->beginRenderTarget(*m_renderTarget);
    cmdList->setViewport({ 0.f, 0.f, (f32)viewportState._viewpotSize._width, (f32)viewportState._viewpotSize._height });
    cmdList->setScissor({ 0.f, 0.f, (f32)viewportState._viewpotSize._width, (f32)viewportState._viewpotSize._height });
    cmdList->setPipelineState(*m_pipeline);

    cmdList->bindDescriptorSet(0,
        {
            renderer::Descriptor(renderer::Descriptor::ConstantBuffer{ &viewportState._viewportBuffer, 0, sizeof(viewportState._viewportBuffer)}, 0)
        });

    ObjectHandle composite = scene.m_globalResources.get("render_target");
    ASSERT(composite.isValid(), "must be valid");
    renderer::Texture2D* compositeTexture = objectFromHandle<renderer::Texture2D>(composite);

    ObjectHandle gbuffer_material = scene.m_globalResources.get("gbuffer_material");
    ASSERT(gbuffer_material.isValid(), "must be valid");
    renderer::Texture2D* gbuffer_materialTexture = objectFromHandle<renderer::Texture2D>(gbuffer_material);

    ObjectHandle selected_objects = scene.m_globalResources.get("selected_objects");
    ASSERT(selected_objects.isValid(), "must be valid");
    renderer::Texture2D* selected_objectsTexture = objectFromHandle<renderer::Texture2D>(selected_objects);

    ObjectHandle sampler_state_h = scene.m_globalResources.get("linear_sampler_clamp");
    ASSERT(sampler_state_h.isValid(), "must be valid");
    renderer::SamplerState* sampler_state = objectFromHandle<renderer::SamplerState>(sampler_state_h);

    struct OutlineBuffer
    {
        math::float4 lineColor;
        f32          lineThickness;
    };
    OutlineBuffer constantBuffer;
    constantBuffer.lineColor = { 1.f, 1.f, 0.f, 1.f };
    constantBuffer.lineThickness = 2.f;

    cmdList->bindDescriptorSet(1,
        {
            renderer::Descriptor(renderer::Descriptor::ConstantBuffer{ &constantBuffer, 0, sizeof(constantBuffer)}, 1),
            renderer::Descriptor(sampler_state, 2),
            renderer::Descriptor(renderer::TextureView(compositeTexture, 0, 0), 3),
            renderer::Descriptor(renderer::TextureView(gbuffer_materialTexture, 0, 0), 4),
            renderer::Descriptor(renderer::TextureView(selected_objectsTexture, 0, 0), 5),
            renderer::Descriptor(m_readbackObjectID, 6),
        });

    cmdList->draw(renderer::GeometryBufferDesc(), 0, 3, 0, 1);
    cmdList->endRenderTarget();

    scene.m_globalResources.bind("render_target", m_renderTarget->getColorTexture<renderer::Texture2D>(0));
}

void RenderPipelineOutlineStage::createRenderTarget(Device* device, scene::SceneData& data)
{
    ASSERT(m_renderTarget == nullptr, "must be nullptr");
    m_renderTarget = new renderer::RenderTargetState(device, data.m_viewportState._viewpotSize, 1, 0, "outline_pass");

    renderer::Texture2D* outline = new renderer::Texture2D(device, renderer::TextureUsage::TextureUsage_Attachment | renderer::TextureUsage::TextureUsage_Sampled,
        renderer::Format::Format_R16G16B16A16_SFloat, data.m_viewportState._viewpotSize, renderer::TextureSamples::TextureSamples_x1, "outline");

    m_renderTarget->setColorTexture(0, outline,
        {
            renderer::RenderTargetLoadOp::LoadOp_DontCare, renderer::RenderTargetStoreOp::StoreOp_Store, color::Color(0.0f)
        },
        {
            renderer::TransitionOp::TransitionOp_Undefined, renderer::TransitionOp::TransitionOp_ColorAttachment
        }
    );
}

void RenderPipelineOutlineStage::destroyRenderTarget(Device* device, scene::SceneData& data)
{
    ASSERT(m_renderTarget, "must be valid");
    renderer::Texture2D* outline = m_renderTarget->getColorTexture<renderer::Texture2D>(0);
    delete outline;

    delete m_renderTarget;
    m_renderTarget = nullptr;
}

} //namespace renderer
} //namespace v3d