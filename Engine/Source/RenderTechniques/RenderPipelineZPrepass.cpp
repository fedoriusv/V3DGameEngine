#include "RenderPipelineZPrepass.h"
#include "Utils/Logger.h"

#include "Resource/ResourceManager.h"
#include "Resource/Loader/AssetSourceFileLoader.h"
#include "Resource/Loader/ShaderSourceFileLoader.h"
#include "Resource/Loader/ModelFileLoader.h"

#include "Renderer/PipelineState.h"
#include "Renderer/ShaderProgram.h"

#include "Scene/ModelHandler.h"
#include "Scene/Geometry/Mesh.h"

#include "Task/RenderTask/RenderTask.h"

#include "FrameProfiler.h"

namespace v3d
{
namespace scene
{

#if REVERSED_DEPTH
    f32 k_clearValue = 0.0f;
#else
    f32 k_clearValue = 1.0f;
#endif

RenderPipelineZPrepassStage::RenderPipelineZPrepassStage(RenderTechnique* technique, scene::ModelHandler* modelHandler) noexcept
    : RenderPipelineStage(technique, "ZPrepass")
    , m_modelHandler(modelHandler)
    , m_cmdList(nullptr)

    , m_depthRenderTarget(nullptr)
    , m_depthPipeline(nullptr)
{
}

RenderPipelineZPrepassStage::~RenderPipelineZPrepassStage()
{
}

void RenderPipelineZPrepassStage::create(renderer::Device* device, scene::SceneData& scene, scene::FrameData& frame)
{
    m_cmdList = device->createCommandList<renderer::CmdListRender>(renderer::Device::GraphicMask);
    createRenderTarget(device, scene);

    const renderer::VertexShader* vertShader = resource::ResourceManager::getInstance()->loadShader<renderer::VertexShader, resource::ShaderSourceFileLoader>(device,
        "gbuffer.hlsl", "gbuffer_standard_vs", {}, {}, resource::ShaderCompileFlag::ShaderCompile_UseDXCompilerForSpirV);
    const renderer::FragmentShader* fragShader = resource::ResourceManager::getInstance()->loadShader<renderer::FragmentShader, resource::ShaderSourceFileLoader>(device,
        "gbuffer.hlsl", "gbuffer_depth_ps", {}, {}, resource::ShaderCompileFlag::ShaderCompile_UseDXCompilerForSpirV);

    m_depthPipeline = V3D_NEW(renderer::GraphicsPipelineState, memory::MemoryLabel::MemoryGame)(device, scene::VertexFormatStandardDesc, m_depthRenderTarget->getRenderPassDesc(),
        V3D_NEW(renderer::ShaderProgram, memory::MemoryLabel::MemoryGame)(device, vertShader, fragShader), "zprepass_pipeline");

   m_depthPipeline->setPrimitiveTopology(renderer::PrimitiveTopology::PrimitiveTopology_TriangleList);
   m_depthPipeline->setFrontFace(renderer::FrontFace::FrontFace_Clockwise);
   m_depthPipeline->setCullMode(renderer::CullMode::CullMode_None);
#if REVERSED_DEPTH
   m_depthPipeline->setDepthCompareOp(renderer::CompareOperation::GreaterOrEqual);
#else
   m_depthPipeline->setDepthCompareOp(renderer::CompareOperation::LessOrEqual);
#endif
    m_depthPipeline->setDepthWrite(true);
    m_depthPipeline->setDepthTest(true);
    m_depthPipeline->setColorMask(0, renderer::ColorMask::ColorMask_All);
    m_depthPipeline->setStencilTest(true);
    m_depthPipeline->setStencilCompareOp(renderer::CompareOperation::Always, 0xFF);
    m_depthPipeline->setStencilOp(renderer::StencilOperation::Replace, renderer::StencilOperation::Keep, renderer::StencilOperation::Keep);

    BIND_SHADER_PARAMETER(m_depthPipeline, m_depthParameters, cb_Viewport);
    BIND_SHADER_PARAMETER(m_depthPipeline, m_depthParameters, cb_Model);
}

void RenderPipelineZPrepassStage::destroy(renderer::Device* device, scene::SceneData& scene, scene::FrameData& frame)
{
    destroyRenderTarget(device, scene);

    const renderer::ShaderProgram* program = m_depthPipeline->getShaderProgram();
    V3D_DELETE(program, memory::MemoryLabel::MemoryGame);

    V3D_DELETE(m_depthPipeline, memory::MemoryLabel::MemoryGame);
    m_depthPipeline = nullptr;

    device->destroyCommandList(m_cmdList);
    m_cmdList = nullptr;
}

void RenderPipelineZPrepassStage::prepare(renderer::Device* device, scene::SceneData& scene, scene::FrameData& frame)
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

void RenderPipelineZPrepassStage::execute(renderer::Device* device, scene::SceneData& scene, scene::FrameData& frame)
{
    auto renderJob = [this, device](renderer::CmdListRender* cmdList, const scene::SceneData& scene, const scene::FrameData& frame) -> void
        {
            TRACE_PROFILER_SCOPE("ZPrepass", color::rgba8::GREEN);
            DEBUG_MARKER_SCOPE(cmdList, "ZPrepass", color::rgbaf::GREEN);

            const scene::ViewportState& viewportState = scene.m_viewportState;

            if (scene.m_renderLists[toEnumType(scene::RenderPipelinePass::Opaque)].empty())
            {
                cmdList->clear(m_depthRenderTarget->getDepthStencilTexture<renderer::Texture2D>(), k_clearValue, 0U);
            }
            else
            {
                cmdList->beginRenderTarget(*m_depthRenderTarget);
                cmdList->setViewport({ 0.f, 0.f, (f32)viewportState._viewpotSize._width, (f32)viewportState._viewpotSize._height });
                cmdList->setScissor({ 0.f, 0.f, (f32)viewportState._viewpotSize._width, (f32)viewportState._viewpotSize._height });
                cmdList->setPipelineState(*m_depthPipeline);

                cmdList->bindDescriptorSet(m_depthPipeline->getShaderProgram(), 0,
                    {
                        renderer::Descriptor(renderer::Descriptor::ConstantBuffer{ &viewportState._viewportBuffer, 0, sizeof(viewportState._viewportBuffer)}, m_depthParameters.cb_Viewport)
                    });

                for (auto& entry : scene.m_renderLists[toEnumType(scene::RenderPipelinePass::Opaque)])
                {
                    cmdList->setStencilRef(0x1);
                    const scene::DrawNodeEntry& itemMesh = *static_cast<scene::DrawNodeEntry*>(entry);

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
                    constantBuffer.modelMatrix = itemMesh.object->getTransform().getMatrix();
                    constantBuffer.prevModelMatrix = itemMesh.object->getPrevTransform().getMatrix();
                    constantBuffer.normalMatrix = constantBuffer.modelMatrix.getInversed();
                    constantBuffer.normalMatrix.makeTransposed();
                    constantBuffer.tintColour = math::float4{ 1.0, 1.0, 1.0, 1.0 };
                    constantBuffer.objectID = itemMesh.object->ID();

                    cmdList->bindDescriptorSet(m_depthPipeline->getShaderProgram(), 1,
                        {
                            renderer::Descriptor(renderer::Descriptor::ConstantBuffer{ &constantBuffer, 0, sizeof(constantBuffer) }, m_depthParameters.cb_Model),
                        });

                    DEBUG_MARKER_SCOPE(cmdList, std::format("Object [{}], pipeline [{}]", itemMesh.object->m_name, m_depthPipeline->getName()), color::rgbaf::LTGREY);
                    const scene::Mesh& mesh = *static_cast<scene::Mesh*>(itemMesh.geometry);
                    ASSERT(mesh.getVertexAttribDesc()._inputBindings[0]._stride == sizeof(scene::VertexFormatStandard), "must be same");
                    renderer::GeometryBufferDesc desc(mesh.getIndexBuffer(), 0, mesh.getVertexBuffer(0), 0, sizeof(scene::VertexFormatStandard), 0);
                    cmdList->drawIndexed(desc, 0, mesh.getIndexBuffer()->getIndicesCount(), 0, 0, 1);
                }

                cmdList->endRenderTarget();
            }
        };

    ASSERT(m_cmdList, "must be valid");
    addRenderJob("ZPrepass Job", renderJob, m_cmdList, scene, frame);
}

void RenderPipelineZPrepassStage::createRenderTarget(renderer::Device* device, scene::SceneData& scene)
{
    ASSERT(m_depthRenderTarget == nullptr, "must be nullptr");
    m_depthRenderTarget = V3D_NEW(renderer::RenderTargetState, memory::MemoryLabel::MemoryGame)(device, scene.m_viewportState._viewpotSize, 0, 0);

    renderer::Texture2D* depthStencilAttachment = V3D_NEW(renderer::Texture2D, memory::MemoryLabel::MemoryGame)(device, renderer::TextureUsage::TextureUsage_Attachment | renderer::TextureUsage::TextureUsage_Sampled,
        scene.m_settings._depthFormat, scene.m_viewportState._viewpotSize, renderer::TextureSamples::TextureSamples_x1, "depth_stencil");
    m_depthRenderTarget->setDepthStencilTexture(depthStencilAttachment,
        {
            renderer::RenderTargetLoadOp::LoadOp_Clear, renderer::RenderTargetStoreOp::StoreOp_Store, k_clearValue
        },
        {
            renderer::RenderTargetLoadOp::LoadOp_Clear, renderer::RenderTargetStoreOp::StoreOp_Store, 0U
        },
        {
            renderer::TransitionOp::TransitionOp_DepthStencilAttachment, renderer::TransitionOp::TransitionOp_DepthStencilAttachment
        });

    scene.m_globalResources.bind("depth_stencil", depthStencilAttachment);
}

void RenderPipelineZPrepassStage::destroyRenderTarget(renderer::Device* device, scene::SceneData& scene)
{
    ASSERT(m_depthRenderTarget, "must be valid");
    renderer::Texture2D* depthStencilAttachment = m_depthRenderTarget->getDepthStencilTexture<renderer::Texture2D>();
    V3D_DELETE(depthStencilAttachment, memory::MemoryLabel::MemoryGame);

    V3D_DELETE(m_depthRenderTarget, memory::MemoryLabel::MemoryGame);
    m_depthRenderTarget = nullptr;
}

} //namespace renderer
} //namespace v3d