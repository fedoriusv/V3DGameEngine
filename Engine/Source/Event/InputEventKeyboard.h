#pragma once

#include "Common.h"
#include "InputEvent.h"
#include "KeyCodes.h"

namespace v3d
{
namespace event
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    struct KeyboardInputEvent : InputEvent
    {
        enum KeyboardPressInputEvent
        {
            KeyboardUnknown = 0,
            KeyboardPressDown,
            KeyboardPressUp,

            KeyboardCount
        };

        KeyboardInputEvent();
        virtual ~KeyboardInputEvent();

        bool operator==(const KeyboardInputEvent& event) const;

        c8                      _character;
        KeyCode                 _key;
        KeyboardPressInputEvent _event;
        u16                     _modifers;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace event
} //namespace v3d
