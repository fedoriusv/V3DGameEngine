#include "SimpleTriangle.h"

#include "Renderer/Shader.h"
#include "Renderer/Object/Texture.h"

#include "Resource/ResourceLoaderManager.h"
#include "Resource/ShaderSourceStreamLoader.h"

#include "Stream/StreamManager.h"


using namespace v3d;

SimpleTriangle::SimpleTriangle()
    : m_CommandList(nullptr)

    , m_Program(nullptr)
    , m_RenderTarget(nullptr)
    , m_Pipeline(nullptr)

    , m_Camera()
{
}

SimpleTriangle::~SimpleTriangle()
{
}

void SimpleTriangle::init(v3d::renderer::CommandList* commandList, const core::Dimension2D& size)
{
    m_CommandList = commandList;
    ASSERT(m_CommandList, "nullptr");

    const std::string vertexSource("\
        struct VS_INPUT\n\
        {\n\
            float3 Position;\n\
            float3 Color;\n\
        };\n\
        \n\
        struct VS_OUTPUT\n\
        {\n\
            float4 Pos : SV_POSITION;\n\
            float4 Col : COLOR;\n\
        };\n\
        \n\
        cbuffer ConstantBuffer\n\
        {\n\
            matrix projectionMatrix;\n\
            matrix modelMatrix;\n\
            matrix viewMatrix;\n\
        };\n\
        \n\
        VS_OUTPUT main(VS_INPUT Input)\n\
        {\n\
            VS_OUTPUT Out;\n\
            Out.Pos = mul(modelMatrix, float4(Input.Position, 1.0));\n\
            Out.Pos = mul(viewMatrix, Out.Pos);\n\
            Out.Pos = mul(projectionMatrix, Out.Pos);\n\
            Out.Col = float4(Input.Color, 1.0);\n\
            return Out;\n\
        }");
    stream::Stream* vertexStream = stream::StreamManager::createMemoryStream(vertexSource);

    renderer::ShaderHeader vertexHeader(renderer::ShaderType::ShaderType_Vertex);
    vertexHeader._contentType = renderer::ShaderHeader::ShaderResource::ShaderResource_Source;
    vertexHeader._shaderLang = renderer::ShaderHeader::ShaderLang::ShaderLang_HLSL;

    renderer::Shader* vertShader = resource::ResourceLoaderManager::getInstance()->composeShader<renderer::Shader, resource::ShaderSourceStreamLoader>(
        m_CommandList->getContext(), "vertex", &vertexHeader, vertexStream);

    const renderer::Shader* fragShader = nullptr;

    //m_Program = m_CommandList->createObject<renderer::ShaderProgram>({ vertShader, fragShader });

    m_RenderTarget = m_CommandList->createObject<renderer::RenderTargetState>(size);
    m_RenderTarget->setColorTexture(0, m_CommandList->getBackbuffer(), renderer::RenderTargetLoadOp::LoadOp_Clear, renderer::RenderTargetStoreOp::StoreOp_Store, core::Vector4D(0.0f));
    renderer::Texture2D* depthAttachment = m_CommandList->createObject<renderer::Texture2D>(renderer::TextureUsage::TextureUsage_Attachment, renderer::Format::Format_D32_SFloat_S8_UInt, size, renderer::TextureSamples::TextureSamples_x1);
    m_RenderTarget->setDepthStencilTexture(depthAttachment, renderer::RenderTargetLoadOp::LoadOp_Clear, renderer::RenderTargetStoreOp::StoreOp_DontCare, 1.0f);

    renderer::VertexInputAttribDescription vertexDesc;

    m_Pipeline = m_CommandList->createObject<renderer::GraphicsPipelineState>(vertexDesc, m_Program, m_RenderTarget);
    m_Pipeline->setPrimitiveTopology(renderer::PrimitiveTopology::PrimitiveTopology_TriangleList);
    m_Pipeline->setFrontFace(renderer::FrontFace::FrontFace_CounterClockwise);
    m_Pipeline->setCullMode(renderer::CullMode::CullMode_None);
    m_Pipeline->setColorMask(renderer::ColorMask::ColorMask_All);
    m_Pipeline->setDepthCompareOp(renderer::CompareOperation::CompareOp_Less);
    m_Pipeline->setDepthWrite(true);
    m_Pipeline->setDepthTest(true);

    m_CommandList->setPipelineState(m_Pipeline);
    m_CommandList->setRenderTarget(m_RenderTarget);

    m_CommandList->sumitCommands();
    m_CommandList->flushCommands();
}

void SimpleTriangle::update()
{
    //update uniforms
}

void SimpleTriangle::render()
{
    //render
    m_CommandList->setViewport(core::Rect32(0, 0, m_RenderTarget->getDimension().width, m_RenderTarget->getDimension().height));
    m_CommandList->setScissor(core::Rect32(0, 0, m_RenderTarget->getDimension().width, m_RenderTarget->getDimension().height));

    m_CommandList->setPipelineState(m_Pipeline);
    m_CommandList->setRenderTarget(m_RenderTarget);

    //draw
    //m_CommandList->draw();
}

void SimpleTriangle::terminate()
{
    renderer::Texture2D* depthAttachment = m_RenderTarget->getDepthStencilTexture();
    if (depthAttachment)
    {
        delete depthAttachment;
    }

    if (m_Pipeline)
    {
        delete m_Pipeline;
        m_Pipeline = nullptr;
    }

    if (m_RenderTarget)
    {
        delete m_RenderTarget;
        m_RenderTarget = nullptr;
    }

    if (m_Program)
    {
        delete m_Program;
        m_Program = nullptr;
    }
}