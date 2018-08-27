#include "InputEvent.h"

namespace v3d
{
namespace event
{

InputEvent::InputEvent()
    : _eventType(InputEventType::UnknownInputEvent)
    , _priority(Normal)
    , _timeStamp(0) //TODO: get current time, need class Timer
{
}

InputEvent::~InputEvent()
{
}

bool InputEvent::operator<(const InputEvent& event)
{
    return _priority < event._priority;
}

} //namespace event
} //namespace v3d
