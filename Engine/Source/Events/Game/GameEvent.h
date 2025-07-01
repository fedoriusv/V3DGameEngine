#pragma once

#include "Events/Event.h"
#include "Common.h"
#include "Utils/Timer.h"

namespace v3d
{
namespace event
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief GameEvent struct.
    */
    struct V3D_API GameEvent : Event
    {
        enum class GameEventType : u64
        {
            UnknownGameEvent,
            SelectObject,

            GameEventsCount
        };

        enum Priority : u64
        {
            Low = 10,
            Normal = 20,
            High = 30,
            RealTime = 100,
        };

        GameEvent() noexcept;
        virtual ~GameEvent() = default;

        bool operator<(const GameEvent& event);

        u64                 _timeStamp;
        GameEventType       _eventType : 32;
        Priority            _priority  : 32;
    };

    inline GameEvent::GameEvent() noexcept
        : _timeStamp(utils::Timer::getCurrentTime())
        , _eventType(GameEventType::UnknownGameEvent)
        , _priority(Normal)
    {
        static_assert(sizeof(GameEvent) == 24, "wrong size");
    }

    inline bool GameEvent::operator<(const GameEvent& event)
    {
        return _priority < event._priority;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace event
} //namespace v3d
