#include "Editor.h"

#include "Renderer/Shader.h"
#include "Resource/ResourceManager.h"
#include "Resource/ShaderCompiler.h"
#include "Stream/StreamManager.h"
#include "Utils/Logger.h"

using namespace v3d;
using namespace v3d::renderer;
using namespace v3d::platform;
using namespace v3d::utils;
using namespace v3d::event;

const f32 k_nearValue = 0.1f;
const f32 k_farValue = 30.0f;

EditorScene::EditorScene()
    : m_Device(nullptr)

    , m_Program(nullptr)
    , m_Pipeline(nullptr)
    , m_Geometry(nullptr)

    , m_Camera(new scene::CameraArcballHandler(new scene::Camera(math::Vector3D(0.0f, 0.0f, 0.0f), math::Vector3D(0.0f, 1.0f, 0.0f)), 3.0f, k_nearValue + 1.0f, k_farValue - 10.0f))
    , m_CurrentWindow(nullptr)
    , m_VewiportTarget(nullptr)
{
    InputEventHandler::bind([this](const MouseInputEvent* event)
        {
            this->handleInputEvent(this, event);
        }
    );

    InputEventHandler::bind([this](const KeyboardInputEvent* event)
        {
            this->handleInputEvent(this, event);
        }
    );
}

EditorScene::~EditorScene()
{
}

void EditorScene::init(renderer::Device* device, const v3d::math::Dimension2D& viewportSize)
{
    m_Device = device;
    m_CurrentViewportRect = math::Rect32(0, 0, viewportSize.m_width, viewportSize.m_height);
    renderer::CmdListRender* CmdList = m_Device->createCommandList<renderer::CmdListRender>(Device::GraphicMask);

    m_Camera->setPerspective(45.0f, viewportSize, k_nearValue, k_farValue);

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
            float4 Pos   : SV_POSITION;\n\
            float4 Color : COLOR;\n\
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
            Out.Color = float4(Input.Color, 1.0);\n\
            return Out;\n\
        }");
        const stream::Stream* vertexStream = stream::StreamManager::createMemoryStream(vertexSource);

        resource::ShaderDecoder::ShaderPolicy vertexPolicy;
        vertexPolicy._type = renderer::ShaderType::Vertex;
        vertexPolicy._shaderModel = renderer::ShaderModel::HLSL_6_2;
        vertexPolicy._content = renderer::ShaderContent::Source;
        vertexPolicy._entryPoint = "main";

        vertShader = resource::ShaderCompiler::compileShader<renderer::VertexShader>(m_Device, "vertex", vertexPolicy, vertexStream, resource::ShaderCompileFlag::ShaderCompile_UseDXCompilerForSpirV);
        stream::StreamManager::destroyStream(vertexStream);
    }

    const renderer::FragmentShader* fragShader = nullptr;
    {
        const std::string fragmentSource("\
        struct PS_INPUT\n\
        {\n\
            float4 Col : COLOR;\n\
        };\n\
        \n\
        float4 main(PS_INPUT Input) : SV_TARGET0\n\
        {\n\
            return Input.Col;\n\
        }");
        const stream::Stream* fragmentStream = stream::StreamManager::createMemoryStream(fragmentSource);

        resource::ShaderDecoder::ShaderPolicy fragmentPolicy;
        fragmentPolicy._type = renderer::ShaderType::Fragment;
        fragmentPolicy._shaderModel = renderer::ShaderModel::HLSL_6_2;
        fragmentPolicy._content = renderer::ShaderContent::Source;
        fragmentPolicy._entryPoint = "main";

        fragShader = resource::ShaderCompiler::compileShader<renderer::FragmentShader>(m_Device, "fragment", fragmentPolicy, fragmentStream, resource::ShaderCompileFlag::ShaderCompile_UseDXCompilerForSpirV);
        stream::StreamManager::destroyStream(fragmentStream);
    }

    ASSERT(vertShader && fragShader, "nullptr");
    m_Program = new renderer::ShaderProgram(m_Device, vertShader, fragShader);

    recreateViewport(viewportSize);

    std::vector<math::Vector3D> geometryData = 
    {
        {-1.0f,-1.0f, 0.0f },  { 1.0f, 0.0f, 0.0f },
        { 0.0f, 1.0f, 0.0f },  { 0.0f, 1.0f, 0.0f },
        { 1.0f,-1.0f, 0.0f },  { 0.0f, 0.0f, 1.0f },
    };
    m_Geometry = new renderer::VertexBuffer(m_Device, renderer::Buffer_GPUOnly, static_cast<u32>(geometryData.size()), static_cast<u32>(geometryData.size() * sizeof(math::Vector3D)), "geometry");

    renderer::VertexInputAttributeDesc vertexDesc(
        { 
            renderer::VertexInputAttributeDesc::InputBinding(0,  renderer::InputRate::InputRate_Vertex, sizeof(math::Vector3D) + sizeof(math::Vector3D)),
        }, 
        { 
            renderer::VertexInputAttributeDesc::InputAttribute(0, 0, renderer::Format_R32G32B32_SFloat, 0),
            renderer::VertexInputAttributeDesc::InputAttribute(0, 0, renderer::Format_R32G32B32_SFloat, sizeof(math::Vector3D)),
        });

    m_Pipeline = new renderer::GraphicsPipelineState(m_Device, vertexDesc, m_VewiportTarget->getRenderPassDesc(), m_Program);
    m_Pipeline->setPrimitiveTopology(renderer::PrimitiveTopology::PrimitiveTopology_TriangleList);
    m_Pipeline->setFrontFace(renderer::FrontFace::FrontFace_Clockwise);
    m_Pipeline->setCullMode(renderer::CullMode::CullMode_None);
    m_Pipeline->setColorMask(renderer::ColorMask::ColorMask_All);
    m_Pipeline->setDepthCompareOp(renderer::CompareOperation::CompareOp_GreaterOrEqual);
    m_Pipeline->setDepthWrite(true);
    m_Pipeline->setDepthTest(true);

    CmdList->uploadData(m_Geometry, 0, static_cast<u32>(geometryData.size() * sizeof(math::Vector3D)), geometryData.data());
    m_Device->submit(CmdList, true);

    m_Device->destroyCommandList(CmdList);
}

void EditorScene::recreateViewport(const v3d::math::Dimension2D& viewportSize)
{
    if (m_VewiportTarget)
    {
        renderer::Texture2D* colorAttachment = m_VewiportTarget->getColorTexture<renderer::Texture2D>(0);
        delete colorAttachment;

        renderer::Texture2D* depthAttachment = m_VewiportTarget->getDepthStencilTexture< renderer::Texture2D>();
        delete depthAttachment;

        delete m_VewiportTarget;
        m_VewiportTarget = nullptr;
    }


    m_VewiportTarget = new renderer::RenderTargetState(m_Device, viewportSize);

    renderer::Texture2D* colorAttachment = new renderer::Texture2D(m_Device, renderer::TextureUsage::TextureUsage_Attachment | renderer::TextureUsage::TextureUsage_Sampled | renderer::TextureUsage::TextureUsage_Write,
        renderer::Format::Format_R8G8B8A8_UNorm, viewportSize, renderer::TextureSamples::TextureSamples_x1, "ViewportColorAttachment");
    renderer::Texture2D* depthAttachment = new renderer::Texture2D(m_Device, renderer::TextureUsage::TextureUsage_Attachment | renderer::TextureUsage::TextureUsage_Write,
        renderer::Format::Format_D32_SFloat_S8_UInt, viewportSize, renderer::TextureSamples::TextureSamples_x1, "ViewportDepthAttachment");

    m_VewiportTarget->setColorTexture(0, colorAttachment,
        {
            renderer::RenderTargetLoadOp::LoadOp_Clear, renderer::RenderTargetStoreOp::StoreOp_Store, math::Vector4D(0.0f)
        },
        {
            renderer::TransitionOp::TransitionOp_Undefined, renderer::TransitionOp::TransitionOp_ColorAttachment
        }
    );

    m_VewiportTarget->setDepthStencilTexture(depthAttachment,
        {
            renderer::RenderTargetLoadOp::LoadOp_Clear, renderer::RenderTargetStoreOp::StoreOp_Store, 0.0f
        },
        {
            renderer::RenderTargetLoadOp::LoadOp_DontCare, renderer::RenderTargetStoreOp::StoreOp_DontCare, 0
        },
        {
            renderer::TransitionOp::TransitionOp_Undefined, renderer::TransitionOp::TransitionOp_DepthStencilAttachment
        }
    );
}

void EditorScene::cleanup()
{
    //TODO
}

void EditorScene::update(f32 dt)
{
    m_Camera->update(dt);
}

void EditorScene::render(v3d::renderer::CmdListRender* cmdList)
{
    cmdList->beginRenderTarget(*m_VewiportTarget);

    //update uniforms
    struct UBO
    {
        math::Matrix4D projectionMatrix;
        math::Matrix4D modelMatrix;
        math::Matrix4D viewMatrix;
    };

    //render
    cmdList->setViewport(math::Rect32(0, 0, m_VewiportTarget->getRenderArea().m_width, m_VewiportTarget->getRenderArea().m_height));
    cmdList->setScissor(math::Rect32(0, 0, m_VewiportTarget->getRenderArea().m_width, m_VewiportTarget->getRenderArea().m_height));
    cmdList->setPipelineState(*m_Pipeline);

    UBO ubo1;
    ubo1.projectionMatrix = m_Camera->getCamera().getProjectionMatrix();
    ubo1.modelMatrix.setTranslation(math::Vector3D(-1, 0, 0));
    ubo1.viewMatrix = m_Camera->getCamera().getViewMatrix();

    renderer::Descriptor desc1(renderer::Descriptor::Descriptor_ConstantBuffer);
    desc1._resource = renderer::Descriptor::ConstantBuffer{ &ubo1, 0, sizeof(UBO) };

    cmdList->bindDescriptorSet(0, { desc1 });
    cmdList->draw(renderer::GeometryBufferDesc(m_Geometry, 0, sizeof(math::Vector3D) + sizeof(math::Vector3D)), 0, 3, 0, 1);

    UBO ubo2;
    ubo2.projectionMatrix = m_Camera->getCamera().getProjectionMatrix();
    ubo2.modelMatrix.setTranslation(math::Vector3D(1, 0, 0));
    ubo2.viewMatrix = m_Camera->getCamera().getViewMatrix();

    renderer::Descriptor desc2(renderer::Descriptor::Descriptor_ConstantBuffer);
    desc2._resource = renderer::Descriptor::ConstantBuffer{ &ubo2, 0, sizeof(UBO) };

    cmdList->bindDescriptorSet(0, { desc2 });
    cmdList->draw(renderer::GeometryBufferDesc(m_Geometry, 0, sizeof(math::Vector3D) + sizeof(math::Vector3D)), 0, 3, 0, 1);

    cmdList->endRenderTarget();
}

void EditorScene::onChanged(const platform::Window* window, const v3d::math::Rect32& viewport)
{
    if (viewport != m_CurrentViewportRect)
    {
        if (m_CurrentViewportRect.getWidth() != viewport.getWidth() || m_CurrentViewportRect.getHeight() != viewport.getHeight())
        {
            recreateViewport({ (u32)viewport.getWidth(), (u32)viewport.getHeight() });
        }

        m_CurrentViewportRect = viewport;
    }
    m_CurrentWindow = window;
}

const renderer::Texture2D* EditorScene::getOutputTexture() const
{
    return m_VewiportTarget->getColorTexture<renderer::Texture2D>(0);
}

const v3d::math::Rect32& EditorScene::getViewportArea() const
{
    return m_CurrentViewportRect;
}

bool EditorScene::handleInputEvent(event::InputEventHandler* handler, const event::InputEvent* event)
{
    if (m_CurrentWindow && m_CurrentWindow->ID() == event->_windowID && m_CurrentWindow->isFocused())
    {
        if (event->_eventType == event::InputEvent::InputEventType::MouseInputEvent)
        {
            const event::MouseInputEvent* mouseEvent = static_cast<const event::MouseInputEvent*>(event);
            if (m_CurrentViewportRect.isPointInside({ mouseEvent->_absoluteCoordinates.m_x, mouseEvent->_absoluteCoordinates.m_y }))
            {
                m_Camera->handleMouseCallback(handler, mouseEvent);
            }

            return true;
        }
        else if (event->_eventType == event::InputEvent::InputEventType::TouchInputEvent)
        {
            const event::TouchInputEvent* touchEvent = static_cast<const event::TouchInputEvent*>(event);
            m_Camera->handleTouchCallback(handler, touchEvent);

            return true;
        }
    }

    return false;
}


