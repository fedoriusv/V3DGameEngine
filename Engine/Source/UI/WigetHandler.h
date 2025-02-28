#pragma once

#include "Common.h"

namespace v3d
{
namespace platform
{
    class Window;
} //namespace platform

namespace event
{
    class InputEventHandler;
    struct MouseInputEvent;
    struct KeyboardInputEvent;
} //namespace event

namespace renderer
{
    class Device;
    class CmdListRender;
    class RenderTargetState;
} //namespace renderer

namespace ui
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    class Wiget;
    class WigetLayout;
    class WigetHandler;

    /////////////////////////////////////////////////////////////////////////////////////////////////////
    
    class WigetHandler
    {
    public:

        template<class TWigetHander>
        static TWigetHander* createWigetHander(renderer::Device* device, renderer::CmdListRender* cmdList, renderer::RenderTargetState* renderTarget)
        {
            static_assert(std::is_base_of<WigetHandler, TWigetHander>(), "wrong type");
            WigetHandler* handler = V3D_NEW(TWigetHander, memory::MemoryLabel::MemorySystem)(device);
            if (!handler->create(cmdList, renderTarget))
            {
                handler->destroy();

                return nullptr;
            }

            return static_cast<TWigetHander*>(handler);
        }

        template<class TWigetHander>
        static void destroyWigetHander(WigetHandler* handler)
        {
            static_assert(std::is_base_of<WigetHandler, TWigetHander>(), "wrong type");
            ASSERT(handler, "must be valid");
            handler->destroy();
        }

        virtual WigetLayout* createWigetLayout(const std::string& title, const math::Dimension2D& size, const math::Point2D& pos) = 0;
        virtual void destroyWigetLayout(WigetLayout* layout) = 0;

        virtual WigetLayout* createWigetMenuLayout() = 0;
        virtual void createWigetMenuLayout(WigetLayout* layout) = 0;

        virtual void update(platform::Window* window, f32 dt);
        virtual void render(renderer::CmdListRender* cmdList);

        virtual void handleMouseCallback(event::InputEventHandler* handler, const event::MouseInputEvent* event) = 0;
        virtual void handleKeyboardCallback(const v3d::event::InputEventHandler* handler, const event::KeyboardInputEvent* event) = 0;

    public:

        virtual bool drawButton(Wiget* button, f32 dt) = 0;

        virtual bool beginDrawMenu(Wiget* menu, f32 dt) = 0;
        virtual bool endDrawMenu(Wiget* menu, f32 dt) = 0;
        virtual bool drawMenuItem(Wiget* item, f32 dt) = 0;

    protected:

        WigetHandler(renderer::Device* device) noexcept;
        virtual ~WigetHandler();

        virtual bool create(renderer::CmdListRender* cmdList, renderer::RenderTargetState* renderTarget) = 0;
        virtual void destroy() = 0;

        friend WigetLayout;

        renderer::Device* const     m_device;
        std::vector<WigetLayout*>   m_wigetLayouts;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} // namespace ui
} // namespace v3d