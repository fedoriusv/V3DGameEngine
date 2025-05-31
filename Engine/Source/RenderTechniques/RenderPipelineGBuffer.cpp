#include "RenderPipelineGBuffer.h"

#include "Resource/ResourceManager.h"

#include "Resource/Loader/AssetSourceFileLoader.h"
#include "Resource/Loader/ShaderSourceFileLoader.h"
#include "Resource/Loader/ModelFileLoader.h"

namespace v3d
{
namespace renderer
{

struct GBufferStandardVertex
{
    math::float3 position;
    math::float3 normal;
    math::float3 tangent;
    math::float3 binormal;
    math::float2 UV;
};

static renderer::VertexInputAttributeDesc GBufferStandardVertexDesc(
        {
            renderer::VertexInputAttributeDesc::InputBinding(0,  renderer::InputRate::InputRate_Vertex, sizeof(GBufferStandardVertex)),
        },
        {
            renderer::VertexInputAttributeDesc::InputAttribute(0, 0, renderer::Format_R32G32B32_SFloat, offsetof(GBufferStandardVertex, position)),
            renderer::VertexInputAttributeDesc::InputAttribute(0, 0, renderer::Format_R32G32B32_SFloat, offsetof(GBufferStandardVertex, normal)),
            renderer::VertexInputAttributeDesc::InputAttribute(0, 0, renderer::Format_R32G32B32_SFloat, offsetof(GBufferStandardVertex, tangent)),
            renderer::VertexInputAttributeDesc::InputAttribute(0, 0, renderer::Format_R32G32B32_SFloat, offsetof(GBufferStandardVertex, binormal)),
            renderer::VertexInputAttributeDesc::InputAttribute(0, 0, renderer::Format_R32G32_SFloat, offsetof(GBufferStandardVertex, UV)),
        }
    );

RenderPipelineGBufferStage::RenderPipelineGBufferStage(RenderTechnique* technique) noexcept
    : RenderPipelineStage(technique, "gbuffer")
    , m_gBufferRenderTarget(nullptr)
{
}

RenderPipelineGBufferStage::~RenderPipelineGBufferStage()
{
    ASSERT(m_gBufferRenderTarget == nullptr, "must be nullptr");
}

void RenderPipelineGBufferStage::create(Device* device, scene::Scene::SceneData& state)
{
    createRenderTarget(device, state);

    //TODO json material
    //resource::Resource* material = resource::ResourceManager::getInstance()->load<resource::Resource, resource::AssetSourceFileLoader>("materials/gbuffer_standard.material.json");

    //Material
    {
        const renderer::VertexShader* vertShader = resource::ResourceManager::getInstance()->loadShader<renderer::VertexShader, resource::ShaderSourceFileLoader>(device, 
            "gbuffer.hlsl", "gbuffer_standard_vs", {}, {}/*, resource::ShaderCompileFlag::ShaderCompile_UseDXCompilerForSpirV*/);
        const renderer::FragmentShader* fragShader = resource::ResourceManager::getInstance()->loadShader<renderer::FragmentShader, resource::ShaderSourceFileLoader>(device, 
            "gbuffer.hlsl", "gbuffer_standard_ps", {}, {}/*, resource::ShaderCompileFlag::ShaderCompile_UseDXCompilerForSpirV*/);

        renderer::GraphicsPipelineState* pipeline = new renderer::GraphicsPipelineState(
            device, GBufferStandardVertexDesc, m_gBufferRenderTarget->getRenderPassDesc(), new renderer::ShaderProgram(device, vertShader, fragShader), "gbuffer_pipeline");

        pipeline->setPrimitiveTopology(renderer::PrimitiveTopology::PrimitiveTopology_TriangleList);
        pipeline->setFrontFace(renderer::FrontFace::FrontFace_Clockwise);
        pipeline->setCullMode(renderer::CullMode::CullMode_Back);
        pipeline->setColorMask(renderer::ColorMask::ColorMask_All);
        pipeline->setDepthCompareOp(renderer::CompareOperation::CompareOp_GreaterOrEqual);
        pipeline->setDepthWrite(true);
        pipeline->setDepthTest(true);

        m_pipeline.emplace_back(pipeline);
    }
}

void RenderPipelineGBufferStage::destroy(Device* device, scene::Scene::SceneData& state)
{
    //TODO
}

void RenderPipelineGBufferStage::prepare(Device* device, scene::Scene::SceneData& state)
{
}

void RenderPipelineGBufferStage::execute(Device* device, scene::Scene::SceneData& state)
{
    state.m_renderState.m_cmdList->beginRenderTarget(*m_gBufferRenderTarget);
    state.m_renderState.m_cmdList->setViewport({ 0.f, 0.f, (f32)state.m_viewportState.m_viewpotSize._width, (f32)state.m_viewportState.m_viewpotSize._height });
    state.m_renderState.m_cmdList->setScissor({ 0.f, 0.f, (f32)state.m_viewportState.m_viewpotSize._width, (f32)state.m_viewportState.m_viewpotSize._height });
    state.m_renderState.m_cmdList->setStencilRef(0);

    state.m_renderState.m_cmdList->bindDescriptorSet(0, 
        { 
            renderer::Descriptor(renderer::Descriptor::ConstantBuffer{ &state.m_viewportState._viewportBuffer, 0, sizeof(state.m_viewportState._viewportBuffer)}, 0)
        });

    for (auto& draw : state.m_data)
    {
        if (draw.m_stageID == "gbuffer")
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

void RenderPipelineGBufferStage::changed(Device* device, scene::Scene::SceneData& data)
{
    if (!m_gBufferRenderTarget)
    {
        createRenderTarget(device, data);
    }
    else if (m_gBufferRenderTarget->getRenderArea() != data.m_viewportState.m_viewpotSize)
    {
        destroyRenderTarget(device, data);
        createRenderTarget(device, data);
    }
}

void RenderPipelineGBufferStage::createRenderTarget(Device* device, scene::Scene::SceneData& state)
{
    ASSERT(m_gBufferRenderTarget == nullptr, "must be nullptr");
    m_gBufferRenderTarget = new renderer::RenderTargetState(device, state.m_viewportState.m_viewpotSize, 3, 0, "gbuffer_pass");

    renderer::Texture2D* albedoAttachment = new renderer::Texture2D(device, renderer::TextureUsage::TextureUsage_Attachment | renderer::TextureUsage::TextureUsage_Sampled,
        renderer::Format::Format_R8G8B8A8_UNorm, state.m_viewportState.m_viewpotSize, renderer::TextureSamples::TextureSamples_x1, "gbuffer_albedo");
    m_gBufferRenderTarget->setColorTexture(0, albedoAttachment,
        {
            renderer::RenderTargetLoadOp::LoadOp_Clear, renderer::RenderTargetStoreOp::StoreOp_Store, color::Color(0.0f, 0.0f, 0.0f, 1.0f)
        },
        {
            renderer::TransitionOp::TransitionOp_Undefined, renderer::TransitionOp::TransitionOp_ShaderRead
        }
    );

    renderer::Texture2D* normalsAttachment = new renderer::Texture2D(device, renderer::TextureUsage::TextureUsage_Attachment | renderer::TextureUsage::TextureUsage_Sampled,
        renderer::Format::Format_R8G8B8A8_UNorm, state.m_viewportState.m_viewpotSize, renderer::TextureSamples::TextureSamples_x1, "gbuffer_normals");
    m_gBufferRenderTarget->setColorTexture(1, normalsAttachment,
        {
            renderer::RenderTargetLoadOp::LoadOp_Clear, renderer::RenderTargetStoreOp::StoreOp_Store, color::Color(0.0f, 0.0f, 0.0f, 1.0f)
        },
        {
            renderer::TransitionOp::TransitionOp_Undefined, renderer::TransitionOp::TransitionOp_ShaderRead
        }
    );

    renderer::Texture2D* materialAttachment = new renderer::Texture2D(device, renderer::TextureUsage::TextureUsage_Attachment | renderer::TextureUsage::TextureUsage_Sampled,
        renderer::Format::Format_R8G8B8A8_UNorm, state.m_viewportState.m_viewpotSize, renderer::TextureSamples::TextureSamples_x1, "gbuffer_material");
    m_gBufferRenderTarget->setColorTexture(2, materialAttachment,
        {
            renderer::RenderTargetLoadOp::LoadOp_Clear, renderer::RenderTargetStoreOp::StoreOp_Store, color::Color(0.0f, 0.0f, 0.0f, 1.0f)
        },
        {
            renderer::TransitionOp::TransitionOp_Undefined, renderer::TransitionOp::TransitionOp_ShaderRead
        }
    );

    renderer::Texture2D* depthAttachment = new renderer::Texture2D(device, renderer::TextureUsage::TextureUsage_Attachment | renderer::TextureUsage::TextureUsage_Sampled,
        renderer::Format::Format_D32_SFloat, state.m_viewportState.m_viewpotSize, renderer::TextureSamples::TextureSamples_x1, "depth_stencil");
    m_gBufferRenderTarget->setDepthStencilTexture(depthAttachment,
        {
            renderer::RenderTargetLoadOp::LoadOp_Clear, renderer::RenderTargetStoreOp::StoreOp_Store, 0.0f
        },
        {
            renderer::RenderTargetLoadOp::LoadOp_DontCare, renderer::RenderTargetStoreOp::StoreOp_DontCare, 0U
        },
        {
            renderer::TransitionOp::TransitionOp_Undefined, renderer::TransitionOp::TransitionOp_DepthStencilAttachment
        }
    );

    state.m_globalResources.bind("gbuffer_albedo", albedoAttachment);
    state.m_globalResources.bind("gbuffer_normals", normalsAttachment);
    state.m_globalResources.bind("gbuffer_material", materialAttachment);
    state.m_globalResources.bind("depth_stencil", depthAttachment);
}

void RenderPipelineGBufferStage::destroyRenderTarget(Device* device, scene::Scene::SceneData& data)
{
    ASSERT(m_gBufferRenderTarget, "must be valid");
    renderer::Texture2D* albedoAttachment = m_gBufferRenderTarget->getColorTexture<renderer::Texture2D>(0);
    delete albedoAttachment;

    renderer::Texture2D* normalsAttachment = m_gBufferRenderTarget->getColorTexture<renderer::Texture2D>(1);
    delete normalsAttachment;

    renderer::Texture2D* materialAttachment = m_gBufferRenderTarget->getColorTexture<renderer::Texture2D>(2);
    delete materialAttachment;

    renderer::Texture2D* depthStencilAttachment = m_gBufferRenderTarget->getDepthStencilTexture<renderer::Texture2D>();
    delete depthStencilAttachment;

    delete m_gBufferRenderTarget;
    m_gBufferRenderTarget = nullptr;
}

} //namespace renderer
} //namespace v3d