#pragma once

#include "Common.h"
#include "Event.h"

namespace v3d
{
namespace event
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    class EventHandler
    {
    protected:

        EventHandler() {};

    public:

        virtual         ~EventHandler() {};

        virtual bool    onEvent(Event* event) = 0;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace event
} //namespace v3d
