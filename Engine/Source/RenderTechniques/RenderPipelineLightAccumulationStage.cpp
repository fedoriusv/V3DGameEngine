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
    , m_pipeline(nullptr)
    , m_lightRenderTarget(nullptr)

    , m_debugPunctualLightShadows(false)
{
}

RenderPipelineLightAccumulationStage::~RenderPipelineLightAccumulationStage()
{
}

void RenderPipelineLightAccumulationStage::create(renderer::Device* device, scene::SceneData& scene, scene::FrameData& frame)
{
    createRenderTarget(device, scene);
    createPipelines(device, scene);

    m_sphereVolume = scene::MeshHelper::createSphere(device, 1.f, 32, 32, "pointLight");
    m_coneVolume = scene::MeshHelper::createCone(device, 1.f, 1.f, 32, "spotLight");
}

void RenderPipelineLightAccumulationStage::destroy(renderer::Device* device, scene::SceneData& scene, scene::FrameData& frame)
{
    destroyRenderTarget(device, scene);
    destroyPipelines(device, scene);

    V3D_DELETE(m_sphereVolume, memory::MemoryLabel::MemoryObject);
    V3D_DELETE(m_coneVolume, memory::MemoryLabel::MemoryObject);
}

void RenderPipelineLightAccumulationStage::prepare(renderer::Device* device, scene::SceneData& scene, scene::FrameData& frame)
{
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
            ASSERT(shadowData_handle.isValid(), "must be valid");
            RenderPipelineShadowStage::PipelineData* shadowData = shadowData_handle.as<RenderPipelineShadowStage::PipelineData>();

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

            ObjectHandle samplerState_handle = scene.m_globalResources.get("linear_sampler_clamp");
            ASSERT(samplerState_handle.isValid(), "must be valid");
            renderer::SamplerState* samplerState = samplerState_handle.as<renderer::SamplerState>();

            ObjectHandle shadowmaps_handle = scene.m_globalResources.get("shadowmaps_array");
            if (!shadowmaps_handle.isValid())
            {
                shadowmaps_handle = scene.m_globalResources.get("default_array");
                ASSERT(shadowmaps_handle.isValid(), "must be valid");
            }
            renderer::Texture2D* shadowmapsTexture = shadowmaps_handle.as<renderer::Texture2D>();

            m_lightRenderTarget->setColorTexture(0, renderTargetTexture,
                {
                    renderer::RenderTargetLoadOp::LoadOp_Load, renderer::RenderTargetStoreOp::StoreOp_Store, color::Color(0.0f)
                },
                {
                    renderer::TransitionOp::TransitionOp_ColorAttachment, renderer::TransitionOp::TransitionOp_ColorAttachment
                });

            cmdList->beginRenderTarget(*m_lightRenderTarget);
            cmdList->setViewport({ 0.f, 0.f, (f32)viewportState->viewportSize._x, (f32)viewportState->viewportSize._y });
            cmdList->setScissor({ 0.f, 0.f, (f32)viewportState->viewportSize._x, (f32)viewportState->viewportSize._y });

            int i = 0;
            for (auto& entry : scene.m_renderLists[toEnumType(scene::RenderPipelinePass::PunctualLights)])
            {
                const scene::LightNodeEntry& itemLight = *static_cast<const scene::LightNodeEntry*>(entry);
                const scene::PointLight& light = *static_cast<const scene::PointLight*>(itemLight.light);

                cmdList->setPipelineState(*m_pipeline);
                cmdList->bindDescriptorSet(m_pipeline->getShaderProgram(), 0,
                    {
                        renderer::Descriptor(renderer::Descriptor::ConstantBuffer{ viewportState, 0, sizeof(scene::ViewportState)}, m_parameters[entry->pipelineID].cb_Viewport)
                    });

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

                struct PunctualLightBuffer
                {
                    math::Matrix4D lightSpaceMatrix[6];
                    math::float2   clipNearFar;
                    math::float2   viewSliceOffsetCount;
                    math::Vector3D position;
                    math::float4   color;
                    math::float4   attenuation;
                    f32            intensity;
                    f32            temperature;
                    f32            shadowBaseBias;
                    f32            applyShadow;
                } lightBuffer;


                auto& [pointLightSpaceMatrix, lightPosition, plane, viewsMask] = shadowData->_punctualLightsData[0];



                memcpy(lightBuffer.lightSpaceMatrix, pointLightSpaceMatrix.data(), sizeof(math::Matrix4D) * 6);
                lightBuffer.clipNearFar = plane;
                lightBuffer.viewSliceOffsetCount = { 0.0f, 6.0f };
                lightBuffer.position = itemLight.object->getTransform().getPosition();
                lightBuffer.color = light.getColor();
                lightBuffer.attenuation = light.getAttenuation();
                lightBuffer.intensity = light.getIntensity();
                lightBuffer.temperature = light.getTemperature();
                lightBuffer.shadowBaseBias = scene.m_settings._shadowsParams._punctualLightBias;
                lightBuffer.applyShadow = i == 0 ? true : false;
                ++i;

                cmdList->bindDescriptorSet(m_pipeline->getShaderProgram(), 1,
                    {
                        renderer::Descriptor(renderer::Descriptor::ConstantBuffer{ &constantBuffer, 0, sizeof(constantBuffer)}, m_parameters[entry->pipelineID].cb_Model),
                        renderer::Descriptor(renderer::Descriptor::ConstantBuffer{ &lightBuffer, 0, sizeof(lightBuffer)}, m_parameters[entry->pipelineID].cb_Light),
                        renderer::Descriptor(samplerState, m_parameters[entry->pipelineID].s_SamplerState),
                        renderer::Descriptor(renderer::TextureView(gbufferAlbedoTexture, 0, 0), m_parameters[entry->pipelineID].t_TextureBaseColor),
                        renderer::Descriptor(renderer::TextureView(gbufferNormalsTexture, 0, 0), m_parameters[entry->pipelineID].t_TextureNormal),
                        renderer::Descriptor(renderer::TextureView(gbufferMaterialTexture, 0, 0), m_parameters[entry->pipelineID].t_TextureMaterial),
                        renderer::Descriptor(renderer::TextureView(depthStencilTexture), m_parameters[entry->pipelineID].t_TextureDepth),
                        renderer::Descriptor(renderer::TextureView(shadowmapsTexture), m_parameters[entry->pipelineID].t_TextureShadowmaps),
                    });

                DEBUG_MARKER_SCOPE(cmdList, std::format("Light {}", light.getName()), color::rgbaf::LTGREY);
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

    {
        const renderer::Shader::DefineList defines =
        {
            { "DEBUG_PUNCTUAL_SHADOWMAPS", std::to_string(m_debugPunctualLightShadows) },
        };

        const renderer::VertexShader* vertShader = resource::ResourceManager::getInstance()->loadShader<renderer::VertexShader, resource::ShaderSourceFileLoader>(
            "light.hlsl", "main_vs", {}, {}, resource::ShaderCompileFlag::ShaderCompile_ForceReload);
        const renderer::FragmentShader* fragShader = resource::ResourceManager::getInstance()->loadShader<renderer::FragmentShader, resource::ShaderSourceFileLoader>(
            "light.hlsl", "light_accumulation_ps", defines, {}, resource::ShaderCompileFlag::ShaderCompile_ForceReload);

        renderer::RenderPassDesc desc{};
        desc._countColorAttachment = 1;
        desc._attachmentsDesc[0]._format = scene.m_settings._vewportParams._colorFormat;

        renderer::GraphicsPipelineState* pipeline = V3D_NEW(renderer::GraphicsPipelineState, memory::MemoryLabel::MemoryGame)(device, VertexFormatSimpleLitDesc, desc,
            V3D_NEW(renderer::ShaderProgram, memory::MemoryLabel::MemoryGame)(device, vertShader, fragShader), "lighting");

        pipeline->setPrimitiveTopology(renderer::PrimitiveTopology::PrimitiveTopology_TriangleList);
        pipeline->setFrontFace(renderer::FrontFace::FrontFace_Clockwise);
        pipeline->setCullMode(renderer::CullMode::CullMode_Back);
        pipeline->setPolygonMode(renderer::PolygonMode::PolygonMode_Fill);
#if REVERSED_DEPTH
        pipeline->setDepthCompareOp(renderer::CompareOperation::GreaterOrEqual);
#else
        pipeline->setDepthCompareOp(renderer::CompareOperation::LessOrEqual);
#endif
        pipeline->setDepthTest(false);
        pipeline->setDepthWrite(false);
        pipeline->setBlendEnable(0, true);
        pipeline->setColorMask(0, renderer::ColorMask::ColorMask_All);
        pipeline->setColorBlendFactor(0, renderer::BlendFactor::BlendFactor_One, renderer::BlendFactor::BlendFactor_One);
        pipeline->setColorBlendOp(0, renderer::BlendOperation::BlendOp_Add);

        MaterialParameters parameters;
        BIND_SHADER_PARAMETER(pipeline, parameters, cb_Viewport);
        BIND_SHADER_PARAMETER(pipeline, parameters, cb_Model);
        BIND_SHADER_PARAMETER(pipeline, parameters, cb_Light);
        BIND_SHADER_PARAMETER(pipeline, parameters, s_SamplerState);
        BIND_SHADER_PARAMETER(pipeline, parameters, t_TextureBaseColor);
        BIND_SHADER_PARAMETER(pipeline, parameters, t_TextureNormal);
        BIND_SHADER_PARAMETER(pipeline, parameters, t_TextureMaterial);
        BIND_SHADER_PARAMETER(pipeline, parameters, t_TextureDepth);
        BIND_SHADER_PARAMETER(pipeline, parameters, t_TextureShadowmaps);

        m_pipeline = pipeline;
        m_parameters.push_back(parameters);
    }
}

void RenderPipelineLightAccumulationStage::destroyPipelines(renderer::Device* device, scene::SceneData& scene)
{
    ASSERT(m_pipeline, "must be valid");
    const renderer::ShaderProgram* program = m_pipeline->getShaderProgram();
    V3D_DELETE(program, memory::MemoryLabel::MemoryGame);

    V3D_DELETE(m_pipeline, memory::MemoryLabel::MemoryGame);
    m_pipeline = nullptr;
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