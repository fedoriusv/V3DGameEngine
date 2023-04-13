#include "MyApplication.h"
#include "Event/InputEventReceiver.h"
#include "Renderer/Core/Context.h"

namespace app
{

using namespace v3d;

MyApplication::MyApplication(int& argc, char** argv) noexcept
    : m_Window(nullptr)
    , m_Context(nullptr)
    , m_CommandList(nullptr)
    , m_InputEventHandler(nullptr)

    , m_Scene(nullptr)
{
}

int MyApplication::Execute()
{
    bool running = true;
    if (!Initialize())
    {
        running = false;
    }

    while (running)
    {
        running = platform::Window::updateWindow(m_Window);

        m_Window->getInputEventReceiver()->sendDeferredEvents();
        if (running)
        {
            Running();
        }
    }

    Terminate();
    delete this;

    return 0;
}

bool MyApplication::Initialize()
{
    m_Window = platform::Window::createWindow({ 1920, 1080 }, { 400, 200 }, false, new event::InputEventReceiver());
    if (!m_Window)
    {
        return false;
    }
    m_InputEventHandler = new event::InputEventHandler();

    m_Context = renderer::Context::createContext(m_Window, renderer::Context::RenderType::VulkanRender);
    if (!m_Context)
    {
        return false;
    }
    m_CommandList = new renderer::CommandList(m_Context, renderer::CommandList::CommandListType::ImmediateCommandList);

    m_Scene = new Scene(m_Context, m_CommandList, m_Window->getSize());
    m_InputEventHandler->connect([this](const event::MouseInputEvent* event)
        {
            Scene::MouseCallback(m_Scene, m_InputEventHandler, event);
        });

    m_InputEventHandler->connect([this](const event::TouchInputEvent* event)
        {
            Scene::TouchCallback(m_Scene, m_InputEventHandler, event);
        });

    m_Window->getInputEventReceiver()->attach(event::InputEvent::InputEventType::MouseInputEvent, m_InputEventHandler);
    m_Window->getInputEventReceiver()->attach(event::InputEvent::InputEventType::TouchInputEvent, m_InputEventHandler);

    return true;
}

bool MyApplication::Running()
{
    return m_Scene->Run();
}

void MyApplication::Terminate()
{
    if (m_Scene)
    {
        m_Window->getInputEventReceiver()->dettach(event::InputEvent::InputEventType::MouseInputEvent);
        m_Window->getInputEventReceiver()->dettach(event::InputEvent::InputEventType::TouchInputEvent);

        m_Scene->SendExitSignal();
        m_Scene->Run();

        delete m_Scene;
    }

    if (m_InputEventHandler)
    {
        delete m_InputEventHandler;
        m_InputEventHandler = nullptr;
    }

    delete m_CommandList;
    if (m_Context)
    {
        renderer::Context::destroyContext(m_Context);
    }

    if (m_Window)
    {
        platform::Window::detroyWindow(m_Window);
    }
}

} //namespace app