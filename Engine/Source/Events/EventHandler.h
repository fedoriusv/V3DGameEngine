#pragma once

#include "Common.h"
#include "Event.h"

namespace v3d
{
namespace event
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief EventHandler base class.
    */
    class V3D_API EventHandler
    {
    public:

        virtual ~EventHandler() = default;

        virtual bool onEvent(Event* event) = 0;

    protected:

        EventHandler() noexcept = default;

        EventHandler(const EventHandler&) = delete;
        EventHandler& operator=(const EventHandler&) = delete;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace event
} //namespace v3d
