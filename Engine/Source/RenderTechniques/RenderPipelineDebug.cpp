#include "RenderPipelineDebug.h"
#include "Utils/Logger.h"

#include "Resource/ResourceManager.h"
#include "Resource/Loader/AssetSourceFileLoader.h"
#include "Resource/Loader/ShaderSourceFileLoader.h"
#include "Resource/Loader/ModelFileLoader.h"

#include "Renderer/PipelineState.h"
#include "Renderer/ShaderProgram.h"

#include "Scene/ModelHandler.h"
#include "Scene/Geometry/Mesh.h"
#include "Scene/Material.h"

#include "FrameProfiler.h"

namespace v3d
{
namespace scene
{

RenderPipelineDebugStage::RenderPipelineDebugStage(RenderTechnique* technique, scene::ModelHandler* modelHandler) noexcept
    : RenderPipelineStage(technique, "Debug")
    , m_modelHandler(modelHandler)
    , m_renderTarget(nullptr)
{
}

RenderPipelineDebugStage::~RenderPipelineDebugStage()
{
}

void RenderPipelineDebugStage::create(renderer::Device* device, scene::SceneData& scene, scene::FrameData& frame)
{
    createRenderTarget(device, scene);

    {
        const renderer::VertexShader* vertShader = resource::ResourceManager::getInstance()->loadShader<renderer::VertexShader, resource::ShaderSourceFileLoader>(device,
            "simple.hlsl", "main_vs", {}, {}, resource::ShaderCompileFlag::ShaderCompile_UseDXCompilerForSpirV);
        const renderer::FragmentShader* fragShader = resource::ResourceManager::getInstance()->loadShader<renderer::FragmentShader, resource::ShaderSourceFileLoader>(device,
            "simple.hlsl", "unlit_ps", {}, {}, resource::ShaderCompileFlag::ShaderCompile_UseDXCompilerForSpirV);

        renderer::RenderPassDesc desc{};
        desc._countColorAttachments = 1;
        desc._attachmentsDesc[0]._format = scene.m_settings._colorFormat;
        desc._hasDepthStencilAttahment = true;
        desc._attachmentsDesc.back()._format = scene.m_settings._depthFormat;

        {
            renderer::GraphicsPipelineState* pipeline = V3D_NEW(renderer::GraphicsPipelineState, memory::MemoryLabel::MemoryGame)(device, VertexFormatSimpleLitDesc, desc,
                V3D_NEW(renderer::ShaderProgram, memory::MemoryLabel::MemoryGame)(device, vertShader, fragShader), "debug_solid");

            pipeline->setPrimitiveTopology(renderer::PrimitiveTopology::PrimitiveTopology_TriangleList);
            pipeline->setFrontFace(renderer::FrontFace::FrontFace_Clockwise);
            pipeline->setCullMode(renderer::CullMode::CullMode_None);
            pipeline->setPolygonMode(renderer::PolygonMode::PolygonMode_Fill);
#if ENABLE_REVERSED_Z
            pipeline->setDepthCompareOp(renderer::CompareOperation::GreaterOrEqual);
#else
            pipeline->setDepthCompareOp(renderer::CompareOperation::LessOrEqual);
#endif
            pipeline->setDepthTest(true);
            pipeline->setDepthWrite(false);
            pipeline->setColorMask(0, renderer::ColorMask::ColorMask_All);

            MaterialParameters parameters;
            BIND_SHADER_PARAMETER(pipeline, parameters, cb_Viewport);
            BIND_SHADER_PARAMETER(pipeline, parameters, cb_Model);

            m_pipelines.push_back(pipeline);
            m_parameters.push_back(parameters);
        }

        {
            renderer::GraphicsPipelineState* pipeline = V3D_NEW(renderer::GraphicsPipelineState, memory::MemoryLabel::MemoryGame)(device, VertexFormatSimpleLitDesc, desc,
                V3D_NEW(renderer::ShaderProgram, memory::MemoryLabel::MemoryGame)(device, vertShader, fragShader), "debug_solid");

            pipeline->setPrimitiveTopology(renderer::PrimitiveTopology::PrimitiveTopology_TriangleList);
            pipeline->setFrontFace(renderer::FrontFace::FrontFace_Clockwise);
            pipeline->setCullMode(renderer::CullMode::CullMode_None);
            pipeline->setPolygonMode(renderer::PolygonMode::PolygonMode_Line);
#if ENABLE_REVERSED_Z
            pipeline->setDepthCompareOp(renderer::CompareOperation::GreaterOrEqual);
#else
            pipeline->setDepthCompareOp(renderer::CompareOperation::LessOrEqual);
#endif
            pipeline->setDepthTest(true);
            pipeline->setDepthWrite(false);
            pipeline->setColorMask(0, renderer::ColorMask::ColorMask_All);

            MaterialParameters parameters;
            BIND_SHADER_PARAMETER(pipeline, parameters, cb_Viewport);
            BIND_SHADER_PARAMETER(pipeline, parameters, cb_Model);

            m_pipelines.push_back(pipeline);
            m_parameters.push_back(parameters);
        }

        {
            renderer::GraphicsPipelineState* pipeline = V3D_NEW(renderer::GraphicsPipelineState, memory::MemoryLabel::MemoryGame)(device, VertexFormatSimpleLitDesc, desc,
                V3D_NEW(renderer::ShaderProgram, memory::MemoryLabel::MemoryGame)(device, vertShader, fragShader), "debug_line");

            pipeline->setPrimitiveTopology(renderer::PrimitiveTopology::PrimitiveTopology_LineList);
            pipeline->setFrontFace(renderer::FrontFace::FrontFace_Clockwise);
            pipeline->setCullMode(renderer::CullMode::CullMode_None);
            pipeline->setPolygonMode(renderer::PolygonMode::PolygonMode_Line);
#if ENABLE_REVERSED_Z
            pipeline->setDepthCompareOp(renderer::CompareOperation::GreaterOrEqual);
#else
            pipeline->setDepthCompareOp(renderer::CompareOperation::LessOrEqual);
#endif
            pipeline->setDepthTest(true);
            pipeline->setDepthWrite(false);
            pipeline->setDepthBias(2.0f, 0.0f, 2.0f);
            pipeline->setColorMask(0, renderer::ColorMask::ColorMask_All);

            MaterialParameters parameters;
            BIND_SHADER_PARAMETER(pipeline, parameters, cb_Viewport);
            BIND_SHADER_PARAMETER(pipeline, parameters, cb_Model);

            m_pipelines.push_back(pipeline);
            m_parameters.push_back(parameters);
        }
    }
}

void RenderPipelineDebugStage::destroy(renderer::Device* device, scene::SceneData& scene, scene::FrameData& frame)
{
    destroyRenderTarget(device, scene);

    for (auto pipeline : m_pipelines)
    {
        const renderer::ShaderProgram* program = pipeline->getShaderProgram();
        V3D_DELETE(program, memory::MemoryLabel::MemoryGame);

        V3D_DELETE(pipeline, memory::MemoryLabel::MemoryGame);
        pipeline = nullptr;
    }
    m_pipelines.clear();
}

void RenderPipelineDebugStage::prepare(renderer::Device* device, scene::SceneData& scene, scene::FrameData& frame)
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

void RenderPipelineDebugStage::execute(renderer::Device* device, scene::SceneData& scene, scene::FrameData& frame)
{
    renderer::CmdListRender* cmdList = frame.m_cmdList;
    scene::ViewportState& viewportState = scene.m_viewportState;

    TRACE_PROFILER_SCOPE("Debug", color::rgba8::GREEN);
    DEBUG_MARKER_SCOPE(cmdList, "Debug", color::rgbaf::GREEN);

    ObjectHandle render_target = scene.m_globalResources.get("render_target");
    ASSERT(render_target.isValid(), "must be valid");
    renderer::Texture2D* renderTargetTexture = objectFromHandle<renderer::Texture2D>(render_target);

    ObjectHandle depth_stencil = scene.m_globalResources.get("depth_stencil");
    ASSERT(depth_stencil.isValid(), "must be valid");
    renderer::Texture2D* depthStencilTexture = objectFromHandle<renderer::Texture2D>(depth_stencil);

    m_renderTarget->setColorTexture(0, renderTargetTexture, 
        {
            renderer::RenderTargetLoadOp::LoadOp_Load, renderer::RenderTargetStoreOp::StoreOp_Store, color::Color(0.0f, 0.0f, 0.0f, 1.0f)
        },
        {
            renderer::TransitionOp::TransitionOp_ColorAttachment, renderer::TransitionOp::TransitionOp_ColorAttachment
        });
    m_renderTarget->setDepthStencilTexture(depthStencilTexture,
        {
            renderer::RenderTargetLoadOp::LoadOp_Load, renderer::RenderTargetStoreOp::StoreOp_Store, 0.f
        },
        {
            renderer::RenderTargetLoadOp::LoadOp_Load, renderer::RenderTargetStoreOp::StoreOp_Store, 0U
        },
        {
            renderer::TransitionOp::TransitionOp_DepthStencilAttachment, renderer::TransitionOp::TransitionOp_DepthStencilAttachment
        });

    cmdList->beginRenderTarget(*m_renderTarget);
    cmdList->setViewport({ 0.f, 0.f, (f32)viewportState._viewpotSize._width, (f32)viewportState._viewpotSize._height });
    cmdList->setScissor({ 0.f, 0.f, (f32)viewportState._viewpotSize._width, (f32)viewportState._viewpotSize._height });

    for (auto& entry : scene.m_renderLists[toEnumType(scene::RenderPipelinePass::Debug)])
    {
        const scene::DrawNodeEntry& itemMesh = *static_cast<scene::DrawNodeEntry*>(entry);
        const scene::Mesh& mesh = *static_cast<scene::Mesh*>(itemMesh.geometry);
        const scene::Material& material = *static_cast<scene::Material*>(itemMesh.material);

        cmdList->setPipelineState(*m_pipelines[itemMesh.pipelineID]);

        cmdList->bindDescriptorSet(m_pipelines[itemMesh.pipelineID]->getShaderProgram(), 0,
            {
                renderer::Descriptor(renderer::Descriptor::ConstantBuffer{ &viewportState._viewportBuffer, 0, sizeof(viewportState._viewportBuffer)}, m_parameters[itemMesh.pipelineID].cb_Viewport)
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
        constantBuffer.modelMatrix = itemMesh.object->getTransform().getMatrix();
        constantBuffer.prevModelMatrix = itemMesh.object->getPrevTransform().getMatrix();
        constantBuffer.normalMatrix = constantBuffer.modelMatrix.getTransposed();
        constantBuffer.tintColour = material.getProperty<math::float4>("DiffuseColor");
        constantBuffer.objectID = 0;

        cmdList->bindDescriptorSet(m_pipelines[itemMesh.pipelineID]->getShaderProgram(), 1,
            {
                renderer::Descriptor(renderer::Descriptor::ConstantBuffer{ &constantBuffer, 0, sizeof(constantBuffer)}, m_parameters[itemMesh.pipelineID].cb_Model),
            });

        DEBUG_MARKER_SCOPE(cmdList, std::format("Object {}, pipeline {}", itemMesh.object->ID(), m_pipelines[itemMesh.pipelineID]->getName()), color::rgbaf::LTGREY);
        renderer::GeometryBufferDesc desc(mesh.getIndexBuffer(), 0, mesh.getVertexBuffer(0), 0, sizeof(VertexFormatSimpleLit), 0);
        cmdList->drawIndexed(desc, 0, mesh.getIndexBuffer()->getIndicesCount(), 0, 0, 1);
    }

    cmdList->endRenderTarget();
}

void RenderPipelineDebugStage::createRenderTarget(renderer::Device* device, scene::SceneData& data)
{
    ASSERT(m_renderTarget == nullptr, "must be nullptr");
    m_renderTarget = V3D_NEW(renderer::RenderTargetState, memory::MemoryLabel::MemoryGame)(device, data.m_viewportState._viewpotSize, 1);
}

void RenderPipelineDebugStage::destroyRenderTarget(renderer::Device* device, scene::SceneData& data)
{
    ASSERT(m_renderTarget, "must be valid");
    V3D_DELETE(m_renderTarget, memory::MemoryLabel::MemoryGame);
    m_renderTarget = nullptr;
}

} //namespace scene
} //namespace v3d