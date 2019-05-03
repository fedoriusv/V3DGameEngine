#include "WindowAndroid.h"
#include "Event/InputEventKeyboard.h"
#include "Event/InputEventMouse.h"
#include "Event/InputEventReceiver.h"
#include "Utils/Logger.h"

#ifdef PLATFORM_ANDROID

namespace v3d
{
namespace platform
{

WindowAndroid::WindowAndroid(const WindowParam& params, event::InputEventReceiver* receiver)
    : Window(params, receiver)
{
    LOG_DEBUG("WindowAndroid::WindowAndroid: Created Adroid window %llx", this);
}

bool WindowAndroid::initialize()
{
   //TODO
    return false;
}

bool WindowAndroid::update()
{
    //TODO
    return true;
}

void WindowAndroid::destroy()
{
    //TODO    
}

WindowAndroid::~WindowAndroid()
{
    LOG_DEBUG("WindowAndroid::~WindowAndroid");
    //ASSERT(!m_hWnd, "Not destroyed");
}

void WindowAndroid::minimize()
{
    //TODO
 }

void WindowAndroid::maximize()
{
    //TODO
}

void WindowAndroid::restore()
{
    //TODO
}

void WindowAndroid::setFullScreen(bool value)
{
    //TODO:
    ASSERT(false, "not implemented");
}

void WindowAndroid::setResizeble(bool value)
{
    //TODO:
    ASSERT(false, "not implemented");
}

void WindowAndroid::setTextCaption(const std::string& text)
{
    //TODO
}

void WindowAndroid::setPosition(const core::Point2D& pos)
{
    //TODO
}

bool WindowAndroid::isMaximized() const
{
    return m_params._isMaximized;
}

bool WindowAndroid::isMinimized() const
{
    return  m_params._isMinimized;
}

bool WindowAndroid::isActive() const
{
    //TODO
    return false;
}

bool WindowAndroid::isFocused() const
{
    //TODO
    return false;
}

NativeInstance WindowAndroid::getInstance() const
{
    //TODO
    return nullptr;
}

NativeWindows WindowAndroid::getWindowHandle() const
{
    //TODO
    return nullptr;
}

} //namespace platform
} //namespace v3d
#endif //PLATFORM_ANDROID
