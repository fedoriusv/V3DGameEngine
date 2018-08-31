#pragma once

#include "Common.h"
#include "Event.h"
#include "Utils/NonCopyable.h"

namespace v3d
{
namespace event
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    class EventHandler : public utils::NonCopyable
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
