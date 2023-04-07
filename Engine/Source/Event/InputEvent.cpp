#include "InputEvent.h"

namespace v3d
{
namespace event
{

InputEvent::InputEvent() noexcept
    : _timeStamp(0) //TODO: get current time, need class Timer
    , _eventType(InputEventType::UnknownInputEvent)
    , _priority(Normal)
{
}

bool InputEvent::operator<(const InputEvent& event)
{
    return _priority < event._priority;
}

} //namespace event
} //namespace v3d
