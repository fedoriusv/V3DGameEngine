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
    createRenderTarget(device, scene);

    // ID 0, Sphere
    {
        const renderer::VertexShader* vertShader = resource::ResourceManager::getInstance()->loadShader<renderer::VertexShader, resource::ShaderSourceFileLoader>(device,
            "skybox.hlsl", "skybox_vs", {}, {}, resource::ShaderCompileFlag::ShaderCompile_UseDXCompilerForSpirV);
        const renderer::FragmentShader* fragShader = resource::ResourceManager::getInstance()->loadShader<renderer::FragmentShader, resource::ShaderSourceFileLoader>(device,
            "skybox.hlsl", "skybox_ps", {}, {}, resource::ShaderCompileFlag::ShaderCompile_UseDXCompilerForSpirV);

        renderer::RenderPassDesc desc{};
        desc._countColorAttachment = 1;
        desc._attachmentsDesc[0]._format = scene.m_settings._colorFormat;
        desc._hasDepthStencilAttachment = true;
        desc._attachmentsDesc.back()._format = scene.m_settings._depthFormat;

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
        desc._attachmentsDesc[0]._format = scene.m_settings._colorFormat;
        desc._hasDepthStencilAttachment = true;
        desc._attachmentsDesc.back()._format = scene.m_settings._depthFormat;

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

void RenderPipelineSkyboxStage::prepare(renderer::Device* device, scene::SceneData& scene, scene::FrameData& frame)
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

void RenderPipelineSkyboxStage::execute(renderer::Device* device, scene::SceneData& scene, scene::FrameData& frame)
{
    renderer::CmdListRender* cmdList = frame.m_cmdList;
    scene::ViewportState& viewportState = scene.m_viewportState;

    TRACE_PROFILER_SCOPE("Skybox", color::rgba8::GREEN);
    DEBUG_MARKER_SCOPE(cmdList, "Skybox", color::rgbaf::GREEN);

    ObjectHandle render_target = scene.m_globalResources.get("render_target");
    ASSERT(render_target.isValid(), "must be valid");
    renderer::Texture2D* renderTargetTexture = objectFromHandle<renderer::Texture2D>(render_target);

    ObjectHandle depth_stencil = scene.m_globalResources.get("depth_stencil");
    ASSERT(depth_stencil.isValid(), "must be valid");
    renderer::Texture2D* depthStencilTexture = objectFromHandle<renderer::Texture2D>(depth_stencil);

    if (!scene.m_renderLists[toEnumType(scene::RenderPipelinePass::Skybox)].empty())
    {
        const scene::SkyboxNodeEntry& itemMesh = *static_cast<scene::SkyboxNodeEntry*>(scene.m_renderLists[toEnumType(scene::RenderPipelinePass::Skybox)].front());
        const scene::Skybox& skybox = *static_cast<scene::Skybox*>(itemMesh.skybox);
        const scene::Material& material = *static_cast<scene::Material*>(itemMesh.material);

        ObjectHandle rt_h = scene.m_globalResources.get("render_target");
        ASSERT(rt_h.isValid(), "must be valid");
        renderer::Texture2D* renderTargetTexture = objectFromHandle<renderer::Texture2D>(rt_h);

        ObjectHandle depth_stencil_h = scene.m_globalResources.get("depth_stencil");
        ASSERT(depth_stencil_h.isValid(), "must be valid");
        renderer::Texture2D* depthStencilTexture = objectFromHandle<renderer::Texture2D>(depth_stencil_h);

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
        cmdList->setViewport({ 0.f, 0.f, (f32)viewportState._viewpotSize._width, (f32)viewportState._viewpotSize._height });
        cmdList->setScissor({ 0.f, 0.f, (f32)viewportState._viewpotSize._width, (f32)viewportState._viewpotSize._height });
        cmdList->setPipelineState(*m_pipelines[itemMesh.pipelineID]);

        cmdList->bindDescriptorSet(m_pipelines[itemMesh.pipelineID]->getShaderProgram(), 0,
            {
                renderer::Descriptor(renderer::Descriptor::ConstantBuffer{ &viewportState._viewportBuffer, 0, sizeof(viewportState._viewportBuffer)}, m_parameters.cb_Viewport)
            });

        ObjectHandle sampler_state_h = scene.m_globalResources.get("linear_sampler_repeat");
        ASSERT(sampler_state_h.isValid(), "must be valid");
        renderer::SamplerState* sampler_state = objectFromHandle<renderer::SamplerState>(sampler_state_h);

        cmdList->bindDescriptorSet(m_pipelines[itemMesh.pipelineID]->getShaderProgram(), 1,
            {
                renderer::Descriptor(sampler_state, m_parameters.s_SamplerState),
                renderer::Descriptor(renderer::TextureView(objectFromHandle<renderer::Texture2D>(material.getProperty<ObjectHandle>("BaseColor"))), m_parameters.t_SkyboxTexture),
            });

        DEBUG_MARKER_SCOPE(cmdList, std::format("Skybox {}", skybox.getName()), color::rgbaf::LTGREY);
        renderer::GeometryBufferDesc desc(m_volumes[itemMesh.pipelineID]->getIndexBuffer(), 0, m_volumes[itemMesh.pipelineID]->getVertexBuffer(0), sizeof(VertexFormatSimpleLit), 0);
        cmdList->drawIndexed(desc, 0, m_volumes[itemMesh.pipelineID]->getIndexBuffer()->getIndicesCount(), 0, 0, 1);

        cmdList->endRenderTarget();
    }
}

void RenderPipelineSkyboxStage::createRenderTarget(renderer::Device* device, scene::SceneData& data)
{
    ASSERT(m_renderTarget == nullptr, "must be nullptr");
    m_renderTarget = V3D_NEW(renderer::RenderTargetState, memory::MemoryLabel::MemoryGame)(device, data.m_viewportState._viewpotSize, 1);
}

void RenderPipelineSkyboxStage::destroyRenderTarget(renderer::Device* device, scene::SceneData& data)
{
    ASSERT(m_renderTarget, "must be valid");
    V3D_DELETE(m_renderTarget, memory::MemoryLabel::MemoryGame);
    m_renderTarget = nullptr;
}

} //namespace scene
} //namespace v3d