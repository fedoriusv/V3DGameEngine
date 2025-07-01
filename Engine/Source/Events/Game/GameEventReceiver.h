#pragma once

#include "Common.h"
#include "GameEvent.h"
#include "GameEventHandler.h"

namespace v3d
{
namespace event
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief GameEventReceiver class.
    */
    class V3D_API GameEventReceiver final
    {
    public:

        GameEventReceiver() noexcept;
        ~GameEventReceiver();

        void attach(GameEventHandler* handler);
        void dettach(GameEventHandler* handler);

        void pushEvent(GameEvent* event);

        void sendEvent(GameEvent* event);
        void sendDeferredEvents();

    private:

        GameEventReceiver(const GameEventReceiver&) = delete;
        GameEventReceiver& operator=(const GameEventReceiver&) = delete;

        std::queue<GameEvent*> m_events;
        std::vector<GameEventHandler*> m_handlers;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace event
} //namespace v3d