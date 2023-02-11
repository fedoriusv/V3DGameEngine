#include "MyApplication.h"

#include "Event/InputEventReceiver.h"
#include "Utils/Timer.h"
#include "Utils/Profiler.h"

using namespace v3d;

MyApplication::MyApplication(int& argc, char** argv)
    : m_Window(platform::Window::createWindow({ 1280, 720 }, { 400, 200 }, false, new event::InputEventReceiver()))
    , m_InputEventHandler(new event::InputEventHandler())

    , m_Context(nullptr)
    , m_CommandList(nullptr)

    , m_Scene(nullptr)

{
    ASSERT(m_Window, "windows is nullptr");
}

void MyApplication::Initialize()
{
    m_Context = renderer::Context::createContext(m_Window, renderer::Context::RenderType::VulkanRender);
    ASSERT(m_Context, "context is nullptr");
    m_CommandList = new renderer::CommandList(m_Context, renderer::CommandList::CommandListType::ImmediateCommandList);

    m_Scene = new Scene(*m_CommandList, m_Window->getSize());

    m_InputEventHandler->connect(std::bind(&Scene::mouseHandle, m_Scene, m_InputEventHandler, std::placeholders::_1));
    m_InputEventHandler->connect(std::bind(&Scene::keyboardHandle, m_Scene, m_InputEventHandler, std::placeholders::_1));
    m_InputEventHandler->connect(std::bind(&Scene::touchHandle, m_Scene, m_InputEventHandler, std::placeholders::_1));

    m_Window->getInputEventReceiver()->attach(event::InputEvent::InputEventType::MouseInputEvent, m_InputEventHandler);
    m_Window->getInputEventReceiver()->attach(event::InputEvent::InputEventType::TouchInputEvent, m_InputEventHandler);
    m_Window->getInputEventReceiver()->attach(event::InputEvent::InputEventType::KeyboardInputEvent, m_InputEventHandler);
}

bool MyApplication::Running()
{
    static u64 s_prevTime = 0;
    u64 currentTime = utils::Timer::getCurrentTime();

    const f32 diffTime = static_cast<f32>(std::max<s64>(static_cast<s64>(currentTime) - static_cast<s64>(s_prevTime), 0));
    const f32 deltaTime = diffTime / 1'000.f;
    m_Scene->Run(deltaTime);

    s_prevTime = currentTime;


    static f32 timePassed = 0;
    static u32 FPSCounter = 0;

    timePassed += diffTime;// utils::Timer::getCurrentTime() - currentTime;
    ++FPSCounter;
    if (timePassed >= 1'000.f) //sec
    {
        m_Window->setTextCaption("FPS: " + std::to_string(FPSCounter) + " (" + std::to_string(timePassed/FPSCounter) + "ms)");

        FPSCounter = 0;
        timePassed = 0.f;
    }

    return true;
}

void MyApplication::Exit()
{
    m_Window->getInputEventReceiver()->dettach(event::InputEvent::InputEventType::MouseInputEvent);
    m_Window->getInputEventReceiver()->dettach(event::InputEvent::InputEventType::TouchInputEvent);
    m_Window->getInputEventReceiver()->dettach(event::InputEvent::InputEventType::KeyboardInputEvent);

    if (m_Scene)
    {
        delete m_Scene;
        m_Scene = nullptr;
    }

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

int MyApplication::Execute()
{
    MyApplication::Initialize();
    while (true)
    {
        if (!platform::Window::updateWindow(m_Window))
        {
            break;
        }

        if (m_Window->isValid())
        {
            m_Window->getInputEventReceiver()->sendDeferredEvents();
            MyApplication::Running();
        }

        std::this_thread::yield();
    }

    Exit();
    delete this;

    return 0;
}

MyApplication::~MyApplication()
{
    delete m_InputEventHandler;
    m_InputEventHandler = nullptr;

    platform::Window::detroyWindow(m_Window);
}
