#include "InputEventMouse.h"

namespace v3d
{
namespace event
{

MouseInputEvent::MouseInputEvent() noexcept
    : _event(MouseUnknown)
    , _cursorPosition({ 0, 0 })
    , _wheelValue(0.0f)
    , _key(KeyCode::KeyUknown)
    , _modifers(0)
{
    _eventType = InputEventType::MouseInputEvent;
}

bool MouseInputEvent::operator==(const MouseInputEvent& event) const
{
    return (_cursorPosition == event._cursorPosition && _wheelValue == event._wheelValue && 
        _event == event._event && _key == event._key && _modifers == event._modifers);
}

} //namespace event
} //namespace v3d
