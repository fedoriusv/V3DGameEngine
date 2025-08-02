#pragma once

#if USE_IMGUI
#include "Common.h"
#include "UI/WidgetHandler.h"

namespace v3d
{
namespace ui
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    class ImGuiWidgetHandler;

    class ImGuiWidgetDrawer : public WidgetDrawer
    {
    public:

        ImGuiWidgetDrawer(ImGuiWidgetHandler* handler) noexcept;
        ~ImGuiWidgetDrawer() = default;

        bool draw_MenuBar(Widget* wiget, Widget::State* state, f32 dt) override;
        bool draw_Menu(Widget* wiget, Widget* base, Widget::State* layout, Widget::State* state, f32 dt) override;
        bool draw_MenuItem(Widget* wiget, Widget* base, Widget::State* layout, Widget::State* state) override;

        bool draw_TabBar(Widget* wiget, Widget::State* state, f32 dt) override;
        bool draw_TabItem(Widget* wiget, Widget* base, Widget::State* layout, Widget::State* state) override;

        bool draw_Window(Widget* wiget, Widget::State* state, f32 dt) override;

        bool draw_TreeNode(Widget* widget, Widget* base, Widget::State* layout, Widget::State* state, f32 dt) override;

        bool draw_Text(Widget* wiget, Widget* baseWidget, Widget::State* layoutBaseState, Widget::State* state) override;
        bool draw_Button(Widget* wiget, Widget* baseWidget, Widget::State* layoutBaseState, Widget::State* state) override;
        bool draw_Image(Widget* wiget, Widget* baseWidget, Widget::State* layoutBaseState, Widget::State* state) override;
        bool draw_CheckBox(Widget* wiget, Widget* baseWidget, Widget::State* layoutBaseState, Widget::State* state) override;
        bool draw_RadioButtonGroup(Widget* wiget, Widget* baseWidget, Widget::State* layoutBaseState, Widget::State* state) override;
        bool draw_ComboBox(Widget* wiget, Widget* baseWidget, Widget::State* layoutBaseState, Widget::State* state) override;
        bool draw_ListBox(Widget* wiget, Widget* baseWidget, Widget::State* layoutBaseState, Widget::State* state) override;
        bool draw_InputText(Widget* widget, Widget* baseWidget, Widget::State* layoutBaseState, Widget::State* state) override;
        bool draw_InputValue(Widget* widget, Widget* baseWidget, Widget::State* layoutBaseState, Widget::State* state, bool isRealNumber, u32 array) override;
        bool draw_InputSliderValue(Widget* widget, Widget* baseWidget, Widget::State* layoutBaseState, Widget::State* state, bool isRealNumber) override;
        bool draw_InputDragValue(Widget* widget, Widget* baseWidget, Widget::State* layoutBaseState, Widget::State* state, bool isRealNumber) override;
        bool draw_ColorPalette(Widget* widget, Widget* baseWidget, Widget::State* layoutBaseState, Widget::State* state) override;

        bool draw_Table(Widget* widget, Widget* base, Widget::State* layout, Widget::State* state) override;
        bool draw_TEST(Widget* widget, Widget* baseWidget, Widget::State* layoutBaseState, Widget::State* state) override;

        void draw_BeginLayoutState(Widget* layout, Widget* base, Widget::State* baseLayoutState, Widget::State* state, const math::float2& size) override;
        void draw_EndLayoutState(Widget* layout, Widget* base, Widget::State* baseLayoutState, Widget::State* state, const math::float2& size) override;
        bool draw_Separator(Widget* widget, Widget* base, Widget::State* layout, Widget::State* state) override;

        void draw_Gizmo(Widget* wiget, Widget* base, Widget::State* layout, Widget::State* state) override;
        void draw_ViewManipulator(Widget* wiget, Widget* base, Widget::State* layout, Widget::State* state) override;

        math::float2 get_WindowPadding() const override;
        math::float2 get_LayoutPadding() const override;
        math::float2 get_ItemSpacing() const override;

        math::float2 calculate_TextSize(Widget* wiget, Widget::State* layoutBaseState, Widget::State* state) override;
        math::float2 calculate_ButtonSize(Widget* wiget, Widget::State* layoutBaseState, Widget::State* state) override;
        math::float2 calculate_ImageSize(Widget* wiget, Widget::State* layoutBaseState, Widget::State* state) override;
        math::float2 calculate_CheckBoxSize(Widget* wiget, Widget::State* layoutBaseState, Widget::State* state) override;
        math::float2 calculate_RadioButtonGroupSize(Widget* wiget, Widget::State* layoutBaseState, Widget::State* state) override;
        math::float2 calculate_ComboBoxSize(Widget* wiget, Widget::State* layoutBaseState, Widget::State* state) override;
        math::float2 calculate_ListBoxSize(Widget* wiget, Widget::State* layoutBaseState, Widget::State* state) override;
        math::float2 calculate_InputDragValueSize(Widget* widget, Widget::State* layoutBaseState, Widget::State* state, bool isRealNumber) override;
        math::float2 calculate_TreeNodeSize(Widget* widget, Widget::State* layoutBaseState, Widget::State* state) override;
        math::float2 calculate_ColorPaletteSize(Widget* widget, Widget::State* layoutBaseState, Widget::State* state) override;

        math::float2 calculate_TableSize(Widget* widget, Widget::State* layoutBaseState, Widget::State* state) override;

    private:

        f32 setupHorizontalAligment(Widget::State* layout, f32 originalWidth, f32 itemWidth);
        f32 setupVerticalAligment(Widget::State* layout, f32 originalHeight, f32 itemHeight);

        ImGuiWidgetHandler* m_widgetHandler;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace ui
} //namespace v3d

#endif //USE_IMGUI