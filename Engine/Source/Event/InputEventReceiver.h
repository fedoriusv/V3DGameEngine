#pragma once

#include "InputEvent.h"
#include "InputEventHandler.h"
#include "Common.h"
#include "Utils/NonCopyable.h"

namespace v3d
{
namespace event
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    class InputEventReceiver final : public utils::NonCopyable
    {
    public:

        InputEventReceiver();
        ~InputEventReceiver();

        void reset();

        void attach(InputEvent::InputEventType type, InputEventHandler* handler);
        void dettach(InputEvent::InputEventType type);
        void dettach(InputEvent::InputEventType type, InputEventHandler* handler);

        void pushEvent(InputEvent* event);
        bool sendEvent(InputEvent* event);
        void sendDeferredEvents();

        InputEvent* allocateInputEvent();
        void resetInputEventPool();

    private:

        std::queue<InputEvent*>                                         m_events;
        std::multimap<InputEvent::InputEventType, InputEventHandler*>   m_receivers;

        static const u32   s_eventPoolSize = 32U;
        const u32          k_maxInputEventSize;
        InputEvent*        m_eventPool;
        u32                m_currentEventIndex;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace event
} //namespace v3d

