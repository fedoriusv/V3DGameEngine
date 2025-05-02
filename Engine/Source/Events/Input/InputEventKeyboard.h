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
        ~KeyboardInputEvent() = default;

        bool operator==(const KeyboardInputEvent& event) const;

        KeyboardPressInputEvent _event;
        KeyCode                 _key;
        c8                      _character;
        u8                      _modifers;
    };

    inline KeyboardInputEvent::KeyboardInputEvent() noexcept
        : _event(KeyboardUnknown)
        , _key(KeyCode::KeyUknown)
        , _character(0)
        , _modifers(0)
    {
        _eventType = InputEventType::KeyboardInputEvent;
    }

    inline bool KeyboardInputEvent::operator==(const KeyboardInputEvent& event) const
    {
        return (_character == event._character && _key == event._key && _event == event._event && _modifers == event._modifers);
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace event
} //namespace v3d
