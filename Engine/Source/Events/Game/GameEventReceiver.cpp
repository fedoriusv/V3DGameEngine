#include "GameEventReceiver.h"

namespace v3d
{
namespace event
{

GameEventReceiver::GameEventReceiver(const std::function<void(GameEvent* event)>& deleter) noexcept
    : m_deleter(deleter)
{
}

GameEventReceiver::~GameEventReceiver()
{
}

void GameEventReceiver::attach(GameEventHandler* handler)
{
    auto found = std::find(m_handlers.begin(), m_handlers.end(), handler);
    if (found == m_handlers.end())
    {
        m_handlers.push_back(handler);
    }
}

void GameEventReceiver::dettach(GameEventHandler* handler)
{
    auto found = std::find(m_handlers.begin(), m_handlers.end(), handler);
    if (found != m_handlers.end())
    {
        m_handlers.erase(found);
    }
}

void GameEventReceiver::pushEvent(GameEvent* event)
{
    if (event->_priority == GameEvent::Priority::RealTime)
    {
        sendEvent(event);
    }
    else
    {
        m_events.push(event);
    }
}

void GameEventReceiver::sendEvent(GameEvent* event)
{
    for (auto iter = m_handlers.begin(); iter != m_handlers.end(); ++iter)
    {
        GameEventHandler* handler = *iter;
        handler->onEvent(event);
    }

    m_deleter(event);
}

void GameEventReceiver::sendDeferredEvents()
{
    if (!m_events.empty())
    {
        std::queue<GameEvent*> temp(std::move(m_events));
        ASSERT(m_events.empty(), "must be freed");

        while (!temp.empty())
        {
            GameEvent* event = temp.front();
            sendEvent(event);

            temp.pop();
        }
    }
}


} //namespace event
} //namespace v3d