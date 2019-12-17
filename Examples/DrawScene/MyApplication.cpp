#include "MyApplication.h"
#include "Utils/Logger.h"

#include "Renderer/Context.h"
#include "Event/InputEventReceiver.h"
#include "Platform/Window.h"
#include "Platform/Platform.h"
#include "Resource/ResourceLoaderManager.h"

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

    , m_Scene(nullptr)
    , m_Camera(nullptr)

    , m_captureMouseCursor(false)

    , m_Timer(new utils::Timer())
    , m_frameTime(0.f)
{
    core::Dimension2D widowsSize = { 1920, 1058 };
    m_Window = Window::createWindow(widowsSize, {400, 100}, false, new v3d::event::InputEventReceiver());
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

        if (m_captureMouseCursor && m_Window->isActive())
        {
            core::Rect32 area(m_Window->getPosition().x, m_Window->getPosition().y, m_Window->getPosition().x + m_Window->getSize().width, m_Window->getPosition().y + m_Window->getSize().height);
            core::Point2D centerPosition = area.getCenter();

            core::Point2D cursorPosition = platform::Platform::getCursorPosition();
            if (cursorPosition != centerPosition)
            {
                platform::Platform::setCursorPostion(centerPosition);
            }
        }
    }

    Exit();
    delete this;

    return 0;
}

void MyApplication::Initialize()
{
    resource::ResourceLoaderManager::getInstance()->addPath("examples/drawscene/data/");

    m_Context = renderer::Context::createContext(m_Window, renderer::Context::RenderType::VulkanRender);
    ASSERT(m_Context, "context is nullptr");
    m_CommandList = new renderer::CommandList(m_Context, renderer::CommandList::CommandListType::ImmediateCommandList);

    m_Camera = new CameraFPSHelper(new Camera(core::Vector3D(0.0f, 0.0f, 0.0f), core::Vector3D(0.0f, -1.0f, 0.0f)), core::Vector3D(0.0f, 0.0f, -10.0f));
    m_Camera->setPerspective(45.0f, m_Window->getSize(), 1.0f, 512.f);

    m_InputEventHandler->connect(std::bind(&CameraFPSHelper::rotateHandlerCallback, m_Camera, m_InputEventHandler, std::placeholders::_1, true/*m_captureMouseCursor*/));
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

    Update();
    m_Scene->onRender(commandList);

    commandList.endFrame();
    commandList.presentFrame();
    
    commandList.flushCommands();

    m_Timer->stop();

    static u32 frameCounter;
    static f32 fpsTime;
    u64 diff = m_Timer->getTime<utils::Timer::Duration::Duration_MilliSeconds>();
    if (diff <= 0.0f)
    {
        fpsTime += 1.0f;
    }
    else
    {
        m_frameTime = (static_cast<f32>(diff) / 1000.0f) * m_timeFactor;
        fpsTime += diff;
    }
    ++frameCounter;

    if (fpsTime >= 1000.0f)
    {
        m_Window->setTextCaption("Time: " + std::to_string(fpsTime / frameCounter) + " ms (" + std::to_string(frameCounter) + " FPS)");

        frameCounter = 0;
        fpsTime -= 1000.0f;
    }
    m_Timer->reset();

    return true;
}

void MyApplication::Update()
{
    m_Camera->update(m_frameTime);
    m_Scene->setCameraPosition(m_Camera->getPosition());

    m_Scene->onUpdate(m_frameTime);
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

