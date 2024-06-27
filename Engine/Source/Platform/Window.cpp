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

std::map<u32, Window*> Window::s_windowsList = {};

Window::Window(const WindowParam& params, event::InputEventReceiver* receiver) noexcept
    : m_params(params)
    , m_receiver(receiver)
{
}

Window::~Window()
{
}

Window* Window::createWindow(const math::Dimension2D& size, const math::Point2D& pos, bool fullscreen, bool resizable, event::InputEventReceiver* receiver, const std::wstring& name)
{
    WindowParam params;
    params._name = name.empty() ? L"Window" : name;
    params._size = size;
    params._position = pos;
    params._isFullscreen = fullscreen;
    params._isResizable = fullscreen ? false : resizable;

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
        window->setTextCaption(params._name);

        s_windowsList.emplace(window->ID(), window);
        return window;
    }
    
    LOG_ERROR("Window::createWindow: Can't initialize window");
    V3D_DELETE(window, memory::MemoryLabel::MemorySystem);

    return nullptr;
}

Window* Window::createWindow(const math::Dimension2D& size, const math::Point2D& pos, bool fullscreen, event::InputEventReceiver* receiver, const std::wstring& name)
{
    WindowParam params;
    params._name = name.empty() ? L"Window" : name;
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
        window->setTextCaption(params._name);

        s_windowsList.emplace(window->ID(), window);
        return window;
    }

    LOG_ERROR("Window::createWindow: Can't initialize window");
    window->destroy();
    V3D_DELETE(window, memory::MemoryLabel::MemorySystem);

    return nullptr;
}

Window* Window::createWindow(const math::Dimension2D& size, const math::Point2D& pos, const Window* parent, bool resizable, const std::wstring& name)
{
    WindowParam params;
    params._name = name.empty() ? L"Window" : name;
    params._size = size;
    params._position = pos;
    params._isFullscreen = false;
    params._isResizable = resizable;

    Window* window = nullptr;
#if defined(PLATFORM_WINDOWS)
    window = V3D_NEW(WindowWindows, memory::MemoryLabel::MemorySystem)(params, parent->getInputEventReceiver(), parent);
#elif defined(PLATFORM_XBOX)
    window = V3D_NEW(WindowXBOX, memory::MemoryLabel::MemorySystem)(params, parent->getInputEventReceiver());
#elif defined(PLATFORM_ANDROID)
    window = V3D_NEW(WindowAndroid, memory::MemoryLabel::MemorySystem)(params, parent->getInputEventReceiver());
#endif //PLATFORM

    if (!window)
    {
        ASSERT(false, "unsupported platform");
        LOG_ERROR("Window::createWindow: Unsupported platform");
        return nullptr;
    }

    if (window->initialize())
    {
        window->setTextCaption(params._name);

        s_windowsList.emplace(window->ID(), window);
        return window;
    }

    LOG_ERROR("Window::createWindow: Can't initialize window");
    window->destroy();
    V3D_DELETE(window, memory::MemoryLabel::MemorySystem);

    return nullptr;
}

bool Window::updateEvents(Window* window)
{
    ASSERT(window, "window is nullptr");
    return window->update();
}

void Window::detroyWindow(Window* window)
{
    ASSERT(window, "window is nullptr");

    auto found = s_windowsList.find(window->ID());
    ASSERT(found != s_windowsList.end(), "window is not found");
    s_windowsList.erase(found);

    window->destroy();
    V3D_DELETE(window, memory::MemoryLabel::MemorySystem);
}

} //namespace platform
} //namespace v3d
