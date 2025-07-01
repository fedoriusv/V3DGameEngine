#include "RenderPipelineSOIT.h"

#include "Resource/ResourceManager.h"

#include "Resource/Loader/AssetSourceFileLoader.h"
#include "Resource/Loader/ShaderSourceFileLoader.h"
#include "Resource/Loader/ModelFileLoader.h"

namespace v3d
{
namespace renderer
{

RenderPipelineSOITStage::RenderPipelineSOITStage(RenderTechnique* technique) noexcept
    : RenderPipelineStage(technique, "transparency")
    , m_colorSamples(nullptr)
{
}

RenderPipelineSOITStage::~RenderPipelineSOITStage()
{
}

void RenderPipelineSOITStage::create(Device* device, scene::SceneData& state)
{
    createRenderTarget(device, state);

    {
        const renderer::VertexShader* vertShader = resource::ResourceManager::getInstance()->loadShader<renderer::VertexShader, resource::ShaderSourceFileLoader>(device,
            "gbuffer.hlsl", "gbuffer_standard_vs", {}, {}/*, resource::ShaderCompileFlag::ShaderCompile_UseDXCompilerForSpirV*/);
        const renderer::FragmentShader* fragShader = resource::ResourceManager::getInstance()->loadShader<renderer::FragmentShader, resource::ShaderSourceFileLoader>(device,
            "transparency_soit_mrt.hlsl", "msoit_mrt_ps", {}, {}/*, resource::ShaderCompileFlag::ShaderCompile_UseDXCompilerForSpirV*/);

        renderer::GraphicsPipelineState* pipeline = new renderer::GraphicsPipelineState(
            device, VertexFormatStandardDesc, m_rt[Pass::MultiSamplePass]->getRenderPassDesc(), new renderer::ShaderProgram(device, vertShader, fragShader), "soit_pipeline");

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

        m_pipeline[Pass::MultiSamplePass] = pipeline;
    }

    {
        const renderer::VertexShader* vertShader = resource::ResourceManager::getInstance()->loadShader<renderer::VertexShader, resource::ShaderSourceFileLoader>(device,
            "offscreen.hlsl", "offscreen_vs", {}, {}/*, resource::ShaderCompileFlag::ShaderCompile_UseDXCompilerForSpirV*/);
        const renderer::FragmentShader* fragShader = resource::ResourceManager::getInstance()->loadShader<renderer::FragmentShader, resource::ShaderSourceFileLoader>(device,
            "transparency_soit_mrt.hlsl", "msoit_resolve_ps", {}, {}/*, resource::ShaderCompileFlag::ShaderCompile_UseDXCompilerForSpirV*/);

        renderer::GraphicsPipelineState* pipeline = new renderer::GraphicsPipelineState(
            device, renderer::VertexInputAttributeDesc(), m_rt[Pass::ResolvePass]->getRenderPassDesc(), new renderer::ShaderProgram(device, vertShader, fragShader), "soit_resolve_pipeline");

        pipeline->setPrimitiveTopology(renderer::PrimitiveTopology::PrimitiveTopology_TriangleList);
        pipeline->setFrontFace(renderer::FrontFace::FrontFace_Clockwise);
        pipeline->setCullMode(renderer::CullMode::CullMode_Back);
        pipeline->setDepthCompareOp(renderer::CompareOperation::CompareOp_Always);
        pipeline->setDepthWrite(false);
        pipeline->setDepthTest(false);

        pipeline->setBlendEnable(0, false);
        pipeline->setColorMask(0, renderer::ColorMask::ColorMask_All);
        pipeline->setColorBlendFactor(0, renderer::BlendFactor::BlendFactor_One, renderer::BlendFactor::BlendFactor_OneMinusSrcAlpha);
        pipeline->setColorBlendOp(0, renderer::BlendOperation::BlendOp_Add);

        m_pipeline[Pass::ResolvePass] = pipeline;
    }

    m_sampler = new renderer::SamplerState(device, renderer::SamplerFilter::SamplerFilter_Trilinear, renderer::SamplerAnisotropic::SamplerAnisotropic_4x);
    m_sampler->setWrap(renderer::SamplerWrap::TextureWrap_MirroredRepeat);
}

void RenderPipelineSOITStage::destroy(Device* device, scene::SceneData& state)
{
}

void RenderPipelineSOITStage::prepare(Device* device, scene::SceneData& state)
{
    u32 bufferSize = sizeof(math::uint4) * state.m_viewportState._viewpotSize._width * state.m_viewportState._viewpotSize._height;
    if (!m_colorSamples || m_colorSamples->getSize() != bufferSize)
    {
        delete m_colorSamples;
        m_colorSamples = new renderer::UnorderedAccessBuffer(device, BufferUsage::Buffer_GPUOnly, bufferSize, "color_samples");
    }

    if (!m_rt[Pass::ResolvePass])
    {
        createRenderTarget(device, state);
    }
    else if (m_rt[Pass::ResolvePass]->getRenderArea() != state.m_viewportState._viewpotSize)
    {
        destroyRenderTarget(device, state);
        createRenderTarget(device, state);
    }
}

void RenderPipelineSOITStage::execute(Device* device, scene::SceneData& state)
{
    DEBUG_MARKER_SCOPE(state.m_renderState.m_cmdList, "Transparency", color::colorrgbaf::GREEN);

    {
        DEBUG_MARKER_SCOPE(state.m_renderState.m_cmdList, "Transparency.Clear", color::colorrgbaf::GREEN);
        state.m_renderState.m_cmdList->clear(m_colorSamples, 0);
    }

    //pass 1
    {
        state.m_renderState.m_cmdList->beginRenderTarget(*m_rt[Pass::MultiSamplePass]);
        state.m_renderState.m_cmdList->setViewport({ 0.f, 0.f, (f32)state.m_viewportState._viewpotSize._width, (f32)state.m_viewportState._viewpotSize._height });
        state.m_renderState.m_cmdList->setScissor({ 0.f, 0.f, (f32)state.m_viewportState._viewpotSize._width, (f32)state.m_viewportState._viewpotSize._height });
        state.m_renderState.m_cmdList->setPipelineState(*m_pipeline[Pass::MultiSamplePass]);

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


                ObjectHandle noise_blue = state.m_globalResources.get("noise_blue");
                ASSERT(noise_blue.isValid(), "must be valid");
                renderer::Texture2D* noise_blue_texture = objectFromHandle<renderer::Texture2D>(noise_blue);

                state.m_renderState.m_cmdList->bindDescriptorSet(1,
                    {
                        renderer::Descriptor(renderer::Descriptor::ConstantBuffer{ &constantBuffer, 0, sizeof(constantBuffer)}, 1),
                        renderer::Descriptor(draw.m_sampler, 2),
                        renderer::Descriptor(renderer::TextureView(draw.m_albedo), 3),
                        renderer::Descriptor(renderer::TextureView(draw.m_normals), 4),
                        renderer::Descriptor(renderer::TextureView(draw.m_material), 5),
                        renderer::Descriptor(renderer::TextureView(noise_blue_texture), 6),
                        renderer::Descriptor(m_colorSamples, 7),
                    });

                renderer::GeometryBufferDesc desc(draw.m_IdxBuffer, 0, draw.m_VtxBuffer, 0, sizeof(VertexFormatStandard), 0);
                state.m_renderState.m_cmdList->drawIndexed(desc, 0, draw.m_IdxBuffer->getIndicesCount(), 0, 0, 1);
            }
        }
        state.m_renderState.m_cmdList->endRenderTarget();
    }

    //2 resolve
    {
        ObjectHandle baseColor = state.m_globalResources.get("render_target");
        ASSERT(baseColor.isValid(), "must be valid");
        renderer::Texture2D* baseColorTexture = objectFromHandle<renderer::Texture2D>(baseColor);

        state.m_renderState.m_cmdList->beginRenderTarget(*m_rt[Pass::ResolvePass]);
        state.m_renderState.m_cmdList->setPipelineState(*m_pipeline[Pass::ResolvePass]);

        state.m_renderState.m_cmdList->bindDescriptorSet(0,
            {
                renderer::Descriptor(renderer::Descriptor::ConstantBuffer{ &state.m_viewportState._viewportBuffer, 0, sizeof(state.m_viewportState._viewportBuffer)}, 0)
            });

        state.m_renderState.m_cmdList->bindDescriptorSet(1,
            {
                renderer::Descriptor(m_sampler, 2),
                renderer::Descriptor(renderer::TextureView(baseColorTexture, 0, 0), 3),
                renderer::Descriptor(renderer::TextureView(m_rt[Pass::MultiSamplePass]->getColorTexture<renderer::Texture2D>(0), 0, 0), 4),
                renderer::Descriptor(m_colorSamples, 7),
            });

        state.m_renderState.m_cmdList->draw(renderer::GeometryBufferDesc(), 0, 3, 0, 1);
        state.m_renderState.m_cmdList->endRenderTarget();

        state.m_globalResources.bind("render_target", m_rt[Pass::ResolvePass]->getColorTexture<renderer::Texture2D>(0));
    }
}

void RenderPipelineSOITStage::createRenderTarget(Device* device, scene::SceneData& data)
{
    //1
    m_rt[Pass::MultiSamplePass] = new renderer::RenderTargetState(device, data.m_viewportState._viewpotSize, 1);

    renderer::Texture2D* texture = new renderer::Texture2D(device, renderer::TextureUsage::TextureUsage_Attachment | renderer::TextureUsage::TextureUsage_Sampled,
        renderer::Format::Format_R32_SFloat, data.m_viewportState._viewpotSize, renderer::TextureSamples::TextureSamples_x1, "total_alpha");
    m_rt[Pass::MultiSamplePass]->setColorTexture(0, texture,
        {
            renderer::RenderTargetLoadOp::LoadOp_Clear, renderer::RenderTargetStoreOp::StoreOp_Store, color::Color(0.0f, 0.0f, 0.0f, 0.0f)
        },
            {
                renderer::TransitionOp::TransitionOp_Undefined, renderer::TransitionOp::TransitionOp_ColorAttachment
            }
    );

    ObjectHandle depth_stencil = data.m_globalResources.get("depth_stencil");
    ASSERT(depth_stencil.isValid(), "must be valid");
    renderer::Texture2D* depth_stencil_texture = objectFromHandle<renderer::Texture2D>(depth_stencil);
    m_rt[Pass::MultiSamplePass]->setDepthStencilTexture(depth_stencil_texture,
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
    m_rt[Pass::ResolvePass] = new renderer::RenderTargetState(device, data.m_viewportState._viewpotSize, 1);
    renderer::Texture2D* final = new renderer::Texture2D(device, renderer::TextureUsage::TextureUsage_Attachment | renderer::TextureUsage::TextureUsage_Sampled,
        renderer::Format::Format_R32G32B32A32_SFloat, data.m_viewportState._viewpotSize, renderer::TextureSamples::TextureSamples_x1, "st_final");
    m_rt[Pass::ResolvePass]->setColorTexture(0, final,
        {
            renderer::RenderTargetLoadOp::LoadOp_DontCare, renderer::RenderTargetStoreOp::StoreOp_Store, color::Color(0.0f)
        },
        {
            renderer::TransitionOp::TransitionOp_Undefined, renderer::TransitionOp::TransitionOp_ColorAttachment
        });

    data.m_globalResources.bind("full_composite", final);
}

void RenderPipelineSOITStage::destroyRenderTarget(Device* device, scene::SceneData& data)
{
    {
        ASSERT(m_rt[Pass::MultiSamplePass], "must be valid");
        renderer::Texture2D* color0 = m_rt[Pass::MultiSamplePass]->getColorTexture<renderer::Texture2D>(0);
        delete color0;

        delete  m_rt[Pass::MultiSamplePass];
        m_rt[Pass::MultiSamplePass] = nullptr;
    }

    {
        ASSERT(m_rt[Pass::ResolvePass], "must be valid");
        renderer::Texture2D* color0 = m_rt[Pass::ResolvePass]->getColorTexture<renderer::Texture2D>(0);
        delete color0;

        delete  m_rt[Pass::ResolvePass];
        m_rt[Pass::ResolvePass] = nullptr;
    }
}

} //namespace renderer
} //namespace v3d