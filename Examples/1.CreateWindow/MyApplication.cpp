#include "MyApplication.h"
#include "Utils/Logger.h"

#include "Renderer/Context.h"
#include "Renderer/CommandList.h"

using namespace v3d;
using namespace v3d::platform;
using namespace v3d::utils;
using namespace v3d::event;

MyApplication::MyApplication(int& argc, char** argv)
    : m_Window(nullptr)
{
    m_Window = Window::createWindow({ 1024, 768 }, {800, 500}, false, new v3d::event::InputEventReceiver());
    ASSERT(m_Window, "windows is nullptr");

    m_InputEventHandler = new InputEventHandler();
    m_InputEventHandler->connect([](const KeyboardInputEvent* event)
    {
        if (event->_event == KeyboardInputEvent::KeyboardPressDown)
        {
            LOG_INFO("KeyboardInputEvent Down Key %c modif %x", event->_character, event->_modifers);
        }
        else
        {
            LOG_INFO("KeyboardInputEvent Up Key %c, modif %x", event->_character, event->_modifers);
        }
    });
    m_InputEventHandler->connect([](const MouseInputEvent* event)
    {
        if (event->_event == MouseInputEvent::MousePressDown)
        {
            LOG_INFO("MouseInputEvent Down Key %u modif %x, wheel %f, pos %d, %d", event->_key, event->_modifers, event->_wheelValue, event->_cursorPosition.x, event->_cursorPosition.y);
        }
        else if (event->_event == MouseInputEvent::MousePressUp)
        {
            LOG_INFO("MouseInputEvent UP Key %u modif %x, wheel %f, pos %d, %d", event->_key, event->_modifers, event->_wheelValue, event->_cursorPosition.x, event->_cursorPosition.y);
        }
        else if (event->_event == MouseInputEvent::MouseDoubleClick)
        {
            LOG_INFO("MouseInputEvent MouseDoubleClick Key %u modif %x, wheel %f, pos %d, %d", event->_key, event->_modifers, event->_wheelValue, event->_cursorPosition.x, event->_cursorPosition.y);
        }
        else if (event->_event == MouseInputEvent::MouseMoved)
        {
            LOG_INFO("MouseInputEvent MouseMoved Key %u modif %x, wheel %f, pos %d, %d", event->_key, event->_modifers, event->_wheelValue, event->_cursorPosition.x, event->_cursorPosition.y);
        }
        else if (event->_event == MouseInputEvent::MouseWheel)
        {
            LOG_INFO("MouseInputEvent MouseWheel Key %u modif %x, wheel %f, pos %d, %d", event->_key, event->_modifers, event->_wheelValue, event->_cursorPosition.x, event->_cursorPosition.y);
        }
    });

    m_Window->getInputEventReceiver()->attach(InputEvent::InputEventType::KeyboardInputEvent, m_InputEventHandler);
    m_Window->getInputEventReceiver()->attach(InputEvent::InputEventType::MouseInputEvent, m_InputEventHandler);
}

MyApplication::~MyApplication()
{
    if (m_Window)
    {
        Window::detroyWindow(m_Window);
        m_Window = nullptr;
    }
}

int MyApplication::Execute()
{
    MyApplication::Initialize();
    bool running = true;
    while (running)
    {
        running = Window::updateWindow(m_Window);

        m_Window->getInputEventReceiver()->sendDeferredEvents();

        MyApplication::Running();
    }

    Exit();

    delete this;

    return 0;
}

void MyApplication::Exit()
{
    m_Window->getInputEventReceiver()->dettach(InputEvent::InputEventType::KeyboardInputEvent);
    m_Window->getInputEventReceiver()->dettach(InputEvent::InputEventType::MouseInputEvent);

    delete m_InputEventHandler;
    m_InputEventHandler = nullptr;
}

void MyApplication::Initialize()
{
    //TEST
    LOG_DEBUG("Debug Test");
    LOG("Test");
    LOG_INFO("Info Test");
    LOG_WARNING("Warrnig Test");
    LOG_ERROR("Error Test");
    LOG_FATAL("Fatal Test");

    char cvar[] = "CHAR";
    int ivar = 0;

    LOG_DEBUG("Debug Test, %s, %d", cvar, ivar++);
    LOG("Test, %s, %d", cvar, ivar++);
    LOG_INFO("Info Test, %s, %d", cvar, ivar++);
    LOG_WARNING("Warrnig Test, %s, %d", cvar, ivar++);
    LOG_ERROR("Error Test, %s, %d", cvar, ivar++);
    LOG_FATAL("Fatal Test, %s, %d", cvar, ivar++);

    renderer::Context* context = renderer::Context::createContext(m_Window, renderer::Context::RenderType::VulkanRender);
    renderer::CommandList list(context, renderer::CommandList::CommandListType::DelayedCommandList);
    list.cmdBeginFrame();
    list.cmdEndFrame();

    list.flushCommands();
}

bool MyApplication::Running()
{
    //std::this_thread::sleep_for(std::chrono::seconds(3));
    //m_Window->minimize();
    //m_Window->isActive();
    //std::this_thread::sleep_for(std::chrono::seconds(3));
    //m_Window->restore();
    //m_Window->isActive();

    return true;
}
