#pragma once

#include "Common.h"
#include "Events/EventHandler.h"
#include "GameEvent.h"

namespace v3d
{
namespace event
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    class GameEventReceiver;

    using GameEventCallback = std::function<void(const GameEvent*, GameEvent::GameEventType, u64)>;

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief GameEventHandler class.
    */
    class V3D_API GameEventHandler : public EventHandler
    {

    public:

        GameEventHandler() noexcept;
        ~GameEventHandler();

        void bind(const GameEventCallback& callback);

    private:

        GameEventHandler(const GameEventHandler&) = delete;
        GameEventHandler& operator=(const GameEventHandler&) = delete;

        bool onEvent(Event* event) override;

        friend GameEventReceiver;

        std::vector<GameEventCallback> m_callbacks;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace event
} //namespace v3d