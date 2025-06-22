#include "RenderPipelineMBOIT.h"

#include "Resource/ResourceManager.h"

#include "Resource/Loader/AssetSourceFileLoader.h"
#include "Resource/Loader/ShaderSourceFileLoader.h"
#include "Resource/Loader/ModelFileLoader.h"

namespace v3d
{
namespace renderer
{

RenderPipelineMBOITStage::RenderPipelineMBOITStage(RenderTechnique* technique) noexcept
    : RenderPipelineStage(technique, "transparency")
{
}

RenderPipelineMBOITStage::~RenderPipelineMBOITStage()
{
}

void RenderPipelineMBOITStage::create(Device* device, scene::SceneData& state)
{
    createRenderTarget(device, state);

    //pass 1
    {
        const renderer::VertexShader* vertShader = resource::ResourceManager::getInstance()->loadShader<renderer::VertexShader, resource::ShaderSourceFileLoader>(device,
            "gbuffer.hlsl", "gbuffer_standard_vs", {}, {}/*, resource::ShaderCompileFlag::ShaderCompile_UseDXCompilerForSpirV*/);
        const renderer::FragmentShader* fragShader = resource::ResourceManager::getInstance()->loadShader<renderer::FragmentShader, resource::ShaderSourceFileLoader>(device,
            "transparency_mboit.hlsl", "mboit_pass1_ps", {}, {}/*, resource::ShaderCompileFlag::ShaderCompile_UseDXCompilerForSpirV*/);

        renderer::GraphicsPipelineState* pipeline = new renderer::GraphicsPipelineState(
            device, VertexFormatStandardDesc, m_rt[Pass::MBOIT_Pass1]->getRenderPassDesc(), new renderer::ShaderProgram(device, vertShader, fragShader), "mboit_pass1_ps");

        pipeline->setPrimitiveTopology(renderer::PrimitiveTopology::PrimitiveTopology_TriangleList);
        pipeline->setFrontFace(renderer::FrontFace::FrontFace_Clockwise);
        pipeline->setCullMode(renderer::CullMode::CullMode_None);
#if ENABLE_REVERSED_Z
        pipeline->setDepthCompareOp(renderer::CompareOperation::CompareOp_GreaterOrEqual);
#else
        pipeline->setDepthCompareOp(renderer::CompareOperation::CompareOp_Less);
#endif
        pipeline->setDepthTest(true);
        pipeline->setDepthWrite(false);

         pipeline->setBlendEnable(0, true);
         pipeline->setColorMask(0, renderer::ColorMask::ColorMask_All);
         pipeline->setColorBlendFactor(0, renderer::BlendFactor::BlendFactor_One, renderer::BlendFactor::BlendFactor_One);
         pipeline->setColorBlendOp(0, renderer::BlendOperation::BlendOp_Add);
         pipeline->setAlphaBlendFactor(0, renderer::BlendFactor::BlendFactor_One, renderer::BlendFactor::BlendFactor_One);
         pipeline->setAlphaBlendOp(0, renderer::BlendOperation::BlendOp_Add);

         pipeline->setBlendEnable(1, false);
         //pipeline->setColorMask(1, renderer::ColorMask::ColorMask_All);
         //pipeline->setColorBlendFactor(1, renderer::BlendFactor::BlendFactor_One, renderer::BlendFactor::BlendFactor_One);
         //pipeline->setColorBlendOp(1, renderer::BlendOperation::BlendOp_Add);
         //pipeline->setAlphaBlendFactor(1, renderer::BlendFactor::BlendFactor_One, renderer::BlendFactor::BlendFactor_One);
         //pipeline->setAlphaBlendOp(1, renderer::BlendOperation::BlendOp_Add);

        m_pipeline[Pass::MBOIT_Pass1] = pipeline;
    }

    //pass 2
    {
        const renderer::VertexShader* vertShader = resource::ResourceManager::getInstance()->loadShader<renderer::VertexShader, resource::ShaderSourceFileLoader>(device,
            "gbuffer.hlsl", "gbuffer_standard_vs", {}, {}/*, resource::ShaderCompileFlag::ShaderCompile_UseDXCompilerForSpirV*/);
        const renderer::FragmentShader* fragShader = resource::ResourceManager::getInstance()->loadShader<renderer::FragmentShader, resource::ShaderSourceFileLoader>(device,
            "transparency_mboit.hlsl", "mboit_pass2_ps", {}, {}/*, resource::ShaderCompileFlag::ShaderCompile_UseDXCompilerForSpirV*/);

        renderer::GraphicsPipelineState* pipeline = new renderer::GraphicsPipelineState(
            device, VertexFormatStandardDesc, m_rt[Pass::MBOIT_Pass2]->getRenderPassDesc(), new renderer::ShaderProgram(device, vertShader, fragShader), "mboit_pass1_ps");

        pipeline->setPrimitiveTopology(renderer::PrimitiveTopology::PrimitiveTopology_TriangleList);
        pipeline->setFrontFace(renderer::FrontFace::FrontFace_Clockwise);
        pipeline->setCullMode(renderer::CullMode::CullMode_None);
#if ENABLE_REVERSED_Z
        pipeline->setDepthCompareOp(renderer::CompareOperation::CompareOp_GreaterOrEqual);
#else
        pipeline->setDepthCompareOp(renderer::CompareOperation::CompareOp_Less);
#endif
        pipeline->setDepthTest(true);
        pipeline->setDepthWrite(false);

        pipeline->setBlendEnable(0, true);
        pipeline->setColorMask(0, renderer::ColorMask::ColorMask_All);
        pipeline->setColorBlendFactor(0, renderer::BlendFactor::BlendFactor_One, renderer::BlendFactor::BlendFactor_One);
        pipeline->setColorBlendOp(0, renderer::BlendOperation::BlendOp_Add);
        pipeline->setAlphaBlendFactor(0, renderer::BlendFactor::BlendFactor_One, renderer::BlendFactor::BlendFactor_One);
        pipeline->setAlphaBlendOp(0, renderer::BlendOperation::BlendOp_Add);

        m_pipeline[Pass::MBOIT_Pass2] = pipeline;
    }

    {
        RenderPassDesc desc{};
        desc._countColorAttachments = 1;
        desc._attachmentsDesc[0]._format = Format_R16G16B16A16_SFloat;

        const renderer::VertexShader* vertShader = resource::ResourceManager::getInstance()->loadShader<renderer::VertexShader, resource::ShaderSourceFileLoader>(device,
            "offscreen.hlsl", "offscreen_vs", {}, {}/*, resource::ShaderCompileFlag::ShaderCompile_UseDXCompilerForSpirV*/);
        const renderer::FragmentShader* fragShader = resource::ResourceManager::getInstance()->loadShader<renderer::FragmentShader, resource::ShaderSourceFileLoader>(device,
            "transparency_mboit.hlsl", "mboit_resolve_ps", {}, {}/*, resource::ShaderCompileFlag::ShaderCompile_UseDXCompilerForSpirV*/);

        renderer::GraphicsPipelineState* pipeline = new renderer::GraphicsPipelineState(
            device, renderer::VertexInputAttributeDesc(), desc, new renderer::ShaderProgram(device, vertShader, fragShader), "mboit_resolve_pipeline");

        pipeline->setPrimitiveTopology(renderer::PrimitiveTopology::PrimitiveTopology_TriangleList);
        pipeline->setFrontFace(renderer::FrontFace::FrontFace_Clockwise);
        pipeline->setCullMode(renderer::CullMode::CullMode_Back);
        pipeline->setDepthCompareOp(renderer::CompareOperation::CompareOp_Always);
        pipeline->setDepthWrite(false);
        pipeline->setDepthTest(false);
        pipeline->setBlendEnable(0, false);
        pipeline->setColorMask(0, renderer::ColorMask::ColorMask_All);

        m_pipeline[Pass::CompositionPass] = pipeline;
    }

    m_sampler = new renderer::SamplerState(device, renderer::SamplerFilter::SamplerFilter_Trilinear, renderer::SamplerAnisotropic::SamplerAnisotropic_4x);
    m_sampler->setWrap(renderer::SamplerWrap::TextureWrap_MirroredRepeat);
}

void RenderPipelineMBOITStage::destroy(Device* device, scene::SceneData& state)
{
}

void RenderPipelineMBOITStage::prepare(Device* device, scene::SceneData& state)
{
    if (!m_rt[Pass::MBOIT_Pass1])
    {
        createRenderTarget(device, state);
    }
    else if (m_rt[Pass::MBOIT_Pass1]->getRenderArea() != state.m_viewportState._viewpotSize)
    {
        destroyRenderTarget(device, state);
        createRenderTarget(device, state);
    }
}

void RenderPipelineMBOITStage::execute(Device* device, scene::SceneData& state)
{
    DEBUG_MARKER_SCOPE(state.m_renderState.m_cmdList, "Transparency", color::colorrgbaf::GREEN);

    //pass 1
    {
        state.m_renderState.m_cmdList->beginRenderTarget(*m_rt[Pass::MBOIT_Pass1]);
        state.m_renderState.m_cmdList->setViewport({ 0.f, 0.f, (f32)state.m_viewportState._viewpotSize._width, (f32)state.m_viewportState._viewpotSize._height });
        state.m_renderState.m_cmdList->setScissor({ 0.f, 0.f, (f32)state.m_viewportState._viewpotSize._width, (f32)state.m_viewportState._viewpotSize._height });
        state.m_renderState.m_cmdList->setPipelineState(*m_pipeline[Pass::MBOIT_Pass1]);

        state.m_renderState.m_cmdList->bindDescriptorSet(0,
            {
                renderer::Descriptor(renderer::Descriptor::ConstantBuffer{ &state.m_viewportState._viewportBuffer, 0, sizeof(state.m_viewportState._viewportBuffer)}, 0)
            });

        for (auto& draw : state.m_data)
        {
            if (draw.m_stageID == "transparency")
            {
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

                renderer::GeometryBufferDesc desc(draw.m_IdxBuffer, 0, draw.m_VtxBuffer, 0, sizeof(VertexFormatStandard), 0);
                state.m_renderState.m_cmdList->drawIndexed(desc, 0, draw.m_IdxBuffer->getIndicesCount(), 0, 0, 1);
            }
        }
        state.m_renderState.m_cmdList->endRenderTarget();
    }

    //pass 2
    {
        state.m_renderState.m_cmdList->beginRenderTarget(*m_rt[Pass::MBOIT_Pass2]);
        state.m_renderState.m_cmdList->setViewport({ 0.f, 0.f, (f32)state.m_viewportState._viewpotSize._width, (f32)state.m_viewportState._viewpotSize._height });
        state.m_renderState.m_cmdList->setScissor({ 0.f, 0.f, (f32)state.m_viewportState._viewpotSize._width, (f32)state.m_viewportState._viewpotSize._height });
        state.m_renderState.m_cmdList->setPipelineState(*m_pipeline[Pass::MBOIT_Pass2]);

        state.m_renderState.m_cmdList->bindDescriptorSet(0,
            {
                renderer::Descriptor(renderer::Descriptor::ConstantBuffer{ &state.m_viewportState._viewportBuffer, 0, sizeof(state.m_viewportState._viewportBuffer)}, 0)
            });

        for (auto& draw : state.m_data)
        {
            if (draw.m_stageID == "transparency")
            {
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
                        renderer::Descriptor(renderer::TextureView(m_rt[Pass::MBOIT_Pass1]->getColorTexture<renderer::Texture2D>(0), 0, 0), 6),
                        renderer::Descriptor(renderer::TextureView(m_rt[Pass::MBOIT_Pass1]->getColorTexture<renderer::Texture2D>(1), 0, 0), 7),
                    });

                renderer::GeometryBufferDesc desc(draw.m_IdxBuffer, 0, draw.m_VtxBuffer, 0, sizeof(VertexFormatStandard), 0);
                state.m_renderState.m_cmdList->drawIndexed(desc, 0, draw.m_IdxBuffer->getIndicesCount(), 0, 0, 1);
            }
        }
        state.m_renderState.m_cmdList->endRenderTarget();
    }

    //2 resolve
    {
        //ObjectHandle baseColor = state.m_globalResources.get("render_target");
        //ASSERT(baseColor.isValid(), "must be valid");
        //renderer::Texture2D* baseColorTexture = objectFromHandle<renderer::Texture2D>(baseColor);
        //m_rt[Pass::ResolvePass]->setColorTexture(0, baseColorTexture,
        //    {
        //        renderer::RenderTargetLoadOp::LoadOp_Load, renderer::RenderTargetStoreOp::StoreOp_Store, color::Color(0.0f)
        //    },
        //    {
        //        renderer::TransitionOp::TransitionOp_ColorAttachment, renderer::TransitionOp::TransitionOp_ColorAttachment
        //    });

        ObjectHandle compositionColor = state.m_globalResources.get("composition_target");
        ASSERT(compositionColor.isValid(), "must be valid");
        renderer::Texture2D* composition_Texture = objectFromHandle<renderer::Texture2D>(compositionColor);


        state.m_renderState.m_cmdList->beginRenderTarget(*m_rt[Pass::CompositionPass]);
        state.m_renderState.m_cmdList->setViewport({ 0.f, 0.f, (f32)state.m_viewportState._viewpotSize._width, (f32)state.m_viewportState._viewpotSize._height });
        state.m_renderState.m_cmdList->setScissor({ 0.f, 0.f, (f32)state.m_viewportState._viewpotSize._width, (f32)state.m_viewportState._viewpotSize._height });
        state.m_renderState.m_cmdList->setPipelineState(*m_pipeline[Pass::CompositionPass]);

        state.m_renderState.m_cmdList->bindDescriptorSet(0,
            {
                renderer::Descriptor(renderer::Descriptor::ConstantBuffer{ &state.m_viewportState._viewportBuffer, 0, sizeof(state.m_viewportState._viewportBuffer)}, 0)
            });

        state.m_renderState.m_cmdList->bindDescriptorSet(1,
            {
                renderer::Descriptor(m_sampler, 2),
                renderer::Descriptor(renderer::TextureView(composition_Texture), 3),
                renderer::Descriptor(renderer::TextureView(m_rt[Pass::MBOIT_Pass2]->getColorTexture<renderer::Texture2D>(0), 0, 0), 4),
                renderer::Descriptor(renderer::TextureView(m_rt[Pass::MBOIT_Pass1]->getColorTexture<renderer::Texture2D>(0), 0, 0), 5),
            });

        state.m_renderState.m_cmdList->draw(renderer::GeometryBufferDesc(), 0, 3, 0, 1);
        state.m_renderState.m_cmdList->endRenderTarget();
    }

    state.m_globalResources.bind("render_target", m_rt[Pass::CompositionPass]->getColorTexture<renderer::Texture2D>(0));
}

void RenderPipelineMBOITStage::createRenderTarget(Device* device, scene::SceneData& data)
{
    ObjectHandle depth_stencil = data.m_globalResources.get("depth_stencil");
    ASSERT(depth_stencil.isValid(), "must be valid");
    renderer::Texture2D* depthStencilTexture = objectFromHandle<renderer::Texture2D>(depth_stencil);

    ObjectHandle material = data.m_globalResources.get("gbuffer_material");
    ASSERT(material.isValid(), "must be valid");
    renderer::Texture2D* materialTexture = objectFromHandle<renderer::Texture2D>(material);

    ObjectHandle velocity = data.m_globalResources.get("gbuffer_velocity");
    ASSERT(velocity.isValid(), "must be valid");
    renderer::Texture2D* velocityTexture = objectFromHandle<renderer::Texture2D>(velocity);

    //pass 1
    ASSERT(m_rt[Pass::MBOIT_Pass1] == nullptr, "must be nulptr");
    m_rt[Pass::MBOIT_Pass1] = new renderer::RenderTargetState(device, data.m_viewportState._viewpotSize, 2);

    m_rt[Pass::MBOIT_Pass1]->setColorTexture(0, new renderer::Texture2D(device, renderer::TextureUsage::TextureUsage_Attachment | renderer::TextureUsage::TextureUsage_Sampled,
        renderer::Format::Format_R32_SFloat, data.m_viewportState._viewpotSize, renderer::TextureSamples::TextureSamples_x1, "mboit_od"),
        {
            renderer::RenderTargetLoadOp::LoadOp_Clear, renderer::RenderTargetStoreOp::StoreOp_Store, color::Color(0.0f, 0.0f, 0.0f, 0.0f)
        },
        {
            renderer::TransitionOp::TransitionOp_Undefined, renderer::TransitionOp::TransitionOp_ColorAttachment
        });

    m_rt[Pass::MBOIT_Pass1]->setColorTexture(1, new renderer::Texture2D(device, renderer::TextureUsage::TextureUsage_Attachment | renderer::TextureUsage::TextureUsage_Sampled,
        renderer::Format::Format_R32G32B32A32_SFloat, data.m_viewportState._viewpotSize, renderer::TextureSamples::TextureSamples_x1, "mboit_d1234"),
        {
            renderer::RenderTargetLoadOp::LoadOp_Clear, renderer::RenderTargetStoreOp::StoreOp_Store, color::Color(0.0f, 0.0f, 0.0f, 0.0f)
        },
        {
            renderer::TransitionOp::TransitionOp_Undefined, renderer::TransitionOp::TransitionOp_ColorAttachment
        });

    m_rt[Pass::MBOIT_Pass1]->setDepthStencilTexture(depthStencilTexture,
        {
            renderer::RenderTargetLoadOp::LoadOp_Load, renderer::RenderTargetStoreOp::StoreOp_Store, 0.0f
        },
        {
            renderer::RenderTargetLoadOp::LoadOp_Load, renderer::RenderTargetStoreOp::StoreOp_Store, 0U
        },
        {
            renderer::TransitionOp::TransitionOp_DepthStencilAttachment, renderer::TransitionOp::TransitionOp_DepthStencilAttachment
        });

    //pass 2
    ASSERT(m_rt[Pass::MBOIT_Pass2] == nullptr, "must be nulptr");
    m_rt[Pass::MBOIT_Pass2] = new renderer::RenderTargetState(device, data.m_viewportState._viewpotSize, 3);

    m_rt[Pass::MBOIT_Pass2]->setColorTexture(0, new renderer::Texture2D(device, renderer::TextureUsage::TextureUsage_Attachment | renderer::TextureUsage::TextureUsage_Sampled,
        renderer::Format::Format_R32G32B32A32_SFloat, data.m_viewportState._viewpotSize, renderer::TextureSamples::TextureSamples_x1, "mboit_resolve"),
        {
            renderer::RenderTargetLoadOp::LoadOp_Clear, renderer::RenderTargetStoreOp::StoreOp_Store, color::Color(0.0f, 0.0f, 0.0f, 0.0f)
        },
        {
            renderer::TransitionOp::TransitionOp_Undefined, renderer::TransitionOp::TransitionOp_ColorAttachment
        });

    m_rt[Pass::MBOIT_Pass2]->setColorTexture(1, materialTexture,
        {
            renderer::RenderTargetLoadOp::LoadOp_Load, renderer::RenderTargetStoreOp::StoreOp_Store, color::Color(0.0f, 0.0f, 0.0f, 0.0f)
        },
        {
            renderer::TransitionOp::TransitionOp_ColorAttachment, renderer::TransitionOp::TransitionOp_ColorAttachment
        });

    m_rt[Pass::MBOIT_Pass2]->setColorTexture(2, velocityTexture,
        {
            renderer::RenderTargetLoadOp::LoadOp_Load, renderer::RenderTargetStoreOp::StoreOp_Store, color::Color(0.0f, 0.0f, 0.0f, 0.0f)
        },
        {
            renderer::TransitionOp::TransitionOp_ColorAttachment, renderer::TransitionOp::TransitionOp_ColorAttachment
        });

    m_rt[Pass::MBOIT_Pass2]->setDepthStencilTexture(depthStencilTexture,
        {
            renderer::RenderTargetLoadOp::LoadOp_Load, renderer::RenderTargetStoreOp::StoreOp_Store, 0.0f
        },
        {
            renderer::RenderTargetLoadOp::LoadOp_Load, renderer::RenderTargetStoreOp::StoreOp_Store, 0U
        },
        {
            renderer::TransitionOp::TransitionOp_DepthStencilAttachment, renderer::TransitionOp::TransitionOp_DepthStencilAttachment
        });

    //resolve
    m_rt[Pass::CompositionPass] = new renderer::RenderTargetState(device, data.m_viewportState._viewpotSize, 1);

    m_rt[Pass::CompositionPass]->setColorTexture(0, new renderer::Texture2D(device, renderer::TextureUsage::TextureUsage_Attachment | renderer::TextureUsage::TextureUsage_Sampled,
        renderer::Format::Format_R16G16B16A16_SFloat, data.m_viewportState._viewpotSize, renderer::TextureSamples::TextureSamples_x1, "mboit_composition"),
        {
            renderer::RenderTargetLoadOp::LoadOp_DontCare, renderer::RenderTargetStoreOp::StoreOp_Store, color::Color(0.0f)
        },
        {
            renderer::TransitionOp::TransitionOp_Undefined, renderer::TransitionOp::TransitionOp_ColorAttachment
        }
    );
}

void RenderPipelineMBOITStage::destroyRenderTarget(Device* device, scene::SceneData& data)
{
    {
        ASSERT(m_rt[Pass::MBOIT_Pass1], "must be valid");

        renderer::Texture2D* texture0 = m_rt[Pass::MBOIT_Pass1]->getColorTexture<renderer::Texture2D>(0);
        delete texture0;
        renderer::Texture2D* texture1 = m_rt[Pass::MBOIT_Pass1]->getColorTexture<renderer::Texture2D>(1);
        delete texture1;

        delete m_rt[Pass::MBOIT_Pass1];
        m_rt[Pass::MBOIT_Pass1] = nullptr;
    }

    {
        ASSERT(m_rt[Pass::MBOIT_Pass2], "must be valid");

        renderer::Texture2D* texture0 = m_rt[Pass::MBOIT_Pass2]->getColorTexture<renderer::Texture2D>(0);
        delete texture0;

        delete m_rt[Pass::MBOIT_Pass2];
        m_rt[Pass::MBOIT_Pass2] = nullptr;
    }

    {
        ASSERT(m_rt[Pass::CompositionPass], "must be valid");

        renderer::Texture2D* texture0 = m_rt[Pass::CompositionPass]->getColorTexture<renderer::Texture2D>(0);
        delete texture0;

        delete m_rt[Pass::CompositionPass];
        m_rt[Pass::CompositionPass] = nullptr;
    }
}

} //namespace renderer
} //namespace v3d