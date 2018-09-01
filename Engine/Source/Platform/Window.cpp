#include "Window.h"
#include "Event/InputEventReceiver.h"
#include "Utils/Logger.h"

#ifdef PLATFORM_WINDOWS
    #include "WindowWindows.h"
#endif //PLATFORM_WINDOWS

namespace v3d
{
namespace platform
{

Window::Window(const WindowParam& params, event::InputEventReceiver* receiver)
    : m_params(params)
    , m_receiver(receiver)
{
}

Window::~Window()
{
}

Window* Window::createWindow(const core::Dimension2D& size, const core::Point2D& pos, bool fullscreen, bool resizable)
{
    WindowParam params;
    params._size = size;
    params._position = pos;
    params._isFullscreen = fullscreen;
    params._isResizable = fullscreen ? false : resizable;

#ifdef PLATFORM_WINDOWS
    Window* window = new WindowWindows(params, nullptr);
#endif //PLATFORM_WINDOWS

    if (window->initialize())
    {
        return window;
    }
    
    LOG_ERROR("Window::createWindow: Can't initialize window");
    delete window;

    return nullptr;
}

Window* Window::createWindow(const core::Dimension2D& size, const core::Point2D& pos, bool fullscreen, event::InputEventReceiver* receiver)
{
    WindowParam params;
    params._size = size;
    params._position = pos;
    params._isFullscreen = fullscreen;
    params._isResizable = false;

#ifdef PLATFORM_WINDOWS
    Window* window = new WindowWindows(params, receiver);
#endif //_PLATFORM_WINDOWS_

    if (window->initialize())
    {
        return window;
    }

    LOG_ERROR("Window::createWindow: Can't initialize window");
    delete window;

    return nullptr;
}

bool Window::updateWindow(Window* window)
{
    ASSERT(window, "window is nullptr");
    return window->update();
}

void Window::detroyWindow(Window* window)
{
    ASSERT(window, "window is nullptr");
    window->destroy();

    if (window->m_receiver)
    {
        delete window->m_receiver;
        window->m_receiver = nullptr;
    }

    delete window;
}

event::InputEventReceiver* Window::getInputEventReceiver() const
{
    return m_receiver;
}

} //namespace platform
} //namespace v3d
