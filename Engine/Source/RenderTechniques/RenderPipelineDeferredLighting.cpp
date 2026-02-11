#include "RenderPipelineDeferredLighting.h"
#include "Utils/Logger.h"

#include "Resource/ResourceManager.h"
#include "Resource/Loader/AssetSourceFileLoader.h"
#include "Resource/Loader/ShaderSourceFileLoader.h"
#include "Resource/Loader/ModelFileLoader.h"

#include "Scene/SceneNode.h"

#include "Renderer/PipelineState.h"
#include "Renderer/ShaderProgram.h"

#include "FrameProfiler.h"

namespace v3d
{
namespace scene
{

RenderPipelineDeferredLightingStage::RenderPipelineDeferredLightingStage(RenderTechnique* technique) noexcept
    : RenderPipelineStage(technique, "DeferredLighting")
    , m_deferredRenderTarget(nullptr)
    , m_pipeline(nullptr)

    , m_debugShadowCascades(false)
    , m_debugPunctualLightShadows(false)
{
}

RenderPipelineDeferredLightingStage::~RenderPipelineDeferredLightingStage()
{
}

void RenderPipelineDeferredLightingStage::create(renderer::Device* device, SceneData& scene, FrameData& frame)
{
    createRenderTarget(device, scene, frame);
    m_debugShadowCascades = scene.m_settings._shadowsParams._debugShadowCascades;
    m_debugPunctualLightShadows = scene.m_settings._shadowsParams._debugPunctualLightShadows;

    const renderer::Shader::DefineList defines =
    {
        { "DEBUG_SHADOWMAP_CASCADES", std::to_string(m_debugShadowCascades) },
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

void RenderPipelineDeferredLightingStage::destroy(renderer::Device* device, SceneData& scene, scene::FrameData& frame)
{
    destroyRenderTarget(device, scene, frame);

    const renderer::ShaderProgram* program = m_pipeline->getShaderProgram();
    V3D_DELETE(program, memory::MemoryLabel::MemoryGame);

    V3D_DELETE(m_pipeline, memory::MemoryLabel::MemoryGame);
    m_pipeline = nullptr;
}

void RenderPipelineDeferredLightingStage::prepare(renderer::Device* device, SceneData& scene, scene::FrameData& frame)
{
#if DEBUG
    if (m_debugShadowCascades != scene.m_settings._shadowsParams._debugShadowCascades ||
        m_debugPunctualLightShadows != scene.m_settings._shadowsParams._debugPunctualLightShadows)
    {
        destroy(device, scene, frame);
        create(device, scene, frame);
    }
#endif

    if (!m_deferredRenderTarget)
    {
        createRenderTarget(device, scene, frame);
    }
    else if (m_deferredRenderTarget->getRenderArea() != scene.m_viewportSize)
    {
        destroyRenderTarget(device, scene, frame);
        createRenderTarget(device, scene, frame);
    }
}

void RenderPipelineDeferredLightingStage::execute(renderer::Device* device, SceneData& scene, FrameData& frame)
{
    auto renderJob = [this](renderer::Device* device, renderer::CmdListRender* cmdList, const scene::SceneData& scene, const scene::FrameData& frame) -> void
        {
            TRACE_PROFILER_SCOPE("DeferredLighting", color::rgba8::GREEN);
            DEBUG_MARKER_SCOPE(cmdList, "DeferredLighting", color::rgbaf::GREEN);

            ObjectHandle viewportState_handle = frame.m_frameResources.get("viewport_state");
            ASSERT(viewportState_handle.isValid(), "must be valid");
            scene::ViewportState* viewportState = viewportState_handle.as<scene::ViewportState>();

            ObjectHandle depthStencil_handle = scene.m_globalResources.get("depth_stencil");
            ASSERT(depthStencil_handle.isValid(), "must be valid");
            renderer::Texture2D* depthStencilTexture = depthStencil_handle.as<renderer::Texture2D>();

            ObjectHandle gbuffer_albedo_handle = scene.m_globalResources.get("gbuffer_albedo");
            ASSERT(gbuffer_albedo_handle.isValid(), "must be valid");
            renderer::Texture2D* gbufferAlbedoTexture = gbuffer_albedo_handle.as<renderer::Texture2D>();

            ObjectHandle gbuffer_normals_handle = scene.m_globalResources.get("gbuffer_normals");
            ASSERT(gbuffer_normals_handle.isValid(), "must be valid");
            renderer::Texture2D* gbufferNormalsTexture = gbuffer_normals_handle.as<renderer::Texture2D>();

            ObjectHandle gbuffer_material_handle = scene.m_globalResources.get("gbuffer_material");
            ASSERT(gbuffer_material_handle.isValid(), "must be valid");
            renderer::Texture2D* gbufferMaterialTexture = gbuffer_material_handle.as<renderer::Texture2D>();

            ObjectHandle samplerState_handle = scene.m_globalResources.get("linear_sampler_clamp");
            ASSERT(samplerState_handle.isValid(), "must be valid");
            renderer::SamplerState* samplerState = samplerState_handle.as<renderer::SamplerState>();

            ObjectHandle screenSpaceShadows_handle = scene.m_globalResources.get("screen_space_shadow");
            if (!screenSpaceShadows_handle.isValid())
            {
                screenSpaceShadows_handle = scene.m_globalResources.get("default_black");
                ASSERT(screenSpaceShadows_handle.isValid(), "must be valid");
            }
            renderer::Texture2D* screenspaceShadowsTexture = screenSpaceShadows_handle.as<renderer::Texture2D>();

            cmdList->beginRenderTarget(*m_deferredRenderTarget);
            cmdList->setViewport({ 0.f, 0.f, (f32)viewportState->viewportSize._x, (f32)viewportState->viewportSize._y });
            cmdList->setScissor({ 0.f, 0.f, (f32)viewportState->viewportSize._x, (f32)viewportState->viewportSize._y });
            cmdList->setStencilRef(0x0);
            cmdList->setPipelineState(*m_pipeline);
            cmdList->bindDescriptorSet(m_pipeline->getShaderProgram(), 0,
                {
                    renderer::Descriptor(renderer::Descriptor::ConstantBuffer{ viewportState, 0, sizeof(scene::ViewportState)}, m_parameters.cb_Viewport)
                });

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
                f32           _pad[2];
            };

            LightBuffer lightBuffer;
            lightBuffer.position = entry->object->getTransform().getPosition();
            lightBuffer.direction = entry->object->getDirection();
            lightBuffer.color = dirLight.getColor();
            lightBuffer.attenuation = { 1.f,  1.f,  1.f, 0.f };
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
                    renderer::Descriptor(renderer::TextureView(screenspaceShadowsTexture, 0, 0), m_parameters.t_TextureScreenSpaceShadows),
                });

            cmdList->draw(renderer::GeometryBufferDesc(), 0, 3, 0, 1);
            cmdList->endRenderTarget();
        };

    addRenderJob("DeferredLighting Job", renderJob, device, scene, true);
}

void RenderPipelineDeferredLightingStage::createRenderTarget(renderer::Device* device, SceneData& scene, scene::FrameData& frame)
{
    ASSERT(m_deferredRenderTarget == nullptr, "must be nullptr");
    m_deferredRenderTarget = V3D_NEW(renderer::RenderTargetState, memory::MemoryLabel::MemoryGame)(device, scene.m_viewportSize, 1);

    m_deferredRenderTarget->setColorTexture(0, V3D_NEW(renderer::Texture2D, memory::MemoryLabel::MemoryGame)(device, renderer::TextureUsage::TextureUsage_Attachment | renderer::TextureUsage::TextureUsage_Sampled,
        renderer::Format::Format_R16G16B16A16_SFloat, scene.m_viewportSize, renderer::TextureSamples::TextureSamples_x1, "deffered_lighting"),
        {
            renderer::RenderTargetLoadOp::LoadOp_DontCare, renderer::RenderTargetStoreOp::StoreOp_Store, color::Color(0.0f)
        },
        {
            renderer::TransitionOp::TransitionOp_ColorAttachment, renderer::TransitionOp::TransitionOp_ColorAttachment
        }
    );

    ObjectHandle depthStencil_handle = scene.m_globalResources.get("depth_stencil");
    ASSERT(depthStencil_handle.isValid(), "must be valid");
    renderer::Texture2D* depthAttachment = depthStencil_handle.as<renderer::Texture2D>();
    m_deferredRenderTarget->setDepthStencilTexture(depthAttachment,
        {
            renderer::RenderTargetLoadOp::LoadOp_Load, renderer::RenderTargetStoreOp::StoreOp_Store, 0.0f
        },
        {
            renderer::RenderTargetLoadOp::LoadOp_Load, renderer::RenderTargetStoreOp::StoreOp_Store, 0U
        },
        {
            renderer::TransitionOp::TransitionOp_DepthStencilReadOnly, renderer::TransitionOp::TransitionOp_DepthStencilAttachment
        }
    );

    scene.m_globalResources.bind("color_target", m_deferredRenderTarget->getColorTexture<renderer::Texture2D>(0));
}

void RenderPipelineDeferredLightingStage::destroyRenderTarget(renderer::Device* device, SceneData& scene, scene::FrameData& frame)
{
    ASSERT(m_deferredRenderTarget, "must be valid");
    renderer::Texture2D* textrue = m_deferredRenderTarget->getColorTexture<renderer::Texture2D>(0);
    V3D_DELETE(textrue, memory::MemoryLabel::MemoryGame);

    V3D_DELETE(m_deferredRenderTarget, memory::MemoryLabel::MemoryGame);
    m_deferredRenderTarget = nullptr;
}

} //namespace scene
} //namespace v3d