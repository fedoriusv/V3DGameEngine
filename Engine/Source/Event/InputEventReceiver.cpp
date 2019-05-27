#include "InputEventReceiver.h"
#include "Utils/Logger.h"

namespace v3d
{
namespace event
{

InputEventReceiver::InputEventReceiver()
    : k_maxInputEventSize((u32)core::max(sizeof(MouseInputEvent), sizeof(GamepadInputEvent), sizeof(KeyboardInputEvent)))
    , m_eventPool(nullptr)
    , m_currentEventIndex(0)
{
    m_eventPool = reinterpret_cast<InputEvent*>(malloc(k_maxInputEventSize * s_eventPoolSize));
    resetInputEventPool();
 }

InputEventReceiver::~InputEventReceiver()
{
    InputEventReceiver::reset();
    m_receivers.clear();

    if (m_eventPool)
    {
        free(m_eventPool);
    }
}

void InputEventReceiver::sendDeferredEvents()
{
    if (m_events.empty())
    {
        return;
    }

    std::queue<InputEvent*> temp(m_events);
    InputEventReceiver::reset();

    //LOG_DEBUG("InputEventReceiver queue size %u", temp.size());
    while (!temp.empty())
    {
        InputEvent* event = temp.front();
        if (!sendEvent(event))
        {
            //LOG_DEBUG("InputEventReceiver Event not handle");
        }

        temp.pop();
        //delete event; 
        //no need, deleted through pool
    }

    resetInputEventPool();
}

void InputEventReceiver::resetInputEventPool()
{
    m_currentEventIndex = 0;
#ifdef DEBUG
    memset((void*)m_eventPool, 0, k_maxInputEventSize * s_eventPoolSize);
#endif
}

InputEvent* InputEventReceiver::allocateInputEvent()
{
    ASSERT(m_currentEventIndex < s_eventPoolSize, "out from size");
    return m_eventPool + (m_currentEventIndex++ * k_maxInputEventSize);
}

void InputEventReceiver::pushEvent(InputEvent* event)
{
    m_events.push(event);
}

bool InputEventReceiver::sendEvent(InputEvent* event)
{
    bool result = false;

    for (auto iter = m_receivers.begin(); iter != m_receivers.end(); ++iter)
    {
        if ((*iter).first == event->_eventType)
        {
                InputEventHandler* ptr = (*iter).second;
                result = ptr->onEvent(event);
        }
    }

    return result;
}

void InputEventReceiver::reset()
{
    std::queue<InputEvent*> empty;
    empty.swap(m_events);
}

void InputEventReceiver::attach(InputEvent::InputEventType type, InputEventHandler* handler)
{
    std::pair<InputEvent::InputEventType, InputEventHandler*> rcv = std::make_pair(type, handler);
    auto predFind = [rcv](const std::pair<InputEvent::InputEventType, InputEventHandler*>& item) -> bool
    {
        return (rcv.first == item.first && rcv.second == item.second);
    };

    auto iter = std::find_if(m_receivers.begin(), m_receivers.end(), predFind);
    if (iter == m_receivers.end())
    {
        m_receivers.insert(rcv);
    }

}

void InputEventReceiver::dettach(InputEvent::InputEventType type)
{
    auto predFind = [type](const std::pair<InputEvent::InputEventType, InputEventHandler*>& item) -> bool
    {
        return (type == item.first);
    };

    auto iter = std::find_if(m_receivers.begin(), m_receivers.end(), predFind);
    if (iter != m_receivers.end())
    {
        m_receivers.erase(iter);
    }
}

void InputEventReceiver::dettach(InputEvent::InputEventType type, InputEventHandler* handler)
{
    std::pair<InputEvent::InputEventType, InputEventHandler*> rcv = std::make_pair(type, handler);
    auto predFind = [rcv](const std::pair<InputEvent::InputEventType, InputEventHandler*>& item) -> bool
    {
        return (rcv.first == item.first && rcv.second == item.second);
    };

    auto iter = std::find_if(m_receivers.begin(), m_receivers.end(), predFind);
    if (iter != m_receivers.end())
    {
        m_receivers.erase(iter);
    }
}

} //namespace event
} //namespace v3d
