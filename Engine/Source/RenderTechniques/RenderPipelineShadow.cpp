#include "RenderPipelineShadow.h"
#include "Utils/Logger.h"

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

RenderPipelineShadowStage::RenderPipelineShadowStage(RenderTechnique* technique, scene::ModelHandler* modelHandler) noexcept
    : RenderPipelineStage(technique, "Shadow")
    , m_modelHandler(modelHandler)
    , m_cascadeTextureArray(nullptr)
    , m_SSShadowsRenderTarget(nullptr)
{
}

RenderPipelineShadowStage::~RenderPipelineShadowStage()
{
}

void RenderPipelineShadowStage::create(renderer::Device* device, scene::SceneData& scene, scene::FrameData& frame)
{
    createRenderTarget(device, scene);

    renderer::Shader::DefineList defines =
    {
        { "SHADOWMAP_CASCADE_COUNT", std::to_string(scene.m_settings._shadowmapCascadeCount) },
    };

    {
        const renderer::VertexShader* vertShader = resource::ResourceManager::getInstance()->loadShader<renderer::VertexShader, resource::ShaderSourceFileLoader>(device,
            "light_shadows.hlsl", "light_shadows_vs", defines, {}, resource::ShaderCompileFlag::ShaderCompile_UseDXCompilerForSpirV);
        const renderer::FragmentShader* fragShader = resource::ResourceManager::getInstance()->loadShader<renderer::FragmentShader, resource::ShaderSourceFileLoader>(device,
            "light_shadows.hlsl", "light_shadows_ps", defines, {}, resource::ShaderCompileFlag::ShaderCompile_UseDXCompilerForSpirV);

        renderer::RenderPassDesc desc{};
        desc._countColorAttachment = 0;
        desc._hasDepthStencilAttachment = true;
        desc._attachmentsDesc.back()._format = renderer::Format::Format_D32_SFloat;

        m_cascadeShadowPipeline = V3D_NEW(renderer::GraphicsPipelineState, memory::MemoryLabel::MemoryGame)(device, VertexFormatStandardDesc, desc,
            V3D_NEW(renderer::ShaderProgram, memory::MemoryLabel::MemoryGame)(device, vertShader, fragShader), "shadowmap_pipeline");
        m_cascadeShadowPipeline->setPrimitiveTopology(renderer::PrimitiveTopology::PrimitiveTopology_TriangleList);
        m_cascadeShadowPipeline->setFrontFace(renderer::FrontFace::FrontFace_Clockwise);
        m_cascadeShadowPipeline->setCullMode(renderer::CullMode::CullMode_Back);
#if REVERSED_DEPTH
        m_cascadeShadowPipeline->setDepthCompareOp(renderer::CompareOperation::GreaterOrEqual);
#else
        m_cascadeShadowPipeline->setDepthCompareOp(renderer::CompareOperation::LessOrEqual);
#endif
        m_cascadeShadowPipeline->setDepthWrite(true);
        m_cascadeShadowPipeline->setDepthTest(true);
        m_cascadeShadowPipeline->setColorMask(0, renderer::ColorMask::ColorMask_None);
        m_cascadeShadowPipeline->setDepthBias(0.0f, 0.0f, -4.0f);

        BIND_SHADER_PARAMETER(m_cascadeShadowPipeline, m_cascadeShadowParameters, cb_ShadowBuffer);
    }

    {
        const renderer::VertexShader* vertShader = resource::ResourceManager::getInstance()->loadShader<renderer::VertexShader, resource::ShaderSourceFileLoader>(device,
            "offscreen.hlsl", "offscreen_vs", {}, {}, resource::ShaderCompileFlag::ShaderCompile_UseDXCompilerForSpirV);
        const renderer::FragmentShader* fragShader = resource::ResourceManager::getInstance()->loadShader<renderer::FragmentShader, resource::ShaderSourceFileLoader>(device,
            "screen_space_shadow.hlsl", "screen_space_shadow_ps", defines, {}, resource::ShaderCompileFlag::ShaderCompile_UseDXCompilerForSpirV);

        m_SSShadowsPipeline = V3D_NEW(renderer::GraphicsPipelineState, memory::MemoryLabel::MemoryGame)(device, renderer::VertexInputAttributeDesc(), m_SSShadowsRenderTarget->getRenderPassDesc(),
            V3D_NEW(renderer::ShaderProgram, memory::MemoryLabel::MemoryGame)(device, vertShader, fragShader), "screen_space_shadows_pipeline");
        m_SSShadowsPipeline->setPrimitiveTopology(renderer::PrimitiveTopology::PrimitiveTopology_TriangleList);
        m_SSShadowsPipeline->setFrontFace(renderer::FrontFace::FrontFace_Clockwise);
        m_SSShadowsPipeline->setCullMode(renderer::CullMode::CullMode_Back);
        m_SSShadowsPipeline->setDepthCompareOp(renderer::CompareOperation::Always);
        m_SSShadowsPipeline->setDepthWrite(false);
        m_SSShadowsPipeline->setDepthTest(false);
        m_SSShadowsPipeline->setColorMask(0, renderer::ColorMask::ColorMask_All);
        m_SSShadowsPipeline->setStencilTest(false);
        m_SSShadowsPipeline->setStencilCompareOp(renderer::CompareOperation::NotEqual, 0x0);
        m_SSShadowsPipeline->setStencilOp(renderer::StencilOperation::Keep, renderer::StencilOperation::Keep, renderer::StencilOperation::Keep);

        BIND_SHADER_PARAMETER(m_SSShadowsPipeline, m_SSCascadeShadowParameters, cb_Viewport);
        BIND_SHADER_PARAMETER(m_SSShadowsPipeline, m_SSCascadeShadowParameters, cb_CascadeShadowBuffer);
        BIND_SHADER_PARAMETER(m_SSShadowsPipeline, m_SSCascadeShadowParameters, s_SamplerState);
        BIND_SHADER_PARAMETER(m_SSShadowsPipeline, m_SSCascadeShadowParameters, s_ShadowSamplerState);
        BIND_SHADER_PARAMETER(m_SSShadowsPipeline, m_SSCascadeShadowParameters, t_TextureDepth);
        BIND_SHADER_PARAMETER(m_SSShadowsPipeline, m_SSCascadeShadowParameters, t_TextureNormals);
        BIND_SHADER_PARAMETER(m_SSShadowsPipeline, m_SSCascadeShadowParameters, t_CascadeShadows);

        m_shadowSamplerState = V3D_NEW(renderer::SamplerState, memory::MemoryLabel::MemoryGame)(device, renderer::SamplerFilter::SamplerFilter_Bilinear, renderer::SamplerAnisotropic::SamplerAnisotropic_None);
        m_shadowSamplerState->setWrap(renderer::SamplerWrap::TextureWrap_ClampToBorder);
        m_shadowSamplerState->setEnableCompareOp(true);
        m_shadowSamplerState->setCompareOp(renderer::CompareOperation::LessOrEqual);
        m_shadowSamplerState->setBorderColor({ 0.0f, 0.0f, 0.0f, 0.0f });
    }
}

void RenderPipelineShadowStage::destroy(renderer::Device* device, scene::SceneData& scene, scene::FrameData& frame)
{
    destroyRenderTarget(device, scene);

    {
        const renderer::ShaderProgram* program = m_cascadeShadowPipeline->getShaderProgram();
        V3D_DELETE(program, memory::MemoryLabel::MemoryGame);

        V3D_DELETE(m_cascadeShadowPipeline, memory::MemoryLabel::MemoryGame);
        m_cascadeShadowPipeline = nullptr;
    }

    {
        const renderer::ShaderProgram* program = m_SSShadowsPipeline->getShaderProgram();
        V3D_DELETE(program, memory::MemoryLabel::MemoryGame);

        V3D_DELETE(m_SSShadowsPipeline, memory::MemoryLabel::MemoryGame);
        m_SSShadowsPipeline = nullptr;
    }

    V3D_DELETE(m_shadowSamplerState, memory::MemoryLabel::MemoryGame);
    m_shadowSamplerState = nullptr;
}

void RenderPipelineShadowStage::prepare(renderer::Device* device, scene::SceneData& scene, scene::FrameData& frame)
{
    if (m_cascadeRenderTargets.empty() || !m_SSShadowsRenderTarget)
    {
        createRenderTarget(device, scene);
    }
    else if (m_cascadeRenderTargets[0]->getRenderArea() != scene.m_settings._shadowmapSize || m_SSShadowsRenderTarget->getRenderArea() != scene.m_viewportState._viewpotSize)
    {
        destroyRenderTarget(device, scene);
        createRenderTarget(device, scene);
    }
}

void RenderPipelineShadowStage::execute(renderer::Device* device, scene::SceneData& scene, scene::FrameData& frame)
{
    renderer::CmdListRender* cmdList = frame.m_cmdList;
    scene::ViewportState& viewportState = scene.m_viewportState;

    if (scene.m_renderLists[toEnumType(scene::RenderPipelinePass::Shadowmap)].empty() || scene.m_renderLists[toEnumType(scene::RenderPipelinePass::DirectionLight)].empty())
    {
        return;
    }

    ASSERT(scene.m_renderLists[toEnumType(scene::RenderPipelinePass::DirectionLight)].size() == 1, "supported only one light at the moment");
    if (!scene.m_renderLists[toEnumType(scene::RenderPipelinePass::DirectionLight)].empty())
    {
        TRACE_PROFILER_SCOPE("Shadowmap", color::rgba8::GREEN);
        DEBUG_MARKER_SCOPE(cmdList, "Shadowmap", color::rgbaf::GREEN);

        scene::LightNodeEntry& itemLight = *static_cast<scene::LightNodeEntry*>(scene.m_renderLists[toEnumType(scene::RenderPipelinePass::DirectionLight)][0]);
        const scene::DirectionalLight& dirLight = *static_cast<const scene::DirectionalLight*>(itemLight.light);
        calculateShadowCascades(scene, itemLight.object->getDirection(), itemLight);

        for (u32 layer = 0; layer < scene.m_settings._shadowmapCascadeCount; ++layer)
        {
            cmdList->beginRenderTarget(*m_cascadeRenderTargets[layer]);
            cmdList->setViewport({ 0.f, 0.f, (f32)scene.m_settings._shadowmapSize._width, (f32)scene.m_settings._shadowmapSize._height });
            cmdList->setScissor({ 0.f, 0.f, (f32)scene.m_settings._shadowmapSize._width, (f32)scene.m_settings._shadowmapSize._height });
            cmdList->setPipelineState(*m_cascadeShadowPipeline);

            for (auto& entry : scene.m_renderLists[toEnumType(scene::RenderPipelinePass::Shadowmap)])
            {
                const scene::DrawNodeEntry& itemMesh = *static_cast<scene::DrawNodeEntry*>(entry);

                struct ShadowBuffer
                {
                    math::Matrix4D lightSpaceMatrix;
                    math::Matrix4D modelMatrix;
                } shadowViewBuffer;

                shadowViewBuffer.lightSpaceMatrix = itemLight.lightSpaceMatrix[layer];
                shadowViewBuffer.modelMatrix = itemMesh.object->getTransform().getMatrix();

                cmdList->bindDescriptorSet(m_cascadeShadowPipeline->getShaderProgram(), 0,
                    {
                        renderer::Descriptor(renderer::Descriptor::ConstantBuffer{ &shadowViewBuffer, 0, sizeof(shadowViewBuffer) }, m_cascadeShadowParameters.cb_ShadowBuffer)
                    });

                DEBUG_MARKER_SCOPE(cmdList, std::format("Object [{}], pipeline [{}]", itemMesh.object->m_name, m_cascadeShadowPipeline->getName()), color::rgbaf::LTGREY);

                const scene::Mesh& mesh = *static_cast<scene::Mesh*>(itemMesh.geometry);
                ASSERT(mesh.getVertexAttribDesc()._inputBindings[0]._stride == sizeof(scene::VertexFormatStandard), "must be same");
                renderer::GeometryBufferDesc desc(mesh.getIndexBuffer(), 0, mesh.getVertexBuffer(0), 0, sizeof(scene::VertexFormatStandard), 0);
                cmdList->drawIndexed(desc, 0, mesh.getIndexBuffer()->getIndicesCount(), 0, 0, 1);
            }

            cmdList->endRenderTarget();
        }
    }

    if (!scene.m_renderLists[toEnumType(scene::RenderPipelinePass::PunctualLights)].empty())
    {
        TRACE_PROFILER_SCOPE("PointShadowmap", color::rgba8::GREEN);
        DEBUG_MARKER_SCOPE(cmdList, "PointShadowmap", color::rgbaf::GREEN);

        //TODO
    }

    {
        TRACE_PROFILER_SCOPE("ScreenSpaceShadows", color::rgba8::GREEN);
        DEBUG_MARKER_SCOPE(cmdList, "ScreenSpaceShadows", color::rgbaf::GREEN);

        cmdList->beginRenderTarget(*m_SSShadowsRenderTarget);
        cmdList->setViewport({ 0.f, 0.f, (f32)viewportState._viewpotSize._width, (f32)viewportState._viewpotSize._height });
        cmdList->setScissor({ 0.f, 0.f, (f32)viewportState._viewpotSize._width, (f32)viewportState._viewpotSize._height });
        cmdList->setPipelineState(*m_SSShadowsPipeline);

        cmdList->bindDescriptorSet(m_SSShadowsPipeline->getShaderProgram(), 0,
            {
                renderer::Descriptor(renderer::Descriptor::ConstantBuffer{ &viewportState._viewportBuffer, 0, sizeof(viewportState._viewportBuffer)}, m_SSCascadeShadowParameters.cb_Viewport)
            });

        scene::LightNodeEntry& itemLight = *static_cast<scene::LightNodeEntry*>(scene.m_renderLists[toEnumType(scene::RenderPipelinePass::DirectionLight)][0]);

        struct ShadowCascade
        {
            math::Matrix4D  lightSpaceMatrix;
            f32             cascadeSplit;
            f32            _pad[3];
        };

        struct CascadeShadowBuffer
        {
            ShadowCascade cascade[4];
            math::Vector3D lightDirection;
        } cascadeShadowBuffer;

        for (u32 id = 0; id < itemLight.lightSpaceMatrix.size(); ++id)
        {
            cascadeShadowBuffer.cascade[id].lightSpaceMatrix = itemLight.lightSpaceMatrix[id];
            cascadeShadowBuffer.cascade[id].cascadeSplit = itemLight.cascadeSplits[id];
        }
        cascadeShadowBuffer.lightDirection = itemLight.object->getDirection();

        ObjectHandle depth_stencil_h = scene.m_globalResources.get("depth_stencil");
        ASSERT(depth_stencil_h.isValid(), "must be valid");
        renderer::Texture2D* depthStencilTexture = objectFromHandle<renderer::Texture2D>(depth_stencil_h);

        ObjectHandle gbuffer_normals_h = scene.m_globalResources.get("gbuffer_normals");
        ASSERT(gbuffer_normals_h.isValid(), "must be valid");
        renderer::Texture2D* gbufferNormalsTexture = objectFromHandle<renderer::Texture2D>(gbuffer_normals_h);

        ObjectHandle sampler_state_h = scene.m_globalResources.get("linear_sampler_clamp");
        ASSERT(sampler_state_h.isValid(), "must be valid");
        renderer::SamplerState* sampler_state = objectFromHandle<renderer::SamplerState>(sampler_state_h);

        cmdList->bindDescriptorSet(m_SSShadowsPipeline->getShaderProgram(), 1,
            {
                renderer::Descriptor(renderer::Descriptor::ConstantBuffer{ &cascadeShadowBuffer, 0, sizeof(cascadeShadowBuffer) }, m_SSCascadeShadowParameters.cb_CascadeShadowBuffer),
                renderer::Descriptor(sampler_state, m_SSCascadeShadowParameters.s_SamplerState),
                renderer::Descriptor(m_shadowSamplerState, m_SSCascadeShadowParameters.s_ShadowSamplerState),
                renderer::Descriptor(renderer::TextureView(depthStencilTexture), m_SSCascadeShadowParameters.t_TextureDepth),
                renderer::Descriptor(renderer::TextureView(gbufferNormalsTexture, 0, 0), m_SSCascadeShadowParameters.t_TextureNormals),
                renderer::Descriptor(renderer::TextureView(m_cascadeTextureArray), m_SSCascadeShadowParameters.t_CascadeShadows),
            });

        cmdList->draw(renderer::GeometryBufferDesc(), 0, 3, 0, 1);
        cmdList->endRenderTarget();
    }
}

void RenderPipelineShadowStage::createRenderTarget(renderer::Device* device, scene::SceneData& data)
{
    {
        ASSERT(m_cascadeRenderTargets.empty(), "must be empty");
        ASSERT(m_cascadeTextureArray == nullptr, "must be nullptr");

        m_cascadeTextureArray = V3D_NEW(renderer::Texture2D, memory::MemoryLabel::MemoryGame)(device, renderer::TextureUsage::TextureUsage_Attachment | renderer::TextureUsage::TextureUsage_Sampled | renderer::TextureUsage::TextureUsage_Write,
            renderer::Format::Format_D32_SFloat, data.m_settings._shadowmapSize, data.m_settings._shadowmapCascadeCount, 1, "shadowmap");
        for (u32 layer = 0; layer < data.m_settings._shadowmapCascadeCount; ++layer)
        {
            renderer::RenderTargetState* renderTarget = V3D_NEW(renderer::RenderTargetState, memory::MemoryLabel::MemoryGame)(device, data.m_settings._shadowmapSize, 0);
            renderTarget->setDepthStencilTexture(renderer::TextureView(m_cascadeTextureArray, layer),
                {
                    renderer::RenderTargetLoadOp::LoadOp_Clear, renderer::RenderTargetStoreOp::StoreOp_Store, 0.0f,
                },
            {
                 renderer::RenderTargetLoadOp::LoadOp_DontCare, renderer::RenderTargetStoreOp::StoreOp_DontCare, 0U,
            },
            {
                renderer::TransitionOp::TransitionOp_DepthStencilAttachment, renderer::TransitionOp::TransitionOp_DepthStencilReadOnly
            });

            m_cascadeRenderTargets.push_back(renderTarget);
        }

        data.m_globalResources.bind("shadowmap", m_cascadeTextureArray);
    }

    {
        ASSERT(m_SSShadowsRenderTarget == nullptr, "must be nullptr");
        m_SSShadowsRenderTarget = V3D_NEW(renderer::RenderTargetState, memory::MemoryLabel::MemoryGame)(device, data.m_viewportState._viewpotSize, 1, 0);

        renderer::Texture2D* screenspaceShadow = V3D_NEW(renderer::Texture2D, memory::MemoryLabel::MemoryGame)(device, renderer::TextureUsage::TextureUsage_Attachment | renderer::TextureUsage::TextureUsage_Sampled,
            renderer::Format::Format_R16G16B16A16_SFloat, data.m_viewportState._viewpotSize, renderer::TextureSamples::TextureSamples_x1, "screen_space_shadow");

        m_SSShadowsRenderTarget->setColorTexture(0, screenspaceShadow,
            {
                renderer::RenderTargetLoadOp::LoadOp_DontCare, renderer::RenderTargetStoreOp::StoreOp_Store, color::Color(0.0f)
            },
        {
            renderer::TransitionOp::TransitionOp_ColorAttachment, renderer::TransitionOp::TransitionOp_ShaderRead
        });

        data.m_globalResources.bind("screen_space_shadow", screenspaceShadow);
    }
}

void RenderPipelineShadowStage::destroyRenderTarget(renderer::Device* device, scene::SceneData& data)
{
    {
        ASSERT(m_cascadeTextureArray != nullptr, "must be valid");
        for (u32 layer = 0; layer < data.m_settings._shadowmapCascadeCount; ++layer)
        {
            V3D_DELETE(m_cascadeRenderTargets[layer], memory::MemoryLabel::MemoryGame);
        }
        m_cascadeRenderTargets.clear();

        V3D_DELETE(m_cascadeTextureArray, memory::MemoryLabel::MemoryGame);
        m_cascadeTextureArray = nullptr;
    }

    {
        ASSERT(m_SSShadowsRenderTarget, "must be valid");
        renderer::Texture2D* gamma = m_SSShadowsRenderTarget->getColorTexture<renderer::Texture2D>(0);
        V3D_DELETE(gamma, memory::MemoryLabel::MemoryGame);

        V3D_DELETE(m_SSShadowsRenderTarget, memory::MemoryLabel::MemoryGame);
        m_SSShadowsRenderTarget = nullptr;
    }
}

void RenderPipelineShadowStage::calculateShadowCascades(const scene::SceneData& data, const math::Vector3D& lightDirection, LightNodeEntry& entry)
{
    v3d::scene::Camera& camera = data.m_viewportState._camera->getCamera();
    const u32 cascadeCount = data.m_settings._shadowmapCascadeCount;

    const f32 cascadeSplitLambda = 0.90f;
    f32 cameraNear = camera.getNear();
    f32 cameraFar = std::min(camera.getFar(), data.m_settings._shadowmapLongRange);
    f32 clipRange = cameraFar - cameraNear;

    // Calculate split depths based on view camera frustum
    // Based on method presented in https://developer.nvidia.com/gpugems/GPUGems3/gpugems3_ch10.html
    std::vector<f32> depthSplits(cascadeCount);
    for (u32 i = 0; i < cascadeCount; ++i)
    {
        f32 p = (i + 1) / (f32)(cascadeCount);
        f32 log = cameraNear * std::pow(cameraFar / cameraNear, p);

        f32 uniform = cameraNear + clipRange * p;
        f32 d = cascadeSplitLambda * (log - uniform) + uniform;
        depthSplits[i] = (d - cameraNear) / clipRange;
    }


    // Project frustum corners into world space
    math::Matrix4D depthProjMatrix = math::SMatrix::projectionMatrixPerspective(camera.getFOV() * math::k_degToRad, camera.getAspectRatio(), cameraNear, cameraFar);
    math::Matrix4D invCam = depthProjMatrix * camera.getViewMatrix();
    invCam.makeInverse();

    // Calculate orthographic projection matrix for each cascade
    f32 lastSplitDist = 0.0;
#if REVERSED_DEPTH
    const f32 zNearCorner = 1.0f;
    const f32 zFarCorner = 0.0f;
#else
    const f32 zNearCorner = 0.0f;
    const f32 zFarCorner = 1.0f;
#endif
    for (u32 i = 0; i < cascadeCount; ++i)
    {
        f32 splitDist = depthSplits[i];

        math::Vector3D frustumCorners[8] =
        {
            { -1.0f,  1.0f, zNearCorner },
            {  1.0f,  1.0f, zNearCorner },
            {  1.0f, -1.0f, zNearCorner },
            { -1.0f, -1.0f, zNearCorner },

            { -1.0f,  1.0f, zFarCorner  },
            {  1.0f,  1.0f, zFarCorner  },
            {  1.0f, -1.0f, zFarCorner  },
            { -1.0f, -1.0f, zFarCorner  },
        };


        for (u32 i = 0; i < 8; i++)
        {
            math::Vector4D invCorner = invCam * math::Vector4D(frustumCorners[i].getX(), frustumCorners[i].getY(), frustumCorners[i].getZ(), 1.0f);
            invCorner = invCorner / invCorner.getW();
            frustumCorners[i].set(invCorner.getX(), invCorner.getY(), invCorner.getZ());
        }

        for (u32 i = 0; i < 4; i++)
        {
            math::Vector3D dist = frustumCorners[i + 4] - frustumCorners[i];
            frustumCorners[i + 4] = frustumCorners[i] + (dist * splitDist);
            frustumCorners[i] = frustumCorners[i] + (dist * lastSplitDist);
        }

        // Get frustum center
        math::Vector3D frustumCenter;
        for (u32 i = 0; i < 8; i++)
        {
            frustumCenter += frustumCorners[i];
        }
        frustumCenter /= 8.0f;

        f32 radius = 0.0f;
        for (u32 i = 0; i < 8; i++)
        {
            f32 distance = (frustumCorners[i] - frustumCenter).length();
            radius = std::max(radius, distance);
        }
        radius = std::ceil(radius * 16.0f) / 16.0f;

        math::Vector3D maxExtents;
        maxExtents.set(radius, radius, radius);
        math::Vector3D minExtents = -maxExtents;

        math::Vector3D lightDir(lightDirection);
        lightDir.normalize();

        f32 zNear = -10.0f; //-1.0 - may fix near clip 
        f32 zFar = maxExtents.getZ() - minExtents.getZ();
        math::Matrix4D lightOrthoMatrix = math::SMatrix::projectionMatrixOrtho(minExtents.getX(), maxExtents.getX(), minExtents.getY(), maxExtents.getY(), zNear, zFar);

        math::Vector3D cameraPos = frustumCenter - lightDir * -minExtents.getZ();
        math::Matrix4D lightViewMatrix = math::SMatrix::lookAtMatrix(cameraPos, frustumCenter, math::Vector3D(0.0f, 1.0f, 0.0f));

        //TODO optimize
        math::Matrix4D shadowMatrix = lightOrthoMatrix * lightViewMatrix;
        math::Vector4D shadowOrigin(0.0f, 0.0f, 0.0f, 1.0f);
        shadowOrigin = shadowMatrix * shadowOrigin;
        shadowOrigin = shadowOrigin * data.m_settings._shadowmapSize._height / 2.0f;
        math::Vector4D roundedOrigin(std::round(shadowOrigin.getX()), std::round(shadowOrigin.getY()), std::round(shadowOrigin.getZ()), std::round(shadowOrigin.getW()));
        math::Vector4D roundOffset = roundedOrigin - shadowOrigin;
        roundOffset = roundOffset * 2.0f / data.m_settings._shadowmapSize._height;
        roundOffset.setZ(0.0f);
        roundOffset.setW(0.0f);

        f32 matrix[16];
        lightOrthoMatrix.get(matrix);
        matrix[12] += roundOffset.getX();
        matrix[13] += roundOffset.getY();
        matrix[14] += roundOffset.getZ();
        matrix[15] += roundOffset.getW();
        lightOrthoMatrix.set(matrix);
        //

        entry.cascadeSplits[i] = camera.getNear() + splitDist * clipRange;
        entry.lightSpaceMatrix[i] = lightOrthoMatrix * lightViewMatrix;

        lastSplitDist = depthSplits[i];
    }
}

} //namespace scene
} //namespace v3d
