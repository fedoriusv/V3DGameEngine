#include "InputEvent.h"
#include "Utils/Timer.h"

namespace v3d
{
namespace event
{

InputEvent::InputEvent() noexcept
    : _timeStamp(utils::Timer::getCurrentTime())
    , _eventType(InputEventType::UnknownInputEvent)
    , _priority(Normal)
    , _windowID(0)
{
    static_assert(sizeof(InputEvent) == 24, "wrong size");
}

bool InputEvent::operator<(const InputEvent& event)
{
    return _priority < event._priority;
}

} //namespace event
} //namespace v3d
