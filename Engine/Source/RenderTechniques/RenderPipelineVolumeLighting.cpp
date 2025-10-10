#include "RenderPipelineVolumeLighting.h"
#include "Utils/Logger.h"

#include "Resource/ResourceManager.h"

#include "Resource/Loader/AssetSourceFileLoader.h"
#include "Resource/Loader/ShaderSourceFileLoader.h"
#include "Resource/Loader/ModelFileLoader.h"

#include "Scene/ModelHandler.h"
#include "Scene/Geometry/Mesh.h"

#include "FrameProfiler.h"

namespace v3d
{
namespace scene
{

RenderPipelineVolumeLightingStage::RenderPipelineVolumeLightingStage(RenderTechnique* technique, scene::ModelHandler* modelHandler) noexcept
    : RenderPipelineStage(technique, "VolumeLight")
    , m_modelHandler(modelHandler)
    , m_lightRenderTarget(nullptr)
{
}

RenderPipelineVolumeLightingStage::~RenderPipelineVolumeLightingStage()
{
}

void RenderPipelineVolumeLightingStage::create(renderer::Device* device, scene::SceneData& scene, scene::FrameData& frame)
{
    createRenderTarget(device, scene);

    {
        const renderer::Shader::DefineList defines =
        {
        };

        const renderer::VertexShader* vertShader = resource::ResourceManager::getInstance()->loadShader<renderer::VertexShader, resource::ShaderSourceFileLoader>(device,
            "light.hlsl", "main_vs", defines, {}, resource::ShaderCompileFlag::ShaderCompile_UseDXCompilerForSpirV);
        const renderer::FragmentShader* fragShader = resource::ResourceManager::getInstance()->loadShader<renderer::FragmentShader, resource::ShaderSourceFileLoader>(device,
            "light.hlsl", "light_volume_ps", defines, {}, resource::ShaderCompileFlag::ShaderCompile_UseDXCompilerForSpirV);

        renderer::RenderPassDesc desc{};
        desc._countColorAttachments = 1;
        desc._attachmentsDesc[0]._format = scene.m_settings._colorFormat;

        renderer::GraphicsPipelineState* pipeline = V3D_NEW(renderer::GraphicsPipelineState, memory::MemoryLabel::MemoryGame)(device, VertexFormatSimpleLitDesc, desc,
            V3D_NEW(renderer::ShaderProgram, memory::MemoryLabel::MemoryGame)(device, vertShader, fragShader), "lighting");

        pipeline->setPrimitiveTopology(renderer::PrimitiveTopology::PrimitiveTopology_TriangleList);
        pipeline->setFrontFace(renderer::FrontFace::FrontFace_Clockwise);
        pipeline->setCullMode(renderer::CullMode::CullMode_Back);
        pipeline->setPolygonMode(renderer::PolygonMode::PolygonMode_Fill);
#if ENABLE_REVERSED_Z
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

        m_pipelines.push_back(pipeline);
        m_parameters.push_back(parameters);
    }

    m_sphereVolume = scene::MeshHelper::createSphere(device, 1.f, 16, 16, "pointLight");
}

void RenderPipelineVolumeLightingStage::destroy(renderer::Device* device, scene::SceneData& scene, scene::FrameData& frame)
{
    destroyRenderTarget(device, scene);

    for (auto& pipeline : m_pipelines)
    {
        const renderer::ShaderProgram* program = pipeline->getShaderProgram();
        V3D_DELETE(program, memory::MemoryLabel::MemoryGame);

        V3D_DELETE(pipeline, memory::MemoryLabel::MemoryGame);
        pipeline = nullptr;
    }
    m_pipelines.clear();
}

void RenderPipelineVolumeLightingStage::prepare(renderer::Device* device, scene::SceneData& scene, scene::FrameData& frame)
{
    if (!m_lightRenderTarget)
    {
        createRenderTarget(device, scene);
    }
    else if (m_lightRenderTarget->getRenderArea() != scene.m_viewportState._viewpotSize)
    {
        destroyRenderTarget(device, scene);
        createRenderTarget(device, scene);
    }
}

void RenderPipelineVolumeLightingStage::execute(renderer::Device* device, scene::SceneData& scene, scene::FrameData& frame)
{
    renderer::CmdListRender* cmdList = frame.m_cmdList;
    scene::ViewportState& viewportState = scene.m_viewportState;

    TRACE_PROFILER_SCOPE("VolumeLight", color::rgba8::GREEN);
    DEBUG_MARKER_SCOPE(cmdList, "VolumeLight", color::rgbaf::GREEN);

    if (!scene.m_renderLists[toEnumType(scene::RenderPipelinePass::PunctualLights)].empty())
    {
        ObjectHandle rt_h = scene.m_globalResources.get("render_target");
        ASSERT(rt_h.isValid(), "must be valid");
        renderer::Texture2D* renderTargetTexture = objectFromHandle<renderer::Texture2D>(rt_h);

        ObjectHandle depth_stencil_h = scene.m_globalResources.get("depth_stencil");
        ASSERT(depth_stencil_h.isValid(), "must be valid");
        renderer::Texture2D* depthStencilTexture = objectFromHandle<renderer::Texture2D>(depth_stencil_h);

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

        m_lightRenderTarget->setColorTexture(0, renderTargetTexture,
            {
                renderer::RenderTargetLoadOp::LoadOp_Load, renderer::RenderTargetStoreOp::StoreOp_Store, color::Color(0.0f)
            },
            {
                renderer::TransitionOp::TransitionOp_ColorAttachment, renderer::TransitionOp::TransitionOp_ColorAttachment
            }
        );

        cmdList->transition(depthStencilTexture, renderer::TransitionOp::TransitionOp_ShaderRead);
        cmdList->beginRenderTarget(*m_lightRenderTarget);
        cmdList->setViewport({ 0.f, 0.f, (f32)viewportState._viewpotSize._width, (f32)viewportState._viewpotSize._height });
        cmdList->setScissor({ 0.f, 0.f, (f32)viewportState._viewpotSize._width, (f32)viewportState._viewpotSize._height });

        for (auto& entry : scene.m_renderLists[toEnumType(scene::RenderPipelinePass::PunctualLights)])
        {
            const scene::LightNodeEntry& itemLight = *static_cast<const scene::LightNodeEntry*>(entry);
            const scene::PointLight& light = *static_cast<const scene::PointLight*>(itemLight.light);

            cmdList->setPipelineState(*m_pipelines[entry->pipelineID]);

            cmdList->bindDescriptorSet(m_pipelines[entry->pipelineID]->getShaderProgram(), 0,
                {
                    renderer::Descriptor(renderer::Descriptor::ConstantBuffer{ &viewportState._viewportBuffer, 0, sizeof(viewportState._viewportBuffer)}, m_parameters[entry->pipelineID].cb_Viewport)
                });

            struct ModelBuffer
            {
                math::Matrix4D modelMatrix;
                math::Matrix4D prevModelMatrix;
                math::Matrix4D normalMatrix;
                math::float4   tintColour;
                u64            objectID;
                u64           _pad = 0;
            };

            ModelBuffer constantBuffer;
            constantBuffer.modelMatrix = itemLight.object->getTransform().getMatrix();
            constantBuffer.prevModelMatrix = itemLight.object->getPrevTransform().getMatrix();
            constantBuffer.normalMatrix = constantBuffer.modelMatrix.getTransposed();
            constantBuffer.tintColour = math::float4{1.0, 1.0, 1.0, 1.0};
            constantBuffer.objectID = 0;

            struct LightBuffer
            {
                math::Vector3D position;
                math::Vector3D range;
                math::float4   color;
                math::float4   attenuation;
                f32            intensity;
                f32            temperature;
                f32            type;
                f32           _pad = 0;
            };

            LightBuffer lightBuffer;
            lightBuffer.position = itemLight.object->getTransform().getPosition();
            lightBuffer.range = { light.getRadius(), 0.0, 0.0};
            lightBuffer.color = light.getColor();
            lightBuffer.attenuation = light.getAttenuation();
            lightBuffer.intensity = light.getIntensity();
            lightBuffer.temperature = light.getTemperature();
            lightBuffer.type = light.getType() == typeOf<scene::PointLight>() ? 1 : 2;

            cmdList->bindDescriptorSet(m_pipelines[entry->pipelineID]->getShaderProgram(), 1,
                {
                    renderer::Descriptor(renderer::Descriptor::ConstantBuffer{ &constantBuffer, 0, sizeof(constantBuffer)}, m_parameters[entry->pipelineID].cb_Model),
                    renderer::Descriptor(renderer::Descriptor::ConstantBuffer{ &lightBuffer, 0, sizeof(lightBuffer)}, m_parameters[entry->pipelineID].cb_Light),
                    renderer::Descriptor(samplerState, m_parameters[entry->pipelineID].s_SamplerState),
                    renderer::Descriptor(renderer::TextureView(gbufferAlbedoTexture, 0, 0), m_parameters[entry->pipelineID].t_TextureBaseColor),
                    renderer::Descriptor(renderer::TextureView(gbufferNormalsTexture, 0, 0), m_parameters[entry->pipelineID].t_TextureNormal),
                    renderer::Descriptor(renderer::TextureView(gbufferMaterialTexture, 0, 0), m_parameters[entry->pipelineID].t_TextureMaterial),
                    renderer::Descriptor(renderer::TextureView(depthStencilTexture), m_parameters[entry->pipelineID].t_TextureDepth),
                });

            DEBUG_MARKER_SCOPE(cmdList, std::format("Light {}", light.getName()), color::rgbaf::LTGREY);
            if (light.getType() == typeOf<scene::PointLight>())
            {
                renderer::GeometryBufferDesc desc(m_sphereVolume->getIndexBuffer(), 0, m_sphereVolume->getVertexBuffer(0), 0, sizeof(VertexFormatSimpleLit), 0);
                cmdList->drawIndexed(desc, 0, m_sphereVolume->getIndexBuffer()->getIndicesCount(), 0, 0, 1);
            }
            else if (light.getType() == typeOf<scene::SpotLight>())
            {
                //TODO
            }
        }

        cmdList->endRenderTarget();
        cmdList->transition(depthStencilTexture, renderer::TransitionOp::TransitionOp_DepthStencilAttachment);
    }
}

void RenderPipelineVolumeLightingStage::createRenderTarget(renderer::Device* device, scene::SceneData& data)
{
    ASSERT(m_lightRenderTarget == nullptr, "must be nullptr");
    m_lightRenderTarget = V3D_NEW(renderer::RenderTargetState, memory::MemoryLabel::MemoryGame)(device, data.m_viewportState._viewpotSize, 1);
}

void RenderPipelineVolumeLightingStage::destroyRenderTarget(renderer::Device* device, scene::SceneData& data)
{
    ASSERT(m_lightRenderTarget, "must be valid");
    V3D_DELETE(m_lightRenderTarget, memory::MemoryLabel::MemoryGame);
    m_lightRenderTarget = nullptr;
}

} // namespace scene
} // namespace v3d