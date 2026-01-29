#include "RenderPipelineGBuffer.h"

#include "Utils/Logger.h"
#include "Task/RenderTask/RenderTask.h"

#include "Resource/ResourceManager.h"
#include "Resource/Loader/AssetSourceFileLoader.h"
#include "Resource/Loader/ShaderSourceFileLoader.h"
#include "Resource/Loader/ModelFileLoader.h"

#include "Renderer/PipelineState.h"
#include "Renderer/ShaderProgram.h"

#include "Scene/ModelHandler.h"
#include "Scene/Geometry/Mesh.h"
#include "Scene/Material.h"

#include "FrameProfiler.h"

namespace v3d
{
namespace scene
{

RenderPipelineGBufferStage::RenderPipelineGBufferStage(RenderTechnique* technique, scene::ModelHandler* modelHandler) noexcept
    : RenderPipelineStage(technique, "GBuffer")
    , m_modelHandler(modelHandler)
    , m_cmdList(nullptr)

    , m_GBufferRenderTarget(nullptr)
{
}

RenderPipelineGBufferStage::~RenderPipelineGBufferStage()
{
    ASSERT(m_GBufferRenderTarget == nullptr, "must be nullptr");
}

void RenderPipelineGBufferStage::create(renderer::Device* device, scene::SceneData& scene, scene::FrameData& frame)
{
    m_cmdList = device->createCommandList<renderer::CmdListRender>(renderer::Device::GraphicMask);
    createRenderTarget(device, scene);

    //PBR_MetallicRoughness
    {
        const renderer::Shader::DefineList defines =
        { 
            { "SEPARATE_MATERIALS", "1" },
        };

        const renderer::VertexShader* vertShader = resource::ResourceManager::getInstance()->loadShader<renderer::VertexShader, resource::ShaderSourceFileLoader>(device, 
            "gbuffer.hlsl", "gbuffer_standard_vs", defines, {}, resource::ShaderCompileFlag::ShaderCompile_UseDXCompilerForSpirV);
        const renderer::FragmentShader* fragShader = resource::ResourceManager::getInstance()->loadShader<renderer::FragmentShader, resource::ShaderSourceFileLoader>(device, 
            "gbuffer.hlsl", "gbuffer_standard_ps", defines, {}, resource::ShaderCompileFlag::ShaderCompile_UseDXCompilerForSpirV);

        renderer::GraphicsPipelineState* pipeline = V3D_NEW(renderer::GraphicsPipelineState, memory::MemoryLabel::MemoryGame)(device, VertexFormatStandardDesc, m_GBufferRenderTarget->getRenderPassDesc(),
            V3D_NEW(renderer::ShaderProgram, memory::MemoryLabel::MemoryGame)(device, vertShader, fragShader), "gbuffer_pipeline");

        pipeline->setPrimitiveTopology(renderer::PrimitiveTopology::PrimitiveTopology_TriangleList);
        pipeline->setFrontFace(renderer::FrontFace::FrontFace_Clockwise);
        pipeline->setCullMode(renderer::CullMode::CullMode_None);
#if REVERSED_DEPTH
        pipeline->setDepthCompareOp(renderer::CompareOperation::GreaterOrEqual);
#else
        pipeline->setDepthCompareOp(renderer::CompareOperation::LessOrEqual);
#endif
        pipeline->setDepthTest(true);
        pipeline->setDepthWrite(false);
        pipeline->setColorMask(0, renderer::ColorMask::ColorMask_All);
        pipeline->setColorMask(1, renderer::ColorMask::ColorMask_All);
        pipeline->setColorMask(2, renderer::ColorMask::ColorMask_All);
        pipeline->setColorMask(3, renderer::ColorMask::ColorMask_All);

        MaterialParameters parameters;
        BIND_SHADER_PARAMETER(pipeline, parameters, cb_Viewport);
        BIND_SHADER_PARAMETER(pipeline, parameters, cb_Model);
        BIND_SHADER_PARAMETER(pipeline, parameters, s_SamplerState);
        BIND_SHADER_PARAMETER(pipeline, parameters, t_TextureAlbedo);
        BIND_SHADER_PARAMETER(pipeline, parameters, t_TextureNormal);
        BIND_SHADER_PARAMETER(pipeline, parameters, t_TextureMetalness);
        BIND_SHADER_PARAMETER(pipeline, parameters, t_TextureRoughness);
        BIND_SHADER_PARAMETER(pipeline, parameters, t_TextureHeight);

        m_pipeline.emplace_back(pipeline);
        m_parameters.emplace_back(parameters);
    }

    //PBR_MetallicRoughness
    {
        const renderer::Shader::DefineList defines =
        {
            { "SEPARATE_MATERIALS", "0" },
        };

        const renderer::VertexShader* vertShader = resource::ResourceManager::getInstance()->loadShader<renderer::VertexShader, resource::ShaderSourceFileLoader>(device,
            "gbuffer.hlsl", "gbuffer_standard_vs", defines, {}, resource::ShaderCompileFlag::ShaderCompile_UseDXCompilerForSpirV);
        const renderer::FragmentShader* fragShader = resource::ResourceManager::getInstance()->loadShader<renderer::FragmentShader, resource::ShaderSourceFileLoader>(device,
            "gbuffer.hlsl", "gbuffer_standard_ps", defines, {}, resource::ShaderCompileFlag::ShaderCompile_UseDXCompilerForSpirV);

        renderer::GraphicsPipelineState* pipeline = V3D_NEW(renderer::GraphicsPipelineState, memory::MemoryLabel::MemoryGame)(device, VertexFormatStandardDesc, m_GBufferRenderTarget->getRenderPassDesc(),
            V3D_NEW(renderer::ShaderProgram, memory::MemoryLabel::MemoryGame)(device, vertShader, fragShader), "gbuffer_pipeline");

        pipeline->setPrimitiveTopology(renderer::PrimitiveTopology::PrimitiveTopology_TriangleList);
        pipeline->setFrontFace(renderer::FrontFace::FrontFace_Clockwise);
        pipeline->setCullMode(renderer::CullMode::CullMode_None);
#if REVERSED_DEPTH
        pipeline->setDepthCompareOp(renderer::CompareOperation::GreaterOrEqual);
#else
        pipeline->setDepthCompareOp(renderer::CompareOperation::LessOrEqual);
#endif
        pipeline->setDepthTest(true);
        pipeline->setDepthWrite(false);
        pipeline->setColorMask(0, renderer::ColorMask::ColorMask_All);
        pipeline->setColorMask(1, renderer::ColorMask::ColorMask_All);
        pipeline->setColorMask(2, renderer::ColorMask::ColorMask_All);
        pipeline->setColorMask(3, renderer::ColorMask::ColorMask_All);

        MaterialParameters parameters;
        BIND_SHADER_PARAMETER(pipeline, parameters, cb_Viewport);
        BIND_SHADER_PARAMETER(pipeline, parameters, cb_Model);
        BIND_SHADER_PARAMETER(pipeline, parameters, s_SamplerState);
        BIND_SHADER_PARAMETER(pipeline, parameters, t_TextureAlbedo);
        BIND_SHADER_PARAMETER(pipeline, parameters, t_TextureNormal);
        BIND_SHADER_PARAMETER(pipeline, parameters, t_TextureMaterial);
        BIND_SHADER_PARAMETER(pipeline, parameters, t_TextureHeight);

        m_pipeline.emplace_back(pipeline);
        m_parameters.emplace_back(parameters);
    }

    //PBR_MetallicRoughness alpha
    {
        const renderer::Shader::DefineList defines =
        {
            { "SEPARATE_MATERIALS", "1" },
        };

        const renderer::VertexShader* vertShader = resource::ResourceManager::getInstance()->loadShader<renderer::VertexShader, resource::ShaderSourceFileLoader>(device,
            "gbuffer.hlsl", "gbuffer_standard_vs", defines, {}, resource::ShaderCompileFlag::ShaderCompile_UseDXCompilerForSpirV);
        const renderer::FragmentShader* fragShader = resource::ResourceManager::getInstance()->loadShader<renderer::FragmentShader, resource::ShaderSourceFileLoader>(device,
            "gbuffer.hlsl", "gbuffer_masked_ps", defines, {}, resource::ShaderCompileFlag::ShaderCompile_UseDXCompilerForSpirV);

        renderer::GraphicsPipelineState* pipeline = V3D_NEW(renderer::GraphicsPipelineState, memory::MemoryLabel::MemoryGame)(device, VertexFormatStandardDesc, m_GBufferRenderTarget->getRenderPassDesc(),
            V3D_NEW(renderer::ShaderProgram, memory::MemoryLabel::MemoryGame)(device, vertShader, fragShader), "gbuffer_masked_pipeline");

        pipeline->setPrimitiveTopology(renderer::PrimitiveTopology::PrimitiveTopology_TriangleList);
        pipeline->setFrontFace(renderer::FrontFace::FrontFace_Clockwise);
        pipeline->setCullMode(renderer::CullMode::CullMode_None);
#if REVERSED_DEPTH
        pipeline->setDepthCompareOp(renderer::CompareOperation::GreaterOrEqual);
#else
        pipeline->setDepthCompareOp(renderer::CompareOperation::LessOrEqual);
#endif
        pipeline->setDepthTest(true);
        pipeline->setDepthWrite(true);
        pipeline->setColorMask(0, renderer::ColorMask::ColorMask_All);
        pipeline->setColorMask(1, renderer::ColorMask::ColorMask_All);
        pipeline->setColorMask(2, renderer::ColorMask::ColorMask_All);
        pipeline->setColorMask(3, renderer::ColorMask::ColorMask_All);

        MaterialParameters parameters;
        BIND_SHADER_PARAMETER(pipeline, parameters, cb_Viewport);
        BIND_SHADER_PARAMETER(pipeline, parameters, cb_Model);
        BIND_SHADER_PARAMETER(pipeline, parameters, s_SamplerState);
        BIND_SHADER_PARAMETER(pipeline, parameters, t_TextureAlbedo);
        BIND_SHADER_PARAMETER(pipeline, parameters, t_TextureNormal);
        BIND_SHADER_PARAMETER(pipeline, parameters, t_TextureMetalness);
        BIND_SHADER_PARAMETER(pipeline, parameters, t_TextureRoughness);
        BIND_SHADER_PARAMETER(pipeline, parameters, t_TextureHeight);

        m_pipeline.emplace_back(pipeline);
        m_parameters.emplace_back(parameters);
    }

    //PBR_MetallicRoughness alpha
    {
        const renderer::Shader::DefineList defines =
        {
            { "SEPARATE_MATERIALS", "0" },
        };

        const renderer::VertexShader* vertShader = resource::ResourceManager::getInstance()->loadShader<renderer::VertexShader, resource::ShaderSourceFileLoader>(device,
            "gbuffer.hlsl", "gbuffer_standard_vs", defines, {}, resource::ShaderCompileFlag::ShaderCompile_UseDXCompilerForSpirV);
        const renderer::FragmentShader* fragShader = resource::ResourceManager::getInstance()->loadShader<renderer::FragmentShader, resource::ShaderSourceFileLoader>(device,
            "gbuffer.hlsl", "gbuffer_masked_ps", defines, {}, resource::ShaderCompileFlag::ShaderCompile_UseDXCompilerForSpirV);

        renderer::GraphicsPipelineState* pipeline = V3D_NEW(renderer::GraphicsPipelineState, memory::MemoryLabel::MemoryGame)(device, VertexFormatStandardDesc, m_GBufferRenderTarget->getRenderPassDesc(),
            V3D_NEW(renderer::ShaderProgram, memory::MemoryLabel::MemoryGame)(device, vertShader, fragShader), "gbuffer_masked_pipeline");

        pipeline->setPrimitiveTopology(renderer::PrimitiveTopology::PrimitiveTopology_TriangleList);
        pipeline->setFrontFace(renderer::FrontFace::FrontFace_Clockwise);
        pipeline->setCullMode(renderer::CullMode::CullMode_None);
#if REVERSED_DEPTH
        pipeline->setDepthCompareOp(renderer::CompareOperation::GreaterOrEqual);
#else
        pipeline->setDepthCompareOp(renderer::CompareOperation::LessOrEqual);
#endif
        pipeline->setDepthTest(true);
        pipeline->setDepthWrite(true);
        pipeline->setColorMask(0, renderer::ColorMask::ColorMask_All);
        pipeline->setColorMask(1, renderer::ColorMask::ColorMask_All);
        pipeline->setColorMask(2, renderer::ColorMask::ColorMask_All);
        pipeline->setColorMask(3, renderer::ColorMask::ColorMask_All);

        MaterialParameters parameters;
        BIND_SHADER_PARAMETER(pipeline, parameters, cb_Viewport);
        BIND_SHADER_PARAMETER(pipeline, parameters, cb_Model);
        BIND_SHADER_PARAMETER(pipeline, parameters, s_SamplerState);
        BIND_SHADER_PARAMETER(pipeline, parameters, t_TextureAlbedo);
        BIND_SHADER_PARAMETER(pipeline, parameters, t_TextureNormal);
        BIND_SHADER_PARAMETER(pipeline, parameters, t_TextureMaterial);
        BIND_SHADER_PARAMETER(pipeline, parameters, t_TextureHeight);

        m_pipeline.emplace_back(pipeline);
        m_parameters.emplace_back(parameters);
    }
}

void RenderPipelineGBufferStage::destroy(renderer::Device* device, scene::SceneData& scene, scene::FrameData& frame)
{
    destroyRenderTarget(device, scene);

    for (auto& pipeline : m_pipeline)
    {
        const renderer::ShaderProgram* program = pipeline->getShaderProgram();
        V3D_DELETE(program, memory::MemoryLabel::MemoryGame);

        V3D_DELETE(pipeline, memory::MemoryLabel::MemoryGame);
        pipeline = nullptr;
    }
    m_pipeline.clear();

    device->destroyCommandList(m_cmdList);
    m_cmdList = nullptr;
}

void RenderPipelineGBufferStage::prepare(renderer::Device* device, scene::SceneData& scene, scene::FrameData& frame)
{
    if (!m_GBufferRenderTarget)
    {
        createRenderTarget(device, scene);
    }
    else if (m_GBufferRenderTarget->getRenderArea() != scene.m_viewportState._viewpotSize)
    {
        destroyRenderTarget(device, scene);
        createRenderTarget(device, scene);
    }
}

void RenderPipelineGBufferStage::execute(renderer::Device* device, scene::SceneData& scene, scene::FrameData& frame)
{
    if (scene.m_renderLists[toEnumType(scene::RenderPipelinePass::Opaque)].empty() 
        && scene.m_renderLists[toEnumType(scene::RenderPipelinePass::MaskedOpaque)].empty())
    {
        return;
    }

    auto renderJob = [this, device](renderer::CmdListRender* cmdList, const scene::SceneData& scene, const scene::FrameData& frame) -> void
        {
            TRACE_PROFILER_SCOPE("GBuffer", color::rgba8::GREEN);
            DEBUG_MARKER_SCOPE(cmdList, "GBuffer", color::rgbaf::GREEN);

            const scene::ViewportState& viewportState = scene.m_viewportState;

            cmdList->beginRenderTarget(*m_GBufferRenderTarget);
            cmdList->setViewport({ 0.f, 0.f, (f32)viewportState._viewpotSize._width, (f32)viewportState._viewpotSize._height });
            cmdList->setScissor({ 0.f, 0.f, (f32)viewportState._viewpotSize._width, (f32)viewportState._viewpotSize._height });

            ObjectHandle hLinearSampler = scene.m_globalResources.get("linear_sampler_repeat");
            ASSERT(hLinearSampler.isValid(), "must be valid");
            renderer::SamplerState* sampler = objectFromHandle<renderer::SamplerState>(hLinearSampler);

            for (auto& entry : scene.m_renderLists[toEnumType(scene::RenderPipelinePass::Opaque)])
            {
                const scene::DrawNodeEntry& itemMesh = *static_cast<scene::DrawNodeEntry*>(entry);
                const scene::Mesh& mesh = *static_cast<scene::Mesh*>(itemMesh.geometry);
                const scene::Material& material = *static_cast<scene::Material*>(itemMesh.material);

                cmdList->setPipelineState(*m_pipeline[itemMesh.pipelineID]);

                cmdList->bindDescriptorSet(m_pipeline[itemMesh.pipelineID]->getShaderProgram(), 0,
                    {
                        renderer::Descriptor(renderer::Descriptor::ConstantBuffer{ &viewportState._viewportBuffer, 0, sizeof(viewportState._viewportBuffer)}, m_parameters[itemMesh.pipelineID].cb_Viewport)
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
                constantBuffer.tintColour = material.getProperty<math::float4>("DiffuseColor");
                constantBuffer.objectID = itemMesh.object->ID();

                if (material.getShadingModel() == scene::MaterialShadingModel::PBR_MetallicRoughness)
                {
                    cmdList->bindDescriptorSet(m_pipeline[itemMesh.pipelineID]->getShaderProgram(), 1,
                        {
                            renderer::Descriptor(renderer::Descriptor::ConstantBuffer{ &constantBuffer, 0, sizeof(constantBuffer)}, m_parameters[itemMesh.pipelineID].cb_Model),
                            renderer::Descriptor(sampler, m_parameters[itemMesh.pipelineID].s_SamplerState),
                            renderer::Descriptor(renderer::TextureView(objectFromHandle<renderer::Texture2D>(material.getProperty<ObjectHandle>("BaseColor"))), m_parameters[itemMesh.pipelineID].t_TextureAlbedo),
                            renderer::Descriptor(renderer::TextureView(objectFromHandle<renderer::Texture2D>(material.getProperty<ObjectHandle>("Normals"))), m_parameters[itemMesh.pipelineID].t_TextureNormal),
                            renderer::Descriptor(renderer::TextureView(objectFromHandle<renderer::Texture2D>(material.getProperty<ObjectHandle>("Roughness"))), m_parameters[itemMesh.pipelineID].t_TextureRoughness),
                            renderer::Descriptor(renderer::TextureView(objectFromHandle<renderer::Texture2D>(material.getProperty<ObjectHandle>("Metalness"))), m_parameters[itemMesh.pipelineID].t_TextureMetalness),
                            renderer::Descriptor(renderer::TextureView(objectFromHandle<renderer::Texture2D>(material.getProperty<ObjectHandle>("Displacement"))), m_parameters[itemMesh.pipelineID].t_TextureHeight),
                        });
                }
                else if (material.getShadingModel() == scene::MaterialShadingModel::PBR_Specular)
                {
                    cmdList->bindDescriptorSet(m_pipeline[itemMesh.pipelineID]->getShaderProgram(), 1,
                        {
                            renderer::Descriptor(renderer::Descriptor::ConstantBuffer{ &constantBuffer, 0, sizeof(constantBuffer)}, m_parameters[itemMesh.pipelineID].cb_Model),
                            renderer::Descriptor(sampler, m_parameters[itemMesh.pipelineID].s_SamplerState),
                            renderer::Descriptor(renderer::TextureView(objectFromHandle<renderer::Texture2D>(material.getProperty<ObjectHandle>("Diffuse"))), m_parameters[itemMesh.pipelineID].t_TextureAlbedo),
                            renderer::Descriptor(renderer::TextureView(objectFromHandle<renderer::Texture2D>(material.getProperty<ObjectHandle>("Normals"))), m_parameters[itemMesh.pipelineID].t_TextureNormal),
                            renderer::Descriptor(renderer::TextureView(objectFromHandle<renderer::Texture2D>(material.getProperty<ObjectHandle>("Specular"))), m_parameters[itemMesh.pipelineID].t_TextureMaterial),
                            renderer::Descriptor(renderer::TextureView(objectFromHandle<renderer::Texture2D>(material.getProperty<ObjectHandle>("Displacement"))), m_parameters[itemMesh.pipelineID].t_TextureHeight),
                        });
                }
                else if (material.getShadingModel() == scene::MaterialShadingModel::Custom)
                {
                    //TODO: Rework. Internal V3D material pipeline. Used packed materials (R: ? G: Roughness  B: Metalness)
                    cmdList->bindDescriptorSet(m_pipeline[itemMesh.pipelineID]->getShaderProgram(), 1,
                        {
                            renderer::Descriptor(renderer::Descriptor::ConstantBuffer{ &constantBuffer, 0, sizeof(constantBuffer)}, m_parameters[itemMesh.pipelineID].cb_Model),
                            renderer::Descriptor(sampler, m_parameters[itemMesh.pipelineID].s_SamplerState),
                            renderer::Descriptor(renderer::TextureView(objectFromHandle<renderer::Texture2D>(material.getProperty<ObjectHandle>("Diffuse"))), m_parameters[itemMesh.pipelineID].t_TextureAlbedo),
                            renderer::Descriptor(renderer::TextureView(objectFromHandle<renderer::Texture2D>(material.getProperty<ObjectHandle>("Normals"))), m_parameters[itemMesh.pipelineID].t_TextureNormal),
                            renderer::Descriptor(renderer::TextureView(objectFromHandle<renderer::Texture2D>(material.getProperty<ObjectHandle>("Specular"))), m_parameters[itemMesh.pipelineID].t_TextureMaterial),
                        });
                }
                else
                {
                    ASSERT(false, "");
                }

                DEBUG_MARKER_SCOPE(cmdList, std::format("Object [{}], pipeline [{}]", itemMesh.object->m_name, m_pipeline[itemMesh.pipelineID]->getName()), color::rgbaf::LTGREY);
                ASSERT(mesh.getVertexAttribDesc()._inputBindings[0]._stride == sizeof(VertexFormatStandard), "must be same");
                renderer::GeometryBufferDesc desc(mesh.getIndexBuffer(), 0, mesh.getVertexBuffer(0), sizeof(VertexFormatStandard), 0);
                cmdList->drawIndexed(desc, 0, mesh.getIndexBuffer()->getIndicesCount(), 0, 0, 1);
            }

            for (auto& entry : scene.m_renderLists[toEnumType(scene::RenderPipelinePass::MaskedOpaque)])
            {
                const scene::DrawNodeEntry& itemMesh = *static_cast<scene::DrawNodeEntry*>(entry);
                const scene::Mesh& mesh = *static_cast<scene::Mesh*>(itemMesh.geometry);
                const scene::Material& material = *static_cast<scene::Material*>(itemMesh.material);

                cmdList->setPipelineState(*m_pipeline[itemMesh.pipelineID]);

                ObjectHandle noise = scene.m_globalResources.get("tiling_noise");
                ASSERT(noise.isValid(), "must be valid");
                renderer::Texture2D* noiseTexture = objectFromHandle<renderer::Texture2D>(noise);

                cmdList->bindDescriptorSet(m_pipeline[itemMesh.pipelineID]->getShaderProgram(), 0,
                    {
                        renderer::Descriptor(renderer::Descriptor::ConstantBuffer{ &viewportState._viewportBuffer, 0, sizeof(viewportState._viewportBuffer)}, m_parameters[itemMesh.pipelineID].cb_Viewport)
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
                constantBuffer.tintColour = material.getProperty<math::float4>("DiffuseColor");
                constantBuffer.objectID = itemMesh.object->ID();

                cmdList->bindDescriptorSet(m_pipeline[itemMesh.pipelineID]->getShaderProgram(), 1,
                    {
                        renderer::Descriptor(renderer::Descriptor::ConstantBuffer{ &constantBuffer, 0, sizeof(constantBuffer)}, m_parameters[itemMesh.pipelineID].cb_Model),
                        renderer::Descriptor(sampler, m_parameters[itemMesh.pipelineID].s_SamplerState),
                        renderer::Descriptor(renderer::TextureView(objectFromHandle<renderer::Texture2D>(material.getProperty<ObjectHandle>("BaseColor"))), m_parameters[itemMesh.pipelineID].t_TextureAlbedo),
                        renderer::Descriptor(renderer::TextureView(objectFromHandle<renderer::Texture2D>(material.getProperty<ObjectHandle>("Normals"))), m_parameters[itemMesh.pipelineID].t_TextureNormal),
                        renderer::Descriptor(renderer::TextureView(objectFromHandle<renderer::Texture2D>(material.getProperty<ObjectHandle>("Roughness"))), m_parameters[itemMesh.pipelineID].t_TextureRoughness),
                        renderer::Descriptor(renderer::TextureView(objectFromHandle<renderer::Texture2D>(material.getProperty<ObjectHandle>("Metalness"))), m_parameters[itemMesh.pipelineID].t_TextureMetalness),
                        renderer::Descriptor(renderer::TextureView(noiseTexture, 0, 0), 6),
                    });

                DEBUG_MARKER_SCOPE(cmdList, std::format("Object [{}], pipeline [{}]", itemMesh.object->m_name, m_pipeline[itemMesh.pipelineID]->getName()), color::rgbaf::LTGREY);
                ASSERT(mesh.getVertexAttribDesc()._inputBindings[0]._stride == sizeof(VertexFormatStandard), "must be same");
                renderer::GeometryBufferDesc desc(mesh.getIndexBuffer(), 0, mesh.getVertexBuffer(0), sizeof(VertexFormatStandard), 0);
                cmdList->drawIndexed(desc, 0, mesh.getIndexBuffer()->getIndicesCount(), 0, 0, 1);
            }

            cmdList->endRenderTarget();
        };

    ASSERT(m_cmdList, "must be valid");
    addRenderJob("GBuffer Job", renderJob, m_cmdList, scene, frame);
}

void RenderPipelineGBufferStage::createRenderTarget(renderer::Device* device, scene::SceneData& state)
{
    ASSERT(m_GBufferRenderTarget == nullptr, "must be nullptr");
    m_GBufferRenderTarget = V3D_NEW(renderer::RenderTargetState, memory::MemoryLabel::MemoryGame)(device, state.m_viewportState._viewpotSize, 4, 0);

    renderer::Texture2D* albedoAttachment = V3D_NEW(renderer::Texture2D, memory::MemoryLabel::MemoryGame)(device, renderer::TextureUsage::TextureUsage_Attachment | renderer::TextureUsage::TextureUsage_Sampled,
        renderer::Format::Format_R8G8B8A8_UNorm, state.m_viewportState._viewpotSize, renderer::TextureSamples::TextureSamples_x1, "gbuffer_albedo");
    m_GBufferRenderTarget->setColorTexture(0, albedoAttachment,
        {
            renderer::RenderTargetLoadOp::LoadOp_Clear, renderer::RenderTargetStoreOp::StoreOp_Store, color::Color(0.0f, 0.0f, 0.0f, 1.0f)
        },
        {
            renderer::TransitionOp::TransitionOp_ColorAttachment, renderer::TransitionOp::TransitionOp_ShaderRead
        }
    );

    renderer::Texture2D* normalsAttachment = V3D_NEW(renderer::Texture2D, memory::MemoryLabel::MemoryGame)(device, renderer::TextureUsage::TextureUsage_Attachment | renderer::TextureUsage::TextureUsage_Sampled,
        renderer::Format::Format_R16G16B16A16_SFloat, state.m_viewportState._viewpotSize, renderer::TextureSamples::TextureSamples_x1, "gbuffer_normals");
    m_GBufferRenderTarget->setColorTexture(1, normalsAttachment,
        {
            renderer::RenderTargetLoadOp::LoadOp_Clear, renderer::RenderTargetStoreOp::StoreOp_Store, color::Color(0.0f, 0.0f, 0.0f, 1.0f)
        },
        {
            renderer::TransitionOp::TransitionOp_ColorAttachment, renderer::TransitionOp::TransitionOp_ColorAttachment
        }
    );

    renderer::Texture2D* materialAttachment = V3D_NEW(renderer::Texture2D, memory::MemoryLabel::MemoryGame)(device, renderer::TextureUsage::TextureUsage_Attachment | renderer::TextureUsage::TextureUsage_Sampled,
        renderer::Format::Format_R16G16B16A16_SFloat, state.m_viewportState._viewpotSize, renderer::TextureSamples::TextureSamples_x1, "gbuffer_material");
    m_GBufferRenderTarget->setColorTexture(2, materialAttachment,
        {
            renderer::RenderTargetLoadOp::LoadOp_Clear, renderer::RenderTargetStoreOp::StoreOp_Store, color::Color(0.0f, 0.0f, 0.0f, 1.0f)
        },
        {
            renderer::TransitionOp::TransitionOp_ColorAttachment, renderer::TransitionOp::TransitionOp_ColorAttachment
        }
    );

    renderer::Texture2D* velocityAttachment = V3D_NEW(renderer::Texture2D, memory::MemoryLabel::MemoryGame)(device, renderer::TextureUsage::TextureUsage_Attachment | renderer::TextureUsage::TextureUsage_Sampled,
        renderer::Format::Format_R16G16_SFloat, state.m_viewportState._viewpotSize, renderer::TextureSamples::TextureSamples_x1, "gbuffer_velocity");
    m_GBufferRenderTarget->setColorTexture(3, velocityAttachment,
        {
            renderer::RenderTargetLoadOp::LoadOp_Clear, renderer::RenderTargetStoreOp::StoreOp_Store, color::Color(0.0f, 0.0f, 0.0f, 1.0f)
        },
        {
            renderer::TransitionOp::TransitionOp_ColorAttachment, renderer::TransitionOp::TransitionOp_ColorAttachment
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

void RenderPipelineGBufferStage::destroyRenderTarget(renderer::Device* device, scene::SceneData& data)
{
    ASSERT(m_GBufferRenderTarget, "must be valid");
    renderer::Texture2D* albedoAttachment = m_GBufferRenderTarget->getColorTexture<renderer::Texture2D>(0);
    V3D_DELETE(albedoAttachment, memory::MemoryLabel::MemoryGame);

    renderer::Texture2D* normalsAttachment = m_GBufferRenderTarget->getColorTexture<renderer::Texture2D>(1);
    V3D_DELETE(normalsAttachment, memory::MemoryLabel::MemoryGame);

    renderer::Texture2D* materialAttachment = m_GBufferRenderTarget->getColorTexture<renderer::Texture2D>(2);
    V3D_DELETE(materialAttachment, memory::MemoryLabel::MemoryGame);

    renderer::Texture2D* velocityAttachment = m_GBufferRenderTarget->getColorTexture<renderer::Texture2D>(3);
    V3D_DELETE(velocityAttachment, memory::MemoryLabel::MemoryGame);

    V3D_DELETE(m_GBufferRenderTarget, memory::MemoryLabel::MemoryGame);
    m_GBufferRenderTarget = nullptr;
}

} //namespace scene
} //namespace v3d