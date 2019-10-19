#include "SimpleTriangle.h"

#include "Renderer/Shader.h"
#include "Renderer/Object/Texture.h"

#include "Resource/ResourceLoaderManager.h"
#include "Resource/ShaderSourceStreamLoader.h"

#include "Stream/StreamManager.h"

using namespace v3d;

const f32 k_nearValue = 0.01f;
const f32 k_farValue = 30.0f;

SimpleTriangle::SimpleTriangle()
    : m_CommandList(nullptr)

    , m_Program(nullptr)
    , m_RenderTarget(nullptr)
    , m_Pipeline(nullptr)
    , m_Geometry(nullptr)

    , m_Camera(new scene::CameraArcballHelper(new scene::Camera(core::Vector3D(0.0f, 0.0f, 0.0f), core::Vector3D(0.0f, -1.0f, 0.0f)), 3.0f, k_nearValue + 2.0f, k_farValue - 10.0f))
{
}

SimpleTriangle::~SimpleTriangle()
{
}

void SimpleTriangle::init(v3d::renderer::CommandList* commandList, const core::Dimension2D& size)
{
    m_CommandList = commandList;
    ASSERT(m_CommandList, "nullptr");
    m_Camera->setPerspective(45.0f, size, k_nearValue, k_farValue);

    renderer::Shader* vertShader = nullptr;
    {
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

        vertShader = resource::ResourceLoaderManager::getInstance()->composeShader<renderer::Shader, resource::ShaderSourceStreamLoader>(
            m_CommandList->getContext(), "vertex", &vertexHeader, vertexStream);
    }

    const renderer::Shader* fragShader = nullptr;
    {
        const std::string fragmentSource("\
        struct PS_INPUT\n\
        {\n\
            float4 Pos : SV_POSITION;\n\
            float4 Col : COLOR;\n\
        };\n\
        \n\
        float4 main(PS_INPUT Input) : SV_TARGET0\n\
        {\n\
            return Input.Col;\n\
        }");
        stream::Stream* fragmentStream = stream::StreamManager::createMemoryStream(fragmentSource);

        renderer::ShaderHeader fragmentHeader(renderer::ShaderType::ShaderType_Fragment);
        fragmentHeader._contentType = renderer::ShaderHeader::ShaderResource::ShaderResource_Source;
        fragmentHeader._shaderLang = renderer::ShaderHeader::ShaderLang::ShaderLang_HLSL;

        fragShader = resource::ResourceLoaderManager::getInstance()->composeShader<renderer::Shader, resource::ShaderSourceStreamLoader>(
            m_CommandList->getContext(), "fragment", &fragmentHeader, fragmentStream);
    }

    ASSERT(vertShader && fragShader, "nullptr");
    m_Program = m_CommandList->createObject<renderer::ShaderProgram, std::vector<const renderer::Shader*>>({ vertShader, fragShader });

    m_RenderTarget = m_CommandList->createObject<renderer::RenderTargetState>(size);
    m_RenderTarget->setColorTexture(0, m_CommandList->getBackbuffer(), { renderer::RenderTargetLoadOp::LoadOp_Clear, renderer::RenderTargetStoreOp::StoreOp_Store, core::Vector4D(0.0f) },
        { renderer::TransitionOp::TransitionOp_Undefined, renderer::TransitionOp::TransitionOp_Present });

    renderer::Texture2D* depthAttachment = m_CommandList->createObject<renderer::Texture2D>(renderer::TextureUsage::TextureUsage_Attachment, renderer::Format::Format_D24_UNorm_S8_UInt, size, renderer::TextureSamples::TextureSamples_x1);
    m_RenderTarget->setDepthStencilTexture(depthAttachment, { renderer::RenderTargetLoadOp::LoadOp_Clear, renderer::RenderTargetStoreOp::StoreOp_DontCare, 1.0f}, { renderer::RenderTargetLoadOp::LoadOp_DontCare, renderer::RenderTargetStoreOp::StoreOp_DontCare, 0 },
        { renderer::TransitionOp::TransitionOp_Undefined, renderer::TransitionOp::TransitionOp_DepthStencilAttachmet });

    std::vector<core::Vector3D> geometryData = 
    {
        {-1.0f,-1.0f, 0.0f },  { 1.0f, 0.0f, 0.0f },
        { 1.0f,-1.0f, 0.0f },  { 0.0f, 1.0f, 0.0f },
        { 0.0f, 1.0f, 0.0f },  { 0.0f, 0.0f, 1.0f },
    };
    m_Geometry = m_CommandList->createObject<renderer::VertexStreamBuffer>(renderer::StreamBuffer_Write, static_cast<u32>(geometryData.size() * sizeof(core::Vector3D)), reinterpret_cast<u8*>(geometryData.data()));

    renderer::VertexInputAttribDescription vertexDesc(
        { 
            renderer::VertexInputAttribDescription::InputBinding(0,  renderer::VertexInputAttribDescription::InputRate_Vertex, sizeof(core::Vector3D) + sizeof(core::Vector3D)),
        }, 
        { 
            renderer::VertexInputAttribDescription::InputAttribute(0, 0, renderer::Format_R32G32B32_SFloat, 0),
            renderer::VertexInputAttribDescription::InputAttribute(0, 0, renderer::Format_R32G32B32_SFloat, sizeof(core::Vector3D)),
        });

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

    m_CommandList->submitCommands();
    m_CommandList->flushCommands();
}

void SimpleTriangle::update(f32 dt)
{
    m_Camera->update(dt);

    //update uniforms
    struct UBO
    {
        core::Matrix4D projectionMatrix;
        core::Matrix4D modelMatrix;
        core::Matrix4D viewMatrix;
    };

    UBO ubo;
    ubo.projectionMatrix = m_Camera->getCamera().getProjectionMatrix();
    ubo.modelMatrix.setTranslation(core::Vector3D(0, 0, 0));
    ubo.viewMatrix = m_Camera->getCamera().getViewMatrix();

    m_Program->bindUniformsBuffer<renderer::ShaderType::ShaderType_Vertex>(0, 0, (u32)sizeof(UBO), &ubo);
}

void SimpleTriangle::render()
{
    //render
    m_CommandList->setViewport(core::Rect32(0, 0, m_RenderTarget->getDimension().width, m_RenderTarget->getDimension().height));
    m_CommandList->setScissor(core::Rect32(0, 0, m_RenderTarget->getDimension().width, m_RenderTarget->getDimension().height));

    m_CommandList->setRenderTarget(m_RenderTarget);
    m_CommandList->setPipelineState(m_Pipeline);

    m_CommandList->draw(renderer::StreamBufferDescription(m_Geometry, 0), 0, 3, 1);
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

    if (m_Geometry)
    {
        delete m_Geometry;
        m_Geometry = nullptr;
    }

    resource::ResourceLoaderManager::getInstance()->clear();
    resource::ResourceLoaderManager::getInstance()->freeInstance();

    if (m_Camera)
    {
        delete m_Camera;
        m_Camera = nullptr;
    }
}

bool SimpleTriangle::handleInputEvent(v3d::event::InputEventHandler* handler, const v3d::event::InputEvent* event)
{
    if (event->_eventType == event::InputEvent::InputEventType::MouseInputEvent)
    {
        const event::MouseInputEvent* mouseEvent = static_cast<const event::MouseInputEvent*>(event);
        m_Camera->handlerMouseCallback(handler, mouseEvent);

        return true;
    }
    else if (event->_eventType == event::InputEvent::InputEventType::TouchInputEvent)
    {
        const event::TouchInputEvent* touchEvent = static_cast<const event::TouchInputEvent*>(event);
        m_Camera->handlerTouchCallback(handler, touchEvent);

        return true;
    }

    return false;
}

bool SimpleTriangle::dispachEvent(SimpleTriangle* render, v3d::event::InputEventHandler* handler, const v3d::event::InputEvent* event)
{
    return render->handleInputEvent(handler, event);
}
