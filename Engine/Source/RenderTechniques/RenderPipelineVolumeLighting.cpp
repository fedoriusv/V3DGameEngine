#include "RenderPipelineVolumeLighting.h"

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
//    createRenderTarget(device, scene);
//
//    {
//        const renderer::VertexShader* vertShader = resource::ResourceManager::getInstance()->loadShader<renderer::VertexShader, resource::ShaderSourceFileLoader>(device,
//            "simple.hlsl", "simple_vs", {}, {}, resource::ShaderCompileFlag::ShaderCompile_UseDXCompilerForSpirV);
//        const renderer::FragmentShader* fragShader = resource::ResourceManager::getInstance()->loadShader<renderer::FragmentShader, resource::ShaderSourceFileLoader>(device,
//            "simple.hlsl", "light_primitive_ps", {}, {}, resource::ShaderCompileFlag::ShaderCompile_UseDXCompilerForSpirV);
//
//        renderer::GraphicsPipelineState* pipeline = new renderer::GraphicsPipelineState(device, VertexFormatSimpleDesc, m_lightRenderTarget->getRenderPassDesc(),
//            new renderer::ShaderProgram(device, vertShader, fragShader), "lighting");
//
//        pipeline->setPrimitiveTopology(renderer::PrimitiveTopology::PrimitiveTopology_TriangleList);
//        pipeline->setFrontFace(renderer::FrontFace::FrontFace_Clockwise);
//        pipeline->setCullMode(renderer::CullMode::CullMode_Back);
//        pipeline->setPolygonMode(renderer::PolygonMode::PolygonMode_Fill);
//#if ENABLE_REVERSED_Z
//        pipeline->setDepthCompareOp(renderer::CompareOperation::CompareOp_GreaterOrEqual);
//#else
//        pipeline->setDepthCompareOp(renderer::CompareOperation::CompareOp_LessOrEqual);
//#endif
//        pipeline->setDepthTest(true);
//        pipeline->setDepthWrite(false);
//        pipeline->setColorMask(0, renderer::ColorMask::ColorMask_All);
//        pipeline->setColorBlendFactor(0, renderer::BlendFactor::BlendFactor_One, renderer::BlendFactor::BlendFactor_One);
//        pipeline->setColorBlendOp(0, renderer::BlendOperation::BlendOp_Add);
//
//        m_pipelines.push_back(pipeline);
//    }
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