#include "MyApplication.h"
#include "Utils/Logger.h"

#include "Renderer/CommandList.h"
#include "Renderer/Core/Context.h"

using namespace v3d;
using namespace v3d::platform;
using namespace v3d::utils;
using namespace v3d::event;

MyApplication::MyApplication(int& argc, char** argv)
    : m_MainWindow(nullptr)
    , m_UIWindow(nullptr)
    , m_InputReceiver(new InputEventReceiver())

    , m_InputEventHandler(nullptr)
{
    m_MainWindow = Window::createWindow({ 1024, 768 }, {100, 100}, false, m_InputReceiver, L"Main");
    ASSERT(m_MainWindow, "windows is nullptr");
    m_UIWindow = Window::createWindow({ 250, 250 }, { 1150, 100 }, false, true, m_InputReceiver, L"UI");
    ASSERT(m_UIWindow, "windows is nullptr");

#ifdef PLATFORM_WINDOWS
    m_InputEventHandler = new InputEventHandler();
    m_InputEventHandler->bind([](const KeyboardInputEvent* event)
    {
        if (event->_event == KeyboardInputEvent::KeyboardPressDown)
        {
            LOG_INFO("KeyboardInputEvent, WindowID %u, Down Key %c modif %x", event->_windowID, event->_character, event->_modifers);
        }
        else
        {
            LOG_INFO("KeyboardInputEvent, WindowID %u Up Key %c, modif %x", event->_windowID, event->_character, event->_modifers);
        }
    });
    m_InputEventHandler->bind([](const MouseInputEvent* event)
    {
        if (event->_event == MouseInputEvent::MousePressDown)
        {
            LOG_INFO("MouseInputEvent, WindowID %u Down Key %u modif %x, wheel %f, pos %d, %d", event->_windowID, event->_key, event->_modifers, event->_wheelValue, event->_cursorPosition.m_x, event->_cursorPosition.m_y);
        }
        else if (event->_event == MouseInputEvent::MousePressUp)
        {
            LOG_INFO("MouseInputEvent, WindowID %u UP Key %u modif %x, wheel %f, pos %d, %d", event->_windowID, event->_key, event->_modifers, event->_wheelValue, event->_cursorPosition.m_x, event->_cursorPosition.m_y);
        }
        else if (event->_event == MouseInputEvent::MouseDoubleClick)
        {
            LOG_INFO("MouseInputEvent, WindowID %u MouseDoubleClick Key %u modif %x, wheel %f, pos %d, %d", event->_windowID, event->_key, event->_modifers, event->_wheelValue, event->_cursorPosition.m_x, event->_cursorPosition.m_y);
        }
        else if (event->_event == MouseInputEvent::MouseMoved)
        {
            LOG_INFO("MouseInputEvent, WindowID %u MouseMoved Key %u modif %x, wheel %f, pos %d, %d", event->_windowID, event->_key, event->_modifers, event->_wheelValue, event->_cursorPosition.m_x, event->_cursorPosition.m_y);
        }
        else if (event->_event == MouseInputEvent::MouseWheel)
        {
            LOG_INFO("MouseInputEvent, WindowID %u MouseWheel Key %u modif %x, wheel %f, pos %d, %d", event->_windowID, event->_key, event->_modifers, event->_wheelValue, event->_cursorPosition.m_x, event->_cursorPosition.m_y);
        }
    });
    m_InputEventHandler->bind([this](const SystemEvent* event)
        {
            Window* window = Window::getWindowsByID(event->_windowID);
            if (event->_systemEvent == SystemEvent::Create)
            {
                OnCreate(window);
            }
            else if (event->_systemEvent == SystemEvent::Destroy)
            {
                OnDestroy(window);
            }
            else if (event->_systemEvent == SystemEvent::Resize)
            {
                OnResize(window);
            }
        });

    m_InputReceiver->attach(InputEvent::InputEventType::KeyboardInputEvent, m_InputEventHandler);
    m_InputReceiver->attach(InputEvent::InputEventType::MouseInputEvent, m_InputEventHandler);
    m_InputReceiver->attach(InputEvent::InputEventType::SystemEvent, m_InputEventHandler);
#endif //PLATFORM_WINDOWS
}

int MyApplication::Execute()
{
    MyApplication::Initialize();
    while (true)
    {
        Window::updateEvents(m_MainWindow);
        if (m_UIWindow)
        {
            Window::updateEvents(m_UIWindow);
        }

        m_InputReceiver->sendDeferredEvents();
        if (m_teminateApp)
        {
            break;
        }

        MyApplication::Running();
    }

    Exit();
    delete this;

    memory::memory_test();
    return 0;
}

void MyApplication::OnCreate(Window* window)
{
    LOG_INFO("OnCreate, Window %u is created", window->ID());
}

void MyApplication::OnDestroy(Window* window)
{
    LOG_INFO("OnDestroy, Window %u is destroyed", window->ID());
    if (window == m_UIWindow)
    {
        Window::detroyWindow(m_UIWindow);
        m_UIWindow = nullptr;
    }
    else if (window == m_MainWindow)
    {
        m_teminateApp = true;
    }
}

void MyApplication::OnResize(Window* window)
{
    LOG_INFO("OnCreate, Window %u is resized, width %u, height %u", window->ID(), window->getSize().m_width, window->getSize().m_height);
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
}

bool MyApplication::Running()
{
    /*
    std::this_thread::sleep_for(std::chrono::seconds(3));
    m_Window->minimize();
    m_Window->isActive();
    std::this_thread::sleep_for(std::chrono::seconds(3));
    m_Window->restore();
    m_Window->isActive();
    */
    return true;
}

void MyApplication::Exit()
{
#ifdef PLATFORM_WINDOWS
    m_InputReceiver->dettach(InputEvent::InputEventType::KeyboardInputEvent);
    m_InputReceiver->dettach(InputEvent::InputEventType::MouseInputEvent);
    m_InputReceiver->dettach(InputEvent::InputEventType::SystemEvent);
#endif //PLATFORM_WINDOWS
}

MyApplication::~MyApplication()
{
    if (m_UIWindow)
    {
        Window::detroyWindow(m_UIWindow);
        m_UIWindow = nullptr;
    }

    if (m_MainWindow)
    {
        Window::detroyWindow(m_MainWindow);
        m_MainWindow = nullptr;
    }

    if (m_InputEventHandler)
    {
        delete m_InputEventHandler;
        m_InputEventHandler = nullptr;
    }

    if (m_InputReceiver)
    {
        delete m_InputReceiver;
        m_InputReceiver = nullptr;
    }

    Logger::freeInstance();
}
