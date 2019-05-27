#include "InputEvent.h"

namespace v3d
{
namespace event
{

InputEvent::InputEvent()
    : _timeStamp(0) //TODO: get current time, need class Timer
    , _eventType(InputEventType::UnknownInputEvent)
    , _priority(Normal)
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
