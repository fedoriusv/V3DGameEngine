#pragma once

#include "Common.h"
#include "InputEvent.h"

namespace v3d
{
namespace event
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    struct GamepadInputEvent : InputEvent
    {
        enum GamepadPessInputEvent
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

        bool    operator==(const GamepadInputEvent& event) const;

        GamepadPessInputEvent   _event;
        u16                     _buttons;
        s16                     _pov;
        s16                     _axis[AxesCount];
        s16                     _gamepad;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace event
} //namespace v3d
