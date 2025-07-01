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

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief GameEventHandler class.
    */
    class V3D_API GameEventHandler : public EventHandler
    {

    public:

        GameEventHandler() noexcept;
        ~GameEventHandler();

        void bind(std::function<void(const GameEvent*)> callback);

    private:

        GameEventHandler(const GameEventHandler&) = delete;
        GameEventHandler& operator=(const GameEventHandler&) = delete;

        bool onEvent(Event* event) override;

        friend GameEventReceiver;

        std::vector<std::function<void(const GameEvent*)>> m_callbacks;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace event
} //namespace v3d