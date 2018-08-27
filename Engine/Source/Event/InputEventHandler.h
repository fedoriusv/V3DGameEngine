#pragma once

#include "Common.h"
#include "EventHandler.h"
#include "KeyCodes.h"

#include "InputEventKeyboard.h"
#include "InputEventMouse.h"
#include "InputEventGamepad.h"


namespace v3d
{
namespace event
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    class InputEventReceiver;

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    class InputEventHandler final : public EventHandler
    {

    public:

        InputEventHandler();
        ~InputEventHandler();

        void connect(std::function<void(const KeyboardInputEvent*)> callback);
        void connect(std::function<void(const MouseInputEvent*)> callback);
        void connect(std::function<void(const GamepadInputEvent*)> callback);

        bool isKeyPressed(const KeyCode& code)const;

        bool isLeftMousePressed() const;
        bool isRightMousePressed() const;
        bool isMiddleMousePressed() const;

        bool isGamepadPressed(const GamepadInputEvent::GamepadButton& code) const;

        const core::Point2D& getCursorPosition() const;
        f32 getMouseWheel() const;

    private:

        bool onEvent(Event* event) override;
        void resetKeyPressed();

        void applyModifiers(KeyboardInputEvent* event);
        void applyModifiers(MouseInputEvent* event);

        bool                            m_keysPressed[Key_Codes_Count];
        bool                            m_mouseStates[MouseInputEvent::MouseCount];
        u32                             m_gamepadStates;

        core::Point2D                   m_mousePosition;
        f32                             m_mouseWheel;
        const core::Vector2D            m_mouseWheelRange = { -100.0f, +100.0f };

        friend InputEventReceiver;

        typedef std::function<void(const KeyboardInputEvent*)>   KeyboardCallback;
        typedef std::function<void(const MouseInputEvent*)>      MouseCallback;
        typedef std::function<void(const GamepadInputEvent*)>    GamepadCallback;

        std::vector<KeyboardCallback>   m_keyboardHandlerCallbacks;
        std::vector<MouseCallback>      m_mouseHandlerCallbacks;
        std::vector<GamepadCallback>    m_gamepadHandlerCallbacks;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace event
} //namespace v3d
