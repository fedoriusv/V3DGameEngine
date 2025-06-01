#include "RenderPipelineTransparency.h"

#include "Resource/ResourceManager.h"

#include "Resource/Loader/AssetSourceFileLoader.h"
#include "Resource/Loader/ShaderSourceFileLoader.h"
#include "Resource/Loader/ModelFileLoader.h"

namespace v3d
{
namespace renderer
{

RenderPipelineTransparencyStage::RenderPipelineTransparencyStage(RenderTechnique* technique) noexcept
    : RenderPipelineStage(technique, "transparency")
    , m_transparencyRenderTarget(nullptr)
{
}

RenderPipelineTransparencyStage::~RenderPipelineTransparencyStage()
{
}

void RenderPipelineTransparencyStage::create(Device* device, scene::Scene::SceneData& state)
{
    createRenderTarget(device, state);

    const renderer::VertexShader* vertShader = resource::ResourceManager::getInstance()->loadShader<renderer::VertexShader, resource::ShaderSourceFileLoader>(device,
        "transparency.hlsl", "transparency_vs", {}, {}/*, resource::ShaderCompileFlag::ShaderCompile_UseDXCompilerForSpirV*/);
    const renderer::FragmentShader* fragShader = resource::ResourceManager::getInstance()->loadShader<renderer::FragmentShader, resource::ShaderSourceFileLoader>(device,
        "transparency.hlsl", "transparency_ps", {}, {}/*, resource::ShaderCompileFlag::ShaderCompile_UseDXCompilerForSpirV*/);

    renderer::GraphicsPipelineState* pipeline = new renderer::GraphicsPipelineState(
        device, GBufferStandardVertexDesc, m_transparencyRenderTarget->getRenderPassDesc(), new renderer::ShaderProgram(device, vertShader, fragShader), "transparency_pipeline");

    pipeline->setPrimitiveTopology(renderer::PrimitiveTopology::PrimitiveTopology_TriangleList);
    pipeline->setFrontFace(renderer::FrontFace::FrontFace_Clockwise);
    pipeline->setCullMode(renderer::CullMode::CullMode_None);
    pipeline->setColorMask(renderer::ColorMask::ColorMask_All);
    pipeline->setDepthWrite(true);
    pipeline->setDepthTest(false);
    pipeline->setBlendEnable(true);
    pipeline->setColorBlendFactor(renderer::BlendFactor::BlendFactor_SrcAlpha, renderer::BlendFactor::BlendFactor_OneMinusSrcAlpha);

    m_pipeline.push_back(pipeline);
}

void RenderPipelineTransparencyStage::destroy(Device* device, scene::Scene::SceneData& state)
{
}

void RenderPipelineTransparencyStage::prepare(Device* device, scene::Scene::SceneData& state)
{
}

void RenderPipelineTransparencyStage::execute(Device* device, scene::Scene::SceneData& state)
{
    state.m_renderState.m_cmdList->beginRenderTarget(*m_transparencyRenderTarget);
    state.m_renderState.m_cmdList->setViewport({ 0.f, 0.f, (f32)state.m_viewportState.m_viewpotSize._width, (f32)state.m_viewportState.m_viewpotSize._height });
    state.m_renderState.m_cmdList->setScissor({ 0.f, 0.f, (f32)state.m_viewportState.m_viewpotSize._width, (f32)state.m_viewportState.m_viewpotSize._height });

    state.m_renderState.m_cmdList->bindDescriptorSet(0,
        {
            renderer::Descriptor(renderer::Descriptor::ConstantBuffer{ &state.m_viewportState._viewportBuffer, 0, sizeof(state.m_viewportState._viewportBuffer)}, 0)
        });

    for (auto& draw : state.m_data)
    {
        if (draw.m_stageID == "transparency")
        {
            state.m_renderState.m_cmdList->setPipelineState(*m_pipeline[draw.m_pipelineID]);

            struct ModelBuffer
            {
                math::Matrix4D modelMatrix;
                math::Matrix4D normalMatrix;
                math::float4 tint;
                u64 objectID;
                u64 _pad;
            };
            ModelBuffer constantBuffer;
            constantBuffer.modelMatrix = draw.m_transform.getTransform();
            constantBuffer.normalMatrix = constantBuffer.modelMatrix.getTransposed();
            constantBuffer.tint = draw.m_tint;
            constantBuffer.objectID = draw.m_objectID;

            state.m_renderState.m_cmdList->bindDescriptorSet(1,
                {
                    renderer::Descriptor(renderer::Descriptor::ConstantBuffer{ &constantBuffer, 0, sizeof(constantBuffer)}, 1),
                    renderer::Descriptor(draw.m_sampler, 2),
                    renderer::Descriptor(draw.m_albedo, 3),
                    renderer::Descriptor(draw.m_normals, 4),
                    renderer::Descriptor(draw.m_material, 5)
                });

            renderer::GeometryBufferDesc desc(draw.m_IdxBuffer, 0, draw.m_VtxBuffer, 0, sizeof(GBufferStandardVertex), 0);
            state.m_renderState.m_cmdList->drawIndexed(desc, 0, draw.m_IdxBuffer->getIndicesCount(), 0, 0, 1);
        }
    }
    state.m_renderState.m_cmdList->endRenderTarget();
}

void RenderPipelineTransparencyStage::changed(Device* device, scene::Scene::SceneData& data)
{
    if (!m_transparencyRenderTarget)
    {
        createRenderTarget(device, data);
    }
    else if (m_transparencyRenderTarget->getRenderArea() != data.m_viewportState.m_viewpotSize)
    {
        destroyRenderTarget(device, data);
        createRenderTarget(device, data);
    }
}

void RenderPipelineTransparencyStage::createRenderTarget(Device* device, scene::Scene::SceneData& data)
{
    ASSERT(m_transparencyRenderTarget == nullptr, "must be nullptr");
    m_transparencyRenderTarget = new renderer::RenderTargetState(device, data.m_viewportState.m_viewpotSize, 1, 0, "transparency_pass");

    ObjectHandle opaque = data.m_globalResources.get("render_target");
    ASSERT(opaque.isValid(), "must be valid");
    renderer::Texture2D* opaque_texture = objectFromHandle<renderer::Texture2D>(opaque);

    m_transparencyRenderTarget->setColorTexture(0, opaque_texture,
        {
            renderer::RenderTargetLoadOp::LoadOp_Load, renderer::RenderTargetStoreOp::StoreOp_Store, color::Color(0.0f, 0.0f, 0.0f, 0.0f)
        },
        {
            renderer::TransitionOp::TransitionOp_ColorAttachment, renderer::TransitionOp::TransitionOp_ColorAttachment
        }
    );

    data.m_globalResources.bind("render_target", opaque_texture);
}

void RenderPipelineTransparencyStage::destroyRenderTarget(Device* device, scene::Scene::SceneData& data)
{
    ASSERT(m_transparencyRenderTarget, "must be valid");

    delete m_transparencyRenderTarget;
    m_transparencyRenderTarget = nullptr;
}

} //namespace renderer
} //namespace v3d