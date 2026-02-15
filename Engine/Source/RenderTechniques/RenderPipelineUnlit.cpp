#include "RenderPipelineUnlit.h"
#include "Utils/Logger.h"

#include "Resource/ResourceManager.h"
#include "Resource/Loader/AssetSourceFileLoader.h"
#include "Resource/Loader/ShaderSourceFileLoader.h"
#include "Resource/Loader/ModelFileLoader.h"

#include "Renderer/PipelineState.h"
#include "Renderer/ShaderProgram.h"

#include "Scene/Geometry/Mesh.h"
#include "Scene/Material.h"
#include "Scene/SceneNode.h"

#include "FrameProfiler.h"

namespace v3d
{
namespace scene
{

RenderPipelineUnlitStage::RenderPipelineUnlitStage(RenderTechnique* technique, scene::ModelHandler* modelHandler) noexcept
    : RenderPipelineStage(technique, "Unlit")
    , m_modelHandler(modelHandler)
    , m_renderTarget(nullptr)
{
}

RenderPipelineUnlitStage::~RenderPipelineUnlitStage()
{
}

void RenderPipelineUnlitStage::create(renderer::Device* device, scene::SceneData& scene, scene::FrameData& frame)
{
    //Material 0
    {
        const renderer::VertexShader* vertShader = resource::ResourceManager::getInstance()->loadShader<renderer::VertexShader, resource::ShaderSourceFileLoader>("simple.hlsl", "billboard_vs",
            {}, {}, resource::ShaderCompileFlag::ShaderCompile_UseDXCompilerForSpirV);
        const renderer::FragmentShader* fragShader = resource::ResourceManager::getInstance()->loadShader<renderer::FragmentShader, resource::ShaderSourceFileLoader>("simple.hlsl", "unlit_selectable_ps",
            {}, {}, resource::ShaderCompileFlag::ShaderCompile_UseDXCompilerForSpirV);

        renderer::RenderPassDesc desc{};
        desc._countColorAttachment = 2;
        desc._attachmentsDesc[0]._format = scene.m_settings._vewportParams._colorFormat;
        desc._attachmentsDesc[1]._format = scene.m_settings._vewportParams._colorFormat;
        desc._hasDepthStencilAttachment = true;
        desc._attachmentsDesc.back()._format = scene.m_settings._vewportParams._depthFormat;

        renderer::GraphicsPipelineState* pipeline = V3D_NEW(renderer::GraphicsPipelineState, memory::MemoryLabel::MemoryGame)(device, renderer::VertexInputAttributeDesc(), desc,
            V3D_NEW(renderer::ShaderProgram, memory::MemoryLabel::MemoryGame)(device, vertShader, fragShader), "unlit_billboard");

        pipeline->setPrimitiveTopology(renderer::PrimitiveTopology::PrimitiveTopology_TriangleStrip);
        pipeline->setFrontFace(renderer::FrontFace::FrontFace_Clockwise);
        pipeline->setCullMode(renderer::CullMode::CullMode_None);
        pipeline->setPolygonMode(renderer::PolygonMode::PolygonMode_Fill);
#if REVERSED_DEPTH
        pipeline->setDepthCompareOp(renderer::CompareOperation::GreaterOrEqual);
#else
        pipeline->setDepthCompareOp(renderer::CompareOperation::LessOrEqual);
#endif
        pipeline->setDepthTest(true);
        pipeline->setDepthWrite(true);
        pipeline->setColorMask(0, renderer::ColorMask::ColorMask_All);

        MaterialParameters parameters;
        BIND_SHADER_PARAMETER(pipeline, parameters, cb_Viewport);
        BIND_SHADER_PARAMETER(pipeline, parameters, cb_Model);
        BIND_SHADER_PARAMETER(pipeline, parameters, s_SamplerState);
        BIND_SHADER_PARAMETER(pipeline, parameters, t_TextureBaseColor);

        m_pipelines.push_back(pipeline);
        m_parameters.push_back(parameters);
    }
}

void RenderPipelineUnlitStage::destroy(renderer::Device* device, scene::SceneData& scene, scene::FrameData& frame)
{
    for (auto& pipeline : m_pipelines)
    {
        const renderer::ShaderProgram* program = pipeline->getShaderProgram();
        V3D_DELETE(program, memory::MemoryLabel::MemoryGame);

        V3D_DELETE(pipeline, memory::MemoryLabel::MemoryGame);
        pipeline = nullptr;
    }
    m_pipelines.clear();
}

void RenderPipelineUnlitStage::prepare(renderer::Device* device, scene::SceneData& scene, scene::FrameData& frame)
{
    if (!m_renderTarget)
    {
        m_renderTarget = V3D_NEW(renderer::RenderTargetState, memory::MemoryLabel::MemoryGame)(device, scene.m_viewportSize, 2);
    }
    else if (m_renderTarget->getRenderArea() != scene.m_viewportSize)
    {
        V3D_DELETE(m_renderTarget, memory::MemoryLabel::MemoryGame);
        m_renderTarget = V3D_NEW(renderer::RenderTargetState, memory::MemoryLabel::MemoryGame)(device, scene.m_viewportSize, 2);
    }
}

void RenderPipelineUnlitStage::execute(renderer::Device* device, scene::SceneData& scene, scene::FrameData& frame)
{
    if (scene.m_renderLists[toEnumType(scene::ScenePass::Indicator)].empty())
    {
        return;
    }

    auto renderJob = [this](renderer::Device* device, renderer::CmdListRender* cmdList, const scene::SceneData& scene, const scene::FrameData& frame) -> void
        {
            TRACE_PROFILER_SCOPE("Unlit", color::rgba8::GREEN);
            DEBUG_MARKER_SCOPE(cmdList, "Unlit", color::rgbaf::GREEN);
            ASSERT(!scene.m_renderLists[toEnumType(scene::ScenePass::Indicator)].empty(), "must not be empty");

            ObjectHandle renderTarget_handler = scene.m_globalResources.get("color_target");
            ASSERT(renderTarget_handler.isValid(), "must be valid");
            renderer::Texture2D* renderTargetTexture = renderTarget_handler.as<renderer::Texture2D>();

            ObjectHandle Gbuffer_Material_handler = scene.m_globalResources.get("gbuffer_material");
            ASSERT(Gbuffer_Material_handler.isValid(), "must be valid");
            renderer::Texture2D* gbufferMaterialTexture = Gbuffer_Material_handler.as<renderer::Texture2D>();

            ObjectHandle depthStencil_handler = scene.m_globalResources.get("depth_stencil");
            ASSERT(depthStencil_handler.isValid(), "must be valid");
            renderer::Texture2D* depthStencilTexture = depthStencil_handler.as<renderer::Texture2D>();

            ObjectHandle linearSampler_handler = scene.m_globalResources.get("linear_sampler_repeat");
            ASSERT(linearSampler_handler.isValid(), "must be valid");
            renderer::SamplerState* sampler = linearSampler_handler.as<renderer::SamplerState>();

            ObjectHandle viewportState_handle = frame.m_frameResources.get("viewport_state");
            ASSERT(viewportState_handle.isValid(), "must be valid");
            scene::ViewportState* viewportState = viewportState_handle.as<scene::ViewportState>();

            m_renderTarget->setColorTexture(0, renderTargetTexture,
                {
                    renderer::RenderTargetLoadOp::LoadOp_Load, renderer::RenderTargetStoreOp::StoreOp_Store, color::Color(0.0f)
                },
                {
                    renderer::TransitionOp::TransitionOp_ColorAttachment, renderer::TransitionOp::TransitionOp_ColorAttachment
                });

            m_renderTarget->setColorTexture(1, gbufferMaterialTexture,
                {
                    renderer::RenderTargetLoadOp::LoadOp_Load, renderer::RenderTargetStoreOp::StoreOp_Store, color::Color(0.0f)
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

            for (auto& entry : scene.m_renderLists[toEnumType(scene::ScenePass::Indicator)])
            {
                const scene::DrawNodeEntry& itemMesh = *static_cast<scene::DrawNodeEntry*>(entry);
                const scene::Mesh& mesh = *static_cast<scene::Mesh*>(itemMesh.geometry);
                const scene::Material& material = *static_cast<scene::Material*>(itemMesh.material);

                cmdList->setStencilRef(0x0);
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
                constantBuffer.normalMatrix = constantBuffer.modelMatrix.getInversed();
                constantBuffer.normalMatrix.makeTransposed();
                constantBuffer.tintColour = material.getProperty<math::float4>("Color");
                constantBuffer.objectID = itemMesh.object->ID();

                cmdList->bindDescriptorSet(m_pipelines[itemMesh.pipelineID]->getShaderProgram(), 1,
                    {
                        renderer::Descriptor(renderer::Descriptor::ConstantBuffer{ &constantBuffer, 0, sizeof(constantBuffer)}, m_parameters[itemMesh.pipelineID].cb_Model),
                        renderer::Descriptor(sampler, m_parameters[itemMesh.pipelineID].s_SamplerState),
                        renderer::Descriptor(renderer::TextureView(objectFromHandle<renderer::Texture2D>(material.getProperty<ObjectHandle>("BaseColor"))), m_parameters[itemMesh.pipelineID].t_TextureBaseColor)
                    });

                DEBUG_MARKER_SCOPE(cmdList, std::format("Object {}, pipeline {}", itemMesh.object->ID(), m_pipelines[itemMesh.pipelineID]->getName()), color::rgbaf::LTGREY);
                cmdList->draw(renderer::GeometryBufferDesc(), 0, 4, 0, 1);
            }

            cmdList->endRenderTarget();
        };

    addRenderJob("Unlit Job", renderJob, device, scene, true);
}

} //namespace scene
} //namespace v3d