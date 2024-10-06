#include "InputEventReceiver.h"
#include "Utils/Logger.h"
#include "Platform/Platform.h"
#include "Platform/Window.h"

namespace v3d
{
namespace event
{
    std::array<u32, toEnumType(InputEvent::InputEventType::InputEventsCount)> g_sizeElements =
    {
        sizeof(u64),
        sizeof(MouseInputEvent),
        sizeof(KeyboardInputEvent),
        sizeof(GamepadInputEvent),
        sizeof(TouchInputEvent),
        sizeof(SystemEvent)
    };

InputEventReceiver::InputEventReceiver() noexcept
    : k_maxInputEventSize(*std::max_element(g_sizeElements.begin(), g_sizeElements.end()))
    , m_eventPool(nullptr)
    , m_currentEventIndex(0)
{
    m_eventPool = reinterpret_cast<InputEvent*>(V3D_MALLOC(k_maxInputEventSize * s_eventPoolSize, memory::MemoryLabel::MemorySystem));
    resetInputEventPool();
    LOG_DEBUG("InputEventReceiver::InputEventReceiver constructor %llx, event pool %llx", this, m_eventPool);
 }

InputEventReceiver::~InputEventReceiver()
{
    LOG_DEBUG("InputEventReceiver::InputEventReceiver destructor %llx event pool %llx", this, m_eventPool);
    InputEventReceiver::reset();
    m_receivers.clear();

    if (m_eventPool)
    {
        V3D_FREE(m_eventPool, memory::MemoryLabel::MemorySystem);
    }
}

void InputEventReceiver::sendDeferredEvents()
{
    if (m_events.empty())
    {
        return;
    }

    std::queue<InputEvent*> temp(std::move(m_events));
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
        //no need to delete, ringbuffer pool is used
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
            platform::Window* window = std::get<1>((*iter).second);
            if (!window || event->_windowID == window->ID()) [[likely]]
            {
                InputEventHandler* ptr = std::get<0>((*iter).second);
                result = ptr->onEvent(event);
            }
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
    std::pair<InputEvent::InputEventType, std::tuple<InputEventHandler*, platform::Window*>> rcv = std::make_pair(type, std::make_tuple(handler, nullptr));
    auto predFind = [rcv](const std::pair<InputEvent::InputEventType, std::tuple<InputEventHandler*, platform::Window*>>& item) -> bool
    {
        return (rcv.first == item.first && std::get<0>(rcv.second) == std::get<0>(item.second) && std::get<1>(rcv.second) == std::get<1>(item.second));
    };

    auto iter = std::find_if(m_receivers.begin(), m_receivers.end(), predFind);
    if (iter == m_receivers.end())
    {
        m_receivers.insert(rcv);
    }
}

void InputEventReceiver::attach(InputEvent::InputEventType type, InputEventHandler* handler, platform::Window* window)
{
    std::pair<InputEvent::InputEventType, std::tuple<InputEventHandler*, platform::Window*>> rcv = std::make_pair(type, std::make_tuple(handler, window));
    auto predFind = [rcv](const std::pair<InputEvent::InputEventType, std::tuple<InputEventHandler*, platform::Window*>>& item) -> bool
        {
            return (rcv.first == item.first && std::get<0>(rcv.second) == std::get<0>(item.second) && std::get<1>(rcv.second) == std::get<1>(item.second));
        };

    auto iter = std::find_if(m_receivers.begin(), m_receivers.end(), predFind);
    if (iter == m_receivers.end())
    {
        m_receivers.insert(rcv);
    }
}

void InputEventReceiver::dettach(InputEvent::InputEventType type)
{
    auto predFind = [type](const std::pair<InputEvent::InputEventType, std::tuple<InputEventHandler*, platform::Window*>>& item) -> bool
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
    std::pair<InputEvent::InputEventType, std::tuple<InputEventHandler*, platform::Window*>> rcv = std::make_pair(type, std::make_tuple(handler, nullptr));
    auto predFind = [rcv](const std::pair<InputEvent::InputEventType, std::tuple<InputEventHandler*, platform::Window*>>& item) -> bool
    {
        return (rcv.first == item.first && std::get<0>(rcv.second) == std::get<0>(item.second) && std::get<1>(rcv.second) == std::get<1>(item.second));
    };

    auto iter = std::find_if(m_receivers.begin(), m_receivers.end(), predFind);
    if (iter != m_receivers.end())
    {
        m_receivers.erase(iter);
    }
}

void InputEventReceiver::dettach(InputEvent::InputEventType type, InputEventHandler* handler, platform::Window* window)
{
    std::pair<InputEvent::InputEventType, std::tuple<InputEventHandler*, platform::Window*>> rcv = std::make_pair(type, std::make_tuple(handler, window));
    auto predFind = [rcv](const std::pair<InputEvent::InputEventType, std::tuple<InputEventHandler*, platform::Window*>>& item) -> bool
        {
            return (rcv.first == item.first && std::get<0>(rcv.second) == std::get<0>(item.second) && std::get<1>(rcv.second) == std::get<1>(item.second));
        };

    auto iter = std::find_if(m_receivers.begin(), m_receivers.end(), predFind);
    if (iter != m_receivers.end())
    {
        m_receivers.erase(iter);
    }
}

} //namespace event
} //namespace v3d
