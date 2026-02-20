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
#include "Scene/SceneNode.h"

#include "FrameProfiler.h"

namespace v3d
{
namespace scene
{

RenderPipelineDebugStage::RenderPipelineDebugStage(RenderTechnique* technique, scene::ModelHandler* modelHandler) noexcept
    : RenderPipelineStage(technique, "Debug")
    , m_modelHandler(modelHandler)

    , m_renderTarget(nullptr)

    , m_debugVisualizerPipeline(nullptr)
    , m_debugVisualizerTexture(nullptr)
{
}

RenderPipelineDebugStage::~RenderPipelineDebugStage()
{
}

void RenderPipelineDebugStage::create(renderer::Device* device, scene::SceneData& scene, scene::FrameData& frame)
{
    createRenderTarget(device, scene, frame);

    {
        const renderer::VertexShader* vertShader = resource::ResourceManager::getInstance()->loadShader<renderer::VertexShader, resource::ShaderSourceFileLoader>(
            "simple.hlsl", "simple_vs", {}, {});
        const renderer::FragmentShader* fragShader = resource::ResourceManager::getInstance()->loadShader<renderer::FragmentShader, resource::ShaderSourceFileLoader>(
            "simple.hlsl", "unlit_ps", {}, {});

        renderer::RenderPassDesc desc(scene.m_settings._vewportParams._colorFormat, scene.m_settings._vewportParams._depthFormat);

        {
            renderer::GraphicsPipelineState* pipeline = V3D_NEW(renderer::GraphicsPipelineState, memory::MemoryLabel::MemoryGame)(device, VertexFormatSimpleLitDesc, desc,
                V3D_NEW(renderer::ShaderProgram, memory::MemoryLabel::MemoryGame)(device, vertShader, fragShader), "debug_solid_triangle");

            pipeline->setPrimitiveTopology(renderer::PrimitiveTopology::PrimitiveTopology_TriangleList);
            pipeline->setFrontFace(renderer::FrontFace::FrontFace_Clockwise);
            pipeline->setCullMode(renderer::CullMode::CullMode_None);
            pipeline->setPolygonMode(renderer::PolygonMode::PolygonMode_Fill);
#if REVERSED_DEPTH
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
                V3D_NEW(renderer::ShaderProgram, memory::MemoryLabel::MemoryGame)(device, vertShader, fragShader), "debug_w_triangle");

            pipeline->setPrimitiveTopology(renderer::PrimitiveTopology::PrimitiveTopology_TriangleList);
            pipeline->setFrontFace(renderer::FrontFace::FrontFace_Clockwise);
            pipeline->setCullMode(renderer::CullMode::CullMode_None);
            pipeline->setPolygonMode(renderer::PolygonMode::PolygonMode_Line);
#if REVERSED_DEPTH
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
#if REVERSED_DEPTH
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

    {
        const renderer::VertexShader* vertShader = resource::ResourceManager::getInstance()->loadShader<renderer::VertexShader, resource::ShaderSourceFileLoader>(
            "offscreen.hlsl", "offscreen_vs", {}, {});
        const renderer::FragmentShader* fragShader = resource::ResourceManager::getInstance()->loadShader<renderer::FragmentShader, resource::ShaderSourceFileLoader>(
            "debug.hlsl", "debug_visualizer_ps", {}, {});

        renderer::RenderPassDesc desc(renderer::Format::Format_R8G8B8A8_UNorm);
        m_debugVisualizerPipeline = V3D_NEW(renderer::GraphicsPipelineState, memory::MemoryLabel::MemoryGame)(device, renderer::VertexInputAttributeDesc(), desc,
            V3D_NEW(renderer::ShaderProgram, memory::MemoryLabel::MemoryGame)(device, vertShader, fragShader), "debug_visualizer_pipeline");

        m_debugVisualizerPipeline->setPrimitiveTopology(renderer::PrimitiveTopology::PrimitiveTopology_TriangleList);
        m_debugVisualizerPipeline->setFrontFace(renderer::FrontFace::FrontFace_Clockwise);
        m_debugVisualizerPipeline->setCullMode(renderer::CullMode::CullMode_Back);
        m_debugVisualizerPipeline->setDepthCompareOp(renderer::CompareOperation::Always);
        m_debugVisualizerPipeline->setDepthWrite(false);
        m_debugVisualizerPipeline->setDepthTest(false);
        m_debugVisualizerPipeline->setColorMask(0, renderer::ColorMask::ColorMask_All);

        BIND_SHADER_PARAMETER(m_debugVisualizerPipeline, m_debugVisualizerParameters, cb_Viewport);
        BIND_SHADER_PARAMETER(m_debugVisualizerPipeline, m_debugVisualizerParameters, cb_Visualizer);
        BIND_SHADER_PARAMETER(m_debugVisualizerPipeline, m_debugVisualizerParameters, s_LinearSampler);
        BIND_SHADER_PARAMETER(m_debugVisualizerPipeline, m_debugVisualizerParameters, t_RenderTargetTexture);
        BIND_SHADER_PARAMETER(m_debugVisualizerPipeline, m_debugVisualizerParameters, t_VisualizeTexture);
    }
}

void RenderPipelineDebugStage::destroy(renderer::Device* device, scene::SceneData& scene, scene::FrameData& frame)
{
    destroyRenderTarget(device, scene, frame);

    for (auto pipeline : m_pipelines)
    {
        const renderer::ShaderProgram* program = pipeline->getShaderProgram();
        V3D_DELETE(program, memory::MemoryLabel::MemoryGame);

        V3D_DELETE(pipeline, memory::MemoryLabel::MemoryGame);
        pipeline = nullptr;
    }
    m_pipelines.clear();

    V3D_DELETE(m_debugVisualizerPipeline, memory::MemoryLabel::MemoryGame);
    m_debugVisualizerPipeline = nullptr;
}

void RenderPipelineDebugStage::prepare(renderer::Device* device, scene::SceneData& scene, scene::FrameData& frame)
{
    if (scene.m_renderLists[toEnumType(scene::ScenePass::Debug)].empty() && scene.m_settings._vewportParams._renderTargetID == 0)
    {
        return;

    }
    if (!m_renderTarget)
    {
        createRenderTarget(device, scene, frame);
    }
    else if (m_renderTarget->getRenderArea() != scene.m_viewportSize)
    {
        destroyRenderTarget(device, scene, frame);
        createRenderTarget(device, scene, frame);
    }

    ObjectHandle inputTarget_handle = frame.m_frameResources.get("render_target");
    if (!inputTarget_handle.isValid())
    {
        inputTarget_handle = scene.m_globalResources.get("color_target");
    }

    frame.m_frameResources.bind("input_target_debug", inputTarget_handle);
    frame.m_frameResources.bind("render_target", inputTarget_handle);
    if (scene.m_settings._vewportParams._renderTargetID)
    {
        switch (scene.m_settings._vewportParams._renderTargetID)
        {
        case 1: //GBuffer Albedo
            frame.m_frameResources.bind("input_target_visualize", scene.m_globalResources.get("gbuffer_albedo"));
            break;

        case 2: //GBuffer Normals
            frame.m_frameResources.bind("input_target_visualize", scene.m_globalResources.get("gbuffer_normals"));
            break;

        case 3: //Depth
            frame.m_frameResources.bind("input_target_visualize", scene.m_globalResources.get("depth_stencil"));
            break;

        case 4: //Light Complexity
            frame.m_frameResources.bind("input_target_visualize", scene.m_globalResources.get("color_target")); //TODO
            break;

        default:
            break;
        }
    }
}

void RenderPipelineDebugStage::execute(renderer::Device* device, scene::SceneData& scene, scene::FrameData& frame)
{
    if (scene.m_renderLists[toEnumType(scene::ScenePass::Debug)].empty() && scene.m_settings._vewportParams._renderTargetID == 0)
    {
        return;
    }

    auto renderJob = [this](renderer::Device* device, renderer::CmdListRender* cmdList, const scene::SceneData& scene, const scene::FrameData& frame) -> void
        {
            TRACE_PROFILER_SCOPE("Debug", color::rgba8::GREEN);
            DEBUG_MARKER_SCOPE(cmdList, "Debug", color::rgbaf::GREEN);
            ASSERT(!scene.m_renderLists[toEnumType(scene::ScenePass::Debug)].empty(), "must be not empty");

            ObjectHandle viewportState_handle = frame.m_frameResources.get("viewport_state");
            ASSERT(viewportState_handle.isValid(), "must be valid");
            scene::ViewportState* viewportState = viewportState_handle.as<scene::ViewportState>();

            ObjectHandle renderTarget_handle = frame.m_frameResources.get("input_target_debug");
            ASSERT(renderTarget_handle.isValid(), "must be valid");
            renderer::Texture2D* renderTargetTexture = renderTarget_handle.as<renderer::Texture2D>();

            ObjectHandle visualizeTexture_handle = frame.m_frameResources.get("input_target_visualize");
            if (!visualizeTexture_handle.isValid())
            {
                visualizeTexture_handle = scene.m_globalResources.get("default_black");
                ASSERT(visualizeTexture_handle.isValid(), "must be valid");
            }
            renderer::Texture2D* visualizeTexture = visualizeTexture_handle.as<renderer::Texture2D>();

            ObjectHandle depthStencilTexture_handle = scene.m_globalResources.get("depth_stencil");
            ASSERT(depthStencilTexture_handle.isValid(), "must be valid");
            renderer::Texture2D* depthStencilTexture = depthStencilTexture_handle.as<renderer::Texture2D>();

            ObjectHandle linearSamplerRepeat_handle = scene.m_globalResources.get("linear_sampler_repeat");
            ASSERT(linearSamplerRepeat_handle.isValid(), "must be valid");
            renderer::SamplerState* samplerState = linearSamplerRepeat_handle.as<renderer::SamplerState>();

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
            cmdList->setViewport({ 0.f, 0.f, (f32)viewportState->viewportSize._x, (f32)viewportState->viewportSize._y });
            cmdList->setScissor({ 0.f, 0.f, (f32)viewportState->viewportSize._x, (f32)viewportState->viewportSize._y });

            for (auto& entry : scene.m_renderLists[toEnumType(scene::ScenePass::Debug)])
            {
                const scene::DrawNodeEntry& itemMesh = *static_cast<scene::DrawNodeEntry*>(entry);
                const scene::Mesh& mesh = *static_cast<scene::Mesh*>(itemMesh.geometry);
                const scene::Material& material = *static_cast<scene::Material*>(itemMesh.material);

                cmdList->setPipelineState(*m_pipelines[itemMesh.pipelineID]);
                cmdList->bindDescriptorSet(m_pipelines[itemMesh.pipelineID]->getShaderProgram(), 0,
                    {
                        renderer::Descriptor(renderer::Descriptor::ConstantBuffer{ viewportState, 0, sizeof(scene::ViewportState)}, m_parameters[itemMesh.pipelineID].cb_Viewport)
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
                renderer::GeometryBufferDesc desc(mesh.getIndexBuffer(), 0, mesh.getVertexBuffer(0), sizeof(VertexFormatSimpleLit), 0);
                cmdList->drawIndexed(desc, 0, mesh.getIndexBuffer()->getIndicesCount(), 0, 0, 1);
            }

            cmdList->endRenderTarget();

            if (scene.m_settings._vewportParams._renderTargetID)
            {
                m_renderTarget->setColorTexture(0, m_debugVisualizerTexture,
                    {
                        renderer::RenderTargetLoadOp::LoadOp_DontCare, renderer::RenderTargetStoreOp::StoreOp_Store, color::Color(0.0f, 0.0f, 0.0f, 1.0f)
                    },
                    {
                        renderer::TransitionOp::TransitionOp_ColorAttachment, renderer::TransitionOp::TransitionOp_ShaderRead
                    });

                m_renderTarget->setDepthStencilTexture(renderer::TextureView(), //unbind depth
                    {
                        renderer::RenderTargetLoadOp::LoadOp_DontCare, renderer::RenderTargetStoreOp::StoreOp_DontCare, 0.f
                    },
                    {
                        renderer::RenderTargetLoadOp::LoadOp_DontCare, renderer::RenderTargetStoreOp::StoreOp_DontCare, 0U
                    },
                    {
                        renderer::TransitionOp::TransitionOp_DepthStencilAttachment, renderer::TransitionOp::TransitionOp_DepthStencilAttachment
                    });

                cmdList->beginRenderTarget(*m_renderTarget);
                cmdList->setViewport({ 0.f, 0.f, (f32)viewportState->viewportSize._x, (f32)viewportState->viewportSize._y });
                cmdList->setScissor({ 0.f, 0.f, (f32)viewportState->viewportSize._x, (f32)viewportState->viewportSize._y });

                cmdList->setPipelineState(*m_debugVisualizerPipeline);
                cmdList->bindDescriptorSet(m_debugVisualizerPipeline->getShaderProgram(), 0,
                    {
                        renderer::Descriptor(renderer::Descriptor::ConstantBuffer{ viewportState, 0, sizeof(scene::ViewportState)}, m_debugVisualizerParameters.cb_Viewport)
                    });

                struct Visualizer
                {
                    u32 mode;
                    u32 format;
                } visualizer;

                visualizer.mode = scene.m_settings._vewportParams._renderTargetID;
                visualizer.format = visualizeTexture->getFormat();

                cmdList->bindDescriptorSet(m_debugVisualizerPipeline->getShaderProgram(), 1,
                    {
                        renderer::Descriptor(renderer::Descriptor::ConstantBuffer{ &visualizer, 0, sizeof(visualizer)}, m_debugVisualizerParameters.cb_Visualizer),
                        renderer::Descriptor(samplerState, m_debugVisualizerParameters.s_LinearSampler),
                        renderer::Descriptor(renderer::TextureView(renderTargetTexture, 0, 0), m_debugVisualizerParameters.t_RenderTargetTexture),
                        renderer::Descriptor(renderer::TextureView(visualizeTexture, 0, 0), m_debugVisualizerParameters.t_VisualizeTexture),
                    });

                cmdList->draw(renderer::GeometryBufferDesc(), 0, 3, 0, 1);
                cmdList->endRenderTarget();
            }
        };

    addRenderJob("Debug Job", renderJob, device, scene, true);
}

void RenderPipelineDebugStage::createRenderTarget(renderer::Device* device, scene::SceneData& scene, scene::FrameData& frame)
{
    ASSERT(m_renderTarget == nullptr, "must be nullptr");
    m_renderTarget = V3D_NEW(renderer::RenderTargetState, memory::MemoryLabel::MemoryGame)(device, scene.m_viewportSize, 1);

    ASSERT(m_debugVisualizerTexture == nullptr, "must be nullptr");
    m_debugVisualizerTexture = V3D_NEW(renderer::Texture2D, memory::MemoryLabel::MemoryGame)(device, renderer::TextureUsage::TextureUsage_Attachment | renderer::TextureUsage::TextureUsage_Sampled,
        renderer::Format::Format_R8G8B8A8_UNorm, scene.m_viewportSize, renderer::TextureSamples::TextureSamples_x1, "debug_visualizer");
    scene.m_globalResources.bind("final_visualize", m_debugVisualizerTexture);
}

void RenderPipelineDebugStage::destroyRenderTarget(renderer::Device* device, scene::SceneData& scene, scene::FrameData& frame)
{
    ASSERT(m_renderTarget, "must be valid");
    V3D_DELETE(m_renderTarget, memory::MemoryLabel::MemoryGame);
    m_renderTarget = nullptr;

    ASSERT(m_debugVisualizerTexture, "must be valid");
    V3D_DELETE(m_debugVisualizerTexture, memory::MemoryLabel::MemoryGame);
    m_debugVisualizerTexture = nullptr;
}

} //namespace scene
} //namespace v3d