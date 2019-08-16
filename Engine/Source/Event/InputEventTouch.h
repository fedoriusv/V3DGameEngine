#pragma once

#include "Common.h"
#include "InputEvent.h"
#include "KeyCodes.h"

namespace v3d
{
namespace event
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    struct TouchInputEvent : InputEvent
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
            TouchMotionScroll,

            TouchMotionCount
        };

        TouchInputEvent();
        virtual ~TouchInputEvent();

        bool operator==(const TouchInputEvent& event) const;

        TouchTypeEvent          _event;
        TouchMotionEvent        _motionEvent;
        TouchKeyPressEvent      _keyEvent;
        core::Point2D           _position;
        KeyCode                 _key;
        u8                      _modifers;

    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace event
} //namespace v3d
