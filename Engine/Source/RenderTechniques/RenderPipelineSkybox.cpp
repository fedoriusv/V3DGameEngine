#include "RenderPipelineSkybox.h"
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
#include "Scene/Skybox.h"
#include "Scene/SceneNode.h"

#include "FrameProfiler.h"

namespace v3d
{
namespace scene
{

RenderPipelineSkyboxStage::RenderPipelineSkyboxStage(RenderTechnique* technique, scene::ModelHandler* modelHandler) noexcept
    : RenderPipelineStage(technique, "Skybox")
    , m_modelHandler(modelHandler)
    , m_renderTarget(nullptr)
{
}

RenderPipelineSkyboxStage::~RenderPipelineSkyboxStage()
{
}

void RenderPipelineSkyboxStage::create(renderer::Device* device, scene::SceneData& scene, scene::FrameData& frame)
{
    createRenderTarget(device, scene, frame);

    // ID 0, Sphere
    {
        const renderer::VertexShader* vertShader = resource::ResourceManager::getInstance()->loadShader<renderer::VertexShader, resource::ShaderSourceFileLoader>(device,
            "skybox.hlsl", "skybox_vs", {}, {}, resource::ShaderCompileFlag::ShaderCompile_UseDXCompilerForSpirV);
        const renderer::FragmentShader* fragShader = resource::ResourceManager::getInstance()->loadShader<renderer::FragmentShader, resource::ShaderSourceFileLoader>(device,
            "skybox.hlsl", "skybox_ps", {}, {}, resource::ShaderCompileFlag::ShaderCompile_UseDXCompilerForSpirV);

        renderer::RenderPassDesc desc{};
        desc._countColorAttachment = 1;
        desc._attachmentsDesc[0]._format = scene.m_settings._vewportParams._colorFormat;
        desc._hasDepthStencilAttachment = true;
        desc._attachmentsDesc.back()._format = scene.m_settings._vewportParams._depthFormat;

        renderer::GraphicsPipelineState* pipeline = V3D_NEW(renderer::GraphicsPipelineState, memory::MemoryLabel::MemoryGame)(device, VertexFormatSimpleLitDesc, desc,
            V3D_NEW(renderer::ShaderProgram, memory::MemoryLabel::MemoryGame)(device, vertShader, fragShader), "skybox");

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

        BIND_SHADER_PARAMETER(pipeline, m_parameters, cb_Viewport);
        BIND_SHADER_PARAMETER(pipeline, m_parameters, s_SamplerState);
        BIND_SHADER_PARAMETER(pipeline, m_parameters, t_SkyboxTexture);

        m_pipelines.push_back(pipeline);
        m_volumes.push_back(scene::MeshHelper::createSphere(device, 100.f, 64.f, 64.f, "Skybox"));
    }

    // ID 1, Skybox
    {
        const renderer::VertexShader* vertShader = resource::ResourceManager::getInstance()->loadShader<renderer::VertexShader, resource::ShaderSourceFileLoader>(device,
            "skybox.hlsl", "skybox_vs", {}, {}, resource::ShaderCompileFlag::ShaderCompile_UseDXCompilerForSpirV);
        const renderer::FragmentShader* fragShader = resource::ResourceManager::getInstance()->loadShader<renderer::FragmentShader, resource::ShaderSourceFileLoader>(device,
            "skybox.hlsl", "skybox_ps", {}, {}, resource::ShaderCompileFlag::ShaderCompile_UseDXCompilerForSpirV);

        renderer::RenderPassDesc desc{};
        desc._countColorAttachment = 1;
        desc._attachmentsDesc[0]._format = scene.m_settings._vewportParams._colorFormat;
        desc._hasDepthStencilAttachment = true;
        desc._attachmentsDesc.back()._format = scene.m_settings._vewportParams._depthFormat;

        renderer::GraphicsPipelineState* pipeline = V3D_NEW(renderer::GraphicsPipelineState, memory::MemoryLabel::MemoryGame)(device, VertexFormatSimpleLitDesc, desc,
            V3D_NEW(renderer::ShaderProgram, memory::MemoryLabel::MemoryGame)(device, vertShader, fragShader), "skybox");

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

        BIND_SHADER_PARAMETER(pipeline, m_parameters, cb_Viewport);
        BIND_SHADER_PARAMETER(pipeline, m_parameters, s_SamplerState);
        BIND_SHADER_PARAMETER(pipeline, m_parameters, t_SkyboxTexture);

        m_pipelines.push_back(pipeline);
        m_volumes.push_back(scene::MeshHelper::createCube(device, 100.f, "Skybox"));
    }
}

void RenderPipelineSkyboxStage::destroy(renderer::Device* device, scene::SceneData& scene, scene::FrameData& frame)
{
    destroyRenderTarget(device, scene, frame);

    for (auto& pipeline : m_pipelines)
    {
        const renderer::ShaderProgram* program = pipeline->getShaderProgram();
        V3D_DELETE(program, memory::MemoryLabel::MemoryGame);

        V3D_DELETE(pipeline, memory::MemoryLabel::MemoryGame);
        pipeline = nullptr;
    }
    m_pipelines.clear();
}

void RenderPipelineSkyboxStage::prepare(renderer::Device* device, scene::SceneData& scene, scene::FrameData& frame)
{
    if (!m_renderTarget)
    {
        createRenderTarget(device, scene, frame);
    }
    else if (m_renderTarget->getRenderArea() != scene.m_viewportSize)
    {
        destroyRenderTarget(device, scene, frame);
        createRenderTarget(device, scene, frame);
    }
}

void RenderPipelineSkyboxStage::execute(renderer::Device* device, scene::SceneData& scene, scene::FrameData& frame)
{
    if (scene.m_renderLists[toEnumType(scene::RenderPipelinePass::Skybox)].empty())
    {
        return;
    }

    auto renderJob = [this](renderer::Device* device, renderer::CmdListRender* cmdList, const scene::SceneData& scene, const scene::FrameData& frame) -> void
        {
            TRACE_PROFILER_SCOPE("Skybox", color::rgba8::GREEN);
            DEBUG_MARKER_SCOPE(cmdList, "Skybox", color::rgbaf::GREEN);
            ASSERT(!scene.m_renderLists[toEnumType(scene::RenderPipelinePass::Skybox)].empty(), "must be not empty");

            for (auto& entry : scene.m_renderLists[toEnumType(scene::RenderPipelinePass::Skybox)])
            {
                const scene::SkyboxNodeEntry& itemMesh = *static_cast<scene::SkyboxNodeEntry*>(entry);
                const scene::Skybox& skybox = *static_cast<scene::Skybox*>(itemMesh.skybox);
                const scene::Material& material = *static_cast<scene::Material*>(itemMesh.material);

                ObjectHandle viewportState_handle = frame.m_frameResources.get("viewport_state");
                ASSERT(viewportState_handle.isValid(), "must be valid");
                scene::ViewportState* viewportState = viewportState_handle.as<scene::ViewportState>();

                ObjectHandle renderTarget_handle = scene.m_globalResources.get("color_target");
                ASSERT(renderTarget_handle.isValid(), "must be valid");
                renderer::Texture2D* renderTargetTexture = renderTarget_handle.as<renderer::Texture2D>();

                ObjectHandle depthStencil_handle = scene.m_globalResources.get("depth_stencil");
                ASSERT(depthStencil_handle.isValid(), "must be valid");
                renderer::Texture2D* depthStencilTexture = depthStencil_handle.as<renderer::Texture2D>();

                ObjectHandle samplerState_handle = scene.m_globalResources.get("linear_sampler_repeat");
                ASSERT(samplerState_handle.isValid(), "must be valid");
                renderer::SamplerState* sampler_state = samplerState_handle.as<renderer::SamplerState>();

                m_renderTarget->setColorTexture(0, renderTargetTexture,
                    {
                        renderer::RenderTargetLoadOp::LoadOp_Load, renderer::RenderTargetStoreOp::StoreOp_Store, color::Color(0.0f)
                    },
                    {
                        renderer::TransitionOp::TransitionOp_ColorAttachment, renderer::TransitionOp::TransitionOp_ColorAttachment
                    });

                m_renderTarget->setDepthStencilTexture(depthStencilTexture,
                    {
                        renderer::RenderTargetLoadOp::LoadOp_Load, renderer::RenderTargetStoreOp::StoreOp_Store, 0.0f
                    },
                    {
                        renderer::RenderTargetLoadOp::LoadOp_Load, renderer::RenderTargetStoreOp::StoreOp_Store, 0U
                    },
                    {
                        renderer::TransitionOp::TransitionOp_DepthStencilReadOnly, renderer::TransitionOp::TransitionOp_DepthStencilAttachment
                    });

                cmdList->beginRenderTarget(*m_renderTarget);
                cmdList->setViewport({ 0.f, 0.f, (f32)viewportState->viewportSize._x, (f32)viewportState->viewportSize._y });
                cmdList->setScissor({ 0.f, 0.f, (f32)viewportState->viewportSize._x, (f32)viewportState->viewportSize._y });
                cmdList->setPipelineState(*m_pipelines[itemMesh.pipelineID]);
                cmdList->bindDescriptorSet(m_pipelines[itemMesh.pipelineID]->getShaderProgram(), 0,
                    {
                        renderer::Descriptor(renderer::Descriptor::ConstantBuffer{ viewportState, 0, sizeof(scene::ViewportState)}, m_parameters.cb_Viewport)
                    });

                cmdList->bindDescriptorSet(m_pipelines[itemMesh.pipelineID]->getShaderProgram(), 1,
                    {
                        renderer::Descriptor(sampler_state, m_parameters.s_SamplerState),
                        renderer::Descriptor(renderer::TextureView(material.getProperty<ObjectHandle>("BaseColor").as<renderer::Texture2D>()), m_parameters.t_SkyboxTexture),
                    });

                DEBUG_MARKER_SCOPE(cmdList, std::format("Skybox {}", skybox.getName()), color::rgbaf::LTGREY);
                renderer::GeometryBufferDesc desc(m_volumes[itemMesh.pipelineID]->getIndexBuffer(), 0, m_volumes[itemMesh.pipelineID]->getVertexBuffer(0), sizeof(VertexFormatSimpleLit), 0);
                cmdList->drawIndexed(desc, 0, m_volumes[itemMesh.pipelineID]->getIndexBuffer()->getIndicesCount(), 0, 0, 1);
            }

            cmdList->endRenderTarget();
        };

    addRenderJob("Skybox Job", renderJob, device, scene, true);
}

void RenderPipelineSkyboxStage::createRenderTarget(renderer::Device* device, scene::SceneData& scene, scene::FrameData& frame)
{
    ASSERT(m_renderTarget == nullptr, "must be nullptr");
    m_renderTarget = V3D_NEW(renderer::RenderTargetState, memory::MemoryLabel::MemoryGame)(device, scene.m_viewportSize, 1);
}

void RenderPipelineSkyboxStage::destroyRenderTarget(renderer::Device* device, scene::SceneData& data, scene::FrameData& frame)
{
    ASSERT(m_renderTarget, "must be valid");
    V3D_DELETE(m_renderTarget, memory::MemoryLabel::MemoryGame);
    m_renderTarget = nullptr;
}

} //namespace scene
} //namespace v3d