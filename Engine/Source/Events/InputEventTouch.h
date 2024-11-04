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
    * @brief TouchInputEvent struct.
    */
    struct V3D_API TouchInputEvent : InputEvent
    {
        enum TouchTypeEvent
        {
            TouchUnknown = 0,
            TouchMotion,
            TouchKey,

            TouchCount
        };

        enum TouchKeyPressEvent
        {
            TouchKeyPressUnknown = 0,
            TouchKeyPressDown,
            TouchKeyPressUp,
            TouchKeyPressMultipress,

            TouchKeyCount
        };

        enum TouchMotionEvent
        {
            TouchMotionUnknown = 0,
            TouchMotionDown,
            TouchMotionUp,
            TouchMotionMove,
            TouchMotionMultiTouchDown,
            TouchMotionMultiTouchUp,
            TouchMotionScroll,

            TouchMotionCount
        };

        TouchInputEvent() noexcept;
        ~TouchInputEvent() = default;

        bool operator==(const TouchInputEvent& event) const;

        TouchTypeEvent          _event;
        TouchMotionEvent        _motionEvent;
        TouchKeyPressEvent      _keyEvent;
        math::Point2D           _position;
        KeyCode                 _key;
        u16                     _pointers;
        u8                      _modifers;

    };

    inline TouchInputEvent::TouchInputEvent() noexcept
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

    inline bool TouchInputEvent::operator==(const TouchInputEvent& event) const
    {
        return (_event == event._event && _motionEvent == event._motionEvent && _keyEvent == event._keyEvent &&
            _position == event._position && _key == event._key && _pointers == event._pointers && _modifers == event._modifers);
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace event
} //namespace v3d
