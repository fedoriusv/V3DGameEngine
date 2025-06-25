#include "RenderPipelineGBuffer.h"

#include "Resource/ResourceManager.h"

#include "Resource/Loader/AssetSourceFileLoader.h"
#include "Resource/Loader/ShaderSourceFileLoader.h"
#include "Resource/Loader/ModelFileLoader.h"

namespace v3d
{
namespace renderer
{

RenderPipelineGBufferStage::RenderPipelineGBufferStage(RenderTechnique* technique, scene::ModelHandler* modelHandler) noexcept
    : RenderPipelineStage(technique, "gbuffer")
    , m_GBufferRenderTarget(nullptr)
{
}

RenderPipelineGBufferStage::~RenderPipelineGBufferStage()
{
    ASSERT(m_GBufferRenderTarget == nullptr, "must be nullptr");
}

void RenderPipelineGBufferStage::create(Device* device, scene::SceneData& state)
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
            device, VertexFormatStandardDesc, m_GBufferRenderTarget->getRenderPassDesc(), new renderer::ShaderProgram(device, vertShader, fragShader), "gbuffer_pipeline");

        pipeline->setPrimitiveTopology(renderer::PrimitiveTopology::PrimitiveTopology_TriangleList);
        pipeline->setFrontFace(renderer::FrontFace::FrontFace_Clockwise);
        pipeline->setCullMode(renderer::CullMode::CullMode_Back);
#if ENABLE_REVERSED_Z
        pipeline->setDepthCompareOp(renderer::CompareOperation::CompareOp_GreaterOrEqual);
#else
        pipeline->setDepthCompareOp(renderer::CompareOperation::CompareOp_LessOrEqual);
#endif
        pipeline->setDepthTest(true);
        pipeline->setDepthWrite(true);
        pipeline->setColorMask(0, renderer::ColorMask::ColorMask_All);
        pipeline->setColorMask(1, renderer::ColorMask::ColorMask_All);
        pipeline->setColorMask(2, renderer::ColorMask::ColorMask_All);
        pipeline->setColorMask(3, renderer::ColorMask::ColorMask_All);

        m_pipeline.emplace_back(pipeline);
    }

    {
        const renderer::VertexShader* vertShader = resource::ResourceManager::getInstance()->loadShader<renderer::VertexShader, resource::ShaderSourceFileLoader>(device,
            "gbuffer.hlsl", "gbuffer_standard_vs", {}, {}/*, resource::ShaderCompileFlag::ShaderCompile_UseDXCompilerForSpirV*/);
        const renderer::FragmentShader* fragShader = resource::ResourceManager::getInstance()->loadShader<renderer::FragmentShader, resource::ShaderSourceFileLoader>(device,
            "gbuffer.hlsl", "gbuffer_masked_ps", {}, {}/*, resource::ShaderCompileFlag::ShaderCompile_UseDXCompilerForSpirV*/);

        renderer::GraphicsPipelineState* pipeline = new renderer::GraphicsPipelineState(
            device, VertexFormatStandardDesc, m_GBufferRenderTarget->getRenderPassDesc(), new renderer::ShaderProgram(device, vertShader, fragShader), "gbuffer_masked_pipeline");

        pipeline->setPrimitiveTopology(renderer::PrimitiveTopology::PrimitiveTopology_TriangleList);
        pipeline->setFrontFace(renderer::FrontFace::FrontFace_Clockwise);
        pipeline->setCullMode(renderer::CullMode::CullMode_None);
#if ENABLE_REVERSED_Z
        pipeline->setDepthCompareOp(renderer::CompareOperation::CompareOp_GreaterOrEqual);
#else
        pipeline->setDepthCompareOp(renderer::CompareOperation::CompareOp_LessOrEqual);
#endif
        pipeline->setDepthTest(true);
        pipeline->setDepthWrite(true);
        pipeline->setColorMask(0, renderer::ColorMask::ColorMask_All);
        pipeline->setColorMask(1, renderer::ColorMask::ColorMask_All);
        pipeline->setColorMask(2, renderer::ColorMask::ColorMask_All);
        pipeline->setColorMask(3, renderer::ColorMask::ColorMask_All);

        m_pipeline.emplace_back(pipeline);
    }
}

void RenderPipelineGBufferStage::destroy(Device* device, scene::SceneData& state)
{
    destroyRenderTarget(device, state);

    for (auto& pipeline : m_pipeline)
    {
        const renderer::ShaderProgram* program = pipeline->getShaderProgram();
        delete program;

        delete pipeline;
        pipeline = nullptr;
    }
}

void RenderPipelineGBufferStage::prepare(Device* device, scene::SceneData& state)
{
    if (!m_GBufferRenderTarget)
    {
        createRenderTarget(device, state);
    }
    else if (m_GBufferRenderTarget->getRenderArea() != state.m_viewportState._viewpotSize)
    {
        destroyRenderTarget(device, state);
        createRenderTarget(device, state);
    }
}

void RenderPipelineGBufferStage::execute(Device* device, scene::SceneData& state)
{
    static auto renderTask = [this, &state](Device* device, CmdListRender* cmdList) -> void
        {
            DEBUG_MARKER_SCOPE(state.m_renderState.m_cmdList, "GBuffer", color::colorrgbaf::GREEN);

            state.m_renderState.m_cmdList->beginRenderTarget(*m_GBufferRenderTarget);
            state.m_renderState.m_cmdList->setViewport({ 0.f, 0.f, (f32)state.m_viewportState._viewpotSize._width, (f32)state.m_viewportState._viewpotSize._height });
            state.m_renderState.m_cmdList->setScissor({ 0.f, 0.f, (f32)state.m_viewportState._viewpotSize._width, (f32)state.m_viewportState._viewpotSize._height });
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
                        math::Matrix4D prevModelMatrix;
                        math::Matrix4D normalMatrix;
                        math::float4   tint;
                        u64            objectID;
                        u64           _pad = 0;
                    };
                    ModelBuffer constantBuffer;
                    constantBuffer.modelMatrix = draw.m_transform.getTransform();
                    constantBuffer.prevModelMatrix = draw.m_prevTransform.getTransform();
                    constantBuffer.normalMatrix = constantBuffer.modelMatrix.getTransposed();
                    constantBuffer.tint = draw.m_tint;
                    constantBuffer.objectID = draw.m_objectID;

                    state.m_renderState.m_cmdList->bindDescriptorSet(1,
                        {
                            renderer::Descriptor(renderer::Descriptor::ConstantBuffer{ &constantBuffer, 0, sizeof(constantBuffer)}, 1),
                            renderer::Descriptor(draw.m_sampler, 2),
                            renderer::Descriptor(renderer::TextureView(draw.m_albedo), 3),
                            renderer::Descriptor(renderer::TextureView(draw.m_normals), 4),
                            renderer::Descriptor(renderer::TextureView(draw.m_material), 5),
                        });

                    DEBUG_MARKER_SCOPE(cmdList, std::format("Object {}, pipeline {}", draw.m_objectID, m_pipeline[draw.m_pipelineID]->getName()), color::colorrgbaf::LTGREY);
                    renderer::GeometryBufferDesc desc(draw.m_IdxBuffer, 0, draw.m_VtxBuffer, 0, sizeof(VertexFormatStandard), 0);
                    state.m_renderState.m_cmdList->drawIndexed(desc, 0, draw.m_IdxBuffer->getIndicesCount(), 0, 0, 1);
                }
                else if (draw.m_stageID == "masked")
                {
                    state.m_renderState.m_cmdList->setPipelineState(*m_pipeline[draw.m_pipelineID]);

                    ObjectHandle noise_blue = state.m_globalResources.get("noise_blue");
                    ASSERT(noise_blue.isValid(), "must be valid");
                    renderer::Texture2D* noise_blue_texture = objectFromHandle<renderer::Texture2D>(noise_blue);

                    struct ModelBuffer
                    {
                        math::Matrix4D modelMatrix;
                        math::Matrix4D prevModelMatrix;
                        math::Matrix4D normalMatrix;
                        math::float4   tint;
                        u64            objectID;
                        u64           _pad = 0;
                    };
                    ModelBuffer constantBuffer;
                    constantBuffer.modelMatrix = draw.m_transform.getTransform();
                    constantBuffer.prevModelMatrix = draw.m_prevTransform.getTransform();
                    constantBuffer.normalMatrix = constantBuffer.modelMatrix.getTransposed();
                    constantBuffer.tint = draw.m_tint;
                    constantBuffer.objectID = draw.m_objectID;

                    state.m_renderState.m_cmdList->bindDescriptorSet(1,
                        {
                            renderer::Descriptor(renderer::Descriptor::ConstantBuffer{ &constantBuffer, 0, sizeof(constantBuffer)}, 1),
                            renderer::Descriptor(draw.m_sampler, 2),
                            renderer::Descriptor(renderer::TextureView(draw.m_albedo), 3),
                            renderer::Descriptor(renderer::TextureView(draw.m_normals), 4),
                            renderer::Descriptor(renderer::TextureView(draw.m_material), 5),
                            renderer::Descriptor(renderer::TextureView(noise_blue_texture), 6),
                        });

                    DEBUG_MARKER_SCOPE(cmdList, std::format("Object {}, pipeline {}", draw.m_objectID, m_pipeline[draw.m_pipelineID]->getName()), color::colorrgbaf::LTGREY);
                    renderer::GeometryBufferDesc desc(draw.m_IdxBuffer, 0, draw.m_VtxBuffer, 0, sizeof(VertexFormatStandard), 0);
                    state.m_renderState.m_cmdList->drawIndexed(desc, 0, draw.m_IdxBuffer->getIndicesCount(), 0, 0, 1);
                }
            }
            state.m_renderState.m_cmdList->endRenderTarget();
        };

    //TODO move to another thread
    renderTask(device, state.m_renderState.m_cmdList);
}

void RenderPipelineGBufferStage::createRenderTarget(Device* device, scene::SceneData& state)
{
    ASSERT(m_GBufferRenderTarget == nullptr, "must be nullptr");
    m_GBufferRenderTarget = new renderer::RenderTargetState(device, state.m_viewportState._viewpotSize, 4, 0, "gbuffer_pass");

    renderer::Texture2D* albedoAttachment = new renderer::Texture2D(device, renderer::TextureUsage::TextureUsage_Attachment | renderer::TextureUsage::TextureUsage_Sampled,
        renderer::Format::Format_R8G8B8A8_UNorm, state.m_viewportState._viewpotSize, renderer::TextureSamples::TextureSamples_x1, "gbuffer_albedo");
    m_GBufferRenderTarget->setColorTexture(0, albedoAttachment,
        {
            renderer::RenderTargetLoadOp::LoadOp_Clear, renderer::RenderTargetStoreOp::StoreOp_Store, color::Color(0.0f, 0.0f, 0.0f, 1.0f)
        },
        {
            renderer::TransitionOp::TransitionOp_Undefined, renderer::TransitionOp::TransitionOp_ShaderRead
        }
    );

    renderer::Texture2D* normalsAttachment = new renderer::Texture2D(device, renderer::TextureUsage::TextureUsage_Attachment | renderer::TextureUsage::TextureUsage_Sampled,
        renderer::Format::Format_R16G16B16A16_SFloat, state.m_viewportState._viewpotSize, renderer::TextureSamples::TextureSamples_x1, "gbuffer_normals");
    m_GBufferRenderTarget->setColorTexture(1, normalsAttachment,
        {
            renderer::RenderTargetLoadOp::LoadOp_Clear, renderer::RenderTargetStoreOp::StoreOp_Store, color::Color(0.0f, 0.0f, 0.0f, 1.0f)
        },
        {
            renderer::TransitionOp::TransitionOp_Undefined, renderer::TransitionOp::TransitionOp_ColorAttachment
        }
    );

    renderer::Texture2D* materialAttachment = new renderer::Texture2D(device, renderer::TextureUsage::TextureUsage_Attachment | renderer::TextureUsage::TextureUsage_Sampled,
        renderer::Format::Format_R16G16B16A16_SFloat, state.m_viewportState._viewpotSize, renderer::TextureSamples::TextureSamples_x1, "gbuffer_material");
    m_GBufferRenderTarget->setColorTexture(2, materialAttachment,
        {
            renderer::RenderTargetLoadOp::LoadOp_Clear, renderer::RenderTargetStoreOp::StoreOp_Store, color::Color(0.0f, 0.0f, 0.0f, 1.0f)
        },
        {
            renderer::TransitionOp::TransitionOp_Undefined, renderer::TransitionOp::TransitionOp_ColorAttachment
        }
    );

    renderer::Texture2D* velocityAttachment = new renderer::Texture2D(device, renderer::TextureUsage::TextureUsage_Attachment | renderer::TextureUsage::TextureUsage_Sampled,
        renderer::Format::Format_R16G16_SFloat, state.m_viewportState._viewpotSize, renderer::TextureSamples::TextureSamples_x1, "gbuffer_velocity");
    m_GBufferRenderTarget->setColorTexture(3, velocityAttachment,
        {
            renderer::RenderTargetLoadOp::LoadOp_Clear, renderer::RenderTargetStoreOp::StoreOp_Store, color::Color(0.0f, 0.0f, 0.0f, 1.0f)
        },
        {
            renderer::TransitionOp::TransitionOp_Undefined, renderer::TransitionOp::TransitionOp_ColorAttachment
        }
    );

    state.m_globalResources.bind("gbuffer_albedo", albedoAttachment);
    state.m_globalResources.bind("gbuffer_normals", normalsAttachment);
    state.m_globalResources.bind("gbuffer_material", materialAttachment);
    state.m_globalResources.bind("gbuffer_velocity", velocityAttachment);

    ObjectHandle depth_stencil = state.m_globalResources.get("depth_stencil");
    ASSERT(depth_stencil.isValid(), "must be valid");
    renderer::Texture2D* depthAttachment = objectFromHandle<renderer::Texture2D>(depth_stencil);
    m_GBufferRenderTarget->setDepthStencilTexture(depthAttachment,
        {
            renderer::RenderTargetLoadOp::LoadOp_Load, renderer::RenderTargetStoreOp::StoreOp_Store, 0.0f
        },
        {
            renderer::RenderTargetLoadOp::LoadOp_Load, renderer::RenderTargetStoreOp::StoreOp_Store, 0U
        },
        {
            renderer::TransitionOp::TransitionOp_DepthStencilAttachment, renderer::TransitionOp::TransitionOp_DepthStencilAttachment
        }
    );
}

void RenderPipelineGBufferStage::destroyRenderTarget(Device* device, scene::SceneData& data)
{
    ASSERT(m_GBufferRenderTarget, "must be valid");
    renderer::Texture2D* albedoAttachment = m_GBufferRenderTarget->getColorTexture<renderer::Texture2D>(0);
    delete albedoAttachment;

    renderer::Texture2D* normalsAttachment = m_GBufferRenderTarget->getColorTexture<renderer::Texture2D>(1);
    delete normalsAttachment;

    renderer::Texture2D* materialAttachment = m_GBufferRenderTarget->getColorTexture<renderer::Texture2D>(2);
    delete materialAttachment;

    renderer::Texture2D* velocityAttachment = m_GBufferRenderTarget->getColorTexture<renderer::Texture2D>(3);
    delete velocityAttachment;

    delete m_GBufferRenderTarget;
    m_GBufferRenderTarget = nullptr;
}

} //namespace renderer
} //namespace v3d