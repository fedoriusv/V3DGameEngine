#pragma once

#include "Common.h"
#include "InputEvent.h"
#include "KeyCodes.h"

namespace v3d
{
namespace event
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    struct MouseInputEvent : InputEvent
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

        MouseInputEvent();
        virtual ~MouseInputEvent();

        bool operator==(const MouseInputEvent& event) const;

        core::Point2D           _cursorPosition;
        f32                     _wheelValue;
        MousePressInputEvent    _event;
        KeyCode                 _key;
        u16                     _modifers;

    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace event
} //namespace v3d
