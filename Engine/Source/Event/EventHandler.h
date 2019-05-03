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

        EventHandler() = default;
        EventHandler(const EventHandler&) = delete;
        EventHandler& operator=(const EventHandler&) = delete;

    public:

        virtual         ~EventHandler() {};

        virtual bool    onEvent(Event* event) = 0;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace event
} //namespace v3d
