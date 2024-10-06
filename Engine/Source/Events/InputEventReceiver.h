#pragma once

#include "InputEvent.h"
#include "InputEventHandler.h"
#include "Common.h"

namespace v3d
{
namespace platform
{
    class Window;
} //namespace platform
namespace event
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    class V3D_API InputEventReceiver final
    {
    public:

        InputEventReceiver() noexcept;
        ~InputEventReceiver();

        void reset();

        void attach(InputEvent::InputEventType type, InputEventHandler* handler);
        void attach(InputEvent::InputEventType type, InputEventHandler* handler, platform::Window* window);
        void dettach(InputEvent::InputEventType type);
        void dettach(InputEvent::InputEventType type, InputEventHandler* handler);
        void dettach(InputEvent::InputEventType type, InputEventHandler* handler, platform::Window* window);

        void pushEvent(InputEvent* event);
        bool sendEvent(InputEvent* event);
        void sendDeferredEvents();

        InputEvent* allocateInputEvent();
        void resetInputEventPool();

    private:

        InputEventReceiver(const InputEventReceiver&) = delete;
        InputEventReceiver& operator=(const InputEventReceiver&) = delete;

        std::queue<InputEvent*> m_events;
        std::multimap<InputEvent::InputEventType, std::tuple<InputEventHandler*, platform::Window*>> m_receivers;

        static const u32   s_eventPoolSize = 64U;
        const u32          k_maxInputEventSize;
        InputEvent*        m_eventPool;
        u32                m_currentEventIndex;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace event
} //namespace v3d


