#pragma once

#include "Common.h"
#include "InputEvent.h"

namespace v3d
{
namespace event
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief SystemEvent struct.
    */
    struct V3D_API SystemEvent : InputEvent
    {
        enum SystemEventType
        {
            Empty = 0,
            Create,
            Destroy,
            Move,
            Resize,
            Suspend,
            Resume,
            TextInput,
            CursorIcon,
            Focus,

            SystemEventsCount
        };

        SystemEvent() noexcept;
        ~SystemEvent() = default;

        bool operator==(const SystemEvent& event) const;

        SystemEventType _systemEvent;
        u32 _flag;
        u64 _handle;
    };

    inline SystemEvent::SystemEvent() noexcept
        : _systemEvent(SystemEventType::Empty)
        , _flag(0)
        , _handle(0)
    {
        _eventType = InputEventType::SystemEvent;
    }

    inline bool SystemEvent::operator==(const SystemEvent& event) const
    {
        return _systemEvent == event._systemEvent;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace event
} //namespace v3d