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

Window::Window(const WindowParam& params, event::InputEventReceiver* receiver) noexcept
    : m_params(params)
    , m_receiver(receiver)
{
}

Window::~Window()
{
    ASSERT(!m_receiver, "must be nullptr");
}

Window* Window::createWindow(const math::Dimension2D& size, const math::Point2D& pos, bool fullscreen, bool resizable)
{
    WindowParam params;
    params._size = size;
    params._position = pos;
    params._isFullscreen = fullscreen;
    params._isResizable = fullscreen ? false : resizable;

    Window* window = nullptr;
#if defined(PLATFORM_WINDOWS)
    window = V3D_NEW(WindowWindows, memory::MemoryLabel::MemorySystem)(params, nullptr);
#elif defined(PLATFORM_XBOX)
    window = V3D_NEW(WindowXBOX, memory::MemoryLabel::MemorySystem)(params, nullptr);
#elif defined(PLATFORM_ANDROID)
    window = V3D_NEW(WindowAndroid, memory::MemoryLabel::MemorySystem)(params, nullptr);
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
    V3D_DELETE(window, memory::MemoryLabel::MemorySystem);

    return nullptr;
}

Window* Window::createWindow(const math::Dimension2D& size, const math::Point2D& pos, bool fullscreen, event::InputEventReceiver* receiver)
{
    WindowParam params;
    params._size = size;
    params._position = pos;
    params._isFullscreen = fullscreen;
    params._isResizable = false;

    Window* window = nullptr;
#if defined(PLATFORM_WINDOWS)
    window = V3D_NEW(WindowWindows, memory::MemoryLabel::MemorySystem)(params, receiver);
#elif defined(PLATFORM_XBOX)
    window = V3D_NEW(WindowXBOX, memory::MemoryLabel::MemorySystem)(params, receiver);
#elif defined(PLATFORM_ANDROID)
    window = V3D_NEW(WindowAndroid, memory::MemoryLabel::MemorySystem)(params, receiver);
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
    V3D_DELETE(window, memory::MemoryLabel::MemorySystem);

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

    V3D_DELETE(window, memory::MemoryLabel::MemorySystem);
}

event::InputEventReceiver* Window::getInputEventReceiver() const
{
    ASSERT(m_receiver, "m_receiver is nullptr");
    return m_receiver;
}

const math::Dimension2D & Window::getSize() const
{
    return m_params._size;
}

const math::Point2D & Window::getPosition() const
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
