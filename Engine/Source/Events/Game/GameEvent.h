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
        enum class GameEventType : u32
        {
            UnknownGameEvent,
            SelectObject,
            MultiSelectObjects,
            TransformObject,
            CustomEvent,

            GameEventsCount
        };

        enum Priority : u32
        {
            Low = 10,
            Normal = 20,
            High = 30,
            RealTime = 100,
        };

        GameEvent(GameEventType type) noexcept;
        virtual ~GameEvent() = default;

        bool operator<(const GameEvent& event);

        u64                 _timeStamp;
        GameEventType       _eventType;
        Priority            _priority;
        u64                 _customEventID;
    };

    inline GameEvent::GameEvent(GameEventType type) noexcept
        : _timeStamp(utils::Timer::getCurrentTime())
        , _eventType(type)
        , _priority(Normal)
        , _customEventID(0)
    {
        static_assert(sizeof(GameEvent) == 32, "wrong size");
    }

    inline bool GameEvent::operator<(const GameEvent& event)
    {
        return _priority < event._priority;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace event
} //namespace v3d
