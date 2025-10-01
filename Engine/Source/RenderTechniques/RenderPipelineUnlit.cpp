#include "RenderPipelineUnlit.h"

#include "Resource/ResourceManager.h"
#include "Resource/Loader/AssetSourceFileLoader.h"
#include "Resource/Loader/ShaderSourceFileLoader.h"
#include "Resource/Loader/ModelFileLoader.h"

#include "Renderer/PipelineState.h"
#include "Renderer/ShaderProgram.h"

#include "Scene/Geometry/Mesh.h"
#include "Scene/Material.h"

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
        const renderer::VertexShader* vertShader = resource::ResourceManager::getInstance()->loadShader<renderer::VertexShader, resource::ShaderSourceFileLoader>(device,
            "simple.hlsl", "billboard_vs", {}, {}, resource::ShaderCompileFlag::ShaderCompile_UseDXCompilerForSpirV);
        const renderer::FragmentShader* fragShader = resource::ResourceManager::getInstance()->loadShader<renderer::FragmentShader, resource::ShaderSourceFileLoader>(device,
            "simple.hlsl", "unlit_selectable_ps", {}, {}, resource::ShaderCompileFlag::ShaderCompile_UseDXCompilerForSpirV);

        renderer::RenderPassDesc desc{};
        desc._countColorAttachments = 2;
        desc._attachmentsDesc[0]._format = scene.m_settings._colorFormat;
        desc._attachmentsDesc[1]._format = scene.m_settings._colorFormat;
        desc._hasDepthStencilAttahment = true;
        desc._attachmentsDesc.back()._format = scene.m_settings._depthFormat;

        renderer::GraphicsPipelineState* pipeline = V3D_NEW(renderer::GraphicsPipelineState, memory::MemoryLabel::MemoryGame)(device, renderer::VertexInputAttributeDesc(), desc,
            V3D_NEW(renderer::ShaderProgram, memory::MemoryLabel::MemoryGame)(device, vertShader, fragShader), "unlit_billboard");

        pipeline->setPrimitiveTopology(renderer::PrimitiveTopology::PrimitiveTopology_TriangleStrip);
        pipeline->setFrontFace(renderer::FrontFace::FrontFace_Clockwise);
        pipeline->setCullMode(renderer::CullMode::CullMode_None);
        pipeline->setPolygonMode(renderer::PolygonMode::PolygonMode_Fill);
#if ENABLE_REVERSED_Z
        pipeline->setDepthCompareOp(renderer::CompareOperation::GreaterOrEqual);
#else
        pipeline->setDepthCompareOp(renderer::CompareOperation::LessOrEqual);
#endif
        pipeline->setDepthTest(true);
        pipeline->setDepthWrite(true);
        pipeline->setColorMask(0, renderer::ColorMask::ColorMask_All);

        m_pipelines.push_back(pipeline);
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
        m_renderTarget = V3D_NEW(renderer::RenderTargetState, memory::MemoryLabel::MemoryGame)(device, scene.m_viewportState._viewpotSize, 2);
    }
    else if (m_renderTarget->getRenderArea() != scene.m_viewportState._viewpotSize)
    {
        V3D_DELETE(m_renderTarget, memory::MemoryLabel::MemoryGame);
        m_renderTarget = V3D_NEW(renderer::RenderTargetState, memory::MemoryLabel::MemoryGame)(device, scene.m_viewportState._viewpotSize, 2);
    }
}

void RenderPipelineUnlitStage::execute(renderer::Device* device, scene::SceneData& scene, scene::FrameData& frame)
{
    renderer::CmdListRender* cmdList = frame.m_cmdList;
    scene::ViewportState& viewportState = scene.m_viewportState;

    if (!scene.m_renderLists[toEnumType(scene::RenderPipelinePass::Indicator)].empty())
    {
        TRACE_PROFILER_SCOPE("Unlit", color::rgba8::GREEN);
        DEBUG_MARKER_SCOPE(cmdList, "Unlit", color::rgbaf::GREEN);

        ObjectHandle hRT = scene.m_globalResources.get("render_target");
        ASSERT(hRT.isValid(), "must be valid");
        renderer::Texture2D* renderTargetTexture = objectFromHandle<renderer::Texture2D>(hRT);

        ObjectHandle hGbuffer_material = scene.m_globalResources.get("gbuffer_material");
        ASSERT(hGbuffer_material.isValid(), "must be valid");
        renderer::Texture2D* gbufferMaterialTexture = objectFromHandle<renderer::Texture2D>(hGbuffer_material);

        ObjectHandle hDepth_stencil = scene.m_globalResources.get("depth_stencil");
        ASSERT(hDepth_stencil.isValid(), "must be valid");
        renderer::Texture2D* depthStencilTexture = objectFromHandle<renderer::Texture2D>(hDepth_stencil);

        ObjectHandle hLinearSampler = scene.m_globalResources.get("linear_sampler_repeat");
        ASSERT(hLinearSampler.isValid(), "must be valid");
        renderer::SamplerState* sampler = objectFromHandle<renderer::SamplerState>(hLinearSampler);

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
                renderer::TransitionOp::TransitionOp_ShaderRead, renderer::TransitionOp::TransitionOp_ColorAttachment
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

        for (auto& entry : scene.m_renderLists[toEnumType(scene::RenderPipelinePass::Indicator)])
        {
            const scene::DrawNodeEntry& itemMesh = *static_cast<scene::DrawNodeEntry*>(entry);
            const scene::Mesh& mesh = *static_cast<scene::Mesh*>(itemMesh.geometry);
            const scene::Material& material = *static_cast<scene::Material*>(itemMesh.material);

            cmdList->setStencilRef(0x0);
            cmdList->setPipelineState(*m_pipelines[itemMesh.pipelineID]);

            cmdList->bindDescriptorSet(0,
                {
                    renderer::Descriptor(renderer::Descriptor::ConstantBuffer{ &viewportState._viewportBuffer, 0, sizeof(viewportState._viewportBuffer)}, 0)
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

            cmdList->bindDescriptorSet(1,
                {
                    renderer::Descriptor(renderer::Descriptor::ConstantBuffer{ &constantBuffer, 0, sizeof(constantBuffer)}, 1),
                    renderer::Descriptor(sampler, 2),
                    renderer::Descriptor(renderer::TextureView(objectFromHandle<renderer::Texture2D>(material.getProperty<ObjectHandle>("BaseColor"))), 3)
                });

            DEBUG_MARKER_SCOPE(cmdList, std::format("Object {}, pipeline {}", itemMesh.object->ID(), m_pipelines[itemMesh.pipelineID]->getName()), color::rgbaf::LTGREY);
            cmdList->draw(renderer::GeometryBufferDesc(), 0, 4, 0, 1);
        }

        cmdList->endRenderTarget();
    }
}

} //namespace scene
} //namespace v3d