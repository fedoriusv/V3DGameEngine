#include "InputEventTouch.h"

namespace v3d
{
namespace event
{

TouchInputEvent::TouchInputEvent() noexcept
    : _event(TouchUnknown)
    , _motionEvent(TouchMotionUnknown)
    , _keyEvent(TouchKeyPressUnknown)
    , _position({ 0, 0 })
    , _key(KeyCode::KeyUknown)
    , _pointers(0U)
    , _modifers(0)
{
    _eventType = InputEventType::TouchInputEvent;
}

bool TouchInputEvent::operator==(const TouchInputEvent& event) const
{
    return (_event == event._event && _motionEvent == event._motionEvent && _keyEvent == event._keyEvent &&
     _position == event._position && _key == event._key && _pointers == event._pointers && _modifers == event._modifers);
}

} //namespace event
} //namespace v3d