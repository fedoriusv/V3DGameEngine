#include "MyApplication.h"
#include "Utils/Logger.h"

#include "Renderer/Context.h"
#include "Event/InputEventReceiver.h"
#include "Platform/Window.h"

using namespace v3d;
using namespace v3d::platform;
using namespace v3d::utils;
using namespace v3d::event;
using namespace v3d::renderer;
using namespace v3d::scene;

MyApplication::MyApplication(int& argc, char** argv)
    : m_Window(nullptr)
    , m_InputEventHandler(nullptr)

    , m_Context(nullptr)
    , m_CommandList(nullptr)

    , m_Camera(nullptr)

    , m_Timer(new utils::Timer())
    , m_frameTime(0.f)
{
    core::Dimension2D widowsSize = { 1280, 720 };
    m_Window = Window::createWindow(widowsSize, {800, 500}, false, new v3d::event::InputEventReceiver());
    ASSERT(m_Window, "windows is nullptr");

    m_InputEventHandler = new InputEventHandler();
    m_Window->getInputEventReceiver()->attach(InputEvent::InputEventType::MouseInputEvent, m_InputEventHandler);
    m_Window->getInputEventReceiver()->attach(InputEvent::InputEventType::KeyboardInputEvent, m_InputEventHandler);
}

int MyApplication::Execute()
{
    MyApplication::Initialize();
    bool running = true;
    while (running)
    {
        running = Window::updateWindow(m_Window);

        m_Window->getInputEventReceiver()->sendDeferredEvents();
        if (running)
        {
            MyApplication::Running(*m_CommandList);
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

    m_Camera = new CameraFPSHelper(new Camera(core::Vector3D(0.0f, 0.0f, 0.0f), core::Vector3D(0.0f, -1.0f, 0.0f)), core::Vector3D(0.0f, 0.0f, -10.0f));
    m_Camera->setPerspective(45.0f, m_Window->getSize(), 0.01f, 250.f);

    m_InputEventHandler->connect(std::bind(&CameraFPSHelper::rotateHandlerCallback, m_Camera, m_InputEventHandler, std::placeholders::_1));
    m_InputEventHandler->connect(std::bind(&CameraFPSHelper::moveHandlerCallback, m_Camera, m_InputEventHandler, std::placeholders::_1));

    m_Scene = new scene::Scene(m_Window->getSize());
    m_Scene->setCamera(&m_Camera->getCamera());
    m_Scene->onLoad(*m_CommandList);
}

bool MyApplication::Running(renderer::CommandList& commandList)
{
    m_Timer->start();

    //Frame
    commandList.beginFrame();

    m_Camera->update(m_frameTime);
    m_Scene->onUpdate();
    m_Scene->onRender(commandList);

    commandList.endFrame();
    commandList.presentFrame();
    
    commandList.flushCommands();

    m_Timer->stop();

    u64 diff = m_Timer->getTime<utils::Timer::Duration::Duration_MilliSeconds>();
    m_frameTime = static_cast<f32>(diff) / 1000.0f;
    m_Timer->reset();

    static f32 fpsTimer;
    fpsTimer += static_cast<f32>(diff);
    if (fpsTimer > 1000.0f)
    {
        m_Window->setTextCaption("Time: " + std::to_string(m_frameTime) + " ms (" + std::to_string(fpsTimer) + " FPS)");
        fpsTimer = 0;
    }
    return true;
}

void MyApplication::Exit()
{
    delete m_Camera;
    delete m_Scene;

    delete m_Timer;

    m_Window->getInputEventReceiver()->dettach(InputEvent::InputEventType::MouseInputEvent);

    delete m_CommandList;
    Context::destroyContext(m_Context);
}

MyApplication::~MyApplication()
{
    delete m_InputEventHandler;
    m_InputEventHandler = nullptr;

    Window::detroyWindow(m_Window);
}

