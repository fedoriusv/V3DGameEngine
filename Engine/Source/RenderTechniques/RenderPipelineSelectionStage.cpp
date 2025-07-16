#include "RenderPipelineSelectionStage.h"

#include "Resource/ResourceManager.h"
#include "Resource/Loader/AssetSourceFileLoader.h"
#include "Resource/Loader/ShaderSourceFileLoader.h"
#include "Resource/Loader/ModelFileLoader.h"

namespace v3d
{
namespace renderer
{

RenderPipelineSelectionStage::RenderPipelineSelectionStage(RenderTechnique* technique, scene::ModelHandler* modelHandler) noexcept
    : RenderPipelineStage(technique, "Selection")
    , m_modelHandler(modelHandler)

    , m_renderTarget(nullptr)
    , m_pipeline(nullptr)
{
}

RenderPipelineSelectionStage::~RenderPipelineSelectionStage()
{
}

void RenderPipelineSelectionStage::create(Device* device, scene::SceneData& scene, scene::FrameData& frame)
{
    createRenderTarget(device, scene);

    const renderer::VertexShader* vertShader = resource::ResourceManager::getInstance()->loadShader<renderer::VertexShader, resource::ShaderSourceFileLoader>(device,
        "gbuffer.hlsl", "gbuffer_standard_vs", {}, {}, resource::ShaderCompileFlag::ShaderCompile_UseDXCompilerForSpirV);
    const renderer::FragmentShader* fragShader = resource::ResourceManager::getInstance()->loadShader<renderer::FragmentShader, resource::ShaderSourceFileLoader>(device,
        "gbuffer.hlsl", "gbuffer_selection_ps", {}, {}, resource::ShaderCompileFlag::ShaderCompile_UseDXCompilerForSpirV);

    m_pipeline = new renderer::GraphicsPipelineState(
        device, VertexFormatStandardDesc, m_renderTarget->getRenderPassDesc(), new renderer::ShaderProgram(device, vertShader, fragShader), "selection_pipeline");

    m_pipeline->setPrimitiveTopology(renderer::PrimitiveTopology::PrimitiveTopology_TriangleList);
    m_pipeline->setFrontFace(renderer::FrontFace::FrontFace_Clockwise);
    m_pipeline->setCullMode(renderer::CullMode::CullMode_Back);
#if ENABLE_REVERSED_Z
    m_pipeline->setDepthCompareOp(renderer::CompareOperation::CompareOp_GreaterOrEqual);
#else
    m_pipeline->setDepthCompareOp(renderer::CompareOperation::CompareOp_LessOrEqual);
#endif
    m_pipeline->setDepthWrite(true);
    m_pipeline->setDepthTest(true);
    m_pipeline->setColorMask(0, renderer::ColorMask::ColorMask_All);
}

void RenderPipelineSelectionStage::destroy(Device* device, scene::SceneData& scene, scene::FrameData& frame)
{
    destroyRenderTarget(device, scene);

    const renderer::ShaderProgram* program = m_pipeline->getShaderProgram();
    delete program;

    delete m_pipeline;
    m_pipeline = nullptr;
}

void RenderPipelineSelectionStage::prepare(Device* device, scene::SceneData& scene, scene::FrameData& frame)
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

void RenderPipelineSelectionStage::execute(Device* device, scene::SceneData& scene, scene::FrameData& frame)
{
    renderer::CmdListRender* cmdList = scene.m_renderState.m_cmdList;
    scene::ViewportState& viewportState = scene.m_viewportState;

    DEBUG_MARKER_SCOPE(cmdList, "Selection", color::colorrgbaf::GREEN);

    if (scene.m_lists[toEnumType(scene::MaterialType::Selected)].empty())
    {
        cmdList->clear(m_renderTarget->getColorTexture<renderer::Texture2D>(0), { 0.f, 0.f,  0.f,  0.f });
        return;
    }

    cmdList->beginRenderTarget(*m_renderTarget);
    cmdList->setViewport({ 0.f, 0.f, (f32)viewportState._viewpotSize._width, (f32)viewportState._viewpotSize._height });
    cmdList->setScissor({ 0.f, 0.f, (f32)viewportState._viewpotSize._width, (f32)viewportState._viewpotSize._height });
    cmdList->setStencilRef(0);
    cmdList->setPipelineState(*m_pipeline);

    cmdList->bindDescriptorSet(0,
        {
            renderer::Descriptor(renderer::Descriptor::ConstantBuffer{ &viewportState._viewportBuffer, 0, sizeof(viewportState._viewportBuffer)}, 0)
        });

    for (auto& list : scene.m_lists[toEnumType(scene::MaterialType::Selected)])
    {
        struct ModelBuffer
        {
            math::Matrix4D modelMatrix;
            math::Matrix4D prevModelMatrix;
            math::Matrix4D normalMatrix;
            math::float4   tint;
            u64            objectID;
            u64            _pad = 0;
        };
        ModelBuffer constantBuffer;
        constantBuffer.modelMatrix = list->_transform.getTransform();
        constantBuffer.prevModelMatrix = list->_prevTransform.getTransform();
        constantBuffer.normalMatrix = constantBuffer.modelMatrix.getTransposed();
        constantBuffer.tint = list->_material._tint;
        constantBuffer.objectID = list->_objectID;

        cmdList->bindDescriptorSet(1,
            {
                renderer::Descriptor(renderer::Descriptor::ConstantBuffer{ &constantBuffer, 0, sizeof(constantBuffer)}, 1),
            });

        DEBUG_MARKER_SCOPE(cmdList, std::format("Object {}, pipeline {}", list->_objectID, m_pipeline->getName()), color::colorrgbaf::LTGREY);
        renderer::GeometryBufferDesc desc(list->_geometry._idxBuffer, 0, list->_geometry._vtxBuffer, 0, sizeof(VertexFormatStandard), 0);
        cmdList->drawIndexed(desc, 0, list->_geometry._idxBuffer->getIndicesCount(), 0, 0, 1);
    }

    cmdList->endRenderTarget();
}

void RenderPipelineSelectionStage::createRenderTarget(Device* device, scene::SceneData& data)
{
    ASSERT(m_renderTarget == nullptr, "must be nullptr");
    m_renderTarget = new renderer::RenderTargetState(device, data.m_viewportState._viewpotSize, 1);

    m_renderTarget->setColorTexture(0, new renderer::Texture2D(device, renderer::TextureUsage::TextureUsage_Attachment | renderer::TextureUsage::TextureUsage_Sampled | renderer::TextureUsage::TextureUsage_Write,
        renderer::Format::Format_R32_SFloat, data.m_viewportState._viewpotSize, renderer::TextureSamples::TextureSamples_x1, "selected_objects"),
        {
            renderer::RenderTargetLoadOp::LoadOp_Clear, renderer::RenderTargetStoreOp::StoreOp_Store, color::Color(0.0f, 0.0f, 0.0f, 1.0f)
        },
        {
            renderer::TransitionOp::TransitionOp_Undefined, renderer::TransitionOp::TransitionOp_ShaderRead
        }
    );

    data.m_globalResources.bind("selected_objects", m_renderTarget->getColorTexture<renderer::Texture2D>(0));
}

void RenderPipelineSelectionStage::destroyRenderTarget(Device* device, scene::SceneData& data)
{
    ASSERT(m_renderTarget, "must be valid");
    renderer::Texture2D* texture = m_renderTarget->getColorTexture<renderer::Texture2D>(0);
    delete texture;

    delete m_renderTarget;
    m_renderTarget = nullptr;
}

} // namespace renderer
} // namespace v3d