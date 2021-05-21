#include "Debug.h"

#include "Stream/StreamManager.h"

#include "Resource/ResourceLoaderManager.h"
#include "Resource/ShaderSourceStreamLoader.h"
#include "Resource/ModelFileLoader.h"

#include "Scene/Model.h"

namespace v3d
{

const std::string k_vertexDebug = { "\
        struct VS_INPUT\n\
        {\n\
            float3 Position : POSITION;\n\
            float3 Normal   : NORMAL;\n\
            float2 UV       : TEXTURE;\n\
        };\n\
        \n\
        struct VS_OUTPUT\n\
        {\n\
            float4 Position : SV_POSITION;\n\
        };\n\
        \n\
        struct CBuffer\n\
        {\n\
            matrix projectionMatrix;\n\
            matrix modelMatrix; \n\
            matrix viewMatrix; \n\
        };\n\
        ConstantBuffer<CBuffer> buffer : register(b0, space0);\n\
        \n\
        VS_OUTPUT main(VS_INPUT Input)\n\
        {\n\
            VS_OUTPUT Output;\n\
            Output.Position = mul(buffer.modelMatrix, float4(Input.Position, 1.0));\n\
            Output.Position = mul(buffer.viewMatrix, Output.Position);\n\
            Output.Position = mul(buffer.projectionMatrix, Output.Position);\n\
            return Output;\n\
        }" };

const std::string k_fragmentDebug = { "\
        struct CBuffer\n\
        {\n\
            float4 color;\n\
        };\n\
        ConstantBuffer<CBuffer> color : register(b1, space0);\n\
        \n\
        float4 main() : SV_TARGET0\n\
        {\n\
            return color.color;\n\
        }" };

void DirectionLightDebug::Init(renderer::CommandList* commandList, renderer::RenderTargetState* renderTarget)
{
    const stream::Stream* vertexStream = stream::StreamManager::createMemoryStream(k_vertexDebug);
    renderer::ShaderHeader vertexHeader(renderer::ShaderType::Vertex);
    vertexHeader._contentType = renderer::ShaderHeader::ShaderResource::ShaderResource_Source;
    vertexHeader._shaderModel = renderer::ShaderHeader::ShaderModel::ShaderModel_HLSL_5_1;
    const renderer::Shader* vertShader = resource::ResourceLoaderManager::getInstance()->composeShader<renderer::Shader, resource::ShaderSourceStreamLoader>(commandList->getContext(), "vertex", &vertexHeader, vertexStream);
    delete vertexStream;

    const stream::Stream* fragmentStream = stream::StreamManager::createMemoryStream(k_fragmentDebug);
    renderer::ShaderHeader fragmentHeader(renderer::ShaderType::Fragment);
    fragmentHeader._contentType = renderer::ShaderHeader::ShaderResource::ShaderResource_Source;
    fragmentHeader._shaderModel = renderer::ShaderHeader::ShaderModel::ShaderModel_HLSL_5_1;
    const renderer::Shader* fragShader = resource::ResourceLoaderManager::getInstance()->composeShader<renderer::Shader, resource::ShaderSourceStreamLoader>(commandList->getContext(), "fragment", &fragmentHeader, fragmentStream);
    delete fragmentStream;

    std::vector<core::Vector3D> geometryData =
    {
        {0.0f, 0.0f, 0.0f },  { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f },
        {0.0f, 0.0f, 1.0f },  { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f },
    };
    m_Geometry = commandList->createObject<renderer::VertexStreamBuffer>(renderer::StreamBuffer_Write, static_cast<u32>(geometryData.size() * sizeof(core::Vector3D)), reinterpret_cast<u8*>(geometryData.data()));

    renderer::VertexInputAttribDescription vertexDesc(
        {
            renderer::VertexInputAttribDescription::InputBinding(0,  renderer::VertexInputAttribDescription::InputRate_Vertex, 3 * sizeof(core::Vector3D)),
        },
        {
            renderer::VertexInputAttribDescription::InputAttribute(0, 0, renderer::Format_R32G32B32_SFloat, 0),
            renderer::VertexInputAttribDescription::InputAttribute(0, 0, renderer::Format_R32G32B32_SFloat, sizeof(core::Vector3D)),
            renderer::VertexInputAttribDescription::InputAttribute(0, 0, renderer::Format_R32G32_SFloat, sizeof(core::Vector3D) + sizeof(core::Vector2D)),
        });

    m_Program = commandList->createObject<renderer::ShaderProgram, std::vector<const renderer::Shader*>>({ vertShader, fragShader });
    m_Pipeline = commandList->createObject<renderer::GraphicsPipelineState>(vertexDesc, m_Program.get(), renderTarget);
    m_Pipeline->setPrimitiveTopology(renderer::PrimitiveTopology::PrimitiveTopology_LineList);
    m_Pipeline->setFrontFace(renderer::FrontFace::FrontFace_CounterClockwise);
    m_Pipeline->setCullMode(renderer::CullMode::CullMode_None);
    m_Pipeline->setColorMask(renderer::ColorMask::ColorMask_All);
    m_Pipeline->setDepthCompareOp(renderer::CompareOperation::CompareOp_Always);
    m_Pipeline->setDepthWrite(true);
    m_Pipeline->setDepthTest(true);

    commandList->flushCommands();
}

void DirectionLightDebug::Draw(renderer::CommandList* commandList, scene::Camera* camera, const core::Vector3D& angle, const core::Vector3D& size)
{
    commandList->setPipelineState(m_Pipeline.get());

    struct UBO
    {
        core::Matrix4D projectionMatrix;
        core::Matrix4D modelMatrix;
        core::Matrix4D viewMatrix;
    } ubo;

    {
        ubo.projectionMatrix = camera->getProjectionMatrix();
        ubo.modelMatrix.makeIdentity();
        ubo.modelMatrix.setTranslation(core::Vector3D(0.0f, 2.0f + 0.5f, 0.0f));
        ubo.modelMatrix.setRotation(angle);
        ubo.viewMatrix = camera->getViewMatrix();

        m_Program->bindUniformsBuffer<renderer::ShaderType::Vertex>({ "buffer" }, 0, sizeof(UBO), &ubo);
        m_Program->bindUniformsBuffer<renderer::ShaderType::Fragment>({ "color" }, 0, sizeof(core::Vector4D), &m_LightColor);

        commandList->draw(renderer::StreamBufferDescription(m_Geometry.get(), 0), 0, 2, 1);
    }

    {
        ubo.projectionMatrix = camera->getProjectionMatrix();
        ubo.modelMatrix.makeIdentity();
        ubo.modelMatrix.setTranslation(core::Vector3D(0.5f, 2.0f + 0.0f, 0.0f));
        ubo.modelMatrix.setRotation(angle);
        ubo.viewMatrix = camera->getViewMatrix();

        m_Program->bindUniformsBuffer<renderer::ShaderType::Vertex>({ "buffer" }, 0, sizeof(UBO), & ubo);
        m_Program->bindUniformsBuffer<renderer::ShaderType::Fragment>({ "color" }, 0, sizeof(core::Vector4D), & m_LightColor);

        commandList->draw(renderer::StreamBufferDescription(m_Geometry.get(), 0), 0, 2, 1);
    }

    {
        ubo.projectionMatrix = camera->getProjectionMatrix();
        ubo.modelMatrix.makeIdentity();
        ubo.modelMatrix.setTranslation(core::Vector3D(-0.5f, 2.0f + 0.0f, 0.0f));
        ubo.modelMatrix.setRotation(angle);
        ubo.viewMatrix = camera->getViewMatrix();

        m_Program->bindUniformsBuffer<renderer::ShaderType::Vertex>({ "buffer" }, 0, sizeof(UBO), &ubo);
        m_Program->bindUniformsBuffer<renderer::ShaderType::Fragment>({ "color" }, 0, sizeof(core::Vector4D), &m_LightColor);

        commandList->draw(renderer::StreamBufferDescription(m_Geometry.get(), 0), 0, 2, 1);
    }

}

void DirectionLightDebug::Free()
{
    m_Pipeline = nullptr;
    m_Program = nullptr;

    m_Geometry = nullptr;
}


void LightDebug::Init(v3d::renderer::CommandList* commandList, v3d::renderer::RenderTargetState* renderTarget)
{
    v3d::scene::Model* cube = resource::ResourceLoaderManager::getInstance()->load<v3d::scene::Model, resource::ModelFileLoader>("data/cube.dae");
    m_Geometry = v3d::scene::ModelHelper::createModelHelper(commandList, { cube });

    const stream::Stream* vertexStream = stream::StreamManager::createMemoryStream(k_vertexDebug);
    renderer::ShaderHeader vertexHeader(renderer::ShaderType::Vertex);
    vertexHeader._contentType = renderer::ShaderHeader::ShaderResource::ShaderResource_Source;
    vertexHeader._shaderModel = renderer::ShaderHeader::ShaderModel::ShaderModel_HLSL_5_1;
    const renderer::Shader* vertShader = resource::ResourceLoaderManager::getInstance()->composeShader<renderer::Shader, resource::ShaderSourceStreamLoader>(commandList->getContext(), "vertex", &vertexHeader, vertexStream);
    delete vertexStream;

    const stream::Stream* fragmentStream = stream::StreamManager::createMemoryStream(k_fragmentDebug);
    renderer::ShaderHeader fragmentHeader(renderer::ShaderType::Fragment);
    fragmentHeader._contentType = renderer::ShaderHeader::ShaderResource::ShaderResource_Source;
    fragmentHeader._shaderModel = renderer::ShaderHeader::ShaderModel::ShaderModel_HLSL_5_1;
    const renderer::Shader* fragShader = resource::ResourceLoaderManager::getInstance()->composeShader<renderer::Shader, resource::ShaderSourceStreamLoader>(commandList->getContext(), "fragment", &fragmentHeader, fragmentStream);
    delete fragmentStream;

    m_Program = commandList->createObject<renderer::ShaderProgram, std::vector<const renderer::Shader*>>({ vertShader, fragShader });
    m_Pipeline = commandList->createObject<renderer::GraphicsPipelineState>(m_Geometry->getVertexInputAttribDescription(0, 0), m_Program.get(), renderTarget);
    m_Pipeline->setPrimitiveTopology(renderer::PrimitiveTopology::PrimitiveTopology_TriangleList);
    m_Pipeline->setFrontFace(renderer::FrontFace::FrontFace_CounterClockwise);
    m_Pipeline->setCullMode(renderer::CullMode::CullMode_None);
    m_Pipeline->setColorMask(renderer::ColorMask::ColorMask_All);
    m_Pipeline->setDepthCompareOp(renderer::CompareOperation::CompareOp_Less);
    m_Pipeline->setDepthWrite(true);
    m_Pipeline->setDepthTest(true);

    commandList->flushCommands();
}

void LightDebug::Draw(v3d::renderer::CommandList* commandList, v3d::scene::Camera* camera, const v3d::core::Vector3D& light)
{
    commandList->setPipelineState(m_Pipeline.get());

    struct UBO
    {
        core::Matrix4D projectionMatrix;
        core::Matrix4D modelMatrix;
        core::Matrix4D viewMatrix;
    } ubo;

    ubo.projectionMatrix = camera->getProjectionMatrix();
    ubo.modelMatrix.makeIdentity();
    ubo.modelMatrix.setTranslation(light);
    ubo.viewMatrix = camera->getViewMatrix();

    m_Program->bindUniformsBuffer<renderer::ShaderType::Vertex>({ "buffer" }, 0, sizeof(UBO), &ubo);
    m_Program->bindUniformsBuffer<renderer::ShaderType::Fragment>({ "color" }, 0, sizeof(core::Vector4D), &m_lightColor);

    m_Geometry->draw(commandList);
}

void LightDebug::Free()
{
    m_Pipeline = nullptr;
    m_Program = nullptr;

    delete m_Geometry;
    m_Geometry = nullptr;
}

} //namespace v3d