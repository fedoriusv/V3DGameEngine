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
    : k_maxInputEventSize(math::alignUp<u32>(*std::max_element(g_sizeElements.begin(), g_sizeElements.end()), math::k_defaultAlignment))
    , m_eventPool(V3D_MALLOC(k_maxInputEventSize * s_eventPoolSize, memory::MemoryLabel::MemorySystem))
    , m_currentEventIndex(0)
{
    resetInputEventPool();
    LOG_DEBUG("InputEventReceiver::InputEventReceiver constructor %llx, event pool %llx", this, m_eventPool);
 }

InputEventReceiver::~InputEventReceiver()
{
    LOG_DEBUG("InputEventReceiver::InputEventReceiver destructor %llx event pool %llx", this, m_eventPool);
    InputEventReceiver::reset();
    m_handlers.clear();

    if (m_eventPool)
    {
        V3D_FREE(m_eventPool, memory::MemoryLabel::MemorySystem);
    }
}

void InputEventReceiver::sendDeferredEvents()
{
    if (!m_events.empty())
    {
        std::queue<InputEvent*> temp(std::move(m_events));
        ASSERT(m_events.empty(), "must be freed");

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
    }

    resetInputEventPool();
}

void InputEventReceiver::resetInputEventPool()
{
    m_currentEventIndex = 0;
#ifdef DEBUG
    memset(m_eventPool, 0, k_maxInputEventSize * s_eventPoolSize);
#endif
}

void InputEventReceiver::resetInputHandlers()
{
    for (auto iter = m_handlers.begin(); iter != m_handlers.end(); ++iter)
    {
        InputEventHandler* ptr = std::get<0>((*iter).second);
        ptr->resetEventHandler();
    }
}

void* InputEventReceiver::allocateInputEvent()
{
    ASSERT(m_currentEventIndex < s_eventPoolSize, "out from size");
    void* ptr = reinterpret_cast<u8*>(m_eventPool) + m_currentEventIndex * k_maxInputEventSize;
    ++m_currentEventIndex;
#ifdef DEBUG
    for (u32 i = 0; i < k_maxInputEventSize; ++i)
    {
        u8* ch = (u8*)ptr;
        ASSERT(ch[i] == 0, "must be 0");
    }
#endif
    return ptr;
}

void InputEventReceiver::pushEvent(InputEvent* event)
{
    if (event->_priority == InputEvent::Priority::RealTime)
    {
        sendEvent(event);
    }
    else
    {
        m_events.push(event);
    }
}

bool InputEventReceiver::sendEvent(InputEvent* event)
{
    bool result = false;

    for (auto iter = m_handlers.begin(); iter != m_handlers.end(); ++iter)
    {
        if ((*iter).first == event->_eventType)
        {
            const platform::Window* window = std::get<1>((*iter).second);
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
    std::pair<InputEvent::InputEventType, std::tuple<InputEventHandler*, const platform::Window*>> rcv = std::make_pair(type, std::make_tuple(handler, nullptr));
    auto predFind = [rcv](const std::pair<InputEvent::InputEventType, std::tuple<InputEventHandler*, const platform::Window*>>& item) -> bool
    {
        return (rcv.first == item.first && std::get<0>(rcv.second) == std::get<0>(item.second) && std::get<1>(rcv.second) == std::get<1>(item.second));
    };

    auto iter = std::find_if(m_handlers.begin(), m_handlers.end(), predFind);
    if (iter == m_handlers.end())
    {
        m_handlers.insert(rcv);
    }
}

void InputEventReceiver::attach(InputEvent::InputEventType type, InputEventHandler* handler, const platform::Window* window)
{
    std::pair<InputEvent::InputEventType, std::tuple<InputEventHandler*, const platform::Window*>> rcv = std::make_pair(type, std::make_tuple(handler, window));
    auto predFind = [rcv](const std::pair<InputEvent::InputEventType, std::tuple<InputEventHandler*, const platform::Window*>>& item) -> bool
        {
            return (rcv.first == item.first && std::get<0>(rcv.second) == std::get<0>(item.second) && std::get<1>(rcv.second) == std::get<1>(item.second));
        };

    auto iter = std::find_if(m_handlers.begin(), m_handlers.end(), predFind);
    if (iter == m_handlers.end())
    {
        m_handlers.insert(rcv);
    }
}

void InputEventReceiver::dettach(InputEvent::InputEventType type)
{
    auto predFind = [type](const std::pair<InputEvent::InputEventType, std::tuple<InputEventHandler*, const platform::Window*>>& item) -> bool
    {
        return (type == item.first);
    };

    auto iter = std::find_if(m_handlers.begin(), m_handlers.end(), predFind);
    if (iter != m_handlers.end())
    {
        m_handlers.erase(iter);
    }
}

void InputEventReceiver::dettach(InputEvent::InputEventType type, InputEventHandler* handler)
{
    std::pair<InputEvent::InputEventType, std::tuple<InputEventHandler*, const platform::Window*>> rcv = std::make_pair(type, std::make_tuple(handler, nullptr));
    auto predFind = [rcv](const std::pair<InputEvent::InputEventType, std::tuple<InputEventHandler*, const platform::Window*>>& item) -> bool
    {
        return (rcv.first == item.first && std::get<0>(rcv.second) == std::get<0>(item.second) && std::get<1>(rcv.second) == std::get<1>(item.second));
    };

    auto iter = std::find_if(m_handlers.begin(), m_handlers.end(), predFind);
    if (iter != m_handlers.end())
    {
        m_handlers.erase(iter);
    }
}

void InputEventReceiver::dettach(InputEvent::InputEventType type, InputEventHandler* handler, const platform::Window* window)
{
    std::pair<InputEvent::InputEventType, std::tuple<InputEventHandler*, const platform::Window*>> rcv = std::make_pair(type, std::make_tuple(handler, window));
    auto predFind = [rcv](const std::pair<InputEvent::InputEventType, std::tuple<InputEventHandler*, const platform::Window*>>& item) -> bool
        {
            return (rcv.first == item.first && std::get<0>(rcv.second) == std::get<0>(item.second) && std::get<1>(rcv.second) == std::get<1>(item.second));
        };

    auto iter = std::find_if(m_handlers.begin(), m_handlers.end(), predFind);
    if (iter != m_handlers.end())
    {
        m_handlers.erase(iter);
    }
}

} //namespace event
} //namespace v3d
