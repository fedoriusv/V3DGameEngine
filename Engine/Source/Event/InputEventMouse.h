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

        MouseInputEvent() noexcept;
        virtual ~MouseInputEvent() = default;;

        bool operator==(const MouseInputEvent& event) const;

        MousePressInputEvent    _event;
        math::Point2D           _cursorPosition;
        f32                     _wheelValue;
        KeyCode                 _key;
        u8                      _modifers;

    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace event
} //namespace v3d
