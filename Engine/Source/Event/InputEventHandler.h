#pragma once

#include "Common.h"
#include "EventHandler.h"
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
    class InputEventHandler final : public EventHandler
    {

    public:

        InputEventHandler() noexcept;
        ~InputEventHandler();

        void bind(std::function<void(const KeyboardInputEvent*)> callback);
        void bind(std::function<void(const MouseInputEvent*)> callback);
        void bind(std::function<void(const GamepadInputEvent*)> callback);
        void bind(std::function<void(const TouchInputEvent*)> callback);
        void bind(std::function<void(const SystemEvent*)> callback);

        bool isKeyPressed(const KeyCode& code)const;

        bool isLeftMousePressed() const;
        bool isRightMousePressed() const;
        bool isMiddleMousePressed() const;

        bool isGamepadPressed(const GamepadInputEvent::GamepadButton& code) const;

        bool isScreenTouched(s16 pointer = -1) const;
        bool isMultiScreenTouch() const;

        const math::Point2D& getCursorPosition() const;
        f32 getMouseWheel() const;

    private:

        static const u32 k_maxTouchScreenFingers = 8;

        bool onEvent(Event* event) override;
        void resetKeyPressed();

        void applyModifiers(KeyboardInputEvent* event);
        void applyModifiers(MouseInputEvent* event);
        void applyModifiers(TouchInputEvent * event);

        void applyTouches(u32 mask, bool isPressed);

        bool m_keysPressed[toEnumType(KeyCode::Key_Codes_Count)];
        bool m_mouseStates[MouseInputEvent::MouseCount];

        bool m_touchScreenStates[k_maxTouchScreenFingers];
        bool m_touchScreen;
        bool m_multiScreenTouch;
        
        u32 m_gamepadStates;

        math::Point2D m_mousePosition;
        f32 m_mouseWheel;
        const math::Vector2D m_mouseWheelRange = { -100.0f, +100.0f };

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

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace event
} //namespace v3d
