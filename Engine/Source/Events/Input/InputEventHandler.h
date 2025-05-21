#pragma once

#include "Common.h"
#include "Events/EventHandler.h"
#include "KeyCodes.h"

#include "InputEventKeyboard.h"
#include "InputEventMouse.h"
#include "InputEventGamepad.h"
#include "InputEventTouch.h"
#include "SystemEvent.h"


namespace v3d
{
namespace event
{
    class InputEventReceiver;

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief InputEventHandler class.
    */
    class V3D_API InputEventHandler : public EventHandler
    {

    public:

        InputEventHandler() noexcept;
        ~InputEventHandler();

        void bind(std::function<void(const KeyboardInputEvent*)> callback);
        void bind(std::function<void(const MouseInputEvent*)> callback);
        void bind(std::function<void(const GamepadInputEvent*)> callback);
        void bind(std::function<void(const TouchInputEvent*)> callback);
        void bind(std::function<void(const SystemEvent*)> callback);

        bool isKeyboardInputEventsBound() const;
        bool isMouseInputEventsBound() const;
        bool GamepadInputEventsBound() const;
        bool TouchInputEventsBound() const;

        bool isKeyPressed(const KeyCode& code)const;

        bool isLeftMousePressed() const;
        bool isRightMousePressed() const;
        bool isMiddleMousePressed() const;

        bool isGamepadPressed(const GamepadInputEvent::GamepadButton& code) const;

        bool isScreenTouched(s16 pointer = -1) const;
        bool isMultiScreenTouch() const;

        const math::Point2D& getRelativeCursorPosition() const;
        const math::Point2D& getAbsoluteCursorPosition() const;
        f32 getMouseWheel() const;

    private:

        static const u32 k_maxTouchScreenFingers = 8;

        bool onEvent(Event* event) override;

        void resetEventHandler();

        void resetKeyStates();
        void resetMouseStates();
        void resetTouchStates();

        template<class TInputEvent>
        void applyModifiers(TInputEvent* event);

        void applyTouches(u32 mask, bool isPressed);

        bool m_keysPressed[toEnumType(KeyCode::Key_Codes_Count)];
        bool m_mouseStates[MouseInputEvent::MouseCount];

        bool m_touchScreenStates[k_maxTouchScreenFingers];
        bool m_touchScreen;
        bool m_multiScreenTouch;
        
        u32 m_gamepadStates;

        math::Point2D m_relativeCursorPosition;
        math::Point2D m_absoluteCursorPosition;
        f32 m_mouseWheel;
        const math::TVector2D<f32> m_mouseWheelRange = { -100.0f, +100.0f };

        friend InputEventReceiver;

        typedef std::function<void(const KeyboardInputEvent*)>   KeyboardCallback;
        typedef std::function<void(const MouseInputEvent*)>      MouseCallback;
        typedef std::function<void(const GamepadInputEvent*)>    GamepadCallback;
        typedef std::function<void(const TouchInputEvent*)>      TouchScreenCallback;
        typedef std::function<void(const SystemEvent*)>          SystemEventCallback;

        std::vector<KeyboardCallback>       m_keyboardHandlerCallbacks;
        std::vector<MouseCallback>          m_mouseHandlerCallbacks;
        std::vector<GamepadCallback>        m_gamepadHandlerCallbacks;
        std::vector<TouchScreenCallback>    m_touchHandlerCallbacks;
        std::vector<SystemEventCallback>    m_systemEventCallbacks;
    };

    inline bool InputEventHandler::isKeyPressed(const KeyCode& code) const
    {
        return m_keysPressed[toEnumType(code)];
    }

    inline bool InputEventHandler::isLeftMousePressed() const
    {
        return m_keysPressed[toEnumType(KeyCode::KeyLButton)];
    }

    inline bool InputEventHandler::isRightMousePressed() const
    {
        return m_keysPressed[toEnumType(KeyCode::KeyRButton)];
    }

    inline bool InputEventHandler::isMiddleMousePressed() const
    {
        return m_keysPressed[toEnumType(KeyCode::KeyMButton)];
    }

    template<class TInputEvent>
    inline void InputEventHandler::applyModifiers(TInputEvent* event)
    {
        if (InputEventHandler::isKeyPressed(KeyCode::KeyLControl) || InputEventHandler::isKeyPressed(KeyCode::KeyRControl))
        {
            event->_modifers |= KeyModifierCode::KeyModifier_Ctrl;
        }

        if (InputEventHandler::isKeyPressed(KeyCode::KeyLAlt) || InputEventHandler::isKeyPressed(KeyCode::KeyRAlt))
        {
            event->_modifers |= KeyModifierCode::KeyModifier_Alt;
        }

        if (InputEventHandler::isKeyPressed(KeyCode::KeyLShift) || InputEventHandler::isKeyPressed(KeyCode::KeyRShift))
        {
            event->_modifers |= KeyModifierCode::KeyModifier_Shift;
        }

        if (InputEventHandler::isKeyPressed(KeyCode::KeyCapital))
        {
            event->_modifers |= KeyModifierCode::KeyModifier_CapsLock;
        }
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////


} //namespace event
} //namespace v3d
