#include "InputEventTouch.h"

namespace v3d
{
namespace event
{

TouchInputEvent::TouchInputEvent()
    : _event(TouchUnknown)
    , _motionEvent(TouchMotionUnknown)
    , _keyEvent(TouchKeyPressUnknown)
    , _position({ 0, 0 })
    , _key(KeyUknown)
    , _modifers(0)
{
    _eventType = InputEventType::TouchInputEvent;
}

TouchInputEvent::~TouchInputEvent()
{
}

bool TouchInputEvent::operator==(const TouchInputEvent& event) const
{
    return (_event == event._event && _motionEvent == event._motionEvent && _keyEvent == event._keyEvent &&
     _position == event._position && _key == event._key && _modifers == event._modifers);
}

} //namespace event
} //namespace v3d
