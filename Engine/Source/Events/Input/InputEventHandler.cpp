#include "InputEventHandler.h"
#include "InputEventKeyboard.h"
#include "Utils/Logger.h"

namespace v3d
{
namespace event
{

InputEventHandler::InputEventHandler() noexcept
    : m_gamepadStates(0U)
    , m_relativeCursorPosition({ 0, 0 })
    , m_absoluteCursorPosition({ 0, 0 })
    , m_mouseWheel(0.0f)
{
    resetKeyStates();
    resetMouseStates();
    resetTouchStates();
}

InputEventHandler::~InputEventHandler()
{
    m_keyboardHandlerCallbacks.clear();
    m_mouseHandlerCallbacks.clear();
    m_gamepadHandlerCallbacks.clear();
    m_touchHandlerCallbacks.clear();
    m_systemEventCallbacks.clear();
}

void InputEventHandler::resetEventHandler()
{
    m_mouseWheel = 0.0f;
}

void InputEventHandler::resetKeyStates()
{
    std::fill_n(&m_keysPressed[0], toEnumType(KeyCode::Key_Codes_Count), false);
}

void InputEventHandler::resetMouseStates()
{
    for (u32 state = 0; state < MouseInputEvent::MouseCount; ++state)
    {
        m_mouseStates[state] = false;
    }
    m_relativeCursorPosition = { 0,0 };
    m_absoluteCursorPosition = { 0,0 };
    m_mouseWheel = 0.0f;
}

void InputEventHandler::resetTouchStates()
{
    for (u32 index = 0; index < k_maxTouchScreenFingers; ++index)
    {
        m_touchScreenStates[index] = false;
    }
    m_touchScreen = false;
    m_multiScreenTouch = false;
}

void InputEventHandler::applyTouches(u32 mask, bool isPressed)
{
    //TODO
    /*u32 value = k_maxTouchScreenFingers - 1;
    for (u32 index = 0; index < k_maxTouchScreenFingers; ++index)
    {
        if ((value & mask) == index)
        {
            m_touchScreenStates[index] = isPressed;
        }
    }*/
}

bool InputEventHandler::onEvent(Event* ev)
{
    ASSERT(ev, "Event must be not nullptr");
    InputEvent* event = static_cast<InputEvent*>(ev);
    switch (event->_eventType)
    {
    case InputEvent::InputEventType::KeyboardInputEvent:
    {
        KeyboardInputEvent* keyEvent = static_cast<KeyboardInputEvent*>(event);
        switch (keyEvent->_event)
        {
        case KeyboardInputEvent::KeyboardPressDown:
        {
            applyModifiers(keyEvent);
            m_keysPressed[toEnumType(keyEvent->_key)] = true;
            break;
        }

        case KeyboardInputEvent::KeyboardPressUp:
        {
            applyModifiers(keyEvent);
            m_keysPressed[toEnumType(keyEvent->_key)] = false;
            break;
        }

        default:
        {
            return false;
        }
        }

        for (std::vector<KeyboardCallback>::const_iterator iter = m_keyboardHandlerCallbacks.cbegin(); iter != m_keyboardHandlerCallbacks.cend(); ++iter)
        {
            ASSERT(*iter, "must be valid");
            (*iter)(keyEvent);
        }
        return true;
    };

    case InputEvent::InputEventType::MouseInputEvent:
    {
        MouseInputEvent* mouseEvent = static_cast<MouseInputEvent*>(event);

        for (u32 state = 0; state < MouseInputEvent::MouseCount; ++state)
        {
            m_mouseStates[state] = state == mouseEvent->_event;
        }
        m_relativeCursorPosition = mouseEvent->_clientCoordinates;
        m_absoluteCursorPosition = mouseEvent->_absoluteCoordinates;
        m_mouseWheel = std::clamp(mouseEvent->_wheelValue, m_mouseWheelRange._x, m_mouseWheelRange._y);

        switch (mouseEvent->_event)
        {
        case MouseInputEvent::MouseDoubleClick:
        case MouseInputEvent::MousePressDown:
        {
            applyModifiers(mouseEvent);
            m_keysPressed[toEnumType(mouseEvent->_key)] = true;
            break;
        }

        case MouseInputEvent::MousePressUp:
        {
            applyModifiers(mouseEvent);
            m_keysPressed[toEnumType(mouseEvent->_key)] = false;
            break;
        }

        case MouseInputEvent::MouseWheel:
        case MouseInputEvent::MouseMoved:
        {
            applyModifiers(mouseEvent);
            break;
        }

        default:
        {
            return false;
        }
        }

        for (std::vector<MouseCallback>::const_iterator iter = m_mouseHandlerCallbacks.cbegin(); iter != m_mouseHandlerCallbacks.cend(); ++iter)
        {
            ASSERT(*iter, "must be valid");
            (*iter)(mouseEvent);
        }
        return true;
    };

    case InputEvent::InputEventType::GamepadInputEvent:
    {
        const GamepadInputEvent* gamepadEvent = static_cast<const GamepadInputEvent*>(event);
        m_gamepadStates = gamepadEvent->_buttons;

        for (std::vector<GamepadCallback>::const_iterator iter = m_gamepadHandlerCallbacks.cbegin(); iter != m_gamepadHandlerCallbacks.cend(); ++iter)
        {
            ASSERT(*iter, "must be valid");
            (*iter)(gamepadEvent);
        }
        return true;
    };

    case InputEvent::InputEventType::TouchInputEvent:
    {
        TouchInputEvent* touchEvent = static_cast<TouchInputEvent*>(event);
        if (touchEvent->_event == TouchInputEvent::TouchKey)
        {
            switch (touchEvent->_keyEvent)
            {
            case TouchInputEvent::TouchKeyPressDown:
            {
                applyModifiers(touchEvent);
                m_keysPressed[toEnumType(touchEvent->_key)] = true;
                break;
            }

            case TouchInputEvent::TouchKeyPressUp:
            {
                applyModifiers(touchEvent);
                m_keysPressed[toEnumType(touchEvent->_key)] = false;
                break;
            }

            case TouchInputEvent::TouchKeyPressMultipress:
            {
                applyModifiers(touchEvent);
                break;
            }

            default:
                break;
            };
        } 
        else if (touchEvent->_event == TouchInputEvent::TouchMotion)
        {
            m_relativeCursorPosition = touchEvent->_position;
            m_absoluteCursorPosition = touchEvent->_position;
            switch(touchEvent->_motionEvent)
            {
                case TouchInputEvent::TouchMotionEvent::TouchMotionDown:
                    m_touchScreen = true;
                    break;

                case TouchInputEvent::TouchMotionEvent::TouchMotionMultiTouchDown:
                    applyTouches(touchEvent->_pointers, true);
                    m_multiScreenTouch = true;
                    break;

                case TouchInputEvent::TouchMotionEvent::TouchMotionUp:
                    m_touchScreen = false;
                    break;

                case TouchInputEvent::TouchMotionEvent::TouchMotionMultiTouchUp:
                    applyTouches(touchEvent->_pointers, false);
                    m_multiScreenTouch = false;
                    break;

                default:
                    break;
            }
        }

        for (std::vector<TouchScreenCallback>::const_iterator iter = m_touchHandlerCallbacks.cbegin(); iter != m_touchHandlerCallbacks.cend(); ++iter)
        {
            ASSERT(*iter, "must be valid");
            (*iter)(touchEvent);
        }
        return true;
    };

    case InputEvent::InputEventType::SystemEvent:
    {
        const SystemEvent* systemEvent = static_cast<const SystemEvent*>(event);
        for (std::vector<SystemEventCallback>::const_iterator iter = m_systemEventCallbacks.cbegin(); iter != m_systemEventCallbacks.cend(); ++iter)
        {
            ASSERT(*iter, "must be valid");
            (*iter)(systemEvent);
        }
        return true;
    }

    default:
    {
        return false;
    };
    }
}

void InputEventHandler::bind(std::function<void(const KeyboardInputEvent*)> callback)
{
    if (callback)
    {
        m_keyboardHandlerCallbacks.push_back(callback);
    }
}

void InputEventHandler::bind(std::function<void(const MouseInputEvent*)> callback)
{
    if (callback)
    {
        m_mouseHandlerCallbacks.push_back(callback);
    }
}

void InputEventHandler::bind(std::function<void(const GamepadInputEvent*)> callback)
{
    if (callback)
    {
        m_gamepadHandlerCallbacks.push_back(callback);
    }
}

void InputEventHandler::bind(std::function<void(const TouchInputEvent*)> callback)
{
    if (callback)
    {
        m_touchHandlerCallbacks.push_back(callback);
    }
}

void InputEventHandler::bind(std::function<void(const SystemEvent*)> callback)
{
    if (callback)
    {
        m_systemEventCallbacks.push_back(callback);
    }
}

bool InputEventHandler::isKeyboardInputEventsBound() const
{
    return !m_keyboardHandlerCallbacks.empty();
}

bool InputEventHandler::isMouseInputEventsBound() const
{
    return !m_mouseHandlerCallbacks.empty();
}

bool InputEventHandler::GamepadInputEventsBound() const
{
    return !m_gamepadHandlerCallbacks.empty();
}

bool InputEventHandler::TouchInputEventsBound() const
{
    return !m_touchHandlerCallbacks.empty();
}

bool InputEventHandler::isGamepadPressed(const GamepadInputEvent::GamepadButton& code) const
{
    if (code >= (u32)GamepadInputEvent::ButtonCount)
    {
        return false;
    }

    return (m_gamepadStates & (1 << code)) ? true : false;
}

bool InputEventHandler::isScreenTouched(s16 pointer) const
{
    if (pointer < 0)
    {
        return m_touchScreen;
    }
    else
    {
        ASSERT(pointer < k_maxTouchScreenFingers, "range out");
        return m_touchScreenStates[pointer];
    }
}

bool InputEventHandler::isMultiScreenTouch() const
{
    return m_multiScreenTouch;
}

const math::Point2D& InputEventHandler::getRelativeCursorPosition() const
{
    return m_relativeCursorPosition;
}

const math::Point2D& InputEventHandler::getAbsoluteCursorPosition() const
{
    return m_absoluteCursorPosition;
}

float InputEventHandler::getMouseWheel() const
{
    return m_mouseWheel;
}

} //namespace event
} //namespace v3d
