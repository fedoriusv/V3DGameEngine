#include "Window.h"
#include "Event/InputEventReceiver.h"
#include "Utils/Logger.h"
#include "Utils/Profiler.h"

#if defined(PLATFORM_WINDOWS)
#   include "Windows/WindowWindows.h"
#elif defined(PLATFORM_XBOX)
#   include "XBOX/WindowXBOX.h"
#elif defined(PLATFORM_ANDROID)
#   include "Android/WindowAndroid.h"
#endif //PLATFORM

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
    ASSERT(!m_receiver, "must be nullptr");
}

Window* Window::createWindow(const core::Dimension2D& size, const core::Point2D& pos, bool fullscreen, bool resizable)
{
    WindowParam params;
    params._size = size;
    params._position = pos;
    params._isFullscreen = fullscreen;
    params._isResizable = fullscreen ? false : resizable;

    Window* window = nullptr;
#if defined(PLATFORM_WINDOWS)
    window = new WindowWindows(params, nullptr);
#elif defined(PLATFORM_XBOX)
    window = new WindowXBOX(params, nullptr);
#elif defined(PLATFORM_ANDROID)
    window = new WindowAndroid(params, nullptr);
#endif //PLATFORM

    if (!window)
    {
        ASSERT(false, "unsupported platform");
        LOG_ERROR("Window::createWindow: Unsupported platform");
        return nullptr;
    }

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

    Window* window = nullptr;
#if defined(PLATFORM_WINDOWS)
    window = new WindowWindows(params, receiver);
#elif defined(PLATFORM_XBOX)
    window = new WindowXBOX(params, receiver);
#elif defined(PLATFORM_ANDROID)
    window = new WindowAndroid(params, receiver);
#endif //PLATFORM

    if (!window)
    {
        ASSERT(false, "unsupported platform");
        LOG_ERROR("Window::createWindow: Unsupported platform");
        return nullptr;
    }

    if (window->initialize())
    {
        return window;
    }

    LOG_ERROR("Window::createWindow: Can't initialize window");
    window->destroy();
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
#if FRAME_PROFILER_ENABLE
    utils::ProfileManager::freeInstance();
#endif //FRAME_PROFILER_ENABLE

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
    ASSERT(m_receiver, "m_receiver is nullptr");
    return m_receiver;
}

const core::Dimension2D & Window::getSize() const
{
    return m_params._size;
}

const core::Point2D & Window::getPosition() const
{
    return m_params._position;
}

bool Window::isFullscreen() const
{
    return m_params._isFullscreen;
}

bool Window::isResizable() const
{
    return m_params._isResizable;
}

} //namespace platform
} //namespace v3d
