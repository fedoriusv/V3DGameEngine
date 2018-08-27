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

        GamepadInputEvent();
        virtual ~GamepadInputEvent();

        bool    operator==(const GamepadInputEvent& event) const;

        GamepadPessInputEvent  _event;
        u32                     _buttons;
        s32                     _pov;
        s32                     _axis[AxesCount];
        s32                     _gamepad;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace event
} //namespace v3d
