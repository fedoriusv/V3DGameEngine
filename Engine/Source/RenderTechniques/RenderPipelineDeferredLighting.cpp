#include "RenderPipelineDeferredLighting.h"
#include "Utils/Logger.h"

#include "Resource/ResourceManager.h"
#include "Resource/Loader/AssetSourceFileLoader.h"
#include "Resource/Loader/ShaderSourceFileLoader.h"
#include "Resource/Loader/ModelFileLoader.h"

#include "Renderer/PipelineState.h"
#include "Renderer/ShaderProgram.h"

#include "FrameProfiler.h"

namespace v3d
{
namespace scene
{

constexpr bool k_debugShadow = false;

RenderPipelineDeferredLightingStage::RenderPipelineDeferredLightingStage(RenderTechnique* technique) noexcept
    : RenderPipelineStage(technique, "DeferredLighting")
    , m_deferredRenderTarget(nullptr)
    , m_pipeline(nullptr)
{
}

RenderPipelineDeferredLightingStage::~RenderPipelineDeferredLightingStage()
{
}

void RenderPipelineDeferredLightingStage::create(renderer::Device* device, scene::SceneData& scene, scene::FrameData& frame)
{
    createRenderTarget(device, scene);

    const renderer::Shader::DefineList defines =
    {
        {"DEBUG_SHADOWMAP", std::to_string(k_debugShadow)},
    };

    const renderer::VertexShader* vertShader = resource::ResourceManager::getInstance()->loadShader<renderer::VertexShader, resource::ShaderSourceFileLoader>(device,
        "offscreen.hlsl", "offscreen_vs", defines, {}, resource::ShaderCompileFlag::ShaderCompile_UseDXCompilerForSpirV);
    const renderer::FragmentShader* fragShader = resource::ResourceManager::getInstance()->loadShader<renderer::FragmentShader, resource::ShaderSourceFileLoader>(device,
        "light.hlsl", "deffered_lighting_ps", defines, {}, resource::ShaderCompileFlag::ShaderCompile_UseDXCompilerForSpirV);

    m_pipeline = V3D_NEW(renderer::GraphicsPipelineState, memory::MemoryLabel::MemoryGame)(device, renderer::VertexInputAttributeDesc(), m_deferredRenderTarget->getRenderPassDesc(),
        V3D_NEW(renderer::ShaderProgram, memory::MemoryLabel::MemoryGame)(device, vertShader, fragShader), "deffered_light_pipeline");

    m_pipeline->setPrimitiveTopology(renderer::PrimitiveTopology::PrimitiveTopology_TriangleList);
    m_pipeline->setFrontFace(renderer::FrontFace::FrontFace_Clockwise);
    m_pipeline->setCullMode(renderer::CullMode::CullMode_Back);
    m_pipeline->setDepthCompareOp(renderer::CompareOperation::Always);
    m_pipeline->setDepthWrite(false);
    m_pipeline->setDepthTest(false);
    m_pipeline->setColorMask(0, renderer::ColorMask::ColorMask_All);
    m_pipeline->setStencilTest(false);
    m_pipeline->setStencilCompareOp(renderer::CompareOperation::NotEqual, 0x0);
    m_pipeline->setStencilOp(renderer::StencilOperation::Keep, renderer::StencilOperation::Keep, renderer::StencilOperation::Keep);

    BIND_SHADER_PARAMETER(m_pipeline, m_parameters, cb_Viewport);
    BIND_SHADER_PARAMETER(m_pipeline, m_parameters, cb_Light);
    BIND_SHADER_PARAMETER(m_pipeline, m_parameters, s_SamplerState);
    BIND_SHADER_PARAMETER(m_pipeline, m_parameters, t_TextureBaseColor);
    BIND_SHADER_PARAMETER(m_pipeline, m_parameters, t_TextureNormal);
    BIND_SHADER_PARAMETER(m_pipeline, m_parameters, t_TextureMaterial);
    BIND_SHADER_PARAMETER(m_pipeline, m_parameters, t_TextureDepth);
    BIND_SHADER_PARAMETER(m_pipeline, m_parameters, t_TextureScreenSpaceShadows);
}

void RenderPipelineDeferredLightingStage::destroy(renderer::Device* device, scene::SceneData& scene, scene::FrameData& frame)
{
    destroyRenderTarget(device, scene);

    const renderer::ShaderProgram* program = m_pipeline->getShaderProgram();
    V3D_DELETE(program, memory::MemoryLabel::MemoryGame);

    V3D_DELETE(m_pipeline, memory::MemoryLabel::MemoryGame);
    m_pipeline = nullptr;
}

void RenderPipelineDeferredLightingStage::prepare(renderer::Device* device, scene::SceneData& scene, scene::FrameData& frame)
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

void RenderPipelineDeferredLightingStage::execute(renderer::Device* device, scene::SceneData& scene, scene::FrameData& frame)
{
    renderer::CmdListRender* cmdList = frame.m_cmdList;
    scene::ViewportState& viewportState = scene.m_viewportState;

    TRACE_PROFILER_SCOPE("DeferredLighting", color::rgba8::GREEN);
    DEBUG_MARKER_SCOPE(cmdList, "DeferredLighting", color::rgbaf::GREEN);

    ObjectHandle depth_stencil_h = scene.m_globalResources.get("depth_stencil");
    ASSERT(depth_stencil_h.isValid(), "must be valid");
    renderer::Texture2D* depthStencilTexture = objectFromHandle<renderer::Texture2D>(depth_stencil_h);

    ObjectHandle screen_space_shadow_h = scene.m_globalResources.get("screen_space_shadow");
    ASSERT(screen_space_shadow_h.isValid(), "must be valid");
    renderer::Texture2D* screenspaceShadowTexture = objectFromHandle<renderer::Texture2D>(screen_space_shadow_h);

    cmdList->beginRenderTarget(*m_deferredRenderTarget);
    cmdList->setViewport({ 0.f, 0.f, (f32)viewportState._viewpotSize._width, (f32)viewportState._viewpotSize._height });
    cmdList->setScissor({ 0.f, 0.f, (f32)viewportState._viewpotSize._width, (f32)viewportState._viewpotSize._height });
    cmdList->setStencilRef(0x0);
    cmdList->setPipelineState(*m_pipeline);
    cmdList->bindDescriptorSet(m_pipeline->getShaderProgram(), 0,
        {
            renderer::Descriptor(renderer::Descriptor::ConstantBuffer{ &viewportState._viewportBuffer, 0, sizeof(viewportState._viewportBuffer)}, m_parameters.cb_Viewport)
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

    ASSERT(!scene.m_renderLists[toEnumType(scene::RenderPipelinePass::DirectionLight)].empty(), "must be presented");
    const scene::NodeEntry* entry = scene.m_renderLists[toEnumType(scene::RenderPipelinePass::DirectionLight)][0];
    const scene::LightNodeEntry& itemLight = *static_cast<const scene::LightNodeEntry*>(entry);
    const scene::DirectionalLight& dirLight = *static_cast<const scene::DirectionalLight*>(itemLight.light);

    struct LightBuffer
    {
        math::Vector3D position;
        math::Vector3D direction;
        math::float4   color;
        math::float4   attenuation;
        f32            intensity;
        f32            temperature;
        f32            type;
        f32           _pad = 0;
    };

    LightBuffer lightBuffer;
    lightBuffer.position = entry->object->getTransform().getPosition();
    lightBuffer.direction = entry->object->getDirection();
    lightBuffer.color = dirLight.getColor();
    lightBuffer.attenuation = { 1.f,  1.f,  1.f, 0.f };
    lightBuffer.type = 0;
    lightBuffer.intensity = dirLight.getIntensity();
    lightBuffer.temperature = dirLight.getTemperature();

    cmdList->bindDescriptorSet(m_pipeline->getShaderProgram(), 1,
        {
            renderer::Descriptor(renderer::Descriptor::ConstantBuffer{ &lightBuffer, 0, sizeof(lightBuffer)}, m_parameters.cb_Light),
            renderer::Descriptor(samplerState, m_parameters.s_SamplerState),
            renderer::Descriptor(renderer::TextureView(gbufferAlbedoTexture, 0, 0), m_parameters.t_TextureBaseColor),
            renderer::Descriptor(renderer::TextureView(gbufferNormalsTexture, 0, 0), m_parameters.t_TextureNormal),
            renderer::Descriptor(renderer::TextureView(gbufferMaterialTexture, 0, 0), m_parameters.t_TextureMaterial),
            renderer::Descriptor(renderer::TextureView(depthStencilTexture), m_parameters.t_TextureDepth),
            renderer::Descriptor(renderer::TextureView(screenspaceShadowTexture, 0, 0), m_parameters.t_TextureScreenSpaceShadows),
        });

    cmdList->draw(renderer::GeometryBufferDesc(), 0, 3, 0, 1);
    cmdList->endRenderTarget();

    scene.m_globalResources.bind("render_target", m_deferredRenderTarget->getColorTexture<renderer::Texture2D>(0));
}

void RenderPipelineDeferredLightingStage::createRenderTarget(renderer::Device* device, scene::SceneData& data)
{
    ASSERT(m_deferredRenderTarget == nullptr, "must be nullptr");
    m_deferredRenderTarget = V3D_NEW(renderer::RenderTargetState, memory::MemoryLabel::MemoryGame)(device, data.m_viewportState._viewpotSize, 1);

    m_deferredRenderTarget->setColorTexture(0, V3D_NEW(renderer::Texture2D, memory::MemoryLabel::MemoryGame)(device, renderer::TextureUsage::TextureUsage_Attachment | renderer::TextureUsage::TextureUsage_Sampled,
        renderer::Format::Format_R16G16B16A16_SFloat, data.m_viewportState._viewpotSize, renderer::TextureSamples::TextureSamples_x1, "deffered_lighting"),
        {
            renderer::RenderTargetLoadOp::LoadOp_DontCare, renderer::RenderTargetStoreOp::StoreOp_Store, color::Color(0.0f)
        },
        {
            renderer::TransitionOp::TransitionOp_ColorAttachment, renderer::TransitionOp::TransitionOp_ColorAttachment
        });

    ObjectHandle depth_stencil = data.m_globalResources.get("depth_stencil");
    ASSERT(depth_stencil.isValid(), "must be valid");
    renderer::Texture2D* depthAttachment = objectFromHandle<renderer::Texture2D>(depth_stencil);
    m_deferredRenderTarget->setDepthStencilTexture(depthAttachment,
        {
            renderer::RenderTargetLoadOp::LoadOp_Load, renderer::RenderTargetStoreOp::StoreOp_Store, 0.0f
        },
        {
            renderer::RenderTargetLoadOp::LoadOp_Load, renderer::RenderTargetStoreOp::StoreOp_Store, 0U
        },
        {
            renderer::TransitionOp::TransitionOp_DepthStencilReadOnly, renderer::TransitionOp::TransitionOp_DepthStencilAttachment
        });
}

void RenderPipelineDeferredLightingStage::destroyRenderTarget(renderer::Device* device, scene::SceneData& data)
{
    ASSERT(m_deferredRenderTarget, "must be valid");
    renderer::Texture2D* textrue = m_deferredRenderTarget->getColorTexture<renderer::Texture2D>(0);
    V3D_DELETE(textrue, memory::MemoryLabel::MemoryGame);

    V3D_DELETE(m_deferredRenderTarget, memory::MemoryLabel::MemoryGame);
    m_deferredRenderTarget = nullptr;
}

} //namespace scene
} //namespace v3d