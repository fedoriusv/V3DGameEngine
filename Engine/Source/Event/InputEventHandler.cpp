#include "InputEventHandler.h"
#include "InputEventKeyboard.h"
#include "Utils/Logger.h"

namespace v3d
{
namespace event
{

InputEventHandler::InputEventHandler()
    : m_gamepadStates(0U)
    , m_mousePosition({ 0, 0 })
    , m_mouseWheel(0.0f)
{
    resetKeyPressed();
}

InputEventHandler::~InputEventHandler()
{
    m_keyboardHandlerCallbacks.clear();
    m_mouseHandlerCallbacks.clear();
    m_gamepadHandlerCallbacks.clear();
}

void InputEventHandler::resetKeyPressed()
{
    std::fill_n(&m_keysPressed[0], Key_Codes_Count, false);

    for (u32 state = 0; state < MouseInputEvent::MouseCount; ++state)
    {
        m_mouseStates[state] = false;
    }
    m_mousePosition = { 0,0 };
    m_mouseWheel = 0.0f;
}

void InputEventHandler::applyModifiers(KeyboardInputEvent* event)
{
    if (InputEventHandler::isKeyPressed(KeyMenu))
    {
        event->_modifers |= KeyModifierCode::KeyModifier_Alt;
    }

    if (InputEventHandler::isKeyPressed(KeyControl))
    {
        event->_modifers |= KeyModifierCode::KeyModifier_Ctrl;
    }

    if (InputEventHandler::isKeyPressed(KeyShift))
    {
        event->_modifers |= KeyModifierCode::KeyModifier_Shift;
    }

    if (InputEventHandler::isKeyPressed(KeyCapital))
    {
        event->_modifers |= KeyModifierCode::KeyModifier_CapsLock;
    }
}

void InputEventHandler::applyModifiers(MouseInputEvent * event)
{
    if (InputEventHandler::isKeyPressed(KeyMenu))
    {
        event->_modifers |= KeyModifierCode::KeyModifier_Alt;
    }

    if (InputEventHandler::isKeyPressed(KeyControl))
    {
        event->_modifers |= KeyModifierCode::KeyModifier_Ctrl;
    }

    if (InputEventHandler::isKeyPressed(KeyShift))
    {
        event->_modifers |= KeyModifierCode::KeyModifier_Shift;
    }

    if (InputEventHandler::isKeyPressed(KeyCapital))
    {
        event->_modifers |= KeyModifierCode::KeyModifier_CapsLock;
    }

    event->_wheelValue = m_mouseWheel;
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
            m_keysPressed[keyEvent->_key] = true;
            break;
        }

        case KeyboardInputEvent::KeyboardPressUp:
        {
            applyModifiers(keyEvent);
            m_keysPressed[keyEvent->_key] = false;
            break;
        }

        default:
        {
            return false;
        }
        }

        for (std::vector<KeyboardCallback>::const_iterator iter = m_keyboardHandlerCallbacks.cbegin(); iter != m_keyboardHandlerCallbacks.cend(); ++iter)
        {
            if ((*iter))
            {
                (*iter)(keyEvent);
            }
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
        m_mousePosition = mouseEvent->_cursorPosition;
        m_mouseWheel = core::clamp(m_mouseWheel + mouseEvent->_wheelValue, m_mouseWheelRange.x, m_mouseWheelRange.y);

        switch (mouseEvent->_event)
        {
        case MouseInputEvent::MousePressDown:
        case MouseInputEvent::MousePressUp:
        case MouseInputEvent::MouseDoubleClick:

        /*case MouseInputEvent::MouseWheel:
        case MouseInputEvent::MouseMoved:*/
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
            if ((*iter))
            {
                (*iter)(mouseEvent);
            }
        }
        return true;
    };

    case InputEvent::InputEventType::GamepadInputEvent:
    {
        const GamepadInputEvent* gamepadEvent = static_cast<const GamepadInputEvent*>(event);
        m_gamepadStates = gamepadEvent->_buttons;

       for (std::vector<GamepadCallback>::const_iterator iter = m_gamepadHandlerCallbacks.cbegin(); iter != m_gamepadHandlerCallbacks.cend(); ++iter)
        {
            if ((*iter))
            {
                (*iter)(gamepadEvent);
            }
        }
        return true;
    };

    default:
    {
        return false;
    };
    }
}

void InputEventHandler::connect(std::function<void(const KeyboardInputEvent*)> callback)
{
    if (callback)
    {
        m_keyboardHandlerCallbacks.push_back(callback);
    }
}

void InputEventHandler::connect(std::function<void(const MouseInputEvent*)> callback)
{
    if (callback)
    {
        m_mouseHandlerCallbacks.push_back(callback);
    }
}

void InputEventHandler::connect(std::function<void(const GamepadInputEvent*)> callback)
{
    if (callback)
    {
        m_gamepadHandlerCallbacks.push_back(callback);
    }
}

bool InputEventHandler::isKeyPressed(const KeyCode& code)  const
{
    return m_keysPressed[code];
}

bool InputEventHandler::isLeftMousePressed() const
{
    return m_mouseStates[MouseInputEvent::MousePressDown] && m_keysPressed[KeyLButton];
}

bool InputEventHandler::isRightMousePressed() const
{
    return m_mouseStates[MouseInputEvent::MousePressDown] && m_keysPressed[KeyRButton];
}

bool InputEventHandler::isMiddleMousePressed() const
{
    return m_mouseStates[MouseInputEvent::MousePressDown] && m_keysPressed[KeyMButton];
}

bool InputEventHandler::isGamepadPressed(const GamepadInputEvent::GamepadButton& code) const
{
    if (code >= (u32)GamepadInputEvent::ButtonCount)
    {
        return false;
    }

    return (m_gamepadStates & (1 << code)) ? true : false;
}

const core::Point2D& InputEventHandler::getCursorPosition() const
{
    return m_mousePosition;
}

float InputEventHandler::getMouseWheel() const
{
    return m_mouseWheel;
}

} //namespace event
} //namespace v3d
