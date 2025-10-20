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
{
}

RenderPipelineShadowStage::~RenderPipelineShadowStage()
{
}

void RenderPipelineShadowStage::create(renderer::Device* device, scene::SceneData& scene, scene::FrameData& frame)
{
    createRenderTarget(device, scene);

    const renderer::VertexShader* vertShader = resource::ResourceManager::getInstance()->loadShader<renderer::VertexShader, resource::ShaderSourceFileLoader>(device,
        "light_shadows.hlsl", "light_shadows_vs", {}, {}, resource::ShaderCompileFlag::ShaderCompile_UseDXCompilerForSpirV);
    const renderer::FragmentShader* fragShader = resource::ResourceManager::getInstance()->loadShader<renderer::FragmentShader, resource::ShaderSourceFileLoader>(device,
        "light_shadows.hlsl", "light_shadows_ps", {}, {}, resource::ShaderCompileFlag::ShaderCompile_UseDXCompilerForSpirV);

    renderer::RenderPassDesc desc{};
    desc._countColorAttachment = 0;
    desc._hasDepthStencilAttachment = true;
    desc._attachmentsDesc.back()._format = renderer::Format::Format_D32_SFloat;

    m_pipeline = V3D_NEW(renderer::GraphicsPipelineState, memory::MemoryLabel::MemoryGame)(device, VertexFormatStandardDesc, desc,
        V3D_NEW(renderer::ShaderProgram, memory::MemoryLabel::MemoryGame)(device, vertShader, fragShader), "shadowmap_pipeline");

    m_pipeline->setPrimitiveTopology(renderer::PrimitiveTopology::PrimitiveTopology_TriangleList);
    m_pipeline->setFrontFace(renderer::FrontFace::FrontFace_Clockwise);
    m_pipeline->setCullMode(renderer::CullMode::CullMode_Back);
#if ENABLE_REVERSED_Z
    m_pipeline->setDepthCompareOp(renderer::CompareOperation::GreaterOrEqual);
#else
    m_pipeline->setDepthCompareOp(renderer::CompareOperation::LessOrEqual);
#endif
    m_pipeline->setDepthWrite(true);
    m_pipeline->setDepthTest(true);
    m_pipeline->setColorMask(0, renderer::ColorMask::ColorMask_None);
    m_pipeline->setDepthBias(0.0f, 0.0f, 5.0f);

    BIND_SHADER_PARAMETER(m_pipeline, m_parameters, cb_ShadowBuffer);
}

void RenderPipelineShadowStage::destroy(renderer::Device* device, scene::SceneData& scene, scene::FrameData& frame)
{
    destroyRenderTarget(device, scene);

    const renderer::ShaderProgram* program = m_pipeline->getShaderProgram();
    V3D_DELETE(program, memory::MemoryLabel::MemoryGame);

    V3D_DELETE(m_pipeline, memory::MemoryLabel::MemoryGame);
    m_pipeline = nullptr;
}

void RenderPipelineShadowStage::prepare(renderer::Device* device, scene::SceneData& scene, scene::FrameData& frame)
{
    if (m_cascadeRenderTargets.empty())
    {
        createRenderTarget(device, scene);
    }
    else if (m_cascadeTextureArray->getWidth() != scene.m_settings._shadowmapSize._width || m_cascadeTextureArray->getHeight() != scene.m_settings._shadowmapSize._height)
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

    TRACE_PROFILER_SCOPE("Shadow", color::rgba8::GREEN);
    DEBUG_MARKER_SCOPE(cmdList, "Shadow", color::rgbaf::GREEN);

    ASSERT(scene.m_renderLists[toEnumType(scene::RenderPipelinePass::DirectionLight)].size() == 1, "supported only one light at the moment");
    if (!scene.m_renderLists[toEnumType(scene::RenderPipelinePass::DirectionLight)].empty())
    {
        scene::LightNodeEntry& itemLight = *static_cast<scene::LightNodeEntry*>(scene.m_renderLists[toEnumType(scene::RenderPipelinePass::DirectionLight)][0]);
        const scene::DirectionalLight& dirLight = *static_cast<const scene::DirectionalLight*>(itemLight.light);
        calculateShadowCascades(scene, itemLight.object->getDirection(), itemLight);

        for (u32 layer = 0; layer < scene.m_settings._shadowmapCascadeCount; ++layer)
        {
            cmdList->beginRenderTarget(*m_cascadeRenderTargets[layer]);
            cmdList->setViewport({ 0.f, 0.f, (f32)scene.m_settings._shadowmapSize._width, (f32)scene.m_settings._shadowmapSize._height });
            cmdList->setScissor({ 0.f, 0.f, (f32)scene.m_settings._shadowmapSize._width, (f32)scene.m_settings._shadowmapSize._height });
            cmdList->setPipelineState(*m_pipeline);

            for (auto& entry : scene.m_renderLists[toEnumType(scene::RenderPipelinePass::Shadowmap)])
            {
                const scene::DrawNodeEntry& itemMesh = *static_cast<scene::DrawNodeEntry*>(entry);

                struct ShadowViewBuffer
                {
                    math::Matrix4D lightSpaceMatrix;
                    math::Matrix4D modelMatrix;
                };

                ShadowViewBuffer shadowViewBuffer;
                shadowViewBuffer.lightSpaceMatrix = itemLight.lightSpaceMatrix[layer];
                shadowViewBuffer.modelMatrix = itemMesh.object->getTransform().getMatrix();

                cmdList->bindDescriptorSet(m_pipeline->getShaderProgram(), 0,
                    {
                        renderer::Descriptor(renderer::Descriptor::ConstantBuffer{ &shadowViewBuffer, 0, sizeof(shadowViewBuffer) }, m_parameters.cb_ShadowBuffer)
                    });

                DEBUG_MARKER_SCOPE(cmdList, std::format("Object {}, pipeline {}", itemMesh.object->ID(), m_pipeline->getName()), color::rgbaf::LTGREY);

                const scene::Mesh& mesh = *static_cast<scene::Mesh*>(itemMesh.geometry);
                ASSERT(mesh.getVertexAttribDesc()._inputBindings[0]._stride == sizeof(scene::VertexFormatStandard), "must be same");
                renderer::GeometryBufferDesc desc(mesh.getIndexBuffer(), 0, mesh.getVertexBuffer(0), 0, sizeof(scene::VertexFormatStandard), 0);
                cmdList->drawIndexed(desc, 0, mesh.getIndexBuffer()->getIndicesCount(), 0, 0, 1);
            }

            cmdList->endRenderTarget();
        }
    }
}

void RenderPipelineShadowStage::createRenderTarget(renderer::Device* device, scene::SceneData& data)
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
                renderer::TransitionOp::TransitionOp_DepthStencilAttachment, renderer::TransitionOp::TransitionOp_ShaderRead
            });

        m_cascadeRenderTargets.push_back(renderTarget);
    }

    data.m_globalResources.bind("shadowmap", m_cascadeTextureArray);
}

void RenderPipelineShadowStage::destroyRenderTarget(renderer::Device* device, scene::SceneData& data)
{
    ASSERT(m_cascadeTextureArray != nullptr, "must be valid");
    V3D_DELETE(m_cascadeTextureArray, memory::MemoryLabel::MemoryGame);

    for (u32 layer = 0; layer < data.m_settings._shadowmapCascadeCount; ++layer)
    {
        V3D_DELETE(m_cascadeRenderTargets[layer], memory::MemoryLabel::MemoryGame);
    }
    m_cascadeRenderTargets.clear();
}

void RenderPipelineShadowStage::calculateShadowCascades(const scene::SceneData& data, const math::Vector3D& lightDirection, LightNodeEntry& entry)
{
    v3d::scene::Camera& camera = data.m_viewportState._camera->getCamera();
    const u32 cascadeCount = data.m_settings._shadowmapCascadeCount;

    const f32 cascadeSplitLambda = 0.95f;
    f32 clipRange = camera.getFar() - camera.getNear();

    // Calculate split depths based on view camera frustum
    // Based on method presented in https://developer.nvidia.com/gpugems/GPUGems3/gpugems3_ch10.html
    std::vector<f32> depthSplits(cascadeCount);
    for (u32 i = 0; i < cascadeCount; ++i)
    {
        f32 p = (i + 1) / (f32)(cascadeCount);
        f32 log = camera.getNear() * std::pow(camera.getFar() / camera.getNear(), p);

        f32 uniform = camera.getNear() + clipRange * p;
        f32 d = cascadeSplitLambda * (log - uniform) + uniform;
        depthSplits[i] = (d - camera.getNear()) / clipRange;
    }

    // Calculate orthographic projection matrix for each cascade
    f32 lastSplitDist = 0.0;
    for (u32 i = 0; i < cascadeCount; ++i)
    {
        f32 splitDist = depthSplits[i];

        math::Vector3D frustumCorners[8] =
        {
            { -1.0f,  1.0f, -1.0f },
            {  1.0f,  1.0f, -1.0f },
            {  1.0f, -1.0f, -1.0f },
            { -1.0f, -1.0f, -1.0f },

            { -1.0f,  1.0f,  1.0f },
            {  1.0f,  1.0f,  1.0f },
            {  1.0f, -1.0f,  1.0f },
            { -1.0f, -1.0f,  1.0f },
        };

        // Project frustum corners into world space
        math::Matrix4D invCam = camera.getProjectionMatrix() * camera.getViewMatrix();
        invCam.makeInverse();

        for (u32 i = 0; i < 8; i++)
        {
            math::Vector4D invCorner = invCam * math::Vector4D(frustumCorners[i].getX(), frustumCorners[i].getY(), frustumCorners[i].getZ(), 1.0f);
            invCorner = invCorner / invCorner.getW();
            frustumCorners[i] = math::Vector3D(invCorner.getX(), invCorner.getY(), invCorner.getZ());
        }

        for (u32 i = 0; i < 4; i++)
        {
            math::Vector3D dist = frustumCorners[i + 4] - frustumCorners[i];
            frustumCorners[i + 4] = frustumCorners[i] + (dist * splitDist);
            frustumCorners[i] = frustumCorners[i] + (dist * lastSplitDist);
        }

        // Get frustum center
        math::Vector3D frustumCenter = math::Vector3D(0.0f, 0.0f, 0.0f);
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

        math::Vector3D maxExtents = math::Vector3D(radius, radius, radius);
        math::Vector3D minExtents = -maxExtents;

        math::Vector3D lightDir(-lightDirection);
        lightDir.normalize();

        math::Vector3D cameraPos = frustumCenter - lightDir * -minExtents.getZ();
        math::Matrix4D lightViewMatrix = math::SMatrix::lookAtMatrix(cameraPos, frustumCenter, math::Vector3D(0.0f, 1.0f, 0.0f));

        f32 zNear = -1.0f; //-1.0 - may fix near clip 
        f32 zFar = maxExtents.getZ() - minExtents.getZ();
        math::Matrix4D lightOrthoMatrix = math::SMatrix::projectionMatrixOrtho(minExtents.getX(), maxExtents.getX(), minExtents.getY(), maxExtents.getY(), zNear, zFar);

        entry.casadeSplits[i] = camera.getNear() + splitDist * clipRange;
        entry.lightSpaceMatrix[i] = lightOrthoMatrix * lightViewMatrix;

        lastSplitDist = depthSplits[i];
    }
}

} //namespace scene
} //namespace v3d
