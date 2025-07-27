#include "RenderPipelineMBOIT.h"

#include "Resource/ResourceManager.h"

#include "Resource/Loader/AssetSourceFileLoader.h"
#include "Resource/Loader/ShaderSourceFileLoader.h"
#include "Resource/Loader/ModelFileLoader.h"

#include "Scene/ModelHandler.h"
#include "Scene/Geometry/Mesh.h"

namespace v3d
{
namespace renderer
{

RenderPipelineMBOITStage::RenderPipelineMBOITStage(RenderTechnique* technique) noexcept
    : RenderPipelineStage(technique, "MBOIT")
{
}

RenderPipelineMBOITStage::~RenderPipelineMBOITStage()
{
}

void RenderPipelineMBOITStage::create(Device* device, scene::SceneData& scene, scene::FrameData& frame)
{
    createRenderTarget(device, scene);

    //pass 1
    {
        const renderer::VertexShader* vertShader = resource::ResourceManager::getInstance()->loadShader<renderer::VertexShader, resource::ShaderSourceFileLoader>(device,
            "gbuffer.hlsl", "gbuffer_standard_vs", {}, {}/*, resource::ShaderCompileFlag::ShaderCompile_UseDXCompilerForSpirV*/);
        const renderer::FragmentShader* fragShader = resource::ResourceManager::getInstance()->loadShader<renderer::FragmentShader, resource::ShaderSourceFileLoader>(device,
            "transparency_mboit.hlsl", "mboit_pass1_ps", {}, {}/*, resource::ShaderCompileFlag::ShaderCompile_UseDXCompilerForSpirV*/);

        renderer::GraphicsPipelineState* pipeline = V3D_NEW(renderer::GraphicsPipelineState, memory::MemoryLabel::MemoryGame)(device, VertexFormatStandardDesc, m_rt[Pass::MBOIT_Pass1]->getRenderPassDesc(), 
            V3D_NEW(renderer::ShaderProgram, memory::MemoryLabel::MemoryGame)(device, vertShader, fragShader), "mboit_pass1_ps");

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

        renderer::GraphicsPipelineState* pipeline = V3D_NEW(renderer::GraphicsPipelineState, memory::MemoryLabel::MemoryGame)(device, VertexFormatStandardDesc, m_rt[Pass::MBOIT_Pass2]->getRenderPassDesc(), 
            V3D_NEW(renderer::ShaderProgram, memory::MemoryLabel::MemoryGame)(device, vertShader, fragShader), "mboit_pass1_ps");

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

        renderer::GraphicsPipelineState* pipeline = V3D_NEW(renderer::GraphicsPipelineState, memory::MemoryLabel::MemoryGame)(device, renderer::VertexInputAttributeDesc(), desc, 
            V3D_NEW(renderer::ShaderProgram, memory::MemoryLabel::MemoryGame)(device, vertShader, fragShader), "mboit_resolve_pipeline");

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
}

void RenderPipelineMBOITStage::destroy(Device* device, scene::SceneData& scene, scene::FrameData& frame)
{
    destroyRenderTarget(device, scene);

    for (u32 i = 0; i < Pass::Count; ++i)
    {
        const renderer::ShaderProgram* program = m_pipeline[i]->getShaderProgram();
        V3D_DELETE(program, memory::MemoryLabel::MemoryGame);

        V3D_DELETE(m_pipeline[i], memory::MemoryLabel::MemoryGame);
        m_pipeline[i] = nullptr;
    }
}

void RenderPipelineMBOITStage::prepare(Device* device, scene::SceneData& scene, scene::FrameData& frame)
{
    if (!m_rt[Pass::MBOIT_Pass1])
    {
        createRenderTarget(device, scene);
    }
    else if (m_rt[Pass::MBOIT_Pass1]->getRenderArea() != scene.m_viewportState._viewpotSize)
    {
        destroyRenderTarget(device, scene);
        createRenderTarget(device, scene);
    }
}

void RenderPipelineMBOITStage::execute(Device* device, scene::SceneData& scene, scene::FrameData& frame)
{
    renderer::CmdListRender* cmdList = scene.m_renderState.m_cmdList;
    scene::ViewportState& viewportState = scene.m_viewportState;

    DEBUG_MARKER_SCOPE(cmdList, "Transparency", color::colorrgbaf::GREEN);

    //pass 1
    {
        cmdList->beginRenderTarget(*m_rt[Pass::MBOIT_Pass1]);
        cmdList->setViewport({ 0.f, 0.f, (f32)viewportState._viewpotSize._width, (f32)viewportState._viewpotSize._height });
        cmdList->setScissor({ 0.f, 0.f, (f32)viewportState._viewpotSize._width, (f32)viewportState._viewpotSize._height });
        cmdList->setStencilRef(0);
        cmdList->setPipelineState(*m_pipeline[Pass::MBOIT_Pass1]);

        cmdList->bindDescriptorSet(0,
            {
                renderer::Descriptor(renderer::Descriptor::ConstantBuffer{ &viewportState._viewportBuffer, 0, sizeof(viewportState._viewportBuffer)}, 0)
            });

        for (auto& list : scene.m_lists[toEnumType(scene::MaterialType::Transparency)])
        {
            scene::DrawInstanceDataState& instance = list->_instance;
            const scene::Mesh& mesh = *static_cast<scene::Mesh*>(list->_object);

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
            constantBuffer.modelMatrix = instance._transform.getTransform();
            constantBuffer.prevModelMatrix = instance._prevTransform.getTransform();
            constantBuffer.normalMatrix = constantBuffer.modelMatrix.getTransposed();
            constantBuffer.tint = instance._material._tint;
            constantBuffer.objectID = instance._objectID;

            scene.m_renderState.m_cmdList->bindDescriptorSet(1,
                {
                    renderer::Descriptor(renderer::Descriptor::ConstantBuffer{ &constantBuffer, 0, sizeof(constantBuffer)}, 1),
                    renderer::Descriptor(instance._material._sampler, 2),
                    renderer::Descriptor(renderer::TextureView(instance._material._baseColor), 3),
                    renderer::Descriptor(renderer::TextureView(instance._material._normals), 4),
                    renderer::Descriptor(renderer::TextureView(instance._material._metalness), 5),
                    renderer::Descriptor(renderer::TextureView(instance._material._roughness), 6),
                });

            DEBUG_MARKER_SCOPE(cmdList, std::format("Object {}, pipeline {}", instance._objectID, m_pipeline[instance._pipelineID]->getName()), color::colorrgbaf::LTGREY);
            renderer::GeometryBufferDesc desc(mesh.m_indexBuffer, 0, mesh.m_vertexBuffer[0], 0, sizeof(VertexFormatStandard), 0);
            cmdList->drawIndexed(desc, 0, mesh.m_indexBuffer->getIndicesCount(), 0, 0, 1);
        }
        cmdList->endRenderTarget();
    }

    //pass 2
    {
        cmdList->beginRenderTarget(*m_rt[Pass::MBOIT_Pass2]);
        cmdList->setViewport({ 0.f, 0.f, (f32)viewportState._viewpotSize._width, (f32)viewportState._viewpotSize._height });
        cmdList->setScissor({ 0.f, 0.f, (f32)viewportState._viewpotSize._width, (f32)viewportState._viewpotSize._height });
        cmdList->setStencilRef(0);
        cmdList->setPipelineState(*m_pipeline[Pass::MBOIT_Pass2]);

        cmdList->bindDescriptorSet(0,
            {
                renderer::Descriptor(renderer::Descriptor::ConstantBuffer{ &viewportState._viewportBuffer, 0, sizeof(viewportState._viewportBuffer)}, 0)
            });

        for (auto& list : scene.m_lists[toEnumType(scene::MaterialType::Transparency)])
        {
            scene::DrawInstanceDataState& instance = list->_instance;
            const scene::Mesh& mesh = *static_cast<scene::Mesh*>(list->_object);

            struct ModelBuffer
            {
                math::Matrix4D modelMatrix;
                math::Matrix4D prevModelMatrix;
                math::Matrix4D normalMatrix;
                math::float4   tint;
                u64            objectID;
                u64            _pad = 0;
            };

            ModelBuffer constantBuffer;
            constantBuffer.modelMatrix = instance._transform.getTransform();
            constantBuffer.prevModelMatrix = instance._prevTransform.getTransform();
            constantBuffer.normalMatrix = constantBuffer.modelMatrix.getTransposed();
            constantBuffer.tint = instance._material._tint;
            constantBuffer.objectID = instance._objectID;

            cmdList->bindDescriptorSet(1,
                {
                    renderer::Descriptor(renderer::Descriptor::ConstantBuffer{ &constantBuffer, 0, sizeof(constantBuffer)}, 1),
                    renderer::Descriptor(instance._material._sampler, 2),
                    renderer::Descriptor(renderer::TextureView(instance._material._baseColor), 3),
                    renderer::Descriptor(renderer::TextureView(instance._material._normals), 4),
                    renderer::Descriptor(renderer::TextureView(instance._material._metalness), 5),
                    renderer::Descriptor(renderer::TextureView(instance._material._roughness), 6),
                    renderer::Descriptor(renderer::TextureView(m_rt[Pass::MBOIT_Pass1]->getColorTexture<renderer::Texture2D>(0), 0, 0), 7),
                    renderer::Descriptor(renderer::TextureView(m_rt[Pass::MBOIT_Pass1]->getColorTexture<renderer::Texture2D>(1), 0, 0), 8),
                });

            DEBUG_MARKER_SCOPE(cmdList, std::format("Object {}, pipeline {}", instance._objectID, m_pipeline[instance._pipelineID]->getName()), color::colorrgbaf::LTGREY);
            renderer::GeometryBufferDesc desc(mesh.m_indexBuffer, 0, mesh.m_vertexBuffer[0], 0, sizeof(VertexFormatStandard), 0);
            cmdList->drawIndexed(desc, 0, mesh.m_indexBuffer->getIndicesCount(), 0, 0, 1);
        }
        cmdList->endRenderTarget();
    }

    //2 resolve
    {
        ObjectHandle compositionColor = scene.m_globalResources.get("composition_target");
        ASSERT(compositionColor.isValid(), "must be valid");
        renderer::Texture2D* composition_Texture = objectFromHandle<renderer::Texture2D>(compositionColor);

        ObjectHandle sampler_state_h = scene.m_globalResources.get("linear_sampler_mirror");
        ASSERT(sampler_state_h.isValid(), "must be valid");
        renderer::SamplerState* sampler_state = objectFromHandle<renderer::SamplerState>(sampler_state_h);

        cmdList->beginRenderTarget(*m_rt[Pass::CompositionPass]);
        cmdList->setViewport({ 0.f, 0.f, (f32)viewportState._viewpotSize._width, (f32)viewportState._viewpotSize._height });
        cmdList->setScissor({ 0.f, 0.f, (f32)viewportState._viewpotSize._width, (f32)viewportState._viewpotSize._height });
        cmdList->setPipelineState(*m_pipeline[Pass::CompositionPass]);

        cmdList->bindDescriptorSet(0,
            {
                renderer::Descriptor(renderer::Descriptor::ConstantBuffer{ &viewportState._viewportBuffer, 0, sizeof(viewportState._viewportBuffer)}, 0)
            });

        cmdList->bindDescriptorSet(1,
            {
                renderer::Descriptor(sampler_state, 2),
                renderer::Descriptor(renderer::TextureView(composition_Texture), 3),
                renderer::Descriptor(renderer::TextureView(m_rt[Pass::MBOIT_Pass2]->getColorTexture<renderer::Texture2D>(0), 0, 0), 4),
                renderer::Descriptor(renderer::TextureView(m_rt[Pass::MBOIT_Pass1]->getColorTexture<renderer::Texture2D>(0), 0, 0), 5),
            });

        cmdList->draw(renderer::GeometryBufferDesc(), 0, 3, 0, 1);
        cmdList->endRenderTarget();
    }

    scene.m_globalResources.bind("render_target", m_rt[Pass::CompositionPass]->getColorTexture<renderer::Texture2D>(0));
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
    m_rt[Pass::MBOIT_Pass1] = V3D_NEW(renderer::RenderTargetState, memory::MemoryLabel::MemoryGame)(device, data.m_viewportState._viewpotSize, 2);

    m_rt[Pass::MBOIT_Pass1]->setColorTexture(0, V3D_NEW(renderer::Texture2D, memory::MemoryLabel::MemoryGame)(device, renderer::TextureUsage::TextureUsage_Attachment | renderer::TextureUsage::TextureUsage_Sampled,
        renderer::Format::Format_R32_SFloat, data.m_viewportState._viewpotSize, renderer::TextureSamples::TextureSamples_x1, "mboit_od"),
        {
            renderer::RenderTargetLoadOp::LoadOp_Clear, renderer::RenderTargetStoreOp::StoreOp_Store, color::Color(0.0f, 0.0f, 0.0f, 0.0f)
        },
        {
            renderer::TransitionOp::TransitionOp_Undefined, renderer::TransitionOp::TransitionOp_ColorAttachment
        });

    m_rt[Pass::MBOIT_Pass1]->setColorTexture(1, V3D_NEW(renderer::Texture2D, memory::MemoryLabel::MemoryGame)(device, renderer::TextureUsage::TextureUsage_Attachment | renderer::TextureUsage::TextureUsage_Sampled,
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
    m_rt[Pass::MBOIT_Pass2] = V3D_NEW(renderer::RenderTargetState, memory::MemoryLabel::MemoryGame)(device, data.m_viewportState._viewpotSize, 3);

    m_rt[Pass::MBOIT_Pass2]->setColorTexture(0, V3D_NEW(renderer::Texture2D, memory::MemoryLabel::MemoryGame)(device, renderer::TextureUsage::TextureUsage_Attachment | renderer::TextureUsage::TextureUsage_Sampled,
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
    m_rt[Pass::CompositionPass] = V3D_NEW(renderer::RenderTargetState, memory::MemoryLabel::MemoryGame)(device, data.m_viewportState._viewpotSize, 1);

    m_rt[Pass::CompositionPass]->setColorTexture(0, V3D_NEW(renderer::Texture2D, memory::MemoryLabel::MemoryGame)(device, renderer::TextureUsage::TextureUsage_Attachment | renderer::TextureUsage::TextureUsage_Sampled,
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
        V3D_DELETE(texture0, memory::MemoryLabel::MemoryGame);
        renderer::Texture2D* texture1 = m_rt[Pass::MBOIT_Pass1]->getColorTexture<renderer::Texture2D>(1);
        V3D_DELETE(texture1, memory::MemoryLabel::MemoryGame);

        V3D_DELETE(m_rt[Pass::MBOIT_Pass1], memory::MemoryLabel::MemoryGame);
        m_rt[Pass::MBOIT_Pass1] = nullptr;
    }

    {
        ASSERT(m_rt[Pass::MBOIT_Pass2], "must be valid");

        renderer::Texture2D* texture0 = m_rt[Pass::MBOIT_Pass2]->getColorTexture<renderer::Texture2D>(0);
        V3D_DELETE(texture0, memory::MemoryLabel::MemoryGame);

        V3D_DELETE(m_rt[Pass::MBOIT_Pass2], memory::MemoryLabel::MemoryGame);
        m_rt[Pass::MBOIT_Pass2] = nullptr;
    }

    {
        ASSERT(m_rt[Pass::CompositionPass], "must be valid");

        renderer::Texture2D* texture0 = m_rt[Pass::CompositionPass]->getColorTexture<renderer::Texture2D>(0);
        V3D_DELETE(texture0, memory::MemoryLabel::MemoryGame);

        V3D_DELETE(m_rt[Pass::CompositionPass], memory::MemoryLabel::MemoryGame);
        m_rt[Pass::CompositionPass] = nullptr;
    }
}

} //namespace renderer
} //namespace v3d