#include "InputEventKeyboard.h"

namespace v3d
{
namespace event
{

KeyboardInputEvent::KeyboardInputEvent() noexcept
    : _event(KeyboardUnknown)
    , _key(KeyCode::KeyUknown)
    , _character(0)
    , _modifers(0)
{
    _eventType = InputEventType::KeyboardInputEvent;
}

bool KeyboardInputEvent::operator==(const KeyboardInputEvent& event) const
{
    return (_character == event._character && _key == event._key && _event == event._event && _modifers == event._modifers);
}

} //namespace event
} //namespace v3d
