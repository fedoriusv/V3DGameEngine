#pragma once

#include "Common.h"
#include "Widget.h"
#include "WidgetLayout.h"

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

    class WidgetDrawer;

    /**
    * @brief WidgetHandler base class
    */
    class WidgetHandler
    {
    public:

        template<class TWidgetHander>
        [[nodiscard]] static TWidgetHander* createWidgetHander(renderer::Device* device, const renderer::RenderPassDesc& renderpassDesc, u32 flags = 0)
        {
            static_assert(std::is_base_of<WidgetHandler, TWidgetHander>(), "wrong type");
            WidgetHandler* handler = V3D_NEW(TWidgetHander, memory::MemoryLabel::MemorySystem)(device, flags);
            if (!handler->create(renderpassDesc))
            {
                handler->destroy();

                return nullptr;
            }

            return static_cast<TWidgetHander*>(handler);
        }

        template<class TWidgetHander>
        static void destroyWidgetHander(WidgetHandler* handler)
        {
            static_assert(std::is_base_of<WidgetHandler, TWidgetHander>(), "wrong type");
            ASSERT(handler, "must be valid");
            handler->destroy();
        }

        template<class TWidget, class... TArgs>
        TWidget& createWidget(TArgs&&... args)
        {
            TWidget* obj = V3D_NEW(TWidget, memory::MemoryLabel::MemoryUI)(std::forward<TArgs>(args)...);
            m_Widgets.push_back(obj);
            return *obj;
        }

        virtual void update(const platform::Window* window, const v3d::event::InputEventHandler* handler, f32 dt);
        virtual bool render(renderer::CmdListRender* cmdList);

        virtual void handleMouseCallback(const event::InputEventHandler* handler, const event::MouseInputEvent* event) = 0;
        virtual void handleKeyboardCallback(const v3d::event::InputEventHandler* handler, const event::KeyboardInputEvent* event) = 0;
        virtual void handleGamepadCallback(const v3d::event::InputEventHandler* handler, const event::GamepadInputEvent* event) = 0;
        virtual void handleSystemCallback(const v3d::event::InputEventHandler* handler, const event::SystemEvent* event) = 0;

        WidgetDrawer* getWidgetDrawer() const;

    protected:

        explicit WidgetHandler(renderer::Device* device) noexcept;
        virtual ~WidgetHandler();

        [[nodiscard]] virtual bool create(const renderer::RenderPassDesc& renderpassDesc) = 0;
        virtual void destroy() = 0;

        renderer::Device* const  m_device;
        WidgetDrawer*            m_uiDrawer;
        std::vector<Widget*>     m_Widgets;
    };

    inline WidgetDrawer* WidgetHandler::getWidgetDrawer() const
    {
        return m_uiDrawer;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    class WidgetDrawer
    {
    public:

        WidgetDrawer() noexcept = default;
        virtual ~WidgetDrawer() = default;

        //Menu
        virtual bool draw_MenuBar(Widget* widget, Widget::State* state, f32 dt) = 0;
        virtual bool draw_Menu(Widget* widget, Widget* base, Widget::State* layout, Widget::State* state, f32 dt) = 0;
        virtual bool draw_MenuItem(Widget* widget, Widget* base, Widget::State* layout, Widget::State* state) = 0;

        //TabBar
        virtual bool draw_TabBar(Widget* widget, Widget::State* state, f32 dt) = 0;
        virtual bool draw_TabItem(Widget* widget, Widget* base, Widget::State* layout, Widget::State* state) = 0;

        //Windows
        virtual bool draw_Window(Widget* widget, Widget::State* state, f32 dt) = 0;

        //Widgets
        virtual bool draw_Text(Widget* widget, Widget* base, Widget::State* layout, Widget::State* state) = 0;
        virtual bool draw_Button(Widget* widget, Widget* base, Widget::State* layout, Widget::State* state) = 0;
        virtual bool draw_Image(Widget* widget, Widget* base, Widget::State* layout, Widget::State* state) = 0;
        virtual bool draw_CheckBox(Widget* widget, Widget* base, Widget::State* layout, Widget::State* state) = 0;
        virtual bool draw_RadioButtonGroup(Widget* widget, Widget* base, Widget::State* layout, Widget::State* state) = 0;
        virtual bool draw_ComboBox(Widget* widget, Widget* base, Widget::State* layout, Widget::State* state) = 0;
        virtual bool draw_ListBox(Widget* widget, Widget* base, Widget::State* layout, Widget::State* state) = 0;
        virtual bool draw_InputField(Widget* widget, Widget* base, Widget::State* layout, Widget::State* state) = 0;
        virtual bool draw_InputSlider(Widget* widget, Widget* base, Widget::State* layout, Widget::State* state) = 0;

        //Layout
        virtual void draw_BeginLayoutState(Widget* layout, Widget* base, Widget::State* state) = 0;
        virtual void draw_EndLayoutState(Widget* layout, Widget* base, Widget::State* state) = 0;

        virtual void draw_Gizmo(Widget* widget, Widget* base, Widget::State* layout, Widget::State* state) = 0;
        virtual void draw_ViewManipulator(Widget* widget, Widget* base, Widget::State* layout, Widget::State* state) = 0;

        virtual math::TVector2D<f32> get_LayoutPadding() const = 0;
        virtual math::TVector2D<f32> get_ItemSpacing() const = 0;

        virtual math::TVector2D<f32> calculate_TextSize(Widget* widget, Widget::State* layout, Widget::State* state) = 0;
        virtual math::TVector2D<f32> calculate_ButtonSize(Widget* widget, Widget::State* layout, Widget::State* state) = 0;
        virtual math::TVector2D<f32> calculate_ImageSize(Widget* widget, Widget::State* layout, Widget::State* state) = 0;
        virtual math::TVector2D<f32> calculate_CheckBoxSize(Widget* widget, Widget::State* layout, Widget::State* state) = 0;
        virtual math::TVector2D<f32> calculate_RadioButtonGroupSize(Widget* widget, Widget::State* layout, Widget::State* state) = 0;
        virtual math::TVector2D<f32> calculate_ComboBoxSize(Widget* widget, Widget::State* layout, Widget::State* state) = 0;
        virtual math::TVector2D<f32> calculate_ListBoxSize(Widget* widget, Widget::State* layout, Widget::State* state) = 0;
    };

} // namespace ui
} // namespace v3d