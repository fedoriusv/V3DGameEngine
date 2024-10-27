#pragma once

#include "Common.h"
#include "InputEvent.h"

namespace v3d
{
namespace event
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief GamepadInputEvent struct.
    */
    struct V3D_API GamepadInputEvent : InputEvent
    {
        enum GamepadPessInputEvent : u32
        {
            GamepadUnknown = 0,
            GamepadButtonInput,
            GamepadDpadDirection,
            GamepadPOV,

            GamepadCount
        };

        enum GamepadAxis
        {
            Axis_X = 0,
            Axis_Y,
            Axis_Z,
            Axis_R,
            Axis_U,
            Axis_V,

            AxesCount
        };

        enum GamepadButton
        {
            ButtonNone = -1,

            ButtonA,
            ButtonB,

            ButtonCount = 32
        };

        GamepadInputEvent() noexcept;
        virtual ~GamepadInputEvent() = default;

        bool operator==(const GamepadInputEvent& event) const;

        GamepadPessInputEvent   _event;
        u16                     _buttons;
        s16                     _pov;
        s16                     _axis[AxesCount] = {};
        s16                     _gamepad;
    };

    inline GamepadInputEvent::GamepadInputEvent() noexcept
        : _event(GamepadUnknown)
        , _buttons(0)
        , _pov(0)
        , _gamepad(-1)
    {
        _eventType = InputEventType::GamepadInputEvent;
    }

    inline bool GamepadInputEvent::operator==(const GamepadInputEvent& event) const
    {
        return (_event == event._event && _buttons == event._buttons && _gamepad == event._gamepad);
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace event
} //namespace v3d
