#include "Window.h"
#include "Utils/Logger.h"

#ifdef _PLATFORM_WINDOWS_
    #include "WindowWindows.h"
#endif //_PLATFORM_WINDOWS_

namespace v3d
{
namespace platform
{

Window::Window(const WindowParam& params)
    : m_params(params)
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

#ifdef _PLATFORM_WINDOWS_
    Window* window = new WindowWindows(params);
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

    delete window;
}

} //namespace platform
} //namespace v3d
