// Main.cpp : Defines the entry point for the console application.
//

#include "Common.h"
#include "Platform/Window.h"
#include "Events/InputEventReceiver.h"
#include "Utils/Logger.h"

using namespace v3d;
using namespace v3d::platform;
using namespace v3d::utils;
using namespace v3d::event;

class CreateWindowsApplication
{
public:

    CreateWindowsApplication(int& argc, char** argv) noexcept
        : m_MainWindow(nullptr)
        , m_InputReceiver(new InputEventReceiver())

        , m_InputEventHandler(nullptr)
    {
        m_MainWindow = Window::createWindow({ 1024, 768 }, { 100, 100 }, false, m_InputReceiver, L"Main. Double click to create sub window");
        ASSERT(m_MainWindow, "windows is nullptr");
        Window* UIWindow = Window::createWindow({ 250, 250 }, { 1150, 100 }, false, true, m_InputReceiver, L"UI");
        ASSERT(UIWindow, "windows is nullptr");
        m_ChildWindows.push_back(UIWindow);

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
        m_InputEventHandler->bind([this](const MouseInputEvent* event)
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

                    u32 min = 0;
                    u32 maxW = 1000;
                    u32 maxH = 500;
                    u32 rangeW = maxW - min + 1;
                    u32 rangeH = maxH - min + 1;
                    s32 w = rand() % rangeW + min;
                    s32 h = rand() % rangeH + min;

                    std::string name = "UI_" + std::to_string(utils::Timer::getCurrentTime());
                    std::wstring wname(name.begin(), name.end());
                    Window* UIWindow = Window::createWindow({ 250, 250 }, { w, h }, false, true, m_InputReceiver, wname);
                    ASSERT(UIWindow, "windows is nullptr");
                    m_ChildWindows.push_back(UIWindow);
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

        m_InputReceiver->attach(InputEvent::InputEventType::KeyboardInputEvent, m_InputEventHandler);
        m_InputReceiver->attach(InputEvent::InputEventType::MouseInputEvent, m_InputEventHandler);
#endif //PLATFORM_WINDOWS
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

        m_InputReceiver->attach(InputEvent::InputEventType::SystemEvent, m_InputEventHandler);
    }

    ~CreateWindowsApplication()
    {
        for (Window* w : m_ChildWindows)
        {
            Window::detroyWindow(w);
        }
        m_ChildWindows.clear();

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

        memory::memory_test();
    }

    int execute()
    {
        Initialize();
        while (true)
        {
            Window::updateEvents(m_MainWindow);
            for (Window* w : m_ChildWindows)
            {
                Window::updateEvents(w);
            }

            m_InputReceiver->sendDeferredEvents();
            if (m_teminateApp)
            {
                break;
            }

            Run();
        }

        Exit();
        return 0;
    }

    void OnCreate(v3d::platform::Window* window)
    {
        LOG_INFO("OnCreate, Window %u is created", window->ID());
    }
    
    void OnDestroy(v3d::platform::Window* window)
    {
        LOG_INFO("OnDestroy, Window %u is destroyed", window->ID());
        if (window == m_MainWindow)
        {
            m_teminateApp = true;
        }
        else
        {
            auto found = std::find(m_ChildWindows.begin(), m_ChildWindows.end(), window);
            Window::detroyWindow(window);
            m_ChildWindows.erase(found);
        }
    }
    
    void OnResize(v3d::platform::Window* window)
    {
        LOG_INFO("OnCreate, Window %u is resized, width %u, height %u", window->ID(), window->getSize().m_width, window->getSize().m_height);
    }

private:

    void Initialize()
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
    
    bool Run()
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

    void Exit()
    {
#ifdef PLATFORM_WINDOWS
        m_InputReceiver->dettach(InputEvent::InputEventType::KeyboardInputEvent);
        m_InputReceiver->dettach(InputEvent::InputEventType::MouseInputEvent);
#endif //PLATFORM_WINDOWS
        m_InputReceiver->dettach(InputEvent::InputEventType::SystemEvent);
    }

    v3d::platform::Window* m_MainWindow;
    std::list<v3d::platform::Window*> m_ChildWindows;
    v3d::event::InputEventReceiver* m_InputReceiver;

    v3d::event::InputEventHandler* m_InputEventHandler;

public:

    bool m_teminateApp = false;
};

int main(int argc, char* argv[])
{
    CreateWindowsApplication app(argc, argv);
    return app.execute();
}
