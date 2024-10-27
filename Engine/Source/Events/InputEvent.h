#pragma once

#include "Event.h"
#include "Common.h"
#include "Utils/Timer.h"

namespace v3d
{
namespace event
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief InputEvent struct.
    */
    struct V3D_API InputEvent : Event
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
        Priority            _priority  : 16;
        u64                 _windowID  : 32;
    };

    inline InputEvent::InputEvent() noexcept
        : _timeStamp(utils::Timer::getCurrentTime())
        , _eventType(InputEventType::UnknownInputEvent)
        , _priority(Normal)
        , _windowID(0)
    {
        static_assert(sizeof(InputEvent) == 24, "wrong size");
    }

    inline bool InputEvent::operator<(const InputEvent& event)
    {
        return _priority < event._priority;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace event
} //namespace v3d
