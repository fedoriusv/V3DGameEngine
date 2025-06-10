#include "RenderPipelineDepthOIT.h"

#include "Resource/ResourceManager.h"

#include "Resource/Loader/AssetSourceFileLoader.h"
#include "Resource/Loader/ShaderSourceFileLoader.h"
#include "Resource/Loader/ModelFileLoader.h"

namespace v3d
{
namespace renderer
{

RenderPipelineDepthOITStage::RenderPipelineDepthOITStage(RenderTechnique* technique) noexcept
    : RenderPipelineStage(technique, "transparency")
    , m_transparencyRenderTarget(nullptr)
{
}

RenderPipelineDepthOITStage::~RenderPipelineDepthOITStage()
{
}

void RenderPipelineDepthOITStage::create(Device* device, scene::Scene::SceneData& state)
{
    createRenderTarget(device, state);

    //stochastic pass 1: accumulate alpha
    {
        const renderer::VertexShader* vertShader = resource::ResourceManager::getInstance()->loadShader<renderer::VertexShader, resource::ShaderSourceFileLoader>(device,
            "gbuffer.hlsl", "gbuffer_standard_vs", {}, {}/*, resource::ShaderCompileFlag::ShaderCompile_UseDXCompilerForSpirV*/);
        const renderer::FragmentShader* fragShader = resource::ResourceManager::getInstance()->loadShader<renderer::FragmentShader, resource::ShaderSourceFileLoader>(device,
            "transparency_sdoit.hlsl", "transparency_alpha_ps", {}, {}/*, resource::ShaderCompileFlag::ShaderCompile_UseDXCompilerForSpirV*/);

        renderer::GraphicsPipelineState* pipeline = new renderer::GraphicsPipelineState(
            device, VertexFormatStandardDesc, m_stochasticTransparency_TotalAlpha->getRenderPassDesc(), new renderer::ShaderProgram(device, vertShader, fragShader), "st_total_alpha_pass1");

        pipeline->setPrimitiveTopology(renderer::PrimitiveTopology::PrimitiveTopology_TriangleList);
        pipeline->setFrontFace(renderer::FrontFace::FrontFace_Clockwise);
        pipeline->setCullMode(renderer::CullMode::CullMode_None);
        pipeline->setColorMask(renderer::ColorMask::ColorMask_All);
#if ENABLE_REVERSED_Z
        pipeline->setDepthCompareOp(renderer::CompareOperation::CompareOp_GreaterOrEqual);
#else
        pipeline->setDepthCompareOp(renderer::CompareOperation::CompareOp_Less);
#endif
        pipeline->setDepthTest(true);
        pipeline->setDepthWrite(false);

        pipeline->setBlendEnable(true);
        pipeline->setColorBlendFactor(renderer::BlendFactor::BlendFactor_One, renderer::BlendFactor::BlendFactor_One);
        pipeline->setColorBlendOp(renderer::BlendOperation::BlendOp_Add);
        //pipeline->setColorBlendFactor(renderer::BlendFactor::BlendFactor_Zero, renderer::BlendFactor::BlendFactor_OneMinusSrcColor);
        //pipeline->setColorBlendOp(renderer::BlendOperation::BlendOp_Add);
        //pipeline->setAlphaBlendFactor(renderer::BlendFactor::BlendFactor_One, renderer::BlendFactor::BlendFactor_One);
        //pipeline->setAlphaBlendOp(renderer::BlendOperation::BlendOp_Add);

        m_pipeline[Pass::StochasticTotalAlpha] = pipeline;
    }

    //stochastic pass 2: transparency depth
    {
        const renderer::VertexShader* vertShader = resource::ResourceManager::getInstance()->loadShader<renderer::VertexShader, resource::ShaderSourceFileLoader>(device,
            "gbuffer.hlsl", "gbuffer_standard_vs", {}, {}/*, resource::ShaderCompileFlag::ShaderCompile_UseDXCompilerForSpirV*/);
        const renderer::FragmentShader* fragShader = resource::ResourceManager::getInstance()->loadShader<renderer::FragmentShader, resource::ShaderSourceFileLoader>(device,
            "transparency_sdoit.hlsl", "stochastic_depth_ps", {}, {}/*, resource::ShaderCompileFlag::ShaderCompile_UseDXCompilerForSpirV*/);

        renderer::GraphicsPipelineState* pipeline = new renderer::GraphicsPipelineState(
            device, VertexFormatStandardDesc, m_stochasticTransparency_Depth->getRenderPassDesc(), new renderer::ShaderProgram(device, vertShader, fragShader), "st_transparency_depth_pass2");

        pipeline->setPrimitiveTopology(renderer::PrimitiveTopology::PrimitiveTopology_TriangleList);
        pipeline->setFrontFace(renderer::FrontFace::FrontFace_Clockwise);
        pipeline->setCullMode(renderer::CullMode::CullMode_None);
        pipeline->setColorMask(renderer::ColorMask::ColorMask_All);
#if ENABLE_REVERSED_Z
        pipeline->setDepthCompareOp(renderer::CompareOperation::CompareOp_GreaterOrEqual);
#else
        pipeline->setDepthCompareOp(renderer::CompareOperation::CompareOp_Equal);
#endif
        pipeline->setDepthWrite(true);
        pipeline->setDepthTest(true);

        m_pipeline[Pass::StochasticDepth] = pipeline;
    }

    //stochastic pass 3: accumulate color
    {
        const renderer::VertexShader* vertShader = resource::ResourceManager::getInstance()->loadShader<renderer::VertexShader, resource::ShaderSourceFileLoader>(device,
            "gbuffer.hlsl", "gbuffer_standard_vs", {}, {}/*, resource::ShaderCompileFlag::ShaderCompile_UseDXCompilerForSpirV*/);
        const renderer::FragmentShader* fragShader = resource::ResourceManager::getInstance()->loadShader<renderer::FragmentShader, resource::ShaderSourceFileLoader>(device,
            "transparency_sdoit.hlsl", "transparency_accumulate_color_ps", {}, {}/*, resource::ShaderCompileFlag::ShaderCompile_UseDXCompilerForSpirV*/);

        renderer::GraphicsPipelineState* pipeline = new renderer::GraphicsPipelineState(
            device, VertexFormatStandardDesc, m_stochasticTransparency_AccumulateColor->getRenderPassDesc(), new renderer::ShaderProgram(device, vertShader, fragShader), "st_accumulate_color_pass3");

        pipeline->setPrimitiveTopology(renderer::PrimitiveTopology::PrimitiveTopology_TriangleList);
        pipeline->setFrontFace(renderer::FrontFace::FrontFace_Clockwise);
        pipeline->setCullMode(renderer::CullMode::CullMode_None);
        pipeline->setColorMask(renderer::ColorMask::ColorMask_All);
#if ENABLE_REVERSED_Z
        pipeline->setDepthCompareOp(renderer::CompareOperation::CompareOp_Equal);
#else
        pipeline->setDepthCompareOp(renderer::CompareOperation::CompareOp_Equal);
#endif
        pipeline->setDepthWrite(false);
        pipeline->setDepthTest(true);
        pipeline->setBlendEnable(true);
        pipeline->setColorBlendFactor(renderer::BlendFactor::BlendFactor_One, renderer::BlendFactor::BlendFactor_One);
        pipeline->setColorBlendOp(renderer::BlendOperation::BlendOp_Add);

        m_pipeline[Pass::StochasticTotalAccumulateColor] = pipeline;
    }

    //stochastic pass 4: final
    {
        const renderer::VertexShader* vertShader = resource::ResourceManager::getInstance()->loadShader<renderer::VertexShader, resource::ShaderSourceFileLoader>(device,
            "offscreen.hlsl", "offscreen_vs", {}, {}/*, resource::ShaderCompileFlag::ShaderCompile_UseDXCompilerForSpirV*/);
        const renderer::FragmentShader* fragShader = resource::ResourceManager::getInstance()->loadShader<renderer::FragmentShader, resource::ShaderSourceFileLoader>(device,
            "transparency_sdoit.hlsl", "stochastic_transparency_final_ps", {}, {}/*, resource::ShaderCompileFlag::ShaderCompile_UseDXCompilerForSpirV*/);

        renderer::GraphicsPipelineState* pipeline = new renderer::GraphicsPipelineState(
            device, renderer::VertexInputAttributeDesc(), m_stochasticTransparency_Final->getRenderPassDesc(), new renderer::ShaderProgram(device, vertShader, fragShader), "st_transparency_final_pass4");

        pipeline->setPrimitiveTopology(renderer::PrimitiveTopology::PrimitiveTopology_TriangleList);
        pipeline->setFrontFace(renderer::FrontFace::FrontFace_Clockwise);
        pipeline->setCullMode(renderer::CullMode::CullMode_Back);
        pipeline->setColorMask(renderer::ColorMask::ColorMask_All);
        pipeline->setDepthCompareOp(renderer::CompareOperation::CompareOp_Always);
        pipeline->setDepthWrite(false);
        pipeline->setDepthTest(false);

        m_pipeline[Pass::StochasticTotalFinal] = pipeline;
    }

    m_sampler = new renderer::SamplerState(device, renderer::SamplerFilter::SamplerFilter_Trilinear, renderer::SamplerAnisotropic::SamplerAnisotropic_4x);
    m_sampler->setWrap(renderer::SamplerWrap::TextureWrap_MirroredRepeat);
}

void RenderPipelineDepthOITStage::destroy(Device* device, scene::Scene::SceneData& state)
{
}

void RenderPipelineDepthOITStage::prepare(Device* device, scene::Scene::SceneData& state)
{
}

void RenderPipelineDepthOITStage::execute(Device* device, scene::Scene::SceneData& state)
{
    state.m_renderState.m_cmdList->setViewport({ 0.f, 0.f, (f32)state.m_viewportState.m_viewpotSize._width, (f32)state.m_viewportState.m_viewpotSize._height });
    state.m_renderState.m_cmdList->setScissor({ 0.f, 0.f, (f32)state.m_viewportState.m_viewpotSize._width, (f32)state.m_viewportState.m_viewpotSize._height });

    executeStochasticTransparency(device, state);
}

void RenderPipelineDepthOITStage::executeStochasticTransparency(Device* device, scene::Scene::SceneData& state)
{
    DEBUG_MARKER_SCOPE(state.m_renderState.m_cmdList, "Transparency", color::colorrgbaf::GREEN);

    ObjectHandle noise_blue = state.m_globalResources.get("noise_blue");
    ASSERT(noise_blue.isValid(), "must be valid");
    renderer::Texture2D* noise_blue_texture = objectFromHandle<renderer::Texture2D>(noise_blue);

    //stochastic pass 1: accumulate alpha
    state.m_renderState.m_cmdList->beginRenderTarget(*m_stochasticTransparency_TotalAlpha);
    state.m_renderState.m_cmdList->setPipelineState(*m_pipeline[Pass::StochasticTotalAlpha]);

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
                });

            renderer::GeometryBufferDesc desc(draw.m_IdxBuffer, 0, draw.m_VtxBuffer, 0, sizeof(VertexFormatStandard), 0);
            state.m_renderState.m_cmdList->drawIndexed(desc, 0, draw.m_IdxBuffer->getIndicesCount(), 0, 0, 1);
        }
    }
    state.m_renderState.m_cmdList->endRenderTarget();

    //stochastic transparency depth
    state.m_renderState.m_cmdList->beginRenderTarget(*m_stochasticTransparency_Depth);
    state.m_renderState.m_cmdList->setPipelineState(*m_pipeline[Pass::StochasticDepth]);

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
                    renderer::Descriptor(renderer::TextureView(noise_blue_texture), 4)
                });

            renderer::GeometryBufferDesc desc(draw.m_IdxBuffer, 0, draw.m_VtxBuffer, 0, sizeof(VertexFormatStandard), 0);
            state.m_renderState.m_cmdList->drawIndexed(desc, 0, draw.m_IdxBuffer->getIndicesCount(), 0, 0, 1);
        }
    }
    state.m_renderState.m_cmdList->endRenderTarget();

    //stochastic accumulate color
    state.m_renderState.m_cmdList->beginRenderTarget(*m_stochasticTransparency_AccumulateColor);
    state.m_renderState.m_cmdList->setPipelineState(*m_pipeline[Pass::StochasticTotalAccumulateColor]);

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

    //stochastic final
    state.m_renderState.m_cmdList->beginRenderTarget(*m_stochasticTransparency_Final);
    state.m_renderState.m_cmdList->setPipelineState(*m_pipeline[Pass::StochasticTotalFinal]);

    ObjectHandle baseColor = state.m_globalResources.get("render_target");
    ASSERT(baseColor.isValid(), "must be valid");
    renderer::Texture2D* baseColorTexture = objectFromHandle<renderer::Texture2D>(baseColor);

    state.m_renderState.m_cmdList->bindDescriptorSet(0,
        {
            renderer::Descriptor(renderer::Descriptor::ConstantBuffer{ &state.m_viewportState._viewportBuffer, 0, sizeof(state.m_viewportState._viewportBuffer)}, 0)
        });

    state.m_renderState.m_cmdList->bindDescriptorSet(1,
        {
            renderer::Descriptor(m_sampler, 1),
            renderer::Descriptor(renderer::TextureView(baseColorTexture, 0, 0), 3),
            renderer::Descriptor(renderer::TextureView(m_stochasticTransparency_TotalAlpha->getColorTexture<renderer::Texture2D>(0), 0, 0), 4),
            renderer::Descriptor(renderer::TextureView(m_stochasticTransparency_AccumulateColor->getColorTexture<renderer::Texture2D>(0), 0, 0), 5)
        });

    state.m_renderState.m_cmdList->draw(renderer::GeometryBufferDesc(), 0, 3, 0, 1);
    state.m_renderState.m_cmdList->endRenderTarget();

    state.m_globalResources.bind("render_target", m_stochasticTransparency_Final->getColorTexture<renderer::Texture2D>(0));
}

void RenderPipelineDepthOITStage::changed(Device* device, scene::Scene::SceneData& data)
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

void RenderPipelineDepthOITStage::createRenderTarget(Device* device, scene::Scene::SceneData& data)
{
    ObjectHandle depth_stencil = data.m_globalResources.get("depth_stencil");
    ASSERT(depth_stencil.isValid(), "must be valid");
    renderer::Texture2D* depth_stencil_texture = objectFromHandle<renderer::Texture2D>(depth_stencil);

    //1
    m_stochasticTransparency_TotalAlpha = new renderer::RenderTargetState(device, data.m_viewportState.m_viewpotSize, 1, 0, "st_total_alpha_pass");
    renderer::Texture2D* r32float = new renderer::Texture2D(device, renderer::TextureUsage::TextureUsage_Attachment | renderer::TextureUsage::TextureUsage_Sampled,
        renderer::Format::Format_R32_SFloat, data.m_viewportState.m_viewpotSize, renderer::TextureSamples::TextureSamples_x1, "total_alpha");

    m_stochasticTransparency_TotalAlpha->setColorTexture(0, r32float,
        {
            renderer::RenderTargetLoadOp::LoadOp_Clear, renderer::RenderTargetStoreOp::StoreOp_Store, color::Color(0.0f, 0.0f, 0.0f, 0.0f)
        },
        {
            renderer::TransitionOp::TransitionOp_Undefined, renderer::TransitionOp::TransitionOp_ColorAttachment
        });
    m_stochasticTransparency_TotalAlpha->setDepthStencilTexture(depth_stencil_texture,
        {
            renderer::RenderTargetLoadOp::LoadOp_Load, renderer::RenderTargetStoreOp::StoreOp_Store, 0.0f
        },
        {
            renderer::RenderTargetLoadOp::LoadOp_Load, renderer::RenderTargetStoreOp::StoreOp_Store, 0U
        },
        {
            renderer::TransitionOp::TransitionOp_DepthStencilAttachment, renderer::TransitionOp::TransitionOp_DepthStencilAttachment
        });

    //2
    //TODO copy
    m_stochasticTransparency_Depth = new renderer::RenderTargetState(device, data.m_viewportState.m_viewpotSize, 0, 0, "st_depth_mask_pass");
    m_stochasticTransparency_Depth->setDepthStencilTexture(depth_stencil_texture,
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

    //3
    m_stochasticTransparency_AccumulateColor = new renderer::RenderTargetState(device, data.m_viewportState.m_viewpotSize, 1, 0, "st_accumulate_color_pass");
    renderer::Texture2D* color = new renderer::Texture2D(device, renderer::TextureUsage::TextureUsage_Attachment | renderer::TextureUsage::TextureUsage_Sampled,
        renderer::Format::Format_R8G8B8A8_UNorm, data.m_viewportState.m_viewpotSize, renderer::TextureSamples::TextureSamples_x1, "accumulate_color");

    m_stochasticTransparency_AccumulateColor->setColorTexture(0, color,
        {
            renderer::RenderTargetLoadOp::LoadOp_Clear, renderer::RenderTargetStoreOp::StoreOp_Store, color::Color(0.0f, 0.0f, 0.0f, 0.0f)
        },
            {
                renderer::TransitionOp::TransitionOp_Undefined, renderer::TransitionOp::TransitionOp_ColorAttachment
            }
    );

    m_stochasticTransparency_AccumulateColor->setDepthStencilTexture(depth_stencil_texture,
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

    //4
    m_stochasticTransparency_Final = new renderer::RenderTargetState(device, data.m_viewportState.m_viewpotSize, 1, 0, "st_final_pass");

    renderer::Texture2D* final = new renderer::Texture2D(device, renderer::TextureUsage::TextureUsage_Attachment | renderer::TextureUsage::TextureUsage_Sampled,
        renderer::Format::Format_R8G8B8A8_UNorm, data.m_viewportState.m_viewpotSize, renderer::TextureSamples::TextureSamples_x1, "st_final");

    m_stochasticTransparency_Final->setColorTexture(0, final,
        {
            renderer::RenderTargetLoadOp::LoadOp_DontCare, renderer::RenderTargetStoreOp::StoreOp_Store, color::Color(0.0f)
        },
        {
            renderer::TransitionOp::TransitionOp_Undefined, renderer::TransitionOp::TransitionOp_ColorAttachment
        }
    );

    data.m_globalResources.bind("full_composite", final);
}

void RenderPipelineDepthOITStage::destroyRenderTarget(Device* device, scene::Scene::SceneData& data)
{
    ASSERT(m_transparencyRenderTarget, "must be valid");

    delete m_transparencyRenderTarget;
    m_transparencyRenderTarget = nullptr;

    //st
    {
        //1
        renderer::Texture2D* r32float = m_stochasticTransparency_TotalAlpha->getColorTexture<renderer::Texture2D>(0);
        delete r32float;

        delete m_stochasticTransparency_TotalAlpha;
        m_stochasticTransparency_TotalAlpha = nullptr;

        //2
        delete m_stochasticTransparency_Depth;
        m_stochasticTransparency_Depth = nullptr;

        //3
        renderer::Texture2D* color = m_stochasticTransparency_AccumulateColor->getColorTexture<renderer::Texture2D>(0);
        delete color;

        delete m_stochasticTransparency_AccumulateColor;
        m_stochasticTransparency_AccumulateColor = nullptr;

        //4
        renderer::Texture2D* final = m_stochasticTransparency_Final->getColorTexture<renderer::Texture2D>(0);
        delete final;

        delete m_stochasticTransparency_Final;
        m_stochasticTransparency_Final = nullptr;
    }
}

} //namespace renderer
} //namespace v3d