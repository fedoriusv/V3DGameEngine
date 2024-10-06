#include "SystemEvent.h"

namespace v3d
{
namespace event
{

SystemEvent::SystemEvent() noexcept
    : _systemEvent(SystemEventType::Empty)
{
    _eventType = InputEventType::SystemEvent;
}

bool SystemEvent::operator==(const SystemEvent& event) const
{
    return _systemEvent == event._systemEvent;
}

} //namespace event
} //namespace v3d
