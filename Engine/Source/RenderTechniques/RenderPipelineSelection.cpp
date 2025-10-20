#include "RenderPipelineSelection.h"
#include "Utils/Logger.h"

#include "Resource/ResourceManager.h"
#include "Resource/Loader/AssetSourceFileLoader.h"
#include "Resource/Loader/ShaderSourceFileLoader.h"
#include "Resource/Loader/ModelFileLoader.h"

#include "Renderer/PipelineState.h"
#include "Renderer/ShaderProgram.h"

#include "Scene/ModelHandler.h"
#include "Scene/Geometry/Mesh.h"
#include "Scene/Billboard.h"
#include "Scene/Material.h"

#include "FrameProfiler.h"

namespace v3d
{
namespace scene
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

void RenderPipelineSelectionStage::create(renderer::Device* device, scene::SceneData& scene, scene::FrameData& frame)
{
    createRenderTarget(device, scene);

    const renderer::VertexShader* vertShader = resource::ResourceManager::getInstance()->loadShader<renderer::VertexShader, resource::ShaderSourceFileLoader>(device,
        "gbuffer.hlsl", "gbuffer_standard_vs", {}, {}, resource::ShaderCompileFlag::ShaderCompile_UseDXCompilerForSpirV);
    const renderer::FragmentShader* fragShader = resource::ResourceManager::getInstance()->loadShader<renderer::FragmentShader, resource::ShaderSourceFileLoader>(device,
        "gbuffer.hlsl", "gbuffer_selection_ps", {}, {}, resource::ShaderCompileFlag::ShaderCompile_UseDXCompilerForSpirV);

    m_pipeline = V3D_NEW(renderer::GraphicsPipelineState, memory::MemoryLabel::MemoryGame)(device, VertexFormatStandardDesc, m_renderTarget->getRenderPassDesc(),
        V3D_NEW(renderer::ShaderProgram, memory::MemoryLabel::MemoryGame)(device, vertShader, fragShader), "selection_pipeline");

    m_pipeline->setPrimitiveTopology(renderer::PrimitiveTopology::PrimitiveTopology_TriangleList);
    m_pipeline->setFrontFace(renderer::FrontFace::FrontFace_Clockwise);
    m_pipeline->setCullMode(renderer::CullMode::CullMode_Back);
#if ENABLE_REVERSED_Z
    m_pipeline->setDepthCompareOp(renderer::CompareOperation::GreaterOrEqual);
#else
    m_pipeline->setDepthCompareOp(renderer::CompareOperation::LessOrEqual);
#endif
    m_pipeline->setDepthWrite(false);
    m_pipeline->setDepthTest(false);
    m_pipeline->setColorMask(0, renderer::ColorMask::ColorMask_All);

    BIND_SHADER_PARAMETER(m_pipeline, m_parameters, cb_Viewport);
    BIND_SHADER_PARAMETER(m_pipeline, m_parameters, cb_Model);
}

void RenderPipelineSelectionStage::destroy(renderer::Device* device, scene::SceneData& scene, scene::FrameData& frame)
{
    destroyRenderTarget(device, scene);

    const renderer::ShaderProgram* program = m_pipeline->getShaderProgram();
    V3D_DELETE(program, memory::MemoryLabel::MemoryGame);

    V3D_DELETE(m_pipeline, memory::MemoryLabel::MemoryGame);
    m_pipeline = nullptr;
}

void RenderPipelineSelectionStage::prepare(renderer::Device* device, scene::SceneData& scene, scene::FrameData& frame)
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

void RenderPipelineSelectionStage::execute(renderer::Device* device, scene::SceneData& scene, scene::FrameData& frame)
{
    renderer::CmdListRender* cmdList = frame.m_cmdList;
    scene::ViewportState& viewportState = scene.m_viewportState;

    TRACE_PROFILER_SCOPE("Selection", color::rgba8::GREEN);
    DEBUG_MARKER_SCOPE(cmdList, "Selection", color::rgbaf::GREEN);

    if (scene.m_renderLists[toEnumType(scene::RenderPipelinePass::Selected)].empty())
    {
        cmdList->clear(m_renderTarget->getColorTexture<renderer::Texture2D>(0), { 0.f, 0.f,  0.f,  0.f });
        return;
    }

    cmdList->beginRenderTarget(*m_renderTarget);
    cmdList->setViewport({ 0.f, 0.f, (f32)viewportState._viewpotSize._width, (f32)viewportState._viewpotSize._height });
    cmdList->setScissor({ 0.f, 0.f, (f32)viewportState._viewpotSize._width, (f32)viewportState._viewpotSize._height });
    cmdList->setStencilRef(0);
    cmdList->setPipelineState(*m_pipeline);

    cmdList->bindDescriptorSet(m_pipeline->getShaderProgram(), 0,
        {
            renderer::Descriptor(renderer::Descriptor::ConstantBuffer{ &viewportState._viewportBuffer, 0, sizeof(viewportState._viewportBuffer)}, m_parameters.cb_Viewport)
        });

    for (auto& entry : scene.m_renderLists[toEnumType(scene::RenderPipelinePass::Selected)])
    {
        const scene::DrawNodeEntry& itemMesh = *static_cast<scene::DrawNodeEntry*>(entry);
        const scene::Material& material = *static_cast<scene::Material*>(itemMesh.material);

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
        constantBuffer.normalMatrix = constantBuffer.modelMatrix.getTransposed();
        constantBuffer.tintColour = material.getProperty<math::float4>("ColorDiffuse");
        constantBuffer.objectID = itemMesh.object->ID();

        cmdList->bindDescriptorSet(m_pipeline->getShaderProgram(), 1,
            {
                renderer::Descriptor(renderer::Descriptor::ConstantBuffer{ &constantBuffer, 0, sizeof(constantBuffer)}, m_parameters.cb_Model),
            });

        DEBUG_MARKER_SCOPE(cmdList, std::format("Object {}, pipeline {}", itemMesh.object->ID(), m_pipeline->getName()), color::rgbaf::LTGREY);
        if (itemMesh.geometry)
        {
            const scene::Mesh& mesh = *static_cast<scene::Mesh*>(itemMesh.geometry);
            renderer::GeometryBufferDesc desc(mesh.getIndexBuffer(), 0, mesh.getVertexBuffer(0), 0, sizeof(VertexFormatStandard), 0);
            cmdList->drawIndexed(desc, 0, mesh.getIndexBuffer()->getIndicesCount(), 0, 0, 1);
        }
        else
        {
            cmdList->draw(renderer::GeometryBufferDesc(), 0, 4, 0, 1);
        }
    }

    cmdList->endRenderTarget();
}

void RenderPipelineSelectionStage::createRenderTarget(renderer::Device* device, scene::SceneData& data)
{
    ASSERT(m_renderTarget == nullptr, "must be nullptr");
    m_renderTarget = V3D_NEW(renderer::RenderTargetState, memory::MemoryLabel::MemoryGame)(device, data.m_viewportState._viewpotSize, 1);

    m_renderTarget->setColorTexture(0, V3D_NEW(renderer::Texture2D, memory::MemoryLabel::MemoryGame)(device, renderer::TextureUsage::TextureUsage_Attachment | renderer::TextureUsage::TextureUsage_Sampled | renderer::TextureUsage::TextureUsage_Write,
        renderer::Format::Format_R32_SFloat, data.m_viewportState._viewpotSize, renderer::TextureSamples::TextureSamples_x1, "selected_objects"),
        {
            renderer::RenderTargetLoadOp::LoadOp_Clear, renderer::RenderTargetStoreOp::StoreOp_Store, color::Color(0.0f, 0.0f, 0.0f, 1.0f)
        },
        {
            renderer::TransitionOp::TransitionOp_ColorAttachment, renderer::TransitionOp::TransitionOp_ShaderRead
        });

    data.m_globalResources.bind("selected_objects", m_renderTarget->getColorTexture<renderer::Texture2D>(0));
}

void RenderPipelineSelectionStage::destroyRenderTarget(renderer::Device* device, scene::SceneData& data)
{
    ASSERT(m_renderTarget, "must be valid");
    renderer::Texture2D* texture = m_renderTarget->getColorTexture<renderer::Texture2D>(0);
    V3D_DELETE(texture, memory::MemoryLabel::MemoryGame);

    V3D_DELETE(m_renderTarget, memory::MemoryLabel::MemoryGame);
    m_renderTarget = nullptr;
}

} // namespace scene
} // namespace v3d