#include "ShadowMap.h"

#include "Resource/ResourceLoaderManager.h"
#include "Resource/ShaderSourceStreamLoader.h"

#include "Stream/StreamManager.h"

namespace v3d
{

ShadowMapping::ShadowMapping(renderer::CommandList* cmdList) noexcept
    : m_CmdList(cmdList)
{
}

void ShadowMapping::Init(const renderer::VertexInputAttribDescription& desc)
{
    m_ShadowCamera = new scene::CameraHelper(new scene::Camera({}, core::Vector3D(0.0f, 1.0f, 0.0f), true), {});
    m_ShadowCamera->setOrtho({ -m_Scale, m_Scale, m_Scale, -m_Scale }, 0.01f, 256.f);

    m_DepthAttachment = m_CmdList->createObject<renderer::Texture2D>(renderer::TextureUsage::TextureUsage_Attachment | renderer::TextureUsage::TextureUsage_Sampled,
        renderer::Format::Format_D32_SFloat, core::Dimension2D((u32)m_Size.getWidth(), (u32)m_Size.getHeight()), renderer::TextureSamples::TextureSamples_x1, "DepthShadow");

    m_RenderTarget = m_CmdList->createObject<renderer::RenderTargetState>(core::Dimension2D((u32)m_Size.getWidth(), (u32)m_Size.getHeight()), "ShadowTarget");
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

    renderer::Shader* vertShader = nullptr;
    {
        const std::string vertexSource("\
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
        }");

        const stream::Stream* vertexStream = stream::StreamManager::createMemoryStream(vertexSource);

        renderer::ShaderHeader vertexHeader(renderer::ShaderType::ShaderType_Vertex);
        vertexHeader._contentType = renderer::ShaderHeader::ShaderResource::ShaderResource_Source;
        vertexHeader._shaderModel = renderer::ShaderHeader::ShaderModel::ShaderModel_HLSL_5_1;

        vertShader = resource::ResourceLoaderManager::getInstance()->composeShader<renderer::Shader, resource::ShaderSourceStreamLoader>(m_CmdList->getContext(), "shadowmap", &vertexHeader, vertexStream);
    }

    m_Program = m_CmdList->createObject<renderer::ShaderProgram, std::vector<const renderer::Shader*>>({ vertShader });
    m_Pipeline = m_CmdList->createObject<renderer::GraphicsPipelineState>(desc, m_Program.get(), m_RenderTarget.get());
    m_Pipeline->setPrimitiveTopology(renderer::PrimitiveTopology::PrimitiveTopology_TriangleList);
    m_Pipeline->setFrontFace(renderer::FrontFace::FrontFace_Clockwise);
    m_Pipeline->setCullMode(renderer::CullMode::CullMode_Back);
    m_Pipeline->setColorMask(renderer::ColorMask::ColorMask_None);
    m_Pipeline->setDepthCompareOp(renderer::CompareOperation::CompareOp_Less);
    m_Pipeline->setDepthWrite(true);
    m_Pipeline->setDepthTest(true);
    m_Pipeline->setDepthBias(0.0f, 0.0f, 1.75f);

    m_CmdList->flushCommands();
}

void ShadowMapping::Update(f32 dt, const core::Vector3D& position, const core::Vector3D& target)
{
    m_ShadowCamera->getCamera().setTarget(target);
    m_ShadowCamera->setPosition(position);

    m_ShadowCamera->update(dt);

    m_LightSpaceMatrix = m_ShadowCamera->getProjectionMatrix() * m_ShadowCamera->getViewMatrix();
}

void ShadowMapping::Update(f32 dt, const scene::CameraHelper* worldCamera)
{
    m_ShadowCamera->getCamera().setTarget(worldCamera->getTarget());
    m_ShadowCamera->setPosition(worldCamera->getPosition());

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
    geometry->draw();

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



ShadowMappingPoint::ShadowMappingPoint(renderer::CommandList* cmdList) noexcept
    : m_CmdList(cmdList)
{
}

void ShadowMappingPoint::Init(const renderer::VertexInputAttribDescription& desc)
{
    m_ShadowCamera = new scene::CameraHelper(new scene::Camera({}, core::Vector3D(0.0f, 1.0f, 0.0f)), {});
    m_ShadowCamera->setPerspective(90.f, m_Size, 0.01f, 256.f);

    m_DepthAttachment = m_CmdList->createObject<renderer::TextureCube>(renderer::TextureUsage::TextureUsage_Attachment | renderer::TextureUsage::TextureUsage_Sampled, renderer::Format::Format_D32_SFloat, m_Size, renderer::TextureSamples::TextureSamples_x1, "CubeDepthShadow");

    for (u32 side = 0; side < 6U; ++side)
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

    renderer::Shader* vertShader = nullptr;
    {
        const std::string vertexSource("\
        #version 450\n\
        \n\
        layout(location = 0) in vec3 inPosition;\n\
        layout(location = 1) in vec3 inNormal;\n\
        layout(location = 2) in vec2 inUV;\n\
        \n\
        layout(binding = 0) uniform UBO\n\
        {\n\
            mat4 lightSpaceMatrix;\n\
            mat4 modelMatrix;\n\
        } ubo;\n\
        \n\
        void main()\n\
        {\n\
            vec4 position = ubo.modelMatrix * vec4(inPosition.xyz, 1.0);\n\
            gl_Position = ubo.lightSpaceMatrix * position;\n\
        }");
        const stream::Stream* vertexStream = stream::StreamManager::createMemoryStream(vertexSource);

        renderer::ShaderHeader vertexHeader(renderer::ShaderType::ShaderType_Vertex);
        vertexHeader._contentType = renderer::ShaderHeader::ShaderResource::ShaderResource_Source;
        vertexHeader._shaderModel = renderer::ShaderHeader::ShaderModel::ShaderModel_GLSL_450;

        vertShader = resource::ResourceLoaderManager::getInstance()->composeShader<renderer::Shader, resource::ShaderSourceStreamLoader>(m_CmdList->getContext(), "shadowmap", &vertexHeader, vertexStream);
    }

    m_Program = m_CmdList->createObject<renderer::ShaderProgram, std::vector<const renderer::Shader*>>({ vertShader });
    m_Pipeline = m_CmdList->createObject<renderer::GraphicsPipelineState>(desc, m_Program.get(), m_RenderTarget[0].get());
    m_Pipeline->setPrimitiveTopology(renderer::PrimitiveTopology::PrimitiveTopology_TriangleList);
    m_Pipeline->setFrontFace(renderer::FrontFace::FrontFace_Clockwise);
    m_Pipeline->setCullMode(renderer::CullMode::CullMode_Back);
    m_Pipeline->setColorMask(renderer::ColorMask::ColorMask_None);
    m_Pipeline->setDepthCompareOp(renderer::CompareOperation::CompareOp_Less);
    m_Pipeline->setDepthWrite(true);
    m_Pipeline->setDepthTest(true);
    m_Pipeline->setDepthBias(0.0f, 0.0f, 1.75f);

    m_CmdList->flushCommands();
}

void ShadowMappingPoint::Update(f32 dt, const core::Vector3D& position)
{
    m_ShadowCamera->setPosition(position);

    //X+
    m_ShadowCamera->setTarget(position + core::Vector3D(1.f, 0.f, 0.f));
    m_ShadowCamera->update(dt);
    m_LightSpaceMatrices[0] = m_ShadowCamera->getProjectionMatrix() * m_ShadowCamera->getViewMatrix();

    //X-
    m_ShadowCamera->setTarget(position + core::Vector3D(-1.f, 0.f, 0.f));
    m_ShadowCamera->update(dt);
    m_LightSpaceMatrices[1] = m_ShadowCamera->getProjectionMatrix() * m_ShadowCamera->getViewMatrix();

    //Y+
    m_ShadowCamera->setTarget(position + core::Vector3D(0.f, 1.f, 0.f));
    m_ShadowCamera->update(dt);
    m_LightSpaceMatrices[2] = m_ShadowCamera->getProjectionMatrix() * m_ShadowCamera->getViewMatrix();

    //Y-
    m_ShadowCamera->setTarget(position + core::Vector3D(0.f, -1.f, 0.f));
    m_ShadowCamera->update(dt);
    m_LightSpaceMatrices[3] = m_ShadowCamera->getProjectionMatrix() * m_ShadowCamera->getViewMatrix();

    //Z+
    m_ShadowCamera->setTarget(position + core::Vector3D(0.f, 0.f, 1.f));
    m_ShadowCamera->update(dt);
    m_LightSpaceMatrices[4] = m_ShadowCamera->getProjectionMatrix() * m_ShadowCamera->getViewMatrix();

    //Z-
    m_ShadowCamera->setTarget(position + core::Vector3D(0.f, 0.f, -1.f));
    m_ShadowCamera->update(dt);
    m_LightSpaceMatrices[5] = m_ShadowCamera->getProjectionMatrix() * m_ShadowCamera->getViewMatrix();
}

void ShadowMappingPoint::Draw(scene::ModelHelper* geometry, const scene::Transform& transform)
{
    for (u32 side = 0; side < 6U; ++side)
    {
        m_CmdList->setViewport(core::Rect32(0, 0, m_Size.width, m_Size.height));
        m_CmdList->setScissor(core::Rect32(0, 0, m_Size.width, m_Size.height));
        m_CmdList->setPipelineState(m_Pipeline.get());
        m_CmdList->setRenderTarget(m_RenderTarget[side].get());

        struct UBO
        {
            core::Matrix4D lightSpaceMatrix;
            core::Matrix4D modelMatrix;
        } ubo;


        ubo.lightSpaceMatrix = m_LightSpaceMatrices[side];
        ubo.modelMatrix = transform.getTransform();

        m_Program->bindUniformsBuffer<renderer::ShaderType::ShaderType_Vertex>({ "ubo" }, 0, (u32)sizeof(UBO), &ubo);

        geometry->draw();
        //m_CmdList->submitCommands();
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

    for (u32 side = 0; side < 6U; ++side)
    {
        m_RenderTarget[side] = nullptr;
    }
    m_Program = nullptr;

    m_DepthAttachment = nullptr;
}

} //namespace v3d