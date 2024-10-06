#pragma once

#include "Common.h"
#include "InputEvent.h"
#include "KeyCodes.h"

namespace v3d
{
namespace event
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief KeyboardInputEvent struct.
    */
    struct V3D_API KeyboardInputEvent : InputEvent
    {
        enum KeyboardPressInputEvent
        {
            KeyboardUnknown = 0,
            KeyboardPressDown,
            KeyboardPressUp,

            KeyboardCount
        };

        KeyboardInputEvent() noexcept;
        virtual ~KeyboardInputEvent() = default;

        bool operator==(const KeyboardInputEvent& event) const;

        KeyboardPressInputEvent _event;
        KeyCode                 _key;
        c8                      _character;
        u8                      _modifers;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace event
} //namespace v3d
