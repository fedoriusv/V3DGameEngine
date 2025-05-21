#pragma once

#include "Common.h"
#include "Wiget.h"
#include "WigetLayout.h"

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

    class WigetDrawer;

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

        virtual void update(const platform::Window* window, const v3d::event::InputEventHandler* handler, f32 dt);
        virtual bool render(renderer::CmdListRender* cmdList);

        virtual void handleMouseCallback(const event::InputEventHandler* handler, const event::MouseInputEvent* event) = 0;
        virtual void handleKeyboardCallback(const v3d::event::InputEventHandler* handler, const event::KeyboardInputEvent* event) = 0;
        virtual void handleGamepadCallback(const v3d::event::InputEventHandler* handler, const event::GamepadInputEvent* event) = 0;
        virtual void handleSystemCallback(const v3d::event::InputEventHandler* handler, const event::SystemEvent* event) = 0;

        WigetDrawer* getWigetDrawer() const;

    protected:

        explicit WigetHandler(renderer::Device* device) noexcept;
        virtual ~WigetHandler();

        [[nodiscard]] virtual bool create(renderer::CmdListRender* cmdList, const renderer::RenderPassDesc& renderpassDesc) = 0;
        virtual void destroy() = 0;

        renderer::Device* const m_device;
        WigetDrawer*            m_uiDrawer;
        std::vector<Wiget*>     m_wigets;
    };

    inline WigetDrawer* WigetHandler::getWigetDrawer() const
    {
        return m_uiDrawer;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    class WigetDrawer
    {
    public:

        WigetDrawer() noexcept = default;
        virtual ~WigetDrawer() = default;

        //Menu
        virtual bool draw_MenuBar(Wiget* wiget, Wiget::State* state, f32 dt) = 0;
        virtual bool draw_Menu(Wiget* wiget, Wiget* base, Wiget::State* layout, Wiget::State* state, f32 dt) = 0;
        virtual bool draw_MenuItem(Wiget* wiget, Wiget* base, Wiget::State* layout, Wiget::State* state) = 0;

        //TabBar
        virtual bool draw_TabBar(Wiget* wiget, Wiget::State* state, f32 dt) = 0;
        virtual bool draw_TabItem(Wiget* wiget, Wiget* base, Wiget::State* layout, Wiget::State* state) = 0;

        //Windows
        virtual bool draw_Window(Wiget* wiget, Wiget::State* state, f32 dt) = 0;

        //Wigets
        virtual bool draw_Text(Wiget* wiget, Wiget* base, Wiget::State* layout, Wiget::State* state) = 0;
        virtual bool draw_Button(Wiget* wiget, Wiget* base, Wiget::State* layout, Wiget::State* state) = 0;
        virtual bool draw_Image(Wiget* wiget, Wiget* base, Wiget::State* layout, Wiget::State* state) = 0;
        virtual bool draw_CheckBox(Wiget* wiget, Wiget* base, Wiget::State* layout, Wiget::State* state) = 0;
        virtual bool draw_RadioButtonGroup(Wiget* wiget, Wiget* base, Wiget::State* layout, Wiget::State* state) = 0;
        virtual bool draw_ComboBox(Wiget* wiget, Wiget* base, Wiget::State* layout, Wiget::State* state) = 0;
        virtual bool draw_ListBox(Wiget* wiget, Wiget* base, Wiget::State* layout, Wiget::State* state) = 0;
        virtual bool draw_InputField(Wiget* wiget, Wiget* base, Wiget::State* layout, Wiget::State* state) = 0;
        virtual bool draw_InputSlider(Wiget* wiget, Wiget* base, Wiget::State* layout, Wiget::State* state) = 0;

        //Layout
        virtual void draw_BeginLayoutState(Wiget* layout, Wiget* base, Wiget::State* state) = 0;
        virtual void draw_EndLayoutState(Wiget* layout, Wiget* base, Wiget::State* state) = 0;

        virtual void draw_Gizmo(Wiget* wiget, Wiget* base, Wiget::State* layout, Wiget::State* state) = 0;
        virtual void draw_ViewManipulator(Wiget* wiget, Wiget* base, Wiget::State* layout, Wiget::State* state) = 0;

        virtual math::TVector2D<f32> get_LayoutPadding() const = 0;
        virtual math::TVector2D<f32> get_ItemSpacing() const = 0;

        virtual math::TVector2D<f32> calculate_TextSize(Wiget* wiget, Wiget::State* layout, Wiget::State* state) = 0;
        virtual math::TVector2D<f32> calculate_ButtonSize(Wiget* wiget, Wiget::State* layout, Wiget::State* state) = 0;
        virtual math::TVector2D<f32> calculate_ImageSize(Wiget* wiget, Wiget::State* layout, Wiget::State* state) = 0;
        virtual math::TVector2D<f32> calculate_CheckBoxSize(Wiget* wiget, Wiget::State* layout, Wiget::State* state) = 0;
        virtual math::TVector2D<f32> calculate_RadioButtonGroupSize(Wiget* wiget, Wiget::State* layout, Wiget::State* state) = 0;
        virtual math::TVector2D<f32> calculate_ComboBoxSize(Wiget* wiget, Wiget::State* layout, Wiget::State* state) = 0;
        virtual math::TVector2D<f32> calculate_ListBoxSize(Wiget* wiget, Wiget::State* layout, Wiget::State* state) = 0;
    };

} // namespace ui
} // namespace v3d