#include "RenderPipelineOutline.h"
#include "Utils/Logger.h"

#include "Resource/ResourceManager.h"
#include "Resource/Loader/AssetSourceFileLoader.h"
#include "Resource/Loader/ShaderSourceFileLoader.h"
#include "Resource/Loader/ModelFileLoader.h"

#include "Renderer/ShaderProgram.h"
#include "Renderer/Buffer.h"

#include "Scene/ModelHandler.h"
#include "Scene/Geometry/Mesh.h"

#include "FrameProfiler.h"

namespace v3d
{
namespace scene
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

void RenderPipelineOutlineStage::create(renderer::Device* device, scene::SceneData& scene, scene::FrameData& frame)
{
    createRenderTarget(device, scene);

    const renderer::VertexShader* vertShader = resource::ResourceManager::getInstance()->loadShader<renderer::VertexShader, resource::ShaderSourceFileLoader>(device,
        "offscreen.hlsl", "offscreen_vs", {}, {}, resource::ShaderCompileFlag::ShaderCompile_UseDXCompilerForSpirV);
    const renderer::FragmentShader* fragShader = resource::ResourceManager::getInstance()->loadShader<renderer::FragmentShader, resource::ShaderSourceFileLoader>(device,
        "outline.hlsl", "main_ps", {}, {}, resource::ShaderCompileFlag::ShaderCompile_UseDXCompilerForSpirV);

    m_pipeline = V3D_NEW(renderer::GraphicsPipelineState, memory::MemoryLabel::MemoryGame)(device, renderer::VertexInputAttributeDesc(), m_renderTarget->getRenderPassDesc(), 
        V3D_NEW(renderer::ShaderProgram, memory::MemoryLabel::MemoryGame)(device, vertShader, fragShader), "outline_pipeline");

   m_pipeline->setPrimitiveTopology(renderer::PrimitiveTopology::PrimitiveTopology_TriangleList);
   m_pipeline->setFrontFace(renderer::FrontFace::FrontFace_Clockwise);
   m_pipeline->setCullMode(renderer::CullMode::CullMode_Back);
   m_pipeline->setDepthCompareOp(renderer::CompareOperation::Always);
   m_pipeline->setDepthWrite(false);
   m_pipeline->setDepthTest(false);
   m_pipeline->setColorMask(0, renderer::ColorMask::ColorMask_All);

    m_readbackObjectID = V3D_NEW(renderer::UnorderedAccessBuffer, memory::MemoryLabel::MemoryGame)(device, renderer::BufferUsage::Buffer_GPURead, sizeof(u64) * 2, "objectID");
    m_mappedData._ptr = m_readbackObjectID->map<u32>();
    scene.m_globalResources.bind("readback_objectIDData", &m_mappedData);
}

void RenderPipelineOutlineStage::destroy(renderer::Device* device, scene::SceneData& scene, scene::FrameData& frame)
{
    destroyRenderTarget(device, scene);

    m_readbackObjectID->unmap();
    V3D_DELETE(m_readbackObjectID, memory::MemoryLabel::MemoryGame);
    m_readbackObjectID = nullptr;

    const renderer::ShaderProgram* program = m_pipeline->getShaderProgram();
    V3D_DELETE(program, memory::MemoryLabel::MemoryGame);

    V3D_DELETE(m_pipeline, memory::MemoryLabel::MemoryGame);
    m_pipeline = nullptr;
}

void RenderPipelineOutlineStage::prepare(renderer::Device* device, scene::SceneData& scene, scene::FrameData& frame)
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

void RenderPipelineOutlineStage::execute(renderer::Device* device, scene::SceneData& scene, scene::FrameData& frame)
{
    renderer::CmdListRender* cmdList = frame.m_cmdList;
    scene::ViewportState& viewportState = scene.m_viewportState;

    TRACE_PROFILER_SCOPE("Outline", color::rgba8::GREEN);
    DEBUG_MARKER_SCOPE(cmdList, "Outline", color::rgbaf::GREEN);

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

void RenderPipelineOutlineStage::createRenderTarget(renderer::Device* device, scene::SceneData& data)
{
    ASSERT(m_renderTarget == nullptr, "must be nullptr");
    m_renderTarget = V3D_NEW(renderer::RenderTargetState, memory::MemoryLabel::MemoryGame)(device, data.m_viewportState._viewpotSize, 1, 0, "outline_pass");

    m_renderTarget->setColorTexture(0, V3D_NEW(renderer::Texture2D, memory::MemoryLabel::MemoryGame)(device, renderer::TextureUsage::TextureUsage_Attachment | renderer::TextureUsage::TextureUsage_Sampled,
        renderer::Format::Format_R16G16B16A16_SFloat, data.m_viewportState._viewpotSize, renderer::TextureSamples::TextureSamples_x1, "outline"),
        {
            renderer::RenderTargetLoadOp::LoadOp_DontCare, renderer::RenderTargetStoreOp::StoreOp_Store, color::Color(0.0f)
        },
        {
            renderer::TransitionOp::TransitionOp_Undefined, renderer::TransitionOp::TransitionOp_ColorAttachment
        });
}

void RenderPipelineOutlineStage::destroyRenderTarget(renderer::Device* device, scene::SceneData& data)
{
    ASSERT(m_renderTarget, "must be valid");
    renderer::Texture2D* outline = m_renderTarget->getColorTexture<renderer::Texture2D>(0);
    V3D_DELETE(outline, memory::MemoryLabel::MemoryGame);

    V3D_DELETE(m_renderTarget, memory::MemoryLabel::MemoryGame);
    m_renderTarget = nullptr;
}

} //namespace renderer
} //namespace v3d