#pragma once

#include "InputEvent.h"
#include "InputEventHandler.h"
#include "Common.h"

namespace v3d
{
namespace platform
{
    class Window;
    class WindowWindows;
} //namespace platform
namespace event
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief InputEventReceiver class.
    */
    class V3D_API InputEventReceiver final
    {
    public:

        InputEventReceiver() noexcept;
        ~InputEventReceiver();

        void reset();

        void attach(InputEvent::InputEventType type, InputEventHandler* handler);
        void attach(InputEvent::InputEventType type, InputEventHandler* handler, const platform::Window* window);
        void dettach(InputEvent::InputEventType type);
        void dettach(InputEvent::InputEventType type, InputEventHandler* handler);
        void dettach(InputEvent::InputEventType type, InputEventHandler* handler, const platform::Window* window);

        void pushEvent(InputEvent* event);

        bool sendEvent(InputEvent* event);
        void sendDeferredEvents();

        /**
        * @brief resetInputHandlers. Calls this in the end of frame
        */
        void resetInputHandlers();

    private:

        void* allocateInputEvent();
        void resetInputEventPool();

        InputEventReceiver(const InputEventReceiver&) = delete;
        InputEventReceiver& operator=(const InputEventReceiver&) = delete;

        std::queue<InputEvent*> m_events;
        std::multimap<InputEvent::InputEventType, std::tuple<InputEventHandler*, const platform::Window*>> m_handlers;

        const u32          k_maxInputEventSize;
        u32                m_currentEventIndex;
        void*              m_eventPool;

        static const u32   s_eventPoolSize = 256U;

        friend platform::WindowWindows;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace event
} //namespace v3d


