#include "RenderPipelineVolumeLightingStage.h"

#include "Resource/ResourceManager.h"

#include "Resource/Loader/AssetSourceFileLoader.h"
#include "Resource/Loader/ShaderSourceFileLoader.h"
#include "Resource/Loader/ModelFileLoader.h"

namespace v3d
{
namespace renderer
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

void RenderPipelineVolumeLightingStage::create(Device* device, scene::SceneData& scene, scene::FrameData& frame)
{
    createRenderTarget(device, scene);

    {
        const renderer::VertexShader* vertShader = resource::ResourceManager::getInstance()->loadShader<renderer::VertexShader, resource::ShaderSourceFileLoader>(device,
            "simple.hlsl", "simple_vs", {}, {}, resource::ShaderCompileFlag::ShaderCompile_UseDXCompilerForSpirV);
        const renderer::FragmentShader* fragShader = resource::ResourceManager::getInstance()->loadShader<renderer::FragmentShader, resource::ShaderSourceFileLoader>(device,
            "simple.hlsl", "light_primitive_ps", {}, {}, resource::ShaderCompileFlag::ShaderCompile_UseDXCompilerForSpirV);

        renderer::GraphicsPipelineState* pipeline = new renderer::GraphicsPipelineState(device, VertexFormatSimpleDesc, m_lightRenderTarget->getRenderPassDesc(),
            new renderer::ShaderProgram(device, vertShader, fragShader), "lighting");

        pipeline->setPrimitiveTopology(renderer::PrimitiveTopology::PrimitiveTopology_TriangleList);
        pipeline->setFrontFace(renderer::FrontFace::FrontFace_Clockwise);
        pipeline->setCullMode(renderer::CullMode::CullMode_Back);
        pipeline->setPolygonMode(renderer::PolygonMode::PolygonMode_Fill);
#if ENABLE_REVERSED_Z
        pipeline->setDepthCompareOp(renderer::CompareOperation::CompareOp_GreaterOrEqual);
#else
        pipeline->setDepthCompareOp(renderer::CompareOperation::CompareOp_LessOrEqual);
#endif
        pipeline->setDepthTest(true);
        pipeline->setDepthWrite(false);
        pipeline->setColorMask(0, renderer::ColorMask::ColorMask_All);
        pipeline->setColorBlendFactor(0, renderer::BlendFactor::BlendFactor_One, renderer::BlendFactor::BlendFactor_One);
        pipeline->setColorBlendOp(0, renderer::BlendOperation::BlendOp_Add);

        m_pipelines.push_back(pipeline);
    }
}

void RenderPipelineVolumeLightingStage::destroy(Device* device, scene::SceneData& scene, scene::FrameData& frame)
{
    destroyRenderTarget(device, scene);

    for (auto pipeline : m_pipelines)
    {
        const renderer::ShaderProgram* program = pipeline->getShaderProgram();
        delete program;

        delete pipeline;
        pipeline = nullptr;
    }
    m_pipelines.clear();
}

void RenderPipelineVolumeLightingStage::prepare(Device* device, scene::SceneData& scene, scene::FrameData& frame)
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

void RenderPipelineVolumeLightingStage::execute(Device* device, scene::SceneData& scene, scene::FrameData& frame)
{
    renderer::CmdListRender* cmdList = scene.m_renderState.m_cmdList;
    scene::ViewportState& viewportState = scene.m_viewportState;

    DEBUG_MARKER_SCOPE(cmdList, "VolumeLight", color::colorrgbaf::GREEN);

    cmdList->beginRenderTarget(*m_lightRenderTarget);
    cmdList->setViewport({ 0.f, 0.f, (f32)viewportState._viewpotSize._width, (f32)viewportState._viewpotSize._height });
    cmdList->setScissor({ 0.f, 0.f, (f32)viewportState._viewpotSize._width, (f32)viewportState._viewpotSize._height });

    for (auto& list : scene.m_lists[toEnumType(scene::MaterialType::Lights)])
    {
        cmdList->setStencilRef(0);
        cmdList->setPipelineState(*m_pipelines[list->_pipelineID]);

        cmdList->bindDescriptorSet(0,
            {
                renderer::Descriptor(renderer::Descriptor::ConstantBuffer{ &viewportState._viewportBuffer, 0, sizeof(viewportState._viewportBuffer)}, 0)
            });

        struct ModelBuffer
        {
            math::Matrix4D modelMatrix;
            math::Matrix4D prevModelMatrix;
            math::Matrix4D normalMatrix;
            math::float4   tint;
            u64            objectID;
            u64           _pad = 0;
        };

        ModelBuffer modelBuffer;
        modelBuffer.modelMatrix = list->_transform.getTransform();
        modelBuffer.prevModelMatrix = list->_prevTransform.getTransform();
        modelBuffer.normalMatrix = modelBuffer.modelMatrix.getTransposed();
        modelBuffer.tint = list->_material._tint;
        modelBuffer.objectID = 0;

        ObjectHandle gbuffer_albedo_h = scene.m_globalResources.get("gbuffer_albedo");
        ASSERT(gbuffer_albedo_h.isValid(), "must be valid");
        renderer::Texture2D* gbuffer_albedo_texture = objectFromHandle<renderer::Texture2D>(gbuffer_albedo_h);

        ObjectHandle gbuffer_normals_h = scene.m_globalResources.get("gbuffer_normals");
        ASSERT(gbuffer_normals_h.isValid(), "must be valid");
        renderer::Texture2D* gbuffer_normals_texture = objectFromHandle<renderer::Texture2D>(gbuffer_normals_h);

        ObjectHandle gbuffer_material_h = scene.m_globalResources.get("gbuffer_material");
        ASSERT(gbuffer_material_h.isValid(), "must be valid");
        renderer::Texture2D* gbuffer_material_texture = objectFromHandle<renderer::Texture2D>(gbuffer_material_h);

        ObjectHandle sampler_state_h = scene.m_globalResources.get("linear_sampler_mirror");
        ASSERT(sampler_state_h.isValid(), "must be valid");
        renderer::SamplerState* sampler_state = objectFromHandle<renderer::SamplerState>(sampler_state_h);

        cmdList->bindDescriptorSet(1,
            {
                renderer::Descriptor(renderer::Descriptor::ConstantBuffer{ &modelBuffer, 0, sizeof(modelBuffer)}, 1),
                //TODO
            });

        struct LightBuffer
        {
            math::float4 position;
            math::float4 color;
            f32          type;
            f32          attenuation;
            f32          intensity;
            f32          temperature;
        };

        LightBuffer lightBuffer;

        cmdList->bindDescriptorSet(1,
            {
                renderer::Descriptor(renderer::Descriptor::ConstantBuffer{ &lightBuffer, 0, sizeof(lightBuffer)}, 2),
            });

        DEBUG_MARKER_SCOPE(cmdList, std::format("Primitive {}, pipeline {}", list->_objectID, m_pipelines[list->_pipelineID]->getName()), color::colorrgbaf::LTGREY);
        renderer::GeometryBufferDesc desc(list->_geometry._idxBuffer, 0, list->_geometry._vtxBuffer, 0, sizeof(VertexFormatSimple), 0);
        cmdList->drawIndexed(desc, 0, list->_geometry._idxBuffer->getIndicesCount(), 0, 0, 1);
    }

    cmdList->endRenderTarget();
}

void RenderPipelineVolumeLightingStage::createRenderTarget(Device* device, scene::SceneData& data)
{
    ASSERT(m_lightRenderTarget == nullptr, "must be nullptr");
    m_lightRenderTarget = new renderer::RenderTargetState(device, data.m_viewportState._viewpotSize, 1);
    m_lightRenderTarget->setColorTexture(0, new renderer::Texture2D(device, renderer::TextureUsage::TextureUsage_Attachment | renderer::TextureUsage::TextureUsage_Sampled | renderer::TextureUsage::TextureUsage_Write,
        renderer::Format::Format_R32G32B32A32_SFloat, data.m_viewportState._viewpotSize, renderer::TextureSamples::TextureSamples_x1, "lighting"),
        {
            renderer::RenderTargetLoadOp::LoadOp_Clear, renderer::RenderTargetStoreOp::StoreOp_Store, color::Color(0.0f, 0.0f, 0.0f, 1.0f)
        },
        {
            renderer::TransitionOp::TransitionOp_Undefined, renderer::TransitionOp::TransitionOp_ShaderRead
        }
    );
}

void RenderPipelineVolumeLightingStage::destroyRenderTarget(Device* device, scene::SceneData& data)
{
    ASSERT(m_lightRenderTarget, "must be valid");
    renderer::Texture2D* texture = m_lightRenderTarget->getColorTexture<renderer::Texture2D>(0);
    delete texture;

    delete m_lightRenderTarget;
    m_lightRenderTarget = nullptr;
}

} // namespace renderer
} // namespace v3d