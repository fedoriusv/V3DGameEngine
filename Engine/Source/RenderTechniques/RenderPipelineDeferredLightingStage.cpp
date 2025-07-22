#include "RenderPipelineDeferredLightingStage.h"

#include "Resource/ResourceManager.h"

#include "Resource/Loader/AssetSourceFileLoader.h"
#include "Resource/Loader/ShaderSourceFileLoader.h"
#include "Resource/Loader/ModelFileLoader.h"

namespace v3d
{
namespace renderer
{

RenderPipelineDeferredLightingStage::RenderPipelineDeferredLightingStage(RenderTechnique* technique) noexcept
    : RenderPipelineStage(technique, "DeferredLighting")
    , m_deferredRenderTarget(nullptr)
    , m_pipeline(nullptr)
{
}

RenderPipelineDeferredLightingStage::~RenderPipelineDeferredLightingStage()
{
}

void RenderPipelineDeferredLightingStage::create(Device* device, scene::SceneData& scene, scene::FrameData& frame)
{
    createRenderTarget(device, scene);

    const renderer::VertexShader* vertShader = resource::ResourceManager::getInstance()->loadShader<renderer::VertexShader, resource::ShaderSourceFileLoader>(device,
        "offscreen.hlsl", "offscreen_vs", {}, {}, resource::ShaderCompileFlag::ShaderCompile_UseDXCompilerForSpirV);
    const renderer::FragmentShader* fragShader = resource::ResourceManager::getInstance()->loadShader<renderer::FragmentShader, resource::ShaderSourceFileLoader>(device,
        "light.hlsl", "deffered_lighting_ps", {}, {}, resource::ShaderCompileFlag::ShaderCompile_UseDXCompilerForSpirV);

    m_pipeline = new renderer::GraphicsPipelineState(
        device, renderer::VertexInputAttributeDesc(), m_deferredRenderTarget->getRenderPassDesc(), new renderer::ShaderProgram(device, vertShader, fragShader), "deffered_light_pipeline");

    m_pipeline->setPrimitiveTopology(renderer::PrimitiveTopology::PrimitiveTopology_TriangleList);
    m_pipeline->setFrontFace(renderer::FrontFace::FrontFace_Clockwise);
    m_pipeline->setCullMode(renderer::CullMode::CullMode_Back);
    m_pipeline->setDepthCompareOp(renderer::CompareOperation::CompareOp_Always);
    m_pipeline->setDepthWrite(false);
    m_pipeline->setDepthTest(false);
    m_pipeline->setColorMask(0, renderer::ColorMask::ColorMask_All);
}

void RenderPipelineDeferredLightingStage::destroy(Device* device, scene::SceneData& scene, scene::FrameData& frame)
{
    destroyRenderTarget(device, scene);

    delete m_pipeline;
    m_pipeline = nullptr;
}

void RenderPipelineDeferredLightingStage::prepare(Device* device, scene::SceneData& scene, scene::FrameData& frame)
{
    if (!m_deferredRenderTarget)
    {
        createRenderTarget(device, scene);
    }
    else if (m_deferredRenderTarget->getRenderArea() != scene.m_viewportState._viewpotSize)
    {
        destroyRenderTarget(device, scene);
        createRenderTarget(device, scene);
    }
}

void RenderPipelineDeferredLightingStage::execute(Device* device, scene::SceneData& scene, scene::FrameData& frame)
{
    renderer::CmdListRender* cmdList = scene.m_renderState.m_cmdList;
    scene::ViewportState& viewportState = scene.m_viewportState;

    DEBUG_MARKER_SCOPE(cmdList, "DeferredLighting", color::colorrgbaf::GREEN);

    ObjectHandle depth_stencil_h = scene.m_globalResources.get("depth_stencil");
    ASSERT(depth_stencil_h.isValid(), "must be valid");
    renderer::Texture2D* depthStencilTexture = objectFromHandle<renderer::Texture2D>(depth_stencil_h);

    cmdList->transition(depthStencilTexture, renderer::TransitionOp::TransitionOp_ShaderRead);
    cmdList->beginRenderTarget(*m_deferredRenderTarget);
    cmdList->setViewport({ 0.f, 0.f, (f32)viewportState._viewpotSize._width, (f32)viewportState._viewpotSize._height });
    cmdList->setScissor({ 0.f, 0.f, (f32)viewportState._viewpotSize._width, (f32)viewportState._viewpotSize._height });
    cmdList->setPipelineState(*m_pipeline);

    cmdList->bindDescriptorSet(0,
        {
            renderer::Descriptor(renderer::Descriptor::ConstantBuffer{ &viewportState._viewportBuffer, 0, sizeof(viewportState._viewportBuffer)}, 0)
        });

    ObjectHandle gbuffer_albedo_h = scene.m_globalResources.get("gbuffer_albedo");
    ASSERT(gbuffer_albedo_h.isValid(), "must be valid");
    renderer::Texture2D* gbufferAlbedoTexture = objectFromHandle<renderer::Texture2D>(gbuffer_albedo_h);

    ObjectHandle gbuffer_normals_h = scene.m_globalResources.get("gbuffer_normals");
    ASSERT(gbuffer_normals_h.isValid(), "must be valid");
    renderer::Texture2D* gbufferNormalsTexture = objectFromHandle<renderer::Texture2D>(gbuffer_normals_h);

    ObjectHandle gbuffer_material_h = scene.m_globalResources.get("gbuffer_material");
    ASSERT(gbuffer_material_h.isValid(), "must be valid");
    renderer::Texture2D* gbufferMaterialTexture = objectFromHandle<renderer::Texture2D>(gbuffer_material_h);

    ObjectHandle sampler_state_h = scene.m_globalResources.get("linear_sampler_clamp");
    ASSERT(sampler_state_h.isValid(), "must be valid");
    renderer::SamplerState* samplerState = objectFromHandle<renderer::SamplerState>(sampler_state_h);

    struct LightBuffer
    {
        math::float4 position;
        math::float4 direction;
        math::float4 color;
        f32          type;
        f32          attenuation;
        f32          intensity;
        f32          temperature;
    };

    math::Matrix4D rotate;
    rotate.setRotation(scene.m_diectionalLightState._transform.getRotation());
    math::Vector4D forward(0.f, 0.f, 1.f, 1.f);
    math::Vector4D directional = rotate * forward;

    LightBuffer lightBuffer;
    lightBuffer.position = { scene.m_diectionalLightState._transform.getPosition().getX(), scene.m_diectionalLightState._transform.getPosition().getY(), scene.m_diectionalLightState._transform.getPosition().getZ() };
    lightBuffer.direction = { directional.getX(), directional.getY(), directional.getZ() };
    lightBuffer.color = scene.m_diectionalLightState._color;
    lightBuffer.type = 0;
    lightBuffer.attenuation = 1.f;
    lightBuffer.intensity = scene.m_diectionalLightState._intensity;
    lightBuffer.temperature = scene.m_diectionalLightState._temperature;

    cmdList->bindDescriptorSet(1,
        {
            renderer::Descriptor(renderer::Descriptor::ConstantBuffer{ &lightBuffer, 0, sizeof(lightBuffer)}, 1),
            renderer::Descriptor(samplerState, 2),
            renderer::Descriptor(renderer::TextureView(gbufferAlbedoTexture, 0, 0), 3),
            renderer::Descriptor(renderer::TextureView(gbufferNormalsTexture, 0, 0), 4),
            renderer::Descriptor(renderer::TextureView(gbufferMaterialTexture, 0, 0), 5),
            renderer::Descriptor(renderer::TextureView(depthStencilTexture), 6),
        });

    cmdList->draw(renderer::GeometryBufferDesc(), 0, 3, 0, 1);
    cmdList->endRenderTarget();
    cmdList->transition(depthStencilTexture, renderer::TransitionOp::TransitionOp_DepthStencilAttachment);

    scene.m_globalResources.bind("render_target", m_deferredRenderTarget->getColorTexture<renderer::Texture2D>(0));
}

void RenderPipelineDeferredLightingStage::createRenderTarget(Device* device, scene::SceneData& data)
{
    ASSERT(m_deferredRenderTarget == nullptr, "must be nullptr");
    m_deferredRenderTarget = new renderer::RenderTargetState(device, data.m_viewportState._viewpotSize, 1);

    m_deferredRenderTarget->setColorTexture(0, new renderer::Texture2D(device, renderer::TextureUsage::TextureUsage_Attachment | renderer::TextureUsage::TextureUsage_Sampled,
        renderer::Format::Format_R16G16B16A16_SFloat, data.m_viewportState._viewpotSize, renderer::TextureSamples::TextureSamples_x1, "deffered_lighting"),
        {
            renderer::RenderTargetLoadOp::LoadOp_DontCare, renderer::RenderTargetStoreOp::StoreOp_Store, color::Color(0.0f)
        },
        {
            renderer::TransitionOp::TransitionOp_Undefined, renderer::TransitionOp::TransitionOp_ColorAttachment
        });
}

void RenderPipelineDeferredLightingStage::destroyRenderTarget(Device* device, scene::SceneData& data)
{
    ASSERT(m_deferredRenderTarget, "must be valid");
    renderer::Texture2D* textrue = m_deferredRenderTarget->getColorTexture<renderer::Texture2D>(0);
    delete textrue;

    delete m_deferredRenderTarget;
    m_deferredRenderTarget = nullptr;
}

} //namespace renderer
} //namespace v3d