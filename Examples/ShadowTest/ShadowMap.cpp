#include "ShadowMap.h"

#include "Renderer/Context.h"
#include "Renderer/DeviceCaps.h"

#include "Resource/ResourceLoaderManager.h"
#include "Resource/ShaderSourceStreamLoader.h"

#include "Stream/StreamManager.h"

#define FORCE_USE_DX_COMPILER 0

namespace v3d
{

const std::string k_vertexSource = { "\
    struct VS_INPUT\n\
    {\n\
        float3 Position : POSITION;\n\
        float3 Normal   : NORMAL;\n\
        float2 Texture  : TEXTURE;\n\
    };\n\
    \n\
    struct VS_OUTPUT\n\
    {\n\
        float4 Pos : SV_POSITION;\n\
    };\n\
    \n\
    struct CBuffer\n\
    {\n\
        matrix lightSpaceMatrix;\n\
        matrix modelMatrix; \n\
    }; \n\
    \n\
    ConstantBuffer<CBuffer> ubo;\n\
    \n\
    VS_OUTPUT main(VS_INPUT Input)\n\
    {\n\
        VS_OUTPUT Out;\n\
        Out.Pos = mul(ubo.modelMatrix, float4(Input.Position, 1.0));\n\
        Out.Pos = mul(ubo.lightSpaceMatrix, Out.Pos);\n\
        return Out;\n\
    }" };

ShadowMapping::ShadowMapping(renderer::CommandList* cmdList) noexcept
    : m_CmdList(cmdList)
{
}

void ShadowMapping::Init(const renderer::VertexInputAttribDescription& desc)
{
    m_ShadowCamera = new scene::CameraHelper(new scene::Camera({}, core::Vector3D(0.0f, 1.0f, 0.0f), true), {});
    m_ShadowCamera->setOrtho({ -m_Extent, m_Extent, m_Extent, -m_Extent }, 0.01f, 256.f);

    m_DepthAttachment = m_CmdList->createObject<renderer::Texture2D>(renderer::TextureUsage::TextureUsage_Attachment | renderer::TextureUsage::TextureUsage_Sampled,
        renderer::Format::Format_D32_SFloat, core::Dimension2D((u32)m_Size.getWidth(), (u32)m_Size.getHeight()), renderer::TextureSamples::TextureSamples_x1, "DepthShadow");

    m_RenderTarget = m_CmdList->createObject<renderer::RenderTargetState>(core::Dimension2D((u32)m_Size.getWidth(), (u32)m_Size.getHeight()), 0, "ShadowTarget");
    m_RenderTarget->setDepthStencilTexture(m_DepthAttachment.get(),
        {
            renderer::RenderTargetLoadOp::LoadOp_Clear, renderer::RenderTargetStoreOp::StoreOp_Store, 1.0f, 
        }, 
        {
             renderer::RenderTargetLoadOp::LoadOp_DontCare, renderer::RenderTargetStoreOp::StoreOp_DontCare, 0U, 
        }, 
        {
            renderer::TransitionOp::TransitionOp_Undefined, renderer::TransitionOp::TransitionOp_ShaderRead 
        });

    const stream::Stream* vertexStream = stream::StreamManager::createMemoryStream(k_vertexSource);

    renderer::ShaderHeader vertexHeader(renderer::ShaderType::ShaderType_Vertex);
    vertexHeader._contentType = renderer::ShaderHeader::ShaderResource::ShaderResource_Source;
    vertexHeader._shaderModel = renderer::ShaderHeader::ShaderModel::ShaderModel_HLSL_5_1;
    const renderer::Shader* vertShader = resource::ResourceLoaderManager::getInstance()->composeShader<renderer::Shader, resource::ShaderSourceStreamLoader>(m_CmdList->getContext(), "shadowmap", &vertexHeader, vertexStream);

    m_Program = m_CmdList->createObject<renderer::ShaderProgram, std::vector<const renderer::Shader*>>({ vertShader });
    m_Pipeline = m_CmdList->createObject<renderer::GraphicsPipelineState>(desc, m_Program.get(), m_RenderTarget.get());
    m_Pipeline->setPrimitiveTopology(renderer::PrimitiveTopology::PrimitiveTopology_TriangleList);
    m_Pipeline->setFrontFace(renderer::FrontFace::FrontFace_Clockwise);
    m_Pipeline->setCullMode(renderer::CullMode::CullMode_Back);
    m_Pipeline->setColorMask(renderer::ColorMask::ColorMask_None);
    m_Pipeline->setDepthCompareOp(renderer::CompareOperation::CompareOp_Less);
    m_Pipeline->setDepthWrite(true);
    m_Pipeline->setDepthTest(true);
    m_Pipeline->setDepthBias(0.0f, 0.0f, 5.0f);

    m_CmdList->submitCommands(true);
    m_CmdList->flushCommands();
}

void ShadowMapping::Update(f32 dt, const core::Vector3D& position, const core::Vector3D& target)
{
    m_ShadowCamera->getCamera().setTarget(target);
    m_ShadowCamera->setPosition(position);

    m_ShadowCamera->update(dt);

    m_LightSpaceMatrix = m_ShadowCamera->getProjectionMatrix() * m_ShadowCamera->getViewMatrix();
}

void ShadowMapping::Draw(scene::ModelHelper* geometry, const scene::Transform& transform)
{
    m_CmdList->setRenderTarget(m_RenderTarget.get());
    m_CmdList->setViewport(m_Size);
    m_CmdList->setScissor(m_Size);

    m_CmdList->setPipelineState(m_Pipeline.get());
    {
        struct UBO
        {
            core::Matrix4D lightSpaceMatrix;
            core::Matrix4D modelMatrix;
        } ubo;


        ubo.lightSpaceMatrix = m_LightSpaceMatrix;
        ubo.modelMatrix = transform.getTransform();

        m_Program->bindUniformsBuffer<renderer::ShaderType::ShaderType_Vertex>({ "ubo" }, 0, (u32)sizeof(UBO), &ubo);
    }
    geometry->draw(m_CmdList);
}

void ShadowMapping::Free()
{
    if (m_ShadowCamera)
    {
        delete m_ShadowCamera;
        m_ShadowCamera = nullptr;
    }

    m_Pipeline = nullptr;
    m_RenderTarget = nullptr;
    m_Program = nullptr;

    m_DepthAttachment = nullptr;
}

const core::Matrix4D& ShadowMapping::GetLightSpaceMatrix() const
{
    return m_LightSpaceMatrix;
}

const renderer::Texture2D* ShadowMapping::GetDepthMap() const
{
    return m_DepthAttachment.get();
}

const u32 CascadedShadowMapping::s_CascadeCount;

CascadedShadowMapping::CascadedShadowMapping(renderer::CommandList* cmdList) noexcept
    : m_CmdList(cmdList)
{
    m_RenderTargets.resize(s_CascadeCount);

    m_LightSpaceMatrices.resize(s_CascadeCount);
    m_CascadeSplits.resize(s_CascadeCount);
}

void CascadedShadowMapping::Init(const renderer::VertexInputAttribDescription& desc)
{
    core::Dimension2D dimension((u32)m_Size.getWidth(), (u32)m_Size.getHeight());
    m_DepthAttachment = m_CmdList->createObject<renderer::Texture2DArray>(renderer::TextureUsage::TextureUsage_Attachment | renderer::TextureUsage::TextureUsage_Sampled, renderer::Format::Format_D32_SFloat,
        dimension, s_CascadeCount, renderer::TextureSamples::TextureSamples_x1, "CascadedShadowMap");

    for (u32 layer = 0; layer < s_CascadeCount; ++layer)
    {
        m_RenderTargets[layer] = m_CmdList->createObject<renderer::RenderTargetState>(dimension);
        m_RenderTargets[layer]->setDepthStencilTexture(m_DepthAttachment.get(), layer,
            {
                renderer::RenderTargetLoadOp::LoadOp_Clear, renderer::RenderTargetStoreOp::StoreOp_Store, 1.0f,
            },
            {
                 renderer::RenderTargetLoadOp::LoadOp_DontCare, renderer::RenderTargetStoreOp::StoreOp_DontCare, 0U,
            },
            {
                renderer::TransitionOp::TransitionOp_Undefined, renderer::TransitionOp::TransitionOp_ShaderRead
            });
    }


    const stream::Stream* vertexStream = stream::StreamManager::createMemoryStream(k_vertexSource);

    renderer::ShaderHeader vertexHeader(renderer::ShaderType::ShaderType_Vertex);
    vertexHeader._contentType = renderer::ShaderHeader::ShaderResource::ShaderResource_Source;
    vertexHeader._shaderModel = renderer::ShaderHeader::ShaderModel::ShaderModel_HLSL_5_1;
    const renderer::Shader* vertShader = resource::ResourceLoaderManager::getInstance()->composeShader<renderer::Shader, resource::ShaderSourceStreamLoader>(m_CmdList->getContext(), "shadowmap", &vertexHeader, vertexStream);

    m_Program = m_CmdList->createObject<renderer::ShaderProgram, std::vector<const renderer::Shader*>>({ vertShader });
    m_Pipeline = m_CmdList->createObject<renderer::GraphicsPipelineState>(desc, m_Program.get(), m_RenderTargets[0].get());
    m_Pipeline->setPrimitiveTopology(renderer::PrimitiveTopology::PrimitiveTopology_TriangleList);
    m_Pipeline->setFrontFace(renderer::FrontFace::FrontFace_Clockwise);
    m_Pipeline->setCullMode(renderer::CullMode::CullMode_Back);
    m_Pipeline->setColorMask(renderer::ColorMask::ColorMask_None);
    m_Pipeline->setDepthCompareOp(renderer::CompareOperation::CompareOp_Less);
    m_Pipeline->setDepthWrite(true);
    m_Pipeline->setDepthTest(true);
    m_Pipeline->setDepthBias(0.0f, 0.0f, 5.0f);

    m_CmdList->submitCommands(true);
    m_CmdList->flushCommands();
}

void CascadedShadowMapping::Update(f32 dt, const scene::Camera& camera, const core::Vector3D& position, const core::Vector3D& target)
{
    CalculateShadowCascades(camera, position - target);
}

void CascadedShadowMapping::Draw(scene::ModelHelper* geometry, const scene::Transform& transform)
{
    for (u32 layer = 0; layer < s_CascadeCount; ++layer)
    {
        m_CmdList->setViewport(core::Rect32(0, 0, m_Size.getWidth(), m_Size.getHeight()));
        m_CmdList->setScissor(core::Rect32(0, 0, m_Size.getWidth(), m_Size.getHeight()));
        m_CmdList->setPipelineState(m_Pipeline.get());
        m_CmdList->setRenderTarget(m_RenderTargets[layer].get());

        struct UBO
        {
            core::Matrix4D lightSpaceMatrix;
            core::Matrix4D modelMatrix;
        } ubo;


        ubo.lightSpaceMatrix = m_LightSpaceMatrices[layer];
        ubo.modelMatrix = transform.getTransform();

        m_Program->bindUniformsBuffer<renderer::ShaderType::ShaderType_Vertex>({ "ubo" }, 0, (u32)sizeof(UBO), &ubo);

        geometry->draw(m_CmdList);
    }
}

void CascadedShadowMapping::Free()
{
    m_Pipeline = nullptr;

    for (u32 layer = 0; layer < s_CascadeCount; ++layer)
    {
        m_RenderTargets[layer] = nullptr;
    }
    m_RenderTargets.clear();
    m_Program = nullptr;

    m_DepthAttachment = nullptr;
}

const renderer::Texture2DArray* v3d::CascadedShadowMapping::GetDepthMap() const
{
    return m_DepthAttachment.get();
}

const std::vector<f32>& v3d::CascadedShadowMapping::GetCascadeSplits() const
{
    return m_CascadeSplits;
}

const std::vector<core::Matrix4D>& v3d::CascadedShadowMapping::GetLightSpaceMatrix() const
{
    return m_LightSpaceMatrices;
}

void v3d::CascadedShadowMapping::CalculateShadowCascades(const v3d::scene::Camera& camera, const core::Vector3D& light)
{
    const f32 cascadeSplitLambda = 0.95f;
    f32 clipRange = camera.getFar() - camera.getNear();

    // Calculate split depths based on view camera frustum
    // Based on method presented in https://developer.nvidia.com/gpugems/GPUGems3/gpugems3_ch10.html
    std::vector<f32> depthSplits(s_CascadeCount);
    for (u32 i = 0; i < s_CascadeCount; ++i)
    {
        f32 p = (i + 1) / (f32)(s_CascadeCount);
        f32 log = camera.getNear() * std::pow(camera.getFar() / camera.getNear(), p);

        f32 uniform = camera.getNear() + clipRange * p;
        f32 d = cascadeSplitLambda * (log - uniform) + uniform;
        depthSplits[i] = (d - camera.getNear()) / clipRange;
    }

    // Calculate orthographic projection matrix for each cascade
    f32 lastSplitDist = 0.0;
    for (u32 i = 0; i < s_CascadeCount; ++i)
    {
        f32 splitDist = depthSplits[i];

        core::Vector3D frustumCorners[8] =
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
        core::Matrix4D invCam = camera.getProjectionMatrix() * camera.getViewMatrix();
        invCam.makeInverse();

        for (u32 i = 0; i < 8; i++)
        {
            core::Vector4D invCorner = invCam * core::Vector4D(frustumCorners[i], 1.0f);
            invCorner = invCorner / invCorner.w;
            frustumCorners[i] = core::Vector3D(invCorner.x, invCorner.y, invCorner.z);
        }

        for (u32 i = 0; i < 4; i++)
        {
            core::Vector3D dist = frustumCorners[i + 4] - frustumCorners[i];
            frustumCorners[i + 4] = frustumCorners[i] + (dist * splitDist);
            frustumCorners[i] = frustumCorners[i] + (dist * lastSplitDist);
        }

        // Get frustum center
        core::Vector3D frustumCenter = core::Vector3D(0.0f);
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

        core::Vector3D maxExtents = core::Vector3D(radius);
        core::Vector3D minExtents = -maxExtents;

        core::Vector3D lightDir(-light);
        lightDir.normalize();

        core::Vector3D cameraPos = frustumCenter - lightDir * -minExtents.z;
        core::Matrix4D lightViewMatrix = core::buildLookAtMatrix(cameraPos, frustumCenter, core::Vector3D(0.0f, 1.0f, 0.0f));

        f32 zNear = -1.0f; //-1.0 - may fix near clip 
        f32 zFar = maxExtents.z - minExtents.z;
        core::Matrix4D lightOrthoMatrix = core::buildProjectionMatrixOrtho(minExtents.x, maxExtents.x, minExtents.y, maxExtents.y, zNear, zFar);

        m_CascadeSplits[i] = camera.getNear() + splitDist * clipRange;
        m_LightSpaceMatrices[i] = lightOrthoMatrix * lightViewMatrix;

        lastSplitDist = depthSplits[i];
    }
}

ShadowMappingPoint::ShadowMappingPoint(renderer::CommandList* cmdList) noexcept
    : m_CmdList(cmdList)
{
}

void ShadowMappingPoint::Init(const renderer::VertexInputAttribDescription& desc)
{
    m_ShadowCamera = new scene::CameraHelper(new scene::Camera({}, core::Vector3D(0.0f, 1.0f, 0.0f)), {});
    m_ShadowCamera->setPerspective(90.f, m_Size, 0.01f, 256.f);

    m_DepthAttachment = m_CmdList->createObject<renderer::TextureCube>(renderer::TextureUsage::TextureUsage_Attachment | renderer::TextureUsage::TextureUsage_Sampled, renderer::Format::Format_D32_SFloat, m_Size, renderer::TextureSamples::TextureSamples_x1, "CubeDepthShadow");

    if (m_CmdList->getContext()->getDeviceCaps()->supportMultiview && m_UseMultiview)
    {
        PrepareMuiltiviewShadowMap(desc);
    }
    else
    {
        PrepareShadowMap(desc);
    }

    m_CmdList->flushCommands();
}

void ShadowMappingPoint::PrepareShadowMap(const renderer::VertexInputAttribDescription& desc)
{
    for (u32 side = 0; side < s_PointSidesCount; ++side)
    {
        m_RenderTarget[side] = m_CmdList->createObject<renderer::RenderTargetState>(m_Size);
        m_RenderTarget[side]->setDepthStencilTexture(m_DepthAttachment.get(), side,
            {
                renderer::RenderTargetLoadOp::LoadOp_Clear, renderer::RenderTargetStoreOp::StoreOp_Store, 1.0f,
            },
            {
                 renderer::RenderTargetLoadOp::LoadOp_DontCare, renderer::RenderTargetStoreOp::StoreOp_DontCare, 0U,
            },
            {
                renderer::TransitionOp::TransitionOp_Undefined, renderer::TransitionOp::TransitionOp_ShaderRead
            });
    }


    const renderer::Shader* vertShader = nullptr;
    {
        const std::string vertexSource("\
        struct VS_INPUT\n\
        {\n\
            float3 Position : POSITION;\n\
            float3 Normal   : NORMAL;\n\
            float2 Texture  : TEXTURE;\n\
        };\n\
        \n\
        struct PS_INPUT\n\
        {\n\
            float4 Pos      : SV_POSITION;\n\
            float4 Position : POSITION;\n\
            float4 Light    : LIGHT;\n\
        };\n\
        \n\
        struct CBuffer\n\
        {\n\
            matrix lightSpaceMatrix;\n\
            matrix modelMatrix;\n\
            float4 lightPosition;\n\
        }; \n\
        \n\
        ConstantBuffer<CBuffer> ubo;\n\
        \n\
        PS_INPUT main(VS_INPUT Input)\n\
        {\n\
            PS_INPUT Out;\n\
            Out.Position = mul(ubo.modelMatrix, float4(Input.Position, 1.0));\n\
            Out.Pos = mul(ubo.lightSpaceMatrix, Out.Position);\n\
            Out.Light = ubo.lightPosition;\n\
            return Out;\n\
        }");

        const stream::Stream* vertexStream = stream::StreamManager::createMemoryStream(vertexSource);

        renderer::ShaderHeader vertexHeader(renderer::ShaderType::ShaderType_Vertex);
        vertexHeader._contentType = renderer::ShaderHeader::ShaderResource::ShaderResource_Source;
        vertexHeader._shaderModel = renderer::ShaderHeader::ShaderModel::ShaderModel_HLSL_5_1;

        vertShader = resource::ResourceLoaderManager::getInstance()->composeShader<renderer::Shader, resource::ShaderSourceStreamLoader>(m_CmdList->getContext(), "shadowmap_vetex", &vertexHeader, vertexStream);
    }

    const renderer::Shader* fragShader = nullptr;
    {
        const std::string fragmentSource("\
        struct PS_INPUT\n\
        {\n\
            float4 Pos      : SV_POSITION;\n\
            float4 Position : POSITION;\n\
            float4 Light    : LIGHT;\n\
        };\n\
        \n\
        float main(PS_INPUT Input) : SV_DEPTH\n\
        {\n\
            float3 lightVec = Input.Position.xyz - Input.Light.xyz;\n\
            return (length(lightVec) / (FAR_PLANE - NEAR_PLANE)) + DEPTH_BIAS;\n\
        }");

        const stream::Stream* fragmentStream = stream::StreamManager::createMemoryStream(fragmentSource);

        renderer::ShaderHeader fragmentHeader(renderer::ShaderType::ShaderType_Fragment);
        fragmentHeader._contentType = renderer::ShaderHeader::ShaderResource::ShaderResource_Source;
        fragmentHeader._shaderModel = renderer::ShaderHeader::ShaderModel::ShaderModel_HLSL_5_1;
        fragmentHeader._defines.push_back({ "FAR_PLANE", std::to_string(m_ShadowCamera->getCamera().getFar()) });
        fragmentHeader._defines.push_back({ "NEAR_PLANE", std::to_string(m_ShadowCamera->getCamera().getNear()) });
        fragmentHeader._defines.push_back({ "DEPTH_BIAS", std::to_string(0.0001) });

        fragShader = resource::ResourceLoaderManager::getInstance()->composeShader<renderer::Shader, resource::ShaderSourceStreamLoader>(m_CmdList->getContext(), "shadowmap_fragment", &fragmentHeader, fragmentStream);
    }

    m_Program = m_CmdList->createObject<renderer::ShaderProgram, std::vector<const renderer::Shader*>>({ vertShader, fragShader });
    m_Pipeline = m_CmdList->createObject<renderer::GraphicsPipelineState>(desc, m_Program.get(), m_RenderTarget[0].get());
    m_Pipeline->setPrimitiveTopology(renderer::PrimitiveTopology::PrimitiveTopology_TriangleList);
    m_Pipeline->setFrontFace(renderer::FrontFace::FrontFace_Clockwise);
    m_Pipeline->setCullMode(renderer::CullMode::CullMode_Back);
    m_Pipeline->setColorMask(renderer::ColorMask::ColorMask_None);
    m_Pipeline->setDepthCompareOp(renderer::CompareOperation::CompareOp_Less);
    m_Pipeline->setDepthWrite(true);
    m_Pipeline->setDepthTest(true);
}

void ShadowMappingPoint::PrepareMuiltiviewShadowMap(const renderer::VertexInputAttribDescription& desc)
{
    m_RenderTarget[0] = m_CmdList->createObject<renderer::RenderTargetState>(m_Size, 0b00111111);
    m_RenderTarget[0]->setDepthStencilTexture(m_DepthAttachment.get(),
        {
            renderer::RenderTargetLoadOp::LoadOp_Clear, renderer::RenderTargetStoreOp::StoreOp_Store, 1.0f,
        },
        {
             renderer::RenderTargetLoadOp::LoadOp_DontCare, renderer::RenderTargetStoreOp::StoreOp_DontCare, 0U,
        },
        {
            renderer::TransitionOp::TransitionOp_Undefined, renderer::TransitionOp::TransitionOp_ShaderRead
        });

    const renderer::Shader* vertShader = nullptr;
    {
#if FORCE_USE_DX_COMPILER
        ASSERT(m_CmdList->getContext()->getRenderType() == renderer::Context::RenderType::DirectXRender, "must be dx");
        const std::string vertexSourceDXC("\
        struct VS_INPUT\n\
        {\n\
            [[vk::location(4)]] float3 Position : POSITION;\n\
            [[vk::location(5)]] float3 Normal : NORMAL;\n\
            [[vk::location(6)]] float2 Texture : TEXTURE;\n\
        };\n\
        \n\
        struct PS_INPUT\n\
        {\n\
            float4 Pos      : SV_Position;\n\
            float4 Position : POSITION;\n\
            float4 Light    : LIGHT;\n\
        };\n\
        \n\
        struct CBuffer\n\
        {\n\
            matrix lightSpaceMatrix[6];\n\
            matrix modelMatrix;\n\
            float4 lightPosition;\n\
        }; \n\
        \n\
        ConstantBuffer<CBuffer> ubo;\n\
        \n\
        PS_INPUT main(VS_INPUT Input, uint ViewId : SV_ViewID)\n\
        {\n\
            PS_INPUT Out;\n\
            Out.Position = mul(ubo.modelMatrix, float4(Input.Position, 1.0));\n\
            Out.Pos = mul(ubo.lightSpaceMatrix[ViewId], Out.Position);\n\
            Out.Light = ubo.lightPosition;\n\
            return Out;\n\
        }");

        const stream::Stream* vertexStream = stream::StreamManager::createMemoryStream(vertexSourceDXC);

        renderer::ShaderHeader vertexHeader(renderer::ShaderType::ShaderType_Vertex);
        vertexHeader._contentType = renderer::ShaderHeader::ShaderResource::ShaderResource_Source;
        vertexHeader._shaderModel = renderer::ShaderHeader::ShaderModel::ShaderModel_HLSL_6_1;
        vertexHeader._defines.push_back({ "SIDES_COUNT", std::to_string(s_PointSidesCount) });

        vertShader = resource::ResourceLoaderManager::getInstance()->composeShader<renderer::Shader, resource::ShaderSourceStreamLoader>(m_CmdList->getContext(), "shadowmap_vetex_view", &vertexHeader, vertexStream, resource::ShaderSource_UseDXCompiler);
#else
        ASSERT(m_CmdList->getContext()->getRenderType() == renderer::Context::RenderType::VulkanRender, "must be vulkan");
        const std::string vertexSourceGL("\
        #version 450\n\
        #extension GL_EXT_multiview : enable\n\
        \n\
        layout(location = 0) in vec3 inPosition;\n\
        layout(location = 1) in vec3 inNormal;\n\
        layout(location = 2) in vec2 inUV;\n\
        \n\
        layout(binding = 0) uniform UBO\n\
        {\n\
            mat4 projectionMatrix[SIDES_COUNT];\n\
            mat4 modelMatrix;\n\
            vec4 lightPosition;\n\
        } ubo;\n\
        \n\
        layout(location = 0) out vec4 outPosition;\n\
        layout(location = 1) out vec4 outLight;\n\
        \n\
        void main()\n\
        {\n\
            vec4 position = ubo.modelMatrix * vec4(inPosition.xyz, 1.0);\n\
            \n\
            outPosition = position;\n\
            outLight = ubo.lightPosition;\n\
            \n\
            gl_Position = ubo.projectionMatrix[gl_ViewIndex] * position;\n\
        }");

        const stream::Stream* vertexStream = stream::StreamManager::createMemoryStream(vertexSourceGL);

        renderer::ShaderHeader vertexHeader(renderer::ShaderType::ShaderType_Vertex);
        vertexHeader._contentType = renderer::ShaderHeader::ShaderResource::ShaderResource_Source;
        vertexHeader._shaderModel = renderer::ShaderHeader::ShaderModel::ShaderModel_GLSL_450;
        vertexHeader._defines.push_back({ "SIDES_COUNT", std::to_string(s_PointSidesCount) });

        vertShader = resource::ResourceLoaderManager::getInstance()->composeShader<renderer::Shader, resource::ShaderSourceStreamLoader>(m_CmdList->getContext(), "shadowmap_vetex_view", &vertexHeader, vertexStream);
#endif
    }

    const renderer::Shader* fragShader = nullptr;
    {
        const std::string fragmentSource("\
        struct PS_INPUT\n\
        {\n\
            float4 Pos      : SV_POSITION;\n\
            float4 Position : POSITION;\n\
            float4 Light    : LIGHT;\n\
        };\n\
        \n\
        float main(PS_INPUT Input) : SV_DEPTH\n\
        {\n\
            float3 lightVec = Input.Position.xyz - Input.Light.xyz;\n\
            return (length(lightVec) / (FAR_PLANE - NEAR_PLANE)) + DEPTH_BIAS;\n\
        }");

        const stream::Stream* fragmentStream = stream::StreamManager::createMemoryStream(fragmentSource);

        renderer::ShaderHeader fragmentHeader(renderer::ShaderType::ShaderType_Fragment);
        fragmentHeader._contentType = renderer::ShaderHeader::ShaderResource::ShaderResource_Source;
        fragmentHeader._shaderModel = renderer::ShaderHeader::ShaderModel::ShaderModel_HLSL_5_1;
#if FORCE_USE_DX_COMPILER
        fragmentHeader._shaderModel = renderer::ShaderHeader::ShaderModel::ShaderModel_HLSL_6_1;
#endif
        fragmentHeader._defines.push_back({ "FAR_PLANE", std::to_string(m_ShadowCamera->getCamera().getFar()) });
        fragmentHeader._defines.push_back({ "NEAR_PLANE", std::to_string(m_ShadowCamera->getCamera().getNear()) });
        fragmentHeader._defines.push_back({ "DEPTH_BIAS", std::to_string(0.0001) });

        fragShader = resource::ResourceLoaderManager::getInstance()->composeShader<renderer::Shader, resource::ShaderSourceStreamLoader>(m_CmdList->getContext(), "shadowmap_fragment", &fragmentHeader, fragmentStream
#if FORCE_USE_DX_COMPILER
            , resource::ShaderSource_UseDXCompiler
#endif
            );
    }

    m_Program = m_CmdList->createObject<renderer::ShaderProgram, std::vector<const renderer::Shader*>>({ vertShader, fragShader });
    m_Pipeline = m_CmdList->createObject<renderer::GraphicsPipelineState>(desc, m_Program.get(), m_RenderTarget[0].get());
    m_Pipeline->setPrimitiveTopology(renderer::PrimitiveTopology::PrimitiveTopology_TriangleList);
    m_Pipeline->setFrontFace(renderer::FrontFace::FrontFace_Clockwise);
    m_Pipeline->setCullMode(renderer::CullMode::CullMode_Back);
    m_Pipeline->setColorMask(renderer::ColorMask::ColorMask_None);
    m_Pipeline->setDepthCompareOp(renderer::CompareOperation::CompareOp_Less);
    m_Pipeline->setDepthWrite(true);
    m_Pipeline->setDepthTest(true);
}

void ShadowMappingPoint::Update(f32 dt, const core::Vector3D& position)
{
    m_ShadowCamera->setPosition(position);

    //X+
    m_ShadowCamera->setTarget(position + core::Vector3D(1.f, 0.f, 0.f));
    m_ShadowCamera->setUp(core::Vector3D(0.f, 1.f, 0.f));
    m_ShadowCamera->update(dt);
    m_LightSpaceMatrices[0] = m_ShadowCamera->getProjectionMatrix() * m_ShadowCamera->getViewMatrix();

    //X-
    m_ShadowCamera->setTarget(position + core::Vector3D(-1.f, 0.f, 0.f));
    m_ShadowCamera->setUp(core::Vector3D(0.f, 1.f, 0.f));
    m_ShadowCamera->update(dt);
    m_LightSpaceMatrices[1] = m_ShadowCamera->getProjectionMatrix() * m_ShadowCamera->getViewMatrix();

    //Y+
    m_ShadowCamera->setTarget(position + core::Vector3D(0.f, 1.f, 0.f));
    m_ShadowCamera->setUp(core::Vector3D(0.f, 0.f, -1.f));
    m_ShadowCamera->update(dt);
    m_LightSpaceMatrices[2] = m_ShadowCamera->getProjectionMatrix() * m_ShadowCamera->getViewMatrix();

    //Y-
    m_ShadowCamera->setTarget(position + core::Vector3D(0.f, -1.f, 0.f));
    m_ShadowCamera->setUp(core::Vector3D(0.f, 0.f, 1.f));
    m_ShadowCamera->update(dt);
    m_LightSpaceMatrices[3] = m_ShadowCamera->getProjectionMatrix() * m_ShadowCamera->getViewMatrix();

    //Z+
    m_ShadowCamera->setTarget(position + core::Vector3D(0.f, 0.f, 1.f));
    m_ShadowCamera->setUp(core::Vector3D(0.f, 1.f, 0.f));
    m_ShadowCamera->update(dt);
    m_LightSpaceMatrices[4] = m_ShadowCamera->getProjectionMatrix() * m_ShadowCamera->getViewMatrix();

    //Z-
    m_ShadowCamera->setTarget(position + core::Vector3D(0.f, 0.f, -1.f));
    m_ShadowCamera->setUp(core::Vector3D(0.f, 1.f, 0.f));
    m_ShadowCamera->update(dt);
    m_LightSpaceMatrices[5] = m_ShadowCamera->getProjectionMatrix() * m_ShadowCamera->getViewMatrix();
}

void ShadowMappingPoint::Draw(scene::ModelHelper* geometry, const scene::Transform& transform)
{
    if (m_CmdList->getContext()->getDeviceCaps()->supportMultiview && m_UseMultiview)
    {
        m_CmdList->setViewport(core::Rect32(0, 0, m_Size.width, m_Size.height));
        m_CmdList->setScissor(core::Rect32(0, 0, m_Size.width, m_Size.height));

        m_CmdList->setRenderTarget(m_RenderTarget[0].get());
        m_CmdList->setPipelineState(m_Pipeline.get());
        
        struct UBO
        {
            core::Matrix4D lightSpaceMatrix[s_PointSidesCount];
            core::Matrix4D modelMatrix;
            core::Vector4D lightPosition;
        } ubo;

        memcpy(&ubo.lightSpaceMatrix, m_LightSpaceMatrices.data(), sizeof(core::Matrix4D) * s_PointSidesCount);
        ubo.modelMatrix = transform.getTransform();
        ubo.lightPosition = { m_ShadowCamera->getPosition(), 0.0f };

        m_Program->bindUniformsBuffer<renderer::ShaderType::ShaderType_Vertex>({ "ubo" }, 0, (u32)sizeof(UBO), &ubo);

        geometry->draw(m_CmdList);
    }
    else
    {
        for (u32 side = 0; side < s_PointSidesCount; ++side)
        {
            m_CmdList->setViewport(core::Rect32(0, 0, m_Size.width, m_Size.height));
            m_CmdList->setScissor(core::Rect32(0, 0, m_Size.width, m_Size.height));
            m_CmdList->setPipelineState(m_Pipeline.get());
            m_CmdList->setRenderTarget(m_RenderTarget[side].get());

            struct UBO
            {
                core::Matrix4D lightSpaceMatrix;
                core::Matrix4D modelMatrix;
                core::Vector4D lightPosition;
            } ubo;

            ubo.lightSpaceMatrix = m_LightSpaceMatrices[side];
            ubo.modelMatrix = transform.getTransform();
            ubo.lightPosition = { m_ShadowCamera->getPosition(), 0.0f };

            m_Program->bindUniformsBuffer<renderer::ShaderType::ShaderType_Vertex>({ "ubo" }, 0, (u32)sizeof(UBO), &ubo);

            geometry->draw(m_CmdList);
        }
    }
}

void ShadowMappingPoint::Free()
{
    if (m_ShadowCamera)
    {
        delete m_ShadowCamera;
        m_ShadowCamera = nullptr;
    }

    m_Pipeline = nullptr;

    for (u32 side = 0; side < s_PointSidesCount; ++side)
    {
        m_RenderTarget[side] = nullptr;
    }
    m_Program = nullptr;

    m_DepthAttachment = nullptr;
}

const renderer::TextureCube* ShadowMappingPoint::GetDepthMap() const
{
    return m_DepthAttachment.get();
}

const scene::Camera& v3d::ShadowMappingPoint::GetCamera() const
{
    return m_ShadowCamera->getCamera();
}

} //namespace v3d