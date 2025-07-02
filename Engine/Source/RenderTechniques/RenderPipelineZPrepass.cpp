#include "RenderPipelineZPrepass.h"

#include "Resource/ResourceManager.h"

#include "Resource/Loader/AssetSourceFileLoader.h"
#include "Resource/Loader/ShaderSourceFileLoader.h"
#include "Resource/Loader/ModelFileLoader.h"

#include "Scene/ModelHandler.h"

#include "Task/RenderTask/RenderTask.h"

namespace v3d
{
namespace renderer
{

RenderPipelineZPrepassStage::RenderPipelineZPrepassStage(RenderTechnique* technique, scene::ModelHandler* modelHandler) noexcept
    : RenderPipelineStage(technique, "ZPrepass")
    , m_modelHandler(modelHandler)

    , m_depthRenderTarget(nullptr)
    , m_depthPipeline(nullptr)
{
}

RenderPipelineZPrepassStage::~RenderPipelineZPrepassStage()
{
}

void RenderPipelineZPrepassStage::create(Device* device, scene::SceneData& scene, scene::FrameData& frame)
{
    createRenderTarget(device, scene);

    const renderer::VertexShader* vertShader = resource::ResourceManager::getInstance()->loadShader<renderer::VertexShader, resource::ShaderSourceFileLoader>(device,
        "gbuffer.hlsl", "gbuffer_standard_vs", {}, {}/*, resource::ShaderCompileFlag::ShaderCompile_UseDXCompilerForSpirV*/);
    const renderer::FragmentShader* fragShader = resource::ResourceManager::getInstance()->loadShader<renderer::FragmentShader, resource::ShaderSourceFileLoader>(device,
        "gbuffer.hlsl", "gbuffer_depth_ps", {}, {}/*, resource::ShaderCompileFlag::ShaderCompile_UseDXCompilerForSpirV*/);

    m_depthPipeline = new renderer::GraphicsPipelineState(
        device, VertexFormatStandardDesc, m_depthRenderTarget->getRenderPassDesc(), new renderer::ShaderProgram(device, vertShader, fragShader), "prepass_pipeline");

   m_depthPipeline->setPrimitiveTopology(renderer::PrimitiveTopology::PrimitiveTopology_TriangleList);
   m_depthPipeline->setFrontFace(renderer::FrontFace::FrontFace_Clockwise);
   m_depthPipeline->setCullMode(renderer::CullMode::CullMode_Back);
#if ENABLE_REVERSED_Z
   m_depthPipeline->setDepthCompareOp(renderer::CompareOperation::CompareOp_GreaterOrEqual);
#else
   m_depthPipeline->setDepthCompareOp(renderer::CompareOperation::CompareOp_LessOrEqual);
#endif
    m_depthPipeline->setDepthWrite(true);
    m_depthPipeline->setDepthTest(true);
    m_depthPipeline->setColorMask(0, renderer::ColorMask::ColorMask_All);
}

void RenderPipelineZPrepassStage::destroy(Device* device, scene::SceneData& scene, scene::FrameData& frame)
{
    destroyRenderTarget(device, scene);

    const renderer::ShaderProgram* program = m_depthPipeline->getShaderProgram();
    delete program;

    delete m_depthPipeline;
    m_depthPipeline = nullptr;
}

void RenderPipelineZPrepassStage::prepare(Device* device, scene::SceneData& scene, scene::FrameData& frame)
{
    if (!m_depthRenderTarget)
    {
        createRenderTarget(device, scene);
    }
    else if (m_depthRenderTarget->getRenderArea() != scene.m_viewportState._viewpotSize)
    {
        destroyRenderTarget(device, scene);
        createRenderTarget(device, scene);
    }
}

void RenderPipelineZPrepassStage::execute(Device* device, scene::SceneData& scene, scene::FrameData& frame)
{
    renderer::CmdListRender* cmdList = scene.m_renderState.m_cmdList;
    scene::ViewportState& viewportState = scene.m_viewportState;

    DEBUG_MARKER_SCOPE(cmdList, "ZPrepass", color::colorrgbaf::GREEN);

    cmdList->beginRenderTarget(*m_depthRenderTarget);
    cmdList->setViewport({ 0.f, 0.f, (f32)viewportState._viewpotSize._width, (f32)viewportState._viewpotSize._height });
    cmdList->setScissor({ 0.f, 0.f, (f32)viewportState._viewpotSize._width, (f32)viewportState._viewpotSize._height });
    cmdList->setStencilRef(0);

    cmdList->setPipelineState(*m_depthPipeline);

    cmdList->bindDescriptorSet(0,
        {
            renderer::Descriptor(renderer::Descriptor::ConstantBuffer{ &viewportState._viewportBuffer, 0, sizeof(viewportState._viewportBuffer)}, 0)
        });

    for (auto& list : scene.m_lists[toEnumType(scene::MaterialType::Opaque)])
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

        DEBUG_MARKER_SCOPE(cmdList, std::format("Object {}, pipeline {}", list->_objectID, m_depthPipeline->getName()), color::colorrgbaf::LTGREY);
        renderer::GeometryBufferDesc desc(list->_geometry._idxBuffer, 0, list->_geometry._vtxBuffer, 0, sizeof(VertexFormatStandard), 0);
        cmdList->drawIndexed(desc, 0, list->_geometry._idxBuffer->getIndicesCount(), 0, 0, 1);
    }

    cmdList->endRenderTarget();
}

void RenderPipelineZPrepassStage::createRenderTarget(Device* device, scene::SceneData& scene)
{
#if ENABLE_REVERSED_Z
    f32 clearValue = 0.0f;
#else
    f32 clearValue = 1.0f;
#endif

    ASSERT(m_depthRenderTarget == nullptr, "must be nullptr");
    m_depthRenderTarget = new renderer::RenderTargetState(device, scene.m_viewportState._viewpotSize, 0, 0, "zprepass");

    renderer::Texture2D* depthStencilAttachment = new renderer::Texture2D(device, renderer::TextureUsage::TextureUsage_Attachment | renderer::TextureUsage::TextureUsage_Sampled,
        renderer::Format::Format_D32_SFloat, scene.m_viewportState._viewpotSize, renderer::TextureSamples::TextureSamples_x1, "depth_stencil");
    m_depthRenderTarget->setDepthStencilTexture(depthStencilAttachment,
        {
            renderer::RenderTargetLoadOp::LoadOp_Clear, renderer::RenderTargetStoreOp::StoreOp_Store, clearValue
        },
        {
            renderer::RenderTargetLoadOp::LoadOp_Clear, renderer::RenderTargetStoreOp::StoreOp_Store, 0U
        },
        {
            renderer::TransitionOp::TransitionOp_Undefined, renderer::TransitionOp::TransitionOp_DepthStencilAttachment
        }
    );

    scene.m_globalResources.bind("depth_stencil", depthStencilAttachment);
}

void RenderPipelineZPrepassStage::destroyRenderTarget(Device* device, scene::SceneData& scene)
{
    ASSERT(m_depthRenderTarget, "must be valid");
    renderer::Texture2D* depthStencilAttachment = m_depthRenderTarget->getDepthStencilTexture<renderer::Texture2D>();
    delete depthStencilAttachment;

    delete m_depthRenderTarget;
    m_depthRenderTarget = nullptr;
}

} //namespace renderer
} //namespace v3d