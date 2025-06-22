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
    , m_depthSamples(nullptr)
{
}

RenderPipelineDepthOITStage::~RenderPipelineDepthOITStage()
{
}

void RenderPipelineDepthOITStage::create(Device* device, scene::Scene::SceneData& state)
{
    createRenderTarget(device, state);

    //stochastic pass 1: total alpha
    {
        const renderer::VertexShader* vertShader = resource::ResourceManager::getInstance()->loadShader<renderer::VertexShader, resource::ShaderSourceFileLoader>(device,
            "gbuffer.hlsl", "gbuffer_standard_vs", {}, {}/*, resource::ShaderCompileFlag::ShaderCompile_UseDXCompilerForSpirV*/);
        const renderer::FragmentShader* fragShader = resource::ResourceManager::getInstance()->loadShader<renderer::FragmentShader, resource::ShaderSourceFileLoader>(device,
            "transparency_sdboit.hlsl", "transparency_total_alpha_pass1_ps", {}, {}/*, resource::ShaderCompileFlag::ShaderCompile_UseDXCompilerForSpirV*/);

        renderer::GraphicsPipelineState* pipeline = new renderer::GraphicsPipelineState(
            device, VertexFormatStandardDesc, m_rt[Pass::StochasticTotalAlpha]->getRenderPassDesc(), new renderer::ShaderProgram(device, vertShader, fragShader), "st_total_alpha_pass1");

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

        m_pipeline[Pass::StochasticTotalAlpha] = pipeline;
    }

    //stochastic pass 2: transparency depth
    {
        const renderer::VertexShader* vertShader = resource::ResourceManager::getInstance()->loadShader<renderer::VertexShader, resource::ShaderSourceFileLoader>(device,
            "gbuffer.hlsl", "gbuffer_standard_vs", {}, {}/*, resource::ShaderCompileFlag::ShaderCompile_UseDXCompilerForSpirV*/);
        const renderer::FragmentShader* fragShader = resource::ResourceManager::getInstance()->loadShader<renderer::FragmentShader, resource::ShaderSourceFileLoader>(device,
            "transparency_sdboit.hlsl", "transparency_depth_pass2_ps", {}, {}/*, resource::ShaderCompileFlag::ShaderCompile_UseDXCompilerForSpirV*/);

        renderer::GraphicsPipelineState* pipeline = new renderer::GraphicsPipelineState(
            device, VertexFormatStandardDesc, m_rt[Pass::StochasticDepth]->getRenderPassDesc(), new renderer::ShaderProgram(device, vertShader, fragShader), "st_transparency_depth_pass2");

        pipeline->setPrimitiveTopology(renderer::PrimitiveTopology::PrimitiveTopology_TriangleList);
        pipeline->setFrontFace(renderer::FrontFace::FrontFace_Clockwise);
        pipeline->setCullMode(renderer::CullMode::CullMode_None);
#if ENABLE_REVERSED_Z
        pipeline->setDepthCompareOp(renderer::CompareOperation::CompareOp_GreaterOrEqual);
#else
        pipeline->setDepthCompareOp(renderer::CompareOperation::CompareOp_Equal);
#endif
        pipeline->setDepthTest(true);
        pipeline->setDepthWrite(false);
        pipeline->setColorMask(0, renderer::ColorMask::ColorMask_All);

        m_pipeline[Pass::StochasticDepth] = pipeline;
    }

    //stochastic pass 3: accumulate color
    {
        const renderer::VertexShader* vertShader = resource::ResourceManager::getInstance()->loadShader<renderer::VertexShader, resource::ShaderSourceFileLoader>(device,
            "gbuffer.hlsl", "gbuffer_standard_vs", {}, {}/*, resource::ShaderCompileFlag::ShaderCompile_UseDXCompilerForSpirV*/);
        const renderer::FragmentShader* fragShader = resource::ResourceManager::getInstance()->loadShader<renderer::FragmentShader, resource::ShaderSourceFileLoader>(device,
            "transparency_sdboit.hlsl", "transparency_accumulate_color_pass3_ps", {}, {}/*, resource::ShaderCompileFlag::ShaderCompile_UseDXCompilerForSpirV*/);

        renderer::GraphicsPipelineState* pipeline = new renderer::GraphicsPipelineState(
            device, VertexFormatStandardDesc, m_rt[Pass::StochasticTotalAccumulateColor]->getRenderPassDesc(), new renderer::ShaderProgram(device, vertShader, fragShader), "st_accumulate_color_pass3");

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

        m_pipeline[Pass::StochasticTotalAccumulateColor] = pipeline;
    }

    //stochastic pass 4: final
    {
        const renderer::VertexShader* vertShader = resource::ResourceManager::getInstance()->loadShader<renderer::VertexShader, resource::ShaderSourceFileLoader>(device,
            "offscreen.hlsl", "offscreen_vs", {}, {}/*, resource::ShaderCompileFlag::ShaderCompile_UseDXCompilerForSpirV*/);
        const renderer::FragmentShader* fragShader = resource::ResourceManager::getInstance()->loadShader<renderer::FragmentShader, resource::ShaderSourceFileLoader>(device,
            "transparency_sdboit.hlsl", "transparency_final_ps", {}, {}/*, resource::ShaderCompileFlag::ShaderCompile_UseDXCompilerForSpirV*/);

        renderer::GraphicsPipelineState* pipeline = new renderer::GraphicsPipelineState(
            device, renderer::VertexInputAttributeDesc(), m_rt[Pass::StochasticComposite]->getRenderPassDesc(), new renderer::ShaderProgram(device, vertShader, fragShader), "st_transparency_final_pass4");

        pipeline->setPrimitiveTopology(renderer::PrimitiveTopology::PrimitiveTopology_TriangleList);
        pipeline->setFrontFace(renderer::FrontFace::FrontFace_Clockwise);
        pipeline->setCullMode(renderer::CullMode::CullMode_Back);
        pipeline->setDepthCompareOp(renderer::CompareOperation::CompareOp_Always);
        pipeline->setDepthWrite(false);
        pipeline->setDepthTest(false);
        pipeline->setColorMask(0, renderer::ColorMask::ColorMask_All);

        m_pipeline[Pass::StochasticComposite] = pipeline;
    }

    m_sampler = new renderer::SamplerState(device, renderer::SamplerFilter::SamplerFilter_Trilinear, renderer::SamplerAnisotropic::SamplerAnisotropic_4x);
    m_sampler->setWrap(renderer::SamplerWrap::TextureWrap_MirroredRepeat);
}

void RenderPipelineDepthOITStage::destroy(Device* device, scene::Scene::SceneData& state)
{
}

void RenderPipelineDepthOITStage::prepare(Device* device, scene::Scene::SceneData& state)
{
    if (!m_depthSamples || m_depthSamples->getDimension() != state.m_viewportState.m_viewpotSize)
    {
        delete m_depthSamples;
        m_depthSamples = new renderer::UnorderedAccessTexture2D(device, TextureUsage::TextureUsage_Storage | TextureUsage::TextureUsage_Sampled | TextureUsage::TextureUsage_Write | TextureUsage::TextureUsage_Read,
            Format::Format_R32_UInt, state.m_viewportState.m_viewpotSize, 4, "depth_samples");
    }
}

void RenderPipelineDepthOITStage::execute(Device* device, scene::Scene::SceneData& state)
{
    DEBUG_MARKER_SCOPE(state.m_renderState.m_cmdList, "Transparency", color::colorrgbaf::GREEN);

    {
        DEBUG_MARKER_SCOPE(state.m_renderState.m_cmdList, "Transparency.Clear", color::colorrgbaf::GREEN);
        state.m_renderState.m_cmdList->clear(m_depthSamples, { 0.0, 0.0, 0.0, 0.0 });
    }

    executeStochasticTotalAlpha_Pass1(device, state);
    executeStochasticDepth_Pass2(device, state);
    executeStochasticAccumulateColor_Pass3(device, state);
    executeStochasticComposite_Pass4(device, state);
}

void RenderPipelineDepthOITStage::executeStochasticTotalAlpha_Pass1(Device* device, scene::Scene::SceneData& state)
{
    DEBUG_MARKER_SCOPE(state.m_renderState.m_cmdList, "Transparency.TotalAlpha", color::colorrgbaf::GREEN);

    state.m_renderState.m_cmdList->beginRenderTarget(*m_rt[Pass::StochasticTotalAlpha]);
    state.m_renderState.m_cmdList->setViewport({ 0.f, 0.f, (f32)state.m_viewportState.m_viewpotSize._width, (f32)state.m_viewportState.m_viewpotSize._height });
    state.m_renderState.m_cmdList->setScissor({ 0.f, 0.f, (f32)state.m_viewportState.m_viewpotSize._width, (f32)state.m_viewportState.m_viewpotSize._height });
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

void RenderPipelineDepthOITStage::executeStochasticDepth_Pass2(Device* device, scene::Scene::SceneData& state)
{
    DEBUG_MARKER_SCOPE(state.m_renderState.m_cmdList, "Transparency.Depth", color::colorrgbaf::GREEN);

    ObjectHandle noise_blue = state.m_globalResources.get("noise_blue");
    ASSERT(noise_blue.isValid(), "must be valid");
    renderer::Texture2D* noise_blue_texture = objectFromHandle<renderer::Texture2D>(noise_blue);

    state.m_renderState.m_cmdList->beginRenderTarget(*m_rt[Pass::StochasticDepth]);
    state.m_renderState.m_cmdList->setViewport({ 0.f, 0.f, (f32)state.m_viewportState.m_viewpotSize._width, (f32)state.m_viewportState.m_viewpotSize._height });
    state.m_renderState.m_cmdList->setScissor({ 0.f, 0.f, (f32)state.m_viewportState.m_viewpotSize._width, (f32)state.m_viewportState.m_viewpotSize._height });
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
                    renderer::Descriptor(renderer::TextureView(draw.m_normals), 4),
                    renderer::Descriptor(renderer::TextureView(draw.m_material), 5),
                    renderer::Descriptor(renderer::TextureView(noise_blue_texture), 6),
                    renderer::Descriptor(m_depthSamples, 7),
                });

            renderer::GeometryBufferDesc desc(draw.m_IdxBuffer, 0, draw.m_VtxBuffer, 0, sizeof(VertexFormatStandard), 0);
            state.m_renderState.m_cmdList->drawIndexed(desc, 0, draw.m_IdxBuffer->getIndicesCount(), 0, 0, 1);
        }
    }
    state.m_renderState.m_cmdList->endRenderTarget();
}

void RenderPipelineDepthOITStage::executeStochasticAccumulateColor_Pass3(Device* device, scene::Scene::SceneData& state)
{
    DEBUG_MARKER_SCOPE(state.m_renderState.m_cmdList, "Transparency.AccumulateColor", color::colorrgbaf::GREEN);

    state.m_renderState.m_cmdList->beginRenderTarget(*m_rt[Pass::StochasticTotalAccumulateColor]);
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
                    renderer::Descriptor(m_depthSamples, 7),
                });

            renderer::GeometryBufferDesc desc(draw.m_IdxBuffer, 0, draw.m_VtxBuffer, 0, sizeof(VertexFormatStandard), 0);
            state.m_renderState.m_cmdList->drawIndexed(desc, 0, draw.m_IdxBuffer->getIndicesCount(), 0, 0, 1);
        }
    }
    state.m_renderState.m_cmdList->endRenderTarget();
}

void RenderPipelineDepthOITStage::executeStochasticComposite_Pass4(Device* device, scene::Scene::SceneData& state)
{
    DEBUG_MARKER_SCOPE(state.m_renderState.m_cmdList, "Transparency.Composite", color::colorrgbaf::GREEN);

    state.m_renderState.m_cmdList->beginRenderTarget(*m_rt[Pass::StochasticComposite]);
    state.m_renderState.m_cmdList->setPipelineState(*m_pipeline[Pass::StochasticComposite]);

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
            renderer::Descriptor(renderer::TextureView(m_rt[Pass::StochasticTotalAccumulateColor]->getColorTexture<renderer::Texture2D>(0), 0, 0), 4),
            renderer::Descriptor(renderer::TextureView(m_rt[Pass::StochasticTotalAlpha]->getColorTexture<renderer::Texture2D>(0), 0, 0), 5)
        });

    state.m_renderState.m_cmdList->draw(renderer::GeometryBufferDesc(), 0, 3, 0, 1);
    state.m_renderState.m_cmdList->endRenderTarget();

    state.m_globalResources.bind("render_target", m_rt[Pass::StochasticComposite]->getColorTexture<renderer::Texture2D>(0));
}

void RenderPipelineDepthOITStage::changed(Device* device, scene::Scene::SceneData& data)
{
    if (!m_rt[Pass::StochasticComposite])
    {
        createRenderTarget(device, data);
    }
    else if (m_rt[Pass::StochasticComposite]->getRenderArea() != data.m_viewportState.m_viewpotSize)
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
    m_rt[Pass::StochasticTotalAlpha] = new renderer::RenderTargetState(device, data.m_viewportState.m_viewpotSize, 1);
    m_rt[Pass::StochasticTotalAlpha]->setColorTexture(0, new renderer::Texture2D(device, renderer::TextureUsage::TextureUsage_Attachment | renderer::TextureUsage::TextureUsage_Sampled,
        renderer::Format::Format_R32_SFloat, data.m_viewportState.m_viewpotSize, renderer::TextureSamples::TextureSamples_x1, "total_alpha"),
        {
            renderer::RenderTargetLoadOp::LoadOp_Clear, renderer::RenderTargetStoreOp::StoreOp_Store, color::Color(0.0f, 0.0f, 0.0f, 0.0f)
        },
        {
            renderer::TransitionOp::TransitionOp_Undefined, renderer::TransitionOp::TransitionOp_ColorAttachment
        });

    m_rt[Pass::StochasticTotalAlpha]->setDepthStencilTexture(depth_stencil_texture,
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
    m_rt[Pass::StochasticDepth] = new renderer::RenderTargetState(device, data.m_viewportState.m_viewpotSize, 0);
    m_rt[Pass::StochasticDepth]->setDepthStencilTexture(depth_stencil_texture,
        {
            renderer::RenderTargetLoadOp::LoadOp_Load, renderer::RenderTargetStoreOp::StoreOp_Store, 0.0f
        },
        {
            renderer::RenderTargetLoadOp::LoadOp_Load, renderer::RenderTargetStoreOp::StoreOp_Store, 0U
        },
        {
            renderer::TransitionOp::TransitionOp_DepthStencilAttachment, renderer::TransitionOp::TransitionOp_DepthStencilAttachment
        });

    //3
    m_rt[Pass::StochasticTotalAccumulateColor] = new renderer::RenderTargetState(device, data.m_viewportState.m_viewpotSize, 1);
    m_rt[Pass::StochasticTotalAccumulateColor]->setColorTexture(0, new renderer::Texture2D(device, renderer::TextureUsage::TextureUsage_Attachment | renderer::TextureUsage::TextureUsage_Sampled,
        renderer::Format::Format_R32G32B32A32_SFloat, data.m_viewportState.m_viewpotSize, renderer::TextureSamples::TextureSamples_x1, "st_accumulate_color"),
        {
            renderer::RenderTargetLoadOp::LoadOp_Clear, renderer::RenderTargetStoreOp::StoreOp_Store, color::Color(0.0f, 0.0f, 0.0f, 0.0f)
        },
        {
            renderer::TransitionOp::TransitionOp_Undefined, renderer::TransitionOp::TransitionOp_ColorAttachment
        });

    m_rt[Pass::StochasticTotalAccumulateColor]->setDepthStencilTexture(depth_stencil_texture,
        {
            renderer::RenderTargetLoadOp::LoadOp_Load, renderer::RenderTargetStoreOp::StoreOp_Store, 0.0f
        },
        {
            renderer::RenderTargetLoadOp::LoadOp_Load, renderer::RenderTargetStoreOp::StoreOp_Store, 0U
        },
        {
            renderer::TransitionOp::TransitionOp_DepthStencilAttachment, renderer::TransitionOp::TransitionOp_DepthStencilAttachment
        });

    //4
    m_rt[Pass::StochasticComposite] = new renderer::RenderTargetState(device, data.m_viewportState.m_viewpotSize, 1);
    renderer::Texture2D* final = new renderer::Texture2D(device, renderer::TextureUsage::TextureUsage_Attachment | renderer::TextureUsage::TextureUsage_Sampled,
        renderer::Format::Format_R32G32B32A32_SFloat, data.m_viewportState.m_viewpotSize, renderer::TextureSamples::TextureSamples_x1, "st_final");
    m_rt[Pass::StochasticComposite]->setColorTexture(0, final,
        {
            renderer::RenderTargetLoadOp::LoadOp_DontCare, renderer::RenderTargetStoreOp::StoreOp_Store, color::Color(0.0f)
        },
        {
            renderer::TransitionOp::TransitionOp_Undefined, renderer::TransitionOp::TransitionOp_ColorAttachment
        });

    data.m_globalResources.bind("full_composite", final);
}

void RenderPipelineDepthOITStage::destroyRenderTarget(Device* device, scene::Scene::SceneData& data)
{
    {
        ASSERT(m_rt[Pass::StochasticTotalAlpha], "must be valid");
        renderer::Texture2D* r32float = m_rt[Pass::StochasticTotalAlpha]->getColorTexture<renderer::Texture2D>(0);
        delete r32float;

        delete m_rt[Pass::StochasticTotalAlpha];
        m_rt[Pass::StochasticTotalAlpha] = nullptr;
    }

    {
        ASSERT(m_rt[Pass::StochasticDepth], "must be valid");
        delete m_rt[Pass::StochasticDepth];
        m_rt[Pass::StochasticDepth] = nullptr;
    }

    {
        ASSERT(m_rt[Pass::StochasticTotalAccumulateColor], "must be valid");
        renderer::Texture2D* color = m_rt[Pass::StochasticTotalAccumulateColor]->getColorTexture<renderer::Texture2D>(0);
        delete color;

        delete m_rt[Pass::StochasticTotalAccumulateColor];
        m_rt[Pass::StochasticTotalAccumulateColor] = nullptr;
    }

    {
        ASSERT(m_rt[Pass::StochasticComposite], "must be valid");
        renderer::Texture2D* final = m_rt[Pass::StochasticComposite]->getColorTexture<renderer::Texture2D>(0);
        delete final;

        delete  m_rt[Pass::StochasticComposite];
        m_rt[Pass::StochasticComposite] = nullptr;
    }
}

} //namespace renderer
} //namespace v3d