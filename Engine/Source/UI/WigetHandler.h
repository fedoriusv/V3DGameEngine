#pragma once

#include "Common.h"
#include "Wiget.h"

#include "Renderer/RenderTargetState.h"

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
    struct GamepadInputEvent;
    struct SystemEvent;
} //namespace event

namespace renderer
{
    class Device;
    class CmdListRender;
} //namespace renderer

namespace ui
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief WigetHandler base class
    */
    class WigetHandler
    {
    public:

        template<class TWigetHander>
        [[nodiscard]] static TWigetHander* createWigetHander(renderer::Device* device, renderer::CmdListRender* cmdList, const renderer::RenderPassDesc& renderpassDesc, u32 flags = 0)
        {
            static_assert(std::is_base_of<WigetHandler, TWigetHander>(), "wrong type");
            WigetHandler* handler = V3D_NEW(TWigetHander, memory::MemoryLabel::MemorySystem)(device, flags);
            if (!handler->create(cmdList, renderpassDesc))
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

        template<class TWiget, class... TArgs>
        TWiget& createWiget(TArgs&&... args)
        {
            TWiget* obj = V3D_NEW(TWiget, memory::MemoryLabel::MemoryUI)(std::forward<TArgs>(args)...);
            m_wigets.push_back(obj);
            return *obj;
        }

        //template<class TWiget>
        //TWiget& addWiget(TWiget& wiget)
        //{
        //    TWiget* obj = V3D_NEW(TWiget(wiget);
        //    m_wigets.push_back(obj);
        //    return *obj;
        //}

        virtual void update(const platform::Window* window, const v3d::event::InputEventHandler* handler, f32 dt);
        virtual void render(renderer::CmdListRender* cmdList);

        virtual void handleMouseCallback(const event::InputEventHandler* handler, const event::MouseInputEvent* event) = 0;
        virtual void handleKeyboardCallback(const v3d::event::InputEventHandler* handler, const event::KeyboardInputEvent* event) = 0;
        virtual void handleGamepadCallback(const v3d::event::InputEventHandler* handler, const event::GamepadInputEvent* event) = 0;
        virtual void handleSystemCallback(const v3d::event::InputEventHandler* handler, const event::SystemEvent* event) = 0;

    public:

        //Menu
        virtual bool draw_MenuBar(Wiget* wiget, Wiget::Context* context, f32 dt) = 0;
        virtual bool draw_Menu(Wiget* wiget, Wiget::Context* context, f32 dt) = 0;
        virtual bool draw_MenuItem(Wiget* wiget, Wiget::Context* context, f32 dt) = 0;
        virtual bool draw_TabBar(Wiget* wiget, Wiget::Context* context, f32 dt) = 0;

        //Windows
        virtual bool draw_Window(Wiget* wiget, Wiget::Context* context, f32 dt) = 0;

        //wigets
        virtual bool draw_Button(Wiget* wiget, Wiget::Context* context, f32 dt) = 0;
        virtual bool draw_Image(Wiget* wiget, Wiget::Context* context, f32 dt) = 0;
        virtual bool draw_CheckBox(Wiget* wiget, Wiget::Context* context, f32 dt) = 0;
        virtual bool draw_RadioButtonGroup(Wiget* wiget, Wiget::Context* context, f32 dt) = 0;
        virtual bool draw_ComboBox(Wiget* wiget, Wiget::Context* context, f32 dt) = 0;
        virtual bool draw_ListBox(Wiget* wiget, Wiget::Context* context, f32 dt) = 0;
        virtual bool draw_InputField(Wiget* wiget, Wiget::Context* context, f32 dt) = 0;
        virtual bool draw_InputSlider(Wiget* wiget, Wiget::Context* context, f32 dt) = 0;

    protected:

        explicit WigetHandler(renderer::Device* device) noexcept;
        virtual ~WigetHandler();

        [[nodiscard]] virtual bool create(renderer::CmdListRender* cmdList, const renderer::RenderPassDesc& renderpassDesc) = 0;
        virtual void destroy() = 0;

        renderer::Device* const m_device;
        std::vector<Wiget*>     m_wigets;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} // namespace ui
} // namespace v3d