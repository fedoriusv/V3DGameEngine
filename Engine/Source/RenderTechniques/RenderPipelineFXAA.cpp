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
    : RenderPipelineStage(technique, "FXAA")
    , m_renderTarget(nullptr)
    , m_pipeline(nullptr)
{
}

RenderPipelineFXAAStage::~RenderPipelineFXAAStage()
{
}

void RenderPipelineFXAAStage::create(Device* device, scene::SceneData& scene, scene::FrameData& frame)
{
    createRenderTarget(device, scene);

    const renderer::VertexShader* vertShader = resource::ResourceManager::getInstance()->loadShader<renderer::VertexShader, resource::ShaderSourceFileLoader>(device,
        "offscreen.hlsl", "offscreen_vs", {}, {}, resource::ShaderCompileFlag::ShaderCompile_UseDXCompilerForSpirV);
    const renderer::FragmentShader* fragShader = resource::ResourceManager::getInstance()->loadShader<renderer::FragmentShader, resource::ShaderSourceFileLoader>(device,
        "fxaa.hlsl", "fxaa_ps", {}, {}, resource::ShaderCompileFlag::ShaderCompile_UseDXCompilerForSpirV);

    m_pipeline = new renderer::GraphicsPipelineState(
        device, renderer::VertexInputAttributeDesc(), m_renderTarget->getRenderPassDesc(), new renderer::ShaderProgram(device, vertShader, fragShader), "fxaa_pipeline");

    m_pipeline->setPrimitiveTopology(renderer::PrimitiveTopology::PrimitiveTopology_TriangleList);
    m_pipeline->setFrontFace(renderer::FrontFace::FrontFace_Clockwise);
    m_pipeline->setCullMode(renderer::CullMode::CullMode_Back);
    m_pipeline->setDepthCompareOp(renderer::CompareOperation::CompareOp_Always);
    m_pipeline->setDepthWrite(false);
    m_pipeline->setDepthTest(false);
    m_pipeline->setColorMask(0, renderer::ColorMask::ColorMask_All);
}

void RenderPipelineFXAAStage::destroy(Device* device, scene::SceneData& scene, scene::FrameData& frame)
{
    destroyRenderTarget(device, scene);

    delete m_pipeline;
    m_pipeline = nullptr;
}

void RenderPipelineFXAAStage::prepare(Device* device, scene::SceneData& scene, scene::FrameData& frame)
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

void RenderPipelineFXAAStage::execute(Device* device, scene::SceneData& scene, scene::FrameData& frame)
{
    renderer::CmdListRender* cmdList = scene.m_renderState.m_cmdList;
    scene::ViewportState& viewportState = scene.m_viewportState;

    DEBUG_MARKER_SCOPE(cmdList, "FXAA", color::colorrgbaf::GREEN);

    cmdList->beginRenderTarget(*m_renderTarget);
    cmdList->setViewport({ 0.f, 0.f, (f32)viewportState._viewpotSize._width, (f32)viewportState._viewpotSize._height });
    cmdList->setScissor({ 0.f, 0.f, (f32)viewportState._viewpotSize._width, (f32)viewportState._viewpotSize._height });
    cmdList->setPipelineState(*m_pipeline);

    cmdList->bindDescriptorSet(0,
        {
            renderer::Descriptor(renderer::Descriptor::ConstantBuffer{ &viewportState._viewportBuffer, 0, sizeof(viewportState._viewportBuffer)}, 0)
        });

    ObjectHandle render_target = scene.m_globalResources.get("full_composite");
    ASSERT(render_target.isValid(), "must be valid");
    renderer::Texture2D* texture = objectFromHandle<renderer::Texture2D>(render_target);

    ObjectHandle sampler_state_h = scene.m_globalResources.get("linear_sampler_mirror");
    ASSERT(sampler_state_h.isValid(), "must be valid");
    renderer::SamplerState* sampler_state = objectFromHandle<renderer::SamplerState>(sampler_state_h);

    cmdList->bindDescriptorSet(1,
        {
            renderer::Descriptor(sampler_state, 1),
            renderer::Descriptor(texture, 2)
        });

    cmdList->draw(renderer::GeometryBufferDesc(), 0, 3, 0, 1);

    cmdList->endRenderTarget();

    scene.m_globalResources.bind("render_target", m_renderTarget->getColorTexture<renderer::Texture2D>(0));
}

void RenderPipelineFXAAStage::createRenderTarget(Device* device, scene::SceneData& data)
{
    ASSERT(m_renderTarget == nullptr, "must be nullptr");
    m_renderTarget = new renderer::RenderTargetState(device, data.m_viewportState._viewpotSize, 1, 0, "fxaa_pass");

    renderer::Texture2D* texture = new renderer::Texture2D(device, renderer::TextureUsage::TextureUsage_Attachment | renderer::TextureUsage::TextureUsage_Sampled,
        renderer::Format::Format_R8G8B8A8_UNorm, data.m_viewportState._viewpotSize, renderer::TextureSamples::TextureSamples_x1, "fxaa");

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

void RenderPipelineFXAAStage::destroyRenderTarget(Device* device, scene::SceneData& data)
{
    ASSERT(m_renderTarget, "must be valid");
    renderer::Texture2D* composition = m_renderTarget->getColorTexture<renderer::Texture2D>(0);
    delete composition;

    delete m_renderTarget;
    m_renderTarget = nullptr;
}

} //namespace renderer
} //namespace v3d