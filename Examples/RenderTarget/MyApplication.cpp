#include "MyApplication.h"
#include "Utils/Logger.h"

#include "Event/InputEventReceiver.h"

#include "Renderer/Shader.h"
#include "Resource/Image.h"
#include "Resource/ImageFileLoader.h"
#include "Resource/ModelFileLoader.h"
#include "Resource/ResourceLoaderManager.h"
#include "Resource/ShaderSourceFileLoader.h"

#include "Scene/Model.h"


using namespace v3d;
using namespace v3d::platform;
using namespace v3d::utils;
using namespace v3d::event;

MyApplication::MyApplication(int& argc, char** argv)
    : m_Window(nullptr)
    , m_InputEventHandler(nullptr)

    , m_Context(nullptr)
    , m_CommandList(nullptr)
{
    m_Window = Window::createWindow({ 1280, 720 }, { 400, 200 }, false, new v3d::event::InputEventReceiver());
    ASSERT(m_Window, "windows is nullptr");

    m_InputEventHandler = new InputEventHandler();
    m_InputEventHandler->connect([this](const MouseInputEvent* event)
    {
            m_Camera->handlerMouseCallback(m_InputEventHandler, event);
    });

    m_InputEventHandler->connect([this](const TouchInputEvent* event)
    {
            m_Camera->handlerTouchCallback(m_InputEventHandler, event);
    });

    std::srand(u32(std::time(0)));
    m_Window->getInputEventReceiver()->attach(InputEvent::InputEventType::MouseInputEvent, m_InputEventHandler);
    m_Window->getInputEventReceiver()->attach(InputEvent::InputEventType::TouchInputEvent, m_InputEventHandler);
}

int MyApplication::Execute()
{
    MyApplication::Initialize();
    while (true)
    {
        if (!Window::updateWindow(m_Window))
        {
            break;
        }

        if (m_Window->isValid())
        {
            m_Window->getInputEventReceiver()->sendDeferredEvents();
            MyApplication::Running();
        }
    }

    Exit();
    delete this;

    return 0;
}

void MyApplication::Initialize()
{
    m_Context = renderer::Context::createContext(m_Window, renderer::Context::RenderType::VulkanRender);
    ASSERT(m_Context, "context is nullptr");
    m_CommandList = new renderer::CommandList(m_Context, renderer::CommandList::CommandListType::DelayedCommandList);

    m_Camera = new v3d::scene::CameraArcballHelper(new scene::Camera(core::Vector3D(0.0f, 0.0f, 0.0f), core::Vector3D(0.0f, 1.0f, 0.0f)), 5.0f, 4.0f, 20.0f);
    m_Camera->setPerspective(45.0f, m_Window->getSize(), 0.1f, 256.f);

    resource::ResourceLoaderManager::getInstance()->addPath("examples/rendertarget/");

    //Pass 0
    {
        m_Sampler = m_CommandList->createObject<renderer::SamplerState>(renderer::SamplerFilter::SamplerFilter_Trilinear, renderer::SamplerAnisotropic::SamplerAnisotropic_4x);
        m_Sampler->setWrap(renderer::SamplerWrap::TextureWrap_MirroredRepeat);

        resource::Image* image = resource::ResourceLoaderManager::getInstance()->load<resource::Image, resource::ImageFileLoader>("data/basetex.jpg", resource::ImageLoaderFlag_GenerateMipmaps);
        ASSERT(image, "not found");
        m_Texture = m_CommandList->createObject<renderer::Texture2D>(renderer::TextureUsage_Sampled | renderer::TextureUsage_Write, image->getFormat(), core::Dimension2D(image->getDimension().width, image->getDimension().height), image->getMipMapsCount(), image->getRawData(), "UnlitTexture");

        std::vector<const renderer::Shader*> shaders = resource::ResourceLoaderManager::getInstance()->loadHLSLShader<renderer::Shader, resource::ShaderSourceFileLoader>(m_CommandList->getContext(), "data/texture.hlsl",
            {
                {"main_VS", renderer::ShaderType_Vertex },
                {"main_FS", renderer::ShaderType_Fragment }
            });

        v3d::scene::Model* cube = resource::ResourceLoaderManager::getInstance()->load<v3d::scene::Model, resource::ModelFileLoader>("data/cube.dae");
        m_Geometry = v3d::scene::ModelHelper::createModelHelper(*m_CommandList, { cube });

        m_RenderTarget = m_CommandList->createObject<renderer::RenderTargetState>(m_Window->getSize(), 0, "RenderTarget");

        m_ColorAttachment = m_CommandList->createObject<renderer::Texture2D>(renderer::TextureUsage::TextureUsage_Attachment | renderer::TextureUsage::TextureUsage_Sampled, 
            renderer::Format::Format_R8G8B8A8_UNorm, m_Window->getSize(), renderer::TextureSamples::TextureSamples_x1, "ColorAttachment");
        m_RenderTarget->setColorTexture(0, m_ColorAttachment,
            { 
                renderer::RenderTargetLoadOp::LoadOp_Clear, renderer::RenderTargetStoreOp::StoreOp_Store, core::Vector4D(0.0f) 
            }, 
            { 
                renderer::TransitionOp::TransitionOp_Undefined, renderer::TransitionOp::TransitionOp_ShaderRead
            });

        m_DepthAttachment = m_CommandList->createObject<renderer::Texture2D>(renderer::TextureUsage::TextureUsage_Attachment, renderer::Format::Format_D32_SFloat_S8_UInt, m_Window->getSize(), renderer::TextureSamples::TextureSamples_x1, "DepthAttachment");
        m_RenderTarget->setDepthStencilTexture(m_DepthAttachment, 
            { 
                renderer::RenderTargetLoadOp::LoadOp_Clear, renderer::RenderTargetStoreOp::StoreOp_DontCare, 1.0f 
            }, 
            { 
                renderer::RenderTargetLoadOp::LoadOp_DontCare, renderer::RenderTargetStoreOp::StoreOp_DontCare, 0U 
            },
            { 
                renderer::TransitionOp::TransitionOp_Undefined, renderer::TransitionOp::TransitionOp_DepthStencilAttachmet
            });

        m_Program = m_CommandList->createObject<renderer::ShaderProgram>(shaders);
        m_Pipeline = m_CommandList->createObject<renderer::GraphicsPipelineState>(m_Geometry->getVertexInputAttribDescription(0, 0), m_Program, m_RenderTarget);
        m_Pipeline->setPrimitiveTopology(renderer::PrimitiveTopology::PrimitiveTopology_TriangleList);
        m_Pipeline->setFrontFace(renderer::FrontFace::FrontFace_Clockwise);
        m_Pipeline->setCullMode(renderer::CullMode::CullMode_Back);
        m_Pipeline->setColorMask(renderer::ColorMask::ColorMask_All);
        m_Pipeline->setDepthCompareOp(renderer::CompareOperation::CompareOp_Less);
        m_Pipeline->setDepthWrite(true);
        m_Pipeline->setDepthTest(true);
    }

    //pass 1
    {
        std::vector<const renderer::Shader*> shaders = resource::ResourceLoaderManager::getInstance()->loadHLSLShader<renderer::Shader, resource::ShaderSourceFileLoader>(m_CommandList->getContext(), "data/offscreen.hlsl",
            {
                {"main_VS", renderer::ShaderType_Vertex },
                {"main_FS", renderer::ShaderType_Fragment }
            });

        m_OffscreenRenderTarget = m_CommandList->createObject<renderer::RenderTargetState>(m_Window->getSize(), 0, "OffscreenTarget");
        m_OffscreenRenderTarget->setColorTexture(0, m_CommandList->getBackbuffer(),
            {
                renderer::RenderTargetLoadOp::LoadOp_DontCare, renderer::RenderTargetStoreOp::StoreOp_Store, core::Vector4D(0.0f)
            },
            {
                renderer::TransitionOp::TransitionOp_Undefined, renderer::TransitionOp::TransitionOp_Present
            });

        m_OffscreenProgram = m_CommandList->createObject<renderer::ShaderProgram>(shaders);
        m_OffscreenPipeline = m_CommandList->createObject<renderer::GraphicsPipelineState>(renderer::VertexInputAttribDescription(), m_OffscreenProgram, m_OffscreenRenderTarget);
        m_OffscreenPipeline->setPrimitiveTopology(renderer::PrimitiveTopology::PrimitiveTopology_TriangleList);
        m_OffscreenPipeline->setFrontFace(renderer::FrontFace::FrontFace_Clockwise);
        m_OffscreenPipeline->setCullMode(renderer::CullMode::CullMode_Back);
        m_OffscreenPipeline->setColorMask(renderer::ColorMask::ColorMask_All);
        m_OffscreenPipeline->setDepthCompareOp(renderer::CompareOperation::CompareOp_Always);
        m_OffscreenPipeline->setDepthWrite(false);
        m_OffscreenPipeline->setDepthTest(false);
    }

    m_CommandList->submitCommands(true);
    m_CommandList->flushCommands();
}

bool MyApplication::Running()
{
    m_Camera->update(0);

    //Frame
    m_CommandList->beginFrame();

    //pass 0
    {
        m_CommandList->setViewport(core::Rect32(0, 0, m_RenderTarget->getDimension().width, m_RenderTarget->getDimension().height));
        m_CommandList->setScissor(core::Rect32(0, 0, m_RenderTarget->getDimension().width, m_RenderTarget->getDimension().height));
        m_CommandList->setRenderTarget(m_RenderTarget);
        m_CommandList->setPipelineState(m_Pipeline);

        struct UBO
        {
            core::Matrix4D projectionMatrix;
            core::Matrix4D viewMatrix;
            core::Matrix4D modelMatrix;
        } ubo;

        ubo.projectionMatrix = m_Camera->getProjectionMatrix();
        ubo.viewMatrix = m_Camera->getViewMatrix();
        ubo.modelMatrix.makeIdentity();
        ubo.modelMatrix.setScale({ 100.0f, 100.0f, 100.0f });

        m_Program->bindUniformsBuffer<renderer::ShaderType::ShaderType_Vertex>({ "vs_buffer" }, 0, (u32)sizeof(UBO), &ubo);

        m_Program->bindSampler<renderer::ShaderType::ShaderType_Fragment>({ "colorSampler" }, m_Sampler);
        m_Program->bindTexture<renderer::ShaderType::ShaderType_Fragment, renderer::Texture2D>({ "colorTexture" }, m_Texture);

        m_Geometry->draw();
    }

    //pass 1
    {
        m_CommandList->setViewport(core::Rect32(0, 0, m_OffscreenRenderTarget->getDimension().width, m_OffscreenRenderTarget->getDimension().height));
        m_CommandList->setScissor(core::Rect32(0, 0, m_OffscreenRenderTarget->getDimension().width, m_OffscreenRenderTarget->getDimension().height));
        m_CommandList->setRenderTarget(m_OffscreenRenderTarget);
        m_CommandList->setPipelineState(m_OffscreenPipeline);

        m_OffscreenProgram->bindSampler<renderer::ShaderType::ShaderType_Fragment>({ "colorSampler" }, m_Sampler);
        m_OffscreenProgram->bindTexture<renderer::ShaderType::ShaderType_Fragment, renderer::Texture2D>({ "colorTexture" }, m_ColorAttachment);

        m_CommandList->draw(renderer::StreamBufferDescription(nullptr, 0), 0, 3, 1);
    }

    m_CommandList->endFrame();
    m_CommandList->presentFrame();
    
    m_CommandList->flushCommands();

    return true;
}

void MyApplication::Exit()
{
    //pass 1
    delete m_Pipeline;
    delete m_Program;

    delete m_DepthAttachment;
    delete m_ColorAttachment;
    delete m_RenderTarget;

    delete m_Geometry;
    delete m_Texture;
    delete m_Sampler;

    //pass 2
    delete m_OffscreenPipeline;
    delete m_OffscreenProgram;

    delete m_OffscreenRenderTarget;

    resource::ResourceLoaderManager::getInstance()->clear();
    resource::ResourceLoaderManager::getInstance()->freeInstance();

    m_Window->getInputEventReceiver()->dettach(InputEvent::InputEventType::MouseInputEvent);
    m_Window->getInputEventReceiver()->dettach(InputEvent::InputEventType::TouchInputEvent);

    delete m_Camera;
    if (m_CommandList)
    {
        delete m_CommandList;
        m_CommandList = nullptr;
    }

    if (m_Context)
    {
        renderer::Context::destroyContext(m_Context);
        m_Context = nullptr;
    }
}

MyApplication::~MyApplication()
{
    delete m_InputEventHandler;
    m_InputEventHandler = nullptr;

    Window::detroyWindow(m_Window);
}
