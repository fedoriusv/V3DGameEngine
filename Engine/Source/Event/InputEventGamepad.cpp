#include "InputEventGamepad.h"

namespace v3d
{
namespace event
{

GamepadInputEvent::GamepadInputEvent()
: _event(GamepadUnknown)
, _buttons(0)
, _pov(0)
, _gamepad(-1)
{
    _eventType = InputEventType::GamepadInputEvent;
}

GamepadInputEvent::~GamepadInputEvent()
{
}

bool GamepadInputEvent::operator==(const GamepadInputEvent& event) const
{
    return (_event == event._event && _buttons == event._buttons && _gamepad == event._gamepad);
}

} //namespace event
} //namespace v3d
