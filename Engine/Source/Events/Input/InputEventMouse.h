#pragma once

#include "Common.h"
#include "InputEvent.h"
#include "KeyCodes.h"

namespace v3d
{
namespace event
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief MouseInputEvent struct.
    */
    struct V3D_API MouseInputEvent : InputEvent
    {
        enum MousePressInputEvent
        {
            MouseUnknown = 0,
            MousePressDown,
            MousePressUp,
            MouseDoubleClick,
            MouseMoved,
            MouseWheel,

            MouseCount
        };

        MouseInputEvent() noexcept;
        ~MouseInputEvent() = default;

        bool operator==(const MouseInputEvent& event) const;

        MousePressInputEvent    _event;
        math::Point2D           _clientCoordinates;
        math::Point2D           _absoluteCoordinates;
        f32                     _wheelValue;
        KeyCode                 _key;
        u8                      _modifers;

    };

    inline MouseInputEvent::MouseInputEvent() noexcept
        : _event(MouseUnknown)
        , _clientCoordinates({ 0, 0 })
        , _absoluteCoordinates({ 0, 0 })
        , _wheelValue(0.0f)
        , _key(KeyCode::KeyUknown)
        , _modifers(0)
    {
        _eventType = InputEventType::MouseInputEvent;
    }

    inline bool MouseInputEvent::operator==(const MouseInputEvent& event) const
    {
        return (
            _clientCoordinates == event._clientCoordinates &&
            _absoluteCoordinates == event._absoluteCoordinates &&
            _wheelValue == event._wheelValue &&
            _event == event._event &&
            _key == event._key &&
            _modifers == event._modifers
            );
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace event
} //namespace v3d
