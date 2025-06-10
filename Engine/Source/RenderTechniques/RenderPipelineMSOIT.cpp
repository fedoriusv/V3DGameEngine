#include "RenderPipelineMSOIT.h"

#include "Resource/ResourceManager.h"

#include "Resource/Loader/AssetSourceFileLoader.h"
#include "Resource/Loader/ShaderSourceFileLoader.h"
#include "Resource/Loader/ModelFileLoader.h"

namespace v3d
{
namespace renderer
{

RenderPipelineMSOITStage::RenderPipelineMSOITStage(RenderTechnique* technique) noexcept
    : RenderPipelineStage(technique, "transparency")
{
}

RenderPipelineMSOITStage::~RenderPipelineMSOITStage()
{
}

void RenderPipelineMSOITStage::create(Device* device, scene::Scene::SceneData& state)
{
    createRenderTarget(device, state);

    {
        const renderer::VertexShader* vertShader = resource::ResourceManager::getInstance()->loadShader<renderer::VertexShader, resource::ShaderSourceFileLoader>(device,
            "gbuffer.hlsl", "gbuffer_standard_vs", {}, {}/*, resource::ShaderCompileFlag::ShaderCompile_UseDXCompilerForSpirV*/);
        const renderer::FragmentShader* fragShader = resource::ResourceManager::getInstance()->loadShader<renderer::FragmentShader, resource::ShaderSourceFileLoader>(device,
            "transparency_msoit.hlsl", "msoit_ps", {}, {}/*, resource::ShaderCompileFlag::ShaderCompile_UseDXCompilerForSpirV*/);

        renderer::GraphicsPipelineState* pipeline = new renderer::GraphicsPipelineState(
            device, VertexFormatStandardDesc, m_msaaPass->getRenderPassDesc(), new renderer::ShaderProgram(device, vertShader, fragShader), "msoit_pipeline");

        pipeline->setPrimitiveTopology(renderer::PrimitiveTopology::PrimitiveTopology_TriangleList);
        pipeline->setFrontFace(renderer::FrontFace::FrontFace_Clockwise);
        pipeline->setCullMode(renderer::CullMode::CullMode_None);
        pipeline->setColorMask(renderer::ColorMask::ColorMask_All);
#if ENABLE_REVERSED_Z
        pipeline->setDepthCompareOp(renderer::CompareOperation::CompareOp_GreaterOrEqual);
#else
        pipeline->setDepthCompareOp(renderer::CompareOperation::CompareOp_Less);
#endif
        pipeline->setDepthTest(false); //is missing here!! need MSAA
        pipeline->setDepthWrite(false);

        m_pipeline[Pass::MultiSamplePass] = pipeline;
    }

    {
        const renderer::VertexShader* vertShader = resource::ResourceManager::getInstance()->loadShader<renderer::VertexShader, resource::ShaderSourceFileLoader>(device,
            "offscreen.hlsl", "offscreen_vs", {}, {}/*, resource::ShaderCompileFlag::ShaderCompile_UseDXCompilerForSpirV*/);
        const renderer::FragmentShader* fragShader = resource::ResourceManager::getInstance()->loadShader<renderer::FragmentShader, resource::ShaderSourceFileLoader>(device,
            "transparency_msoit.hlsl", "msoit_resolve_ps", {}, {}/*, resource::ShaderCompileFlag::ShaderCompile_UseDXCompilerForSpirV*/);

        renderer::GraphicsPipelineState* pipeline = new renderer::GraphicsPipelineState(
            device, renderer::VertexInputAttributeDesc(), m_resolvePass->getRenderPassDesc(), new renderer::ShaderProgram(device, vertShader, fragShader), "msoit_resolve_pipeline");

        pipeline->setPrimitiveTopology(renderer::PrimitiveTopology::PrimitiveTopology_TriangleList);
        pipeline->setFrontFace(renderer::FrontFace::FrontFace_Clockwise);
        pipeline->setCullMode(renderer::CullMode::CullMode_Back);
        pipeline->setColorMask(renderer::ColorMask::ColorMask_All);
        pipeline->setDepthCompareOp(renderer::CompareOperation::CompareOp_Always);
        pipeline->setDepthWrite(false);
        pipeline->setDepthTest(false);

        m_pipeline[Pass::ResolvePass] = pipeline;
    }

    m_sampler = new renderer::SamplerState(device, renderer::SamplerFilter::SamplerFilter_Trilinear, renderer::SamplerAnisotropic::SamplerAnisotropic_4x);
    m_sampler->setWrap(renderer::SamplerWrap::TextureWrap_MirroredRepeat);
}

void RenderPipelineMSOITStage::destroy(Device* device, scene::Scene::SceneData& state)
{
}

void RenderPipelineMSOITStage::prepare(Device* device, scene::Scene::SceneData& state)
{
}

void RenderPipelineMSOITStage::execute(Device* device, scene::Scene::SceneData& state)
{
    DEBUG_MARKER_SCOPE(state.m_renderState.m_cmdList, "Transparency", color::colorrgbaf::GREEN);

    //pass 1
    {
        state.m_renderState.m_cmdList->beginRenderTarget(*m_msaaPass);
        state.m_renderState.m_cmdList->setViewport({ 0.f, 0.f, (f32)state.m_viewportState.m_viewpotSize._width, (f32)state.m_viewportState.m_viewpotSize._height });
        state.m_renderState.m_cmdList->setScissor({ 0.f, 0.f, (f32)state.m_viewportState.m_viewpotSize._width, (f32)state.m_viewportState.m_viewpotSize._height });
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

    //2 resolve
    {
        state.m_renderState.m_cmdList->beginRenderTarget(*m_resolvePass);
        state.m_renderState.m_cmdList->setViewport({ 0.f, 0.f, (f32)state.m_viewportState.m_viewpotSize._width, (f32)state.m_viewportState.m_viewpotSize._height });
        state.m_renderState.m_cmdList->setScissor({ 0.f, 0.f, (f32)state.m_viewportState.m_viewpotSize._width, (f32)state.m_viewportState.m_viewpotSize._height });
        state.m_renderState.m_cmdList->setPipelineState(*m_pipeline[Pass::ResolvePass]);

        state.m_renderState.m_cmdList->bindDescriptorSet(0,
            {
                renderer::Descriptor(renderer::Descriptor::ConstantBuffer{ &state.m_viewportState._viewportBuffer, 0, sizeof(state.m_viewportState._viewportBuffer)}, 0)
            });

        ObjectHandle baseColor = state.m_globalResources.get("render_target");
        ASSERT(baseColor.isValid(), "must be valid");
        renderer::Texture2D* baseColorTexture = objectFromHandle<renderer::Texture2D>(baseColor);

        state.m_renderState.m_cmdList->bindDescriptorSet(1,
            {
                renderer::Descriptor(m_sampler, 2),
                renderer::Descriptor(renderer::TextureView(baseColorTexture, 0, 0), 3),
                renderer::Descriptor(renderer::TextureView(m_msaaPass->getColorTexture<renderer::Texture2D>(0), 0, 0), 6),
            });

        state.m_renderState.m_cmdList->draw(renderer::GeometryBufferDesc(), 0, 3, 0, 1);
        state.m_renderState.m_cmdList->endRenderTarget();
    }

    state.m_globalResources.bind("render_target", m_resolvePass->getColorTexture<renderer::Texture2D>(0));
}

void RenderPipelineMSOITStage::changed(Device* device, scene::Scene::SceneData& data)
{
    if (!m_resolvePass)
    {
        createRenderTarget(device, data);
    }
    else if (m_resolvePass->getRenderArea() != data.m_viewportState.m_viewpotSize)
    {
        destroyRenderTarget(device, data);
        createRenderTarget(device, data);
    }
}

void RenderPipelineMSOITStage::createRenderTarget(Device* device, scene::Scene::SceneData& data)
{
    //1
    m_msaaPass = new renderer::RenderTargetState(device, data.m_viewportState.m_viewpotSize, 1);

    renderer::Texture2D* msTexture = new renderer::Texture2D(device, renderer::TextureUsage::TextureUsage_Attachment | renderer::TextureUsage::TextureUsage_Sampled,
        renderer::Format::Format_R16G16B16A16_SFloat, data.m_viewportState.m_viewpotSize, renderer::TextureSamples::TextureSamples_x4, "msoit");
    m_msaaPass->setColorTexture(0, msTexture,
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
    //m_msaaPass->setDepthStencilTexture(depth_stencil_texture,
    //    {
    //        renderer::RenderTargetLoadOp::LoadOp_Load, renderer::RenderTargetStoreOp::StoreOp_Store, 0.0f
    //    },
    //    {
    //        renderer::RenderTargetLoadOp::LoadOp_Load, renderer::RenderTargetStoreOp::StoreOp_Store, 0U
    //    },
    //    {
    //        renderer::TransitionOp::TransitionOp_DepthStencilAttachment, renderer::TransitionOp::TransitionOp_DepthStencilAttachment
    //    });

    //2
    m_resolvePass = new renderer::RenderTargetState(device, data.m_viewportState.m_viewpotSize, 1);

    renderer::Texture2D* texture = new renderer::Texture2D(device, renderer::TextureUsage::TextureUsage_Attachment | renderer::TextureUsage::TextureUsage_Sampled,
        renderer::Format::Format_R16G16B16A16_SFloat, data.m_viewportState.m_viewpotSize, renderer::TextureSamples::TextureSamples_x1, "msoit_resolve");
    m_resolvePass->setColorTexture(0, texture,
        {
            renderer::RenderTargetLoadOp::LoadOp_DontCare, renderer::RenderTargetStoreOp::StoreOp_Store, color::Color(0.0f)
        },
        {
            renderer::TransitionOp::TransitionOp_Undefined, renderer::TransitionOp::TransitionOp_ColorAttachment
        }
    );
}

void RenderPipelineMSOITStage::destroyRenderTarget(Device* device, scene::Scene::SceneData& data)
{
    ASSERT(m_msaaPass, "must be valid");
    renderer::Texture2D* msTexture = m_msaaPass->getColorTexture<renderer::Texture2D>(0);
    delete msTexture;

    delete m_msaaPass;
    m_msaaPass = nullptr;


    ASSERT(m_resolvePass, "must be valid");
    renderer::Texture2D* texture = m_resolvePass->getColorTexture<renderer::Texture2D>(0);
    delete texture;

    delete m_resolvePass;
    m_resolvePass = nullptr;
}

} //namespace renderer
} //namespace v3d