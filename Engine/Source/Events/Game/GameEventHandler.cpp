#include "GameEventHandler.h"

namespace v3d
{
namespace event
{

GameEventHandler::GameEventHandler() noexcept
{
}

GameEventHandler::~GameEventHandler()
{
}

void GameEventHandler::bind(const GameEventCallback& callback)
{
    if (callback)
    {
        m_callbacks.push_back(callback);
    }
}

bool GameEventHandler::onEvent(Event* ev)
{
    ASSERT(ev, "Event must be not nullptr");
    GameEvent* event = static_cast<GameEvent*>(ev);

    for (auto iter = m_callbacks.cbegin(); iter != m_callbacks.cend(); ++iter)
    {
        ASSERT(*iter, "must be valid");
        std::invoke(*iter, event, event->_eventType, event->_customEventID);
    }

    return true;
}

} //namespace event
} //namespace v3d