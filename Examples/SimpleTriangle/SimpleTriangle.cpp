#include "SimpleTriangle.h"

#include "Renderer/Shader.h"
#include "Resource/ResourceManager.h"
#include "Resource/Loader/ShaderCompiler.h"
#include "Stream/StreamManager.h"
#include "Utils/Logger.h"
#include "FrameProfiler.h"

using namespace v3d;
using namespace v3d::renderer;
using namespace v3d::platform;
using namespace v3d::utils;
using namespace v3d::event;

const f32 k_nearValue = 0.1f;
const f32 k_farValue = 30.0f;

SimpleTriangle::SimpleTriangle()
    : m_Device(nullptr)
    , m_CmdList(nullptr)

    , m_Program(nullptr)
    , m_RenderTarget(nullptr)
    , m_Pipeline(nullptr)
    , m_Geometry(nullptr)

    , m_Camera(std::make_unique<scene::Camera>(math::Vector3D(0.0f, 0.0f, 3.0f), math::Vector3D(0.0f, 0.0f, 0.0f)), 3.0f, k_nearValue + 1.0f, k_farValue - 10.0f)
{
}

SimpleTriangle::~SimpleTriangle()
{
}

void SimpleTriangle::init(renderer::Device* device, renderer::Swapchain* swapchain)
{
    m_Device = device;
    m_CmdList = m_Device->createCommandList<renderer::CmdListRender>(Device::GraphicMask);

    m_Rect = { 0, 0, (f32)swapchain->getBackbufferSize()._width, (f32)swapchain->getBackbufferSize()._height };
    m_Camera.setPerspective(45.0f, swapchain->getBackbufferSize(), k_nearValue, k_farValue);

    const renderer::VertexShader* vertShader = nullptr;
    {
        const std::string vertexSource("\
        struct VS_INPUT\n\
        {\n\
            float3 Position : POSITION;\n\
            float3 Color    : COLOR;\n\
        };\n\
        \n\
        struct VS_OUTPUT\n\
        {\n\
            float4 Pos : SV_POSITION;\n\
            float4 Col : COLOR;\n\
            float4 Test : Test;\n\
        };\n\
        \n\
        struct CBuffer\n\
        {\n\
            matrix projectionMatrix;\n\
            matrix modelMatrix; \n\
            matrix viewMatrix; \n\
        }; \n\
        \n\
        ConstantBuffer<CBuffer> buffer;\n\
        \n\
        VS_OUTPUT main(VS_INPUT Input)\n\
        {\n\
            VS_OUTPUT Out;\n\
            Out.Pos = mul(buffer.modelMatrix, float4(Input.Position, 1.0));\n\
            Out.Pos = mul(buffer.viewMatrix, Out.Pos);\n\
            Out.Pos = mul(buffer.projectionMatrix, Out.Pos);\n\
            Out.Col = float4(Input.Color, 1.0);\n\
            Out.Test = Out.Pos;\n\
            return Out;\n\
        }");
        const stream::Stream* vertexStream = stream::StreamManager::createMemoryStream(vertexSource);

        renderer::Shader::LoadPolicy vertexPolicy;
        vertexPolicy.type = renderer::ShaderType::Vertex;
        vertexPolicy.shaderModel = renderer::ShaderModel::HLSL_6_2;
        vertexPolicy.content = renderer::ShaderContent::Source;
        vertexPolicy.entryPoint = "main";

        vertShader = resource::ShaderCompiler::compileShader<renderer::VertexShader>(m_Device, "vertex", vertexPolicy, vertexStream, resource::ShaderCompileFlag::ShaderCompile_UseDXCompilerForSpirV);
        stream::StreamManager::destroyStream(vertexStream);
    }

    const renderer::FragmentShader* fragShader = nullptr;
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
        const stream::Stream* fragmentStream = stream::StreamManager::createMemoryStream(fragmentSource);

        renderer::Shader::LoadPolicy fragmentPolicy;
        fragmentPolicy.type = renderer::ShaderType::Fragment;
        fragmentPolicy.shaderModel = renderer::ShaderModel::HLSL_6_2;
        fragmentPolicy.content = renderer::ShaderContent::Source;
        fragmentPolicy.entryPoint = "main";

        fragShader = resource::ShaderCompiler::compileShader<renderer::FragmentShader>(m_Device, "fragment", fragmentPolicy, fragmentStream, resource::ShaderCompileFlag::ShaderCompile_UseDXCompilerForSpirV);
        stream::StreamManager::destroyStream(fragmentStream);
    }

    ASSERT(vertShader && fragShader, "nullptr");
    m_Program = new renderer::ShaderProgram(m_Device, vertShader, fragShader);

    m_RenderTarget = new renderer::RenderTargetState(m_Device, swapchain->getBackbufferSize());
    m_RenderTarget->setColorTexture(0, swapchain->getBackbuffer(),
        {
            renderer::RenderTargetLoadOp::LoadOp_Clear, renderer::RenderTargetStoreOp::StoreOp_Store, color::Color(0.0f)
        },
        {
            renderer::TransitionOp::TransitionOp_ColorAttachment, renderer::TransitionOp::TransitionOp_Present
        });

    std::vector<math::TVector3D<f32>> geometryData = 
    {
        {-1.0f,-1.0f, 0.0f },  { 1.0f, 0.0f, 0.0f },
        { 0.0f, 1.0f, 0.0f },  { 0.0f, 1.0f, 0.0f },
        { 1.0f,-1.0f, 0.0f },  { 0.0f, 0.0f, 1.0f },
    };
    m_Geometry = new renderer::VertexBuffer(m_Device, renderer::Buffer_GPUOnly, static_cast<u32>(geometryData.size()), static_cast<u32>(geometryData.size() * sizeof(math::TVector3D<f32>)), "geometry");

    renderer::VertexInputAttributeDesc vertexDesc(
        { 
            renderer::VertexInputAttributeDesc::InputBinding(0,  renderer::InputRate::InputRate_Vertex, sizeof(math::TVector3D<f32>) + sizeof(math::TVector3D<f32>)),
        }, 
        { 
            renderer::VertexInputAttributeDesc::InputAttribute(0, 0, renderer::Format_R32G32B32_SFloat, 0),
            renderer::VertexInputAttributeDesc::InputAttribute(0, 0, renderer::Format_R32G32B32_SFloat, sizeof(math::TVector3D<f32>)),
        });

    m_Pipeline = new renderer::GraphicsPipelineState(m_Device, vertexDesc, m_RenderTarget->getRenderPassDesc(), m_Program);
    m_Pipeline->setPrimitiveTopology(renderer::PrimitiveTopology::PrimitiveTopology_TriangleList);
    m_Pipeline->setFrontFace(renderer::FrontFace::FrontFace_Clockwise);
    m_Pipeline->setCullMode(renderer::CullMode::CullMode_None);
    m_Pipeline->setColorMask(0, renderer::ColorMask::ColorMask_All);
    m_Pipeline->setDepthWrite(false);


    m_CmdList->upload(m_Geometry, 0, static_cast<u32>(geometryData.size() * sizeof(math::TVector3D<f32>)), geometryData.data());
    m_Device->submit(m_CmdList, true);
}

void SimpleTriangle::update(f32 dt)
{
    m_Camera.update(dt);
}

void SimpleTriangle::render()
{
    TRACE_PROFILER_FRAME;

    //update uniforms
    struct UBO
    {
        math::Matrix4D projectionMatrix;
        math::Matrix4D modelMatrix;
        math::Matrix4D viewMatrix;
    };

    //render
    m_CmdList->setViewport(m_Rect);
    m_CmdList->setScissor(m_Rect);
    m_CmdList->beginRenderTarget(*m_RenderTarget);

    m_CmdList->setPipelineState(*m_Pipeline);

    UBO ubo1;
    ubo1.projectionMatrix = m_Camera.getCamera().getProjectionMatrix();
    ubo1.modelMatrix.setTranslation({ -1, 0, 0 });
    ubo1.viewMatrix = m_Camera.getViewMatrix();

    renderer::Descriptor desc1(renderer::Descriptor::Descriptor_ConstantBuffer);
    desc1._resource = renderer::Descriptor::ConstantBuffer{ &ubo1, 0, sizeof(UBO) };

    m_CmdList->bindDescriptorSet(m_Program, 0, { desc1 });
    m_CmdList->draw(renderer::GeometryBufferDesc(m_Geometry, sizeof(math::TVector3D<f32>) + sizeof(math::TVector3D<f32>)), 0, 3, 0, 1);

    UBO ubo2;
    ubo2.projectionMatrix = m_Camera.getCamera().getProjectionMatrix();
    ubo2.modelMatrix.setTranslation({ 1, 0, 0 });
    ubo2.viewMatrix = m_Camera.getViewMatrix();

    renderer::Descriptor desc2(renderer::Descriptor::Descriptor_ConstantBuffer);
    desc2._resource = renderer::Descriptor::ConstantBuffer{ &ubo2, 0, sizeof(UBO) };

    m_CmdList->bindDescriptorSet(m_Program, 0, { desc2 });
    m_CmdList->draw(renderer::GeometryBufferDesc(m_Geometry, sizeof(math::TVector3D<f32>) + sizeof(math::TVector3D<f32>)), 0, 3, 0, 1);

    m_CmdList->endRenderTarget();

    m_Device->submit(m_CmdList);
}

void SimpleTriangle::terminate()
{
    renderer::Texture2D* depthAttachment = m_RenderTarget->getDepthStencilTexture<renderer::Texture2D>();
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

    m_Device->destroyCommandList(m_CmdList);
}

bool SimpleTriangle::handleInputEvent(event::InputEventHandler* handler, const event::InputEvent* event)
{
    m_Camera.handleInputEventCallback(handler, event);

    return true;
}

bool SimpleTriangle::dispatchEvent(SimpleTriangle* render, event::InputEventHandler* handler, const event::InputEvent* event)
{
    return render->handleInputEvent(handler, event);
}
