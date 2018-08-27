#pragma once

#include "Event.h"
#include "Common.h"

namespace v3d
{
namespace event
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    struct InputEvent : Event
    {
        enum class InputEventType
        {
            UnknownInputEvent,
            MouseInputEvent,
            KeyboardInputEvent,
            GamepadInputEvent,
            JoystickInputEvent,
            TouchInputEvent
        };

        enum Priority
        {
            Low = 10,
            Normal = 20,
            High = 30
        };

        InputEvent();
        virtual         ~InputEvent();

        bool            operator<(const InputEvent& event);

        InputEventType _eventType;
        Priority       _priority;
        u64            _timeStamp;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace event
} //namespace v3d
