#pragma once

#include "Event.h"
#include "Common.h"

namespace v3d
{
namespace event
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////


    /**
    * @brief InputEvent struct.
    */
    struct InputEvent : Event
    {
        enum class InputEventType : u64
        {
            UnknownInputEvent,
            MouseInputEvent,
            KeyboardInputEvent,
            GamepadInputEvent,
            TouchInputEvent,
            SystemEvent,

            InputEventsCount
        };

        enum Priority : u64
        {
            Low = 10,
            Normal = 20,
            High = 30
        };

        InputEvent() noexcept;
        virtual ~InputEvent() = default;

        bool operator<(const InputEvent& event);

        u64                 _timeStamp;
        InputEventType      _eventType : 16;
        Priority            _priority : 16;
        u64                 _windowID : 32;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace event
} //namespace v3d
