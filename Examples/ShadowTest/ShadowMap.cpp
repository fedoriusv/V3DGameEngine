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
    //m_ShadowCamera->setPerspective(45.0f, core::Dimension2D((u32)m_Size.getWidth(), (u32)m_Size.getHeight()), 1.0f, 256.f);

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
        vertexHeader._shaderLang = renderer::ShaderHeader::ShaderLang::ShaderLang_GLSL;
        vertexHeader._shaderVersion = renderer::ShaderHeader::ShaderModel::ShaderModel_Default;

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

} //namespace v3d