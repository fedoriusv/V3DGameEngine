#include "RenderPipelineLightAccumulationStage.h"
#include "Utils/Logger.h"

#include "Resource/ResourceManager.h"

#include "Resource/Loader/AssetSourceFileLoader.h"
#include "Resource/Loader/ShaderSourceFileLoader.h"
#include "Resource/Loader/ModelFileLoader.h"

#include "Scene/ModelHandler.h"
#include "Scene/Geometry/Mesh.h"
#include "Scene/SceneNode.h"

#include "RenderPipelineShadow.h"
#include "FrameProfiler.h"

namespace v3d
{
namespace scene
{

RenderPipelineLightAccumulationStage::RenderPipelineLightAccumulationStage(RenderTechnique* technique, scene::ModelHandler* modelHandler) noexcept
    : RenderPipelineStage(technique, "LightPass")
    , m_modelHandler(modelHandler)

    , m_shadowSamplerState(nullptr)
    , m_lightRenderTarget(nullptr)

    , m_debugPunctualLightShadows(false)
{
    m_pipeline[0] = nullptr;
    m_pipeline[1] = nullptr;
}

RenderPipelineLightAccumulationStage::~RenderPipelineLightAccumulationStage()
{
}

void RenderPipelineLightAccumulationStage::create(renderer::Device* device, scene::SceneData& scene, scene::FrameData& frame)
{
    if (m_created)
    {
        return;
    }

    createRenderTarget(device, scene);
    createPipelines(device, scene);

    m_sphereVolume = scene::MeshHelper::createSphere(device, 1.f, 32, 32, "pointLight");
    m_coneVolume = scene::MeshHelper::createCone(device, 1.f, 1.f, 32, "spotLight");

    m_created = true;
}

void RenderPipelineLightAccumulationStage::destroy(renderer::Device* device, scene::SceneData& scene, scene::FrameData& frame)
{
    if (m_created)
    {
        destroyRenderTarget(device, scene);
        destroyPipelines(device, scene);

        V3D_DELETE(m_sphereVolume, memory::MemoryLabel::MemoryObject);
        V3D_DELETE(m_coneVolume, memory::MemoryLabel::MemoryObject);

        V3D_DELETE(m_shadowSamplerState, memory::MemoryLabel::MemoryGame);
        m_shadowSamplerState = nullptr;

        m_created = false;
    }
}

void RenderPipelineLightAccumulationStage::prepare(renderer::Device* device, scene::SceneData& scene, scene::FrameData& frame)
{
    ASSERT(m_created, "must be created");
#if DEBUG
    if (m_debugPunctualLightShadows != scene.m_settings._shadowsParams._debugPunctualLightShadows)
    {
        destroy(device, scene, frame);
        create(device, scene, frame);
    }
#endif

    if (!m_lightRenderTarget)
    {
        createRenderTarget(device, scene);
    }
    else if (m_lightRenderTarget->getRenderArea() != scene.m_viewportSize)
    {
        destroyRenderTarget(device, scene);
        createRenderTarget(device, scene);
    }

    if (!m_pipeline)
    {
        createPipelines(device, scene);
    }
}

void RenderPipelineLightAccumulationStage::execute(renderer::Device* device, scene::SceneData& scene, scene::FrameData& frame)
{
    ASSERT(m_created, "must be created");
    if (scene.m_renderLists[toEnumType(scene::ScenePass::PunctualLights)].empty())
    {
        return;
    }

    auto renderJob = [this](renderer::Device* device, renderer::CmdListRender* cmdList, const scene::SceneData& scene, const scene::FrameData& frame) -> void
        {
            TRACE_PROFILER_SCOPE("VolumeLights", color::rgba8::GREEN);
            DEBUG_MARKER_SCOPE(cmdList, "VolumeLights", color::rgbaf::GREEN);
            ASSERT(!scene.m_renderLists[toEnumType(scene::ScenePass::PunctualLights)].empty(), "must not be empty");

            ObjectHandle viewportState_handle = frame.m_frameResources.get("viewport_state");
            ASSERT(viewportState_handle.isValid(), "must be valid");
            scene::ViewportState* viewportState = viewportState_handle.as<scene::ViewportState>();

            ObjectHandle shadowData_handle = frame.m_frameResources.get("shadow_data");
            RenderPipelineShadowStage::PipelineData* shadowData = nullptr;
            if (shadowData_handle.isValid())
            {
               shadowData = shadowData_handle.as<RenderPipelineShadowStage::PipelineData>();
            }

            ObjectHandle rt_handle = scene.m_globalResources.get("color_target");
            ASSERT(rt_handle.isValid(), "must be valid");
            renderer::Texture2D* renderTargetTexture = rt_handle.as<renderer::Texture2D>();

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

            ObjectHandle samplerState_handle = scene.m_globalResources.get("linear_sampler_clamp_border");
            ASSERT(samplerState_handle.isValid(), "must be valid");
            renderer::SamplerState* samplerState = samplerState_handle.as<renderer::SamplerState>();

            ObjectHandle shadowmaps_handle = scene.m_globalResources.get("shadowmaps_array");
            ASSERT(shadowmaps_handle.isValid(), "must be valid");
            renderer::Texture2D* shadowmapsTexture = shadowmaps_handle.as<renderer::Texture2D>();

            m_lightRenderTarget->setColorTexture(0, renderTargetTexture,
                {
                    renderer::RenderTargetLoadOp::LoadOp_Load, renderer::RenderTargetStoreOp::StoreOp_Store, color::Color(0.0f)
                },
                {
                    renderer::TransitionOp::TransitionOp_ColorAttachment, renderer::TransitionOp::TransitionOp_ColorAttachment
                });

            m_lightRenderTarget->setDepthStencilTexture(depthStencilTexture,
                {
                    renderer::RenderTargetLoadOp::LoadOp_Load, renderer::RenderTargetStoreOp::StoreOp_Store, 0.0f
                },
                {
                    renderer::RenderTargetLoadOp::LoadOp_Load, renderer::RenderTargetStoreOp::StoreOp_Store, 0U
                },
                {
                    renderer::TransitionOp::TransitionOp_DepthStencilReadOnly, renderer::TransitionOp::TransitionOp_DepthStencilAttachment
                });

            cmdList->beginRenderTarget(*m_lightRenderTarget);
            cmdList->setViewport({ 0.f, 0.f, (f32)viewportState->viewportSize._x, (f32)viewportState->viewportSize._y });
            cmdList->setScissor({ 0.f, 0.f, (f32)viewportState->viewportSize._x, (f32)viewportState->viewportSize._y });

            for (u32 i = 0; i < scene.m_renderLists[toEnumType(scene::ScenePass::PunctualLights)].size(); ++i)
            {
                auto& entry = scene.m_renderLists[toEnumType(scene::ScenePass::PunctualLights)][i];
                const scene::LightNodeEntry& itemLight = *static_cast<const scene::LightNodeEntry*>(entry);
                const scene::Light& light = *static_cast<const scene::Light*>(itemLight.light);

                struct ModelBuffer
                {
                    math::Matrix4D modelMatrix;
                    math::Matrix4D prevModelMatrix;
                    math::Matrix4D normalMatrix;
                    math::float4   tintColour;
                    u64            objectID;
                    u64           _pad = 0;
                } constantBuffer;

                constantBuffer.modelMatrix = itemLight.object->getTransform().getMatrix();
                constantBuffer.prevModelMatrix = itemLight.object->getPrevTransform().getMatrix();
                constantBuffer.normalMatrix = constantBuffer.modelMatrix.getTransposed();
                constantBuffer.tintColour = math::float4{ 1.0, 1.0, 1.0, 1.0 };
                constantBuffer.objectID = 0;

                struct LightBuffer
                {
                    math::Vector3D position;
                    math::Vector3D direction;
                    math::float4   color;
                    math::float4   attenuation;
                    math::float4   property;
                    f32            intensity;
                    f32            temperature;
                    u32            lightType;
                    u32           _pad = 0;
                } lightBuffer;

                lightBuffer.position = itemLight.object->getTransform().getPosition();
                lightBuffer.direction = itemLight.object->getDirection();
                lightBuffer.color = light.getColor();
                lightBuffer.attenuation = light.getAttenuation();
                lightBuffer.intensity = light.getIntensity();
                lightBuffer.temperature = light.getTemperature();
                lightBuffer.lightType = 0;

                struct ShadowBuffer
                {
                    math::Matrix4D lightSpaceMatrix[6];
                    math::float2   clipNearFar;
                    math::float2   shadowResolution;
                    f32            shadowBaseBias;
                    u32            shadowSliceOffset;
                    u32            shadowFaceMask;
                    u32            shadowPCFMode;
                } shadowBuffer;

                shadowBuffer.clipNearFar = { 0.0f, 0.0f };
                shadowBuffer.shadowResolution = { (f32)scene.m_settings._shadowsParams._size._width, (f32)scene.m_settings._shadowsParams._size._height };
                shadowBuffer.shadowBaseBias = 0.0f;
                shadowBuffer.shadowSliceOffset = 0;
                shadowBuffer.shadowFaceMask = 0b0;
                shadowBuffer.shadowPCFMode = scene.m_settings._shadowsParams._PCF;

                if (i < k_maxPunctualShadowmapCount && shadowData && shadowData->_punctualLightsFlags[i])
                {
                    auto& [pointLightSpaceMatrix, lightPosition, plane, viewsMask] = shadowData->_punctualLightsData[i];
                    memcpy(shadowBuffer.lightSpaceMatrix, pointLightSpaceMatrix.data(), sizeof(math::Matrix4D) * 6);
                    shadowBuffer.clipNearFar = plane;
                    shadowBuffer.shadowBaseBias = scene.m_settings._shadowsParams._punctualLightBias;
                    shadowBuffer.shadowSliceOffset = i * 6;
                    shadowBuffer.shadowFaceMask = viewsMask;
                }

                static auto fillLightByType = [](const scene::Light& light, LightBuffer& lightBuffer, ShadowBuffer& shadowBuffer) -> void
                    {
                        if (light.getType() == typeOf<scene::PointLight>())
                        {
                            const scene::PointLight& pLight = *static_cast<const scene::PointLight*>(&light);
                            lightBuffer.lightType = 1;
                        }
                        else if (light.getType() == typeOf<scene::SpotLight>())
                        {
                            const scene::SpotLight& sLight = *static_cast<const scene::SpotLight*>(&light);
                            lightBuffer.lightType = 2;
                            lightBuffer.property._x = sLight.getOuterAngle();
                            lightBuffer.property._y = sLight.getInnerAngle();
                        }
                    };

                fillLightByType(light, lightBuffer, shadowBuffer);

                DEBUG_MARKER_SCOPE(cmdList, std::format("Light {}", light.getName()), color::rgbaf::LTGREY);
                for (u32 pass = 0; pass < 2; ++pass)
                {
                    cmdList->setPipelineState(*m_pipeline[pass]);
                    cmdList->bindDescriptorSet(m_pipeline[pass]->getShaderProgram(), 0,
                        {
                            renderer::Descriptor(renderer::Descriptor::ConstantBuffer{ viewportState, 0, sizeof(scene::ViewportState)}, m_parameters.cb_Viewport)
                        });

                    cmdList->bindDescriptorSet(m_pipeline[pass]->getShaderProgram(), 1,
                        {
                            renderer::Descriptor(renderer::Descriptor::ConstantBuffer{ &constantBuffer, 0, sizeof(constantBuffer)}, m_parameters.cb_Model),
                            renderer::Descriptor(renderer::Descriptor::ConstantBuffer{ &lightBuffer, 0, sizeof(lightBuffer)}, m_parameters.cb_Light),
                            renderer::Descriptor(samplerState, m_parameters.s_SamplerState),
                            renderer::Descriptor(renderer::TextureView(gbufferAlbedoTexture, 0, 0), m_parameters.t_TextureBaseColor),
                            renderer::Descriptor(renderer::TextureView(gbufferNormalsTexture, 0, 0), m_parameters.t_TextureNormal),
                            renderer::Descriptor(renderer::TextureView(gbufferMaterialTexture, 0, 0), m_parameters.t_TextureMaterial),
                            renderer::Descriptor(renderer::TextureView(depthStencilTexture), m_parameters.t_TextureDepth),
                        });

                    cmdList->bindDescriptorSet(m_pipeline[pass]->getShaderProgram(), 2,
                        {
                            renderer::Descriptor(renderer::Descriptor::ConstantBuffer{ &shadowBuffer, 0, sizeof(shadowBuffer)}, m_parameters.cb_Shadow),
                            renderer::Descriptor(renderer::TextureView(shadowmapsTexture), m_parameters.t_TextureShadowmaps),
                            renderer::Descriptor(m_shadowSamplerState, m_parameters.s_ShadowSamplerState),
                        });

                    if (light.getType() == typeOf<scene::PointLight>())
                    {
                        renderer::GeometryBufferDesc desc(m_sphereVolume->getIndexBuffer(), 0, m_sphereVolume->getVertexBuffer(0), sizeof(VertexFormatSimpleLit), 0);
                        cmdList->drawIndexed(desc, 0, m_sphereVolume->getIndexBuffer()->getIndicesCount(), 0, 0, 1);
                    }
                    else if (light.getType() == typeOf<scene::SpotLight>())
                    {
                        renderer::GeometryBufferDesc desc(m_coneVolume->getIndexBuffer(), 0, m_coneVolume->getVertexBuffer(0), sizeof(VertexFormatSimpleLit), 0);
                        cmdList->drawIndexed(desc, 0, m_coneVolume->getIndexBuffer()->getIndicesCount(), 0, 0, 1);
                    }
                }

                cmdList->clear(depthStencilTexture, 0.f, 0u);
            }

            cmdList->endRenderTarget();
        };

    addRenderJob("VolumeLights Job", renderJob, device, scene, true);
}

void RenderPipelineLightAccumulationStage::onChanged(renderer::Device* device, scene::SceneData& scene, const event::GameEvent* event)
{
    if (event->_eventType == event::GameEvent::GameEventType::HotReload)
    {
        const event::ShaderHotReload* hotReloadEvent = static_cast<const event::ShaderHotReload*>(event);
        destroyPipelines(device, scene);
    }
}

void RenderPipelineLightAccumulationStage::createPipelines(renderer::Device* device, scene::SceneData& scene)
{
    m_debugPunctualLightShadows = scene.m_settings._shadowsParams._debugPunctualLightShadows;

    //Stencil pass
    {
        const renderer::VertexShader* vertShader = resource::ResourceManager::getInstance()->loadShader<renderer::VertexShader, resource::ShaderSourceFileLoader>(
            "light.hlsl", "main_vs", {}, {});
        const renderer::FragmentShader* fragShader = resource::ResourceManager::getInstance()->loadShader<renderer::FragmentShader, resource::ShaderSourceFileLoader>(
            "light.hlsl", "light_stencil_ps", {}, {});

        renderer::RenderPassDesc desc(scene.m_settings._vewportParams._colorFormat, scene.m_settings._vewportParams._depthFormat);
        m_pipeline[0] = V3D_NEW(renderer::GraphicsPipelineState, memory::MemoryLabel::MemoryGame)(device, VertexFormatSimpleLitDesc, desc,
            V3D_NEW(renderer::ShaderProgram, memory::MemoryLabel::MemoryGame)(device, vertShader, fragShader), "lighting_stencil");

        m_pipeline[0]->setPrimitiveTopology(renderer::PrimitiveTopology::PrimitiveTopology_TriangleList);
        m_pipeline[0]->setFrontFace(renderer::FrontFace::FrontFace_Clockwise);
        m_pipeline[0]->setCullMode(renderer::CullMode::CullMode_None);
        m_pipeline[0]->setPolygonMode(renderer::PolygonMode::PolygonMode_Fill);
        m_pipeline[0]->setDepthTest(true);
        m_pipeline[0]->setDepthWrite(false);
        m_pipeline[0]->setBlendEnable(0, true);
        m_pipeline[0]->setColorMask(0, renderer::ColorMask::ColorMask_None);
        m_pipeline[0]->setStencilTest(true);
        m_pipeline[0]->setStencilWriteMask(0xFF);
#if REVERSED_DEPTH
        m_pipeline[0]->setDepthCompareOp(renderer::CompareOperation::GreaterOrEqual);
        m_pipeline[0]->setStencilFrontFaceOp(renderer::StencilOperation::Keep, renderer::StencilOperation::Keep, renderer::StencilOperation::Increment_Wrap);
        m_pipeline[0]->setStencilFrontFaceCompareOp(renderer::CompareOperation::Always, 0xFF);
        m_pipeline[0]->setStencilBackFaceOp(renderer::StencilOperation::Keep, renderer::StencilOperation::Keep, renderer::StencilOperation::Decrement_Wrap);
        m_pipeline[0]->setStencilBackFaceCompareOp(renderer::CompareOperation::Always, 0xFF);
#else
        m_pipeline[0]->setDepthCompareOp(renderer::CompareOperation::LessOrEqual);
#endif
    }

    //Light pass
    {
        const renderer::Shader::DefineList defines =
        {
            { "DEBUG_PUNCTUAL_SHADOWMAPS", std::to_string(m_debugPunctualLightShadows) },
        };

        const renderer::VertexShader* vertShader = resource::ResourceManager::getInstance()->loadShader<renderer::VertexShader, resource::ShaderSourceFileLoader>(
            "light.hlsl", "main_vs", {}, {});
        const renderer::FragmentShader* fragShader = resource::ResourceManager::getInstance()->loadShader<renderer::FragmentShader, resource::ShaderSourceFileLoader>(
            "light.hlsl", "light_accumulation_ps", defines, {}, resource::ShaderCompileFlag::ShaderCompile_ForceReload);

        renderer::RenderPassDesc desc(scene.m_settings._vewportParams._colorFormat, scene.m_settings._vewportParams._depthFormat);
        m_pipeline[1] = V3D_NEW(renderer::GraphicsPipelineState, memory::MemoryLabel::MemoryGame)(device, VertexFormatSimpleLitDesc, desc,
            V3D_NEW(renderer::ShaderProgram, memory::MemoryLabel::MemoryGame)(device, vertShader, fragShader), "lighting");

        m_pipeline[1]->setPrimitiveTopology(renderer::PrimitiveTopology::PrimitiveTopology_TriangleList);
        m_pipeline[1]->setFrontFace(renderer::FrontFace::FrontFace_Clockwise);
        m_pipeline[1]->setCullMode(renderer::CullMode::CullMode_Back);
        m_pipeline[1]->setPolygonMode(renderer::PolygonMode::PolygonMode_Fill);
#if REVERSED_DEPTH
        m_pipeline[1]->setDepthCompareOp(renderer::CompareOperation::GreaterOrEqual);
#else
        m_pipeline[1]->setDepthCompareOp(renderer::CompareOperation::LessOrEqual);
#endif
        m_pipeline[1]->setDepthTest(false);
        m_pipeline[1]->setDepthWrite(false);
        m_pipeline[1]->setStencilTest(true);
        m_pipeline[1]->setStencilWriteMask(0x0);
        m_pipeline[1]->setStencilFrontFaceOp(renderer::StencilOperation::Keep, renderer::StencilOperation::Keep, renderer::StencilOperation::Keep);
        m_pipeline[1]->setStencilFrontFaceCompareOp(renderer::CompareOperation::NotEqual, 0xFF);
        m_pipeline[1]->setStencilBackFaceOp(renderer::StencilOperation::Keep, renderer::StencilOperation::Keep, renderer::StencilOperation::Keep);
        m_pipeline[1]->setStencilBackFaceCompareOp(renderer::CompareOperation::NotEqual, 0x0FF);
        m_pipeline[1]->setBlendEnable(0, true);
        m_pipeline[1]->setColorMask(0, renderer::ColorMask::ColorMask_All);
        m_pipeline[1]->setColorBlendFactor(0, renderer::BlendFactor::BlendFactor_One, renderer::BlendFactor::BlendFactor_One);
        m_pipeline[1]->setColorBlendOp(0, renderer::BlendOperation::BlendOp_Add);

        BIND_SHADER_PARAMETER(m_pipeline[1], m_parameters, cb_Viewport);
        BIND_SHADER_PARAMETER(m_pipeline[1], m_parameters, cb_Model);
        BIND_SHADER_PARAMETER(m_pipeline[1], m_parameters, cb_Light);
        BIND_SHADER_PARAMETER(m_pipeline[1], m_parameters, cb_Shadow);
        BIND_SHADER_PARAMETER(m_pipeline[1], m_parameters, s_SamplerState);
        BIND_SHADER_PARAMETER(m_pipeline[1], m_parameters, t_TextureBaseColor);
        BIND_SHADER_PARAMETER(m_pipeline[1], m_parameters, t_TextureNormal);
        BIND_SHADER_PARAMETER(m_pipeline[1], m_parameters, t_TextureMaterial);
        BIND_SHADER_PARAMETER(m_pipeline[1], m_parameters, t_TextureDepth);
        BIND_SHADER_PARAMETER(m_pipeline[1], m_parameters, t_TextureShadowmaps);
        BIND_SHADER_PARAMETER(m_pipeline[1], m_parameters, s_ShadowSamplerState);
    }

    m_shadowSamplerState = V3D_NEW(renderer::SamplerState, memory::MemoryLabel::MemoryGame)(device, renderer::SamplerFilter::SamplerFilter_Bilinear, renderer::SamplerAnisotropic::SamplerAnisotropic_None);
    m_shadowSamplerState->setWrap(renderer::SamplerWrap::TextureWrap_ClampToBorder);
    m_shadowSamplerState->setEnableCompareOp(true);
    m_shadowSamplerState->setCompareOp(renderer::CompareOperation::LessOrEqual);
    m_shadowSamplerState->setBorderColor({ 0.0f, 0.0f, 0.0f, 0.0f });
}

void RenderPipelineLightAccumulationStage::destroyPipelines(renderer::Device* device, scene::SceneData& scene)
{
    {
        ASSERT(m_pipeline[0], "must be valid");
        const renderer::ShaderProgram* program = m_pipeline[0]->getShaderProgram();
        V3D_DELETE(program, memory::MemoryLabel::MemoryGame);

        V3D_DELETE(m_pipeline[0], memory::MemoryLabel::MemoryGame);
    }

    {
        ASSERT(m_pipeline[1], "must be valid");
        const renderer::ShaderProgram* program = m_pipeline[1]->getShaderProgram();
        V3D_DELETE(program, memory::MemoryLabel::MemoryGame);

        V3D_DELETE(m_pipeline[1], memory::MemoryLabel::MemoryGame);
    }
}

void RenderPipelineLightAccumulationStage::createRenderTarget(renderer::Device* device, scene::SceneData& scene)
{
    ASSERT(m_lightRenderTarget == nullptr, "must be nullptr");
    m_lightRenderTarget = V3D_NEW(renderer::RenderTargetState, memory::MemoryLabel::MemoryGame)(device, scene.m_viewportSize, 1);
}

void RenderPipelineLightAccumulationStage::destroyRenderTarget(renderer::Device* device, scene::SceneData& scene)
{
    ASSERT(m_lightRenderTarget, "must be valid");
    V3D_DELETE(m_lightRenderTarget, memory::MemoryLabel::MemoryGame);
    m_lightRenderTarget = nullptr;
}

} // namespace scene
} // namespace v3d