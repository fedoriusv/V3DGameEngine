#pragma once

#if USE_IMGUI
#include "Common.h"
#include "WigetHandler.h"

namespace v3d
{
namespace ui
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    class ImGuiWigetHandler;

    class ImGuiWigetDrawer : public WigetDrawer
    {
    public:

        ImGuiWigetDrawer(ImGuiWigetHandler* handler) noexcept;
        ~ImGuiWigetDrawer() = default;

        bool draw_MenuBar(Wiget* wiget, Wiget::State* state, f32 dt) override;
        bool draw_Menu(Wiget* wiget, Wiget* base, Wiget::State* layout, Wiget::State* state, f32 dt) override;
        bool draw_MenuItem(Wiget* wiget, Wiget* base, Wiget::State* layout, Wiget::State* state) override;

        bool draw_TabBar(Wiget* wiget, Wiget::State* state, f32 dt) override;
        bool draw_TabItem(Wiget* wiget, Wiget* base, Wiget::State* layout, Wiget::State* state) override;

        bool draw_Window(Wiget* wiget, Wiget::State* state, f32 dt) override;

        bool draw_Text(Wiget* wiget, Wiget* base, Wiget::State* layout, Wiget::State* state) override;
        bool draw_Button(Wiget* wiget, Wiget* base, Wiget::State* layout, Wiget::State* state) override;
        bool draw_Image(Wiget* wiget, Wiget* base, Wiget::State* layout, Wiget::State* state) override;
        bool draw_CheckBox(Wiget* wiget, Wiget* base, Wiget::State* layout, Wiget::State* state) override;
        bool draw_RadioButtonGroup(Wiget* wiget, Wiget* base, Wiget::State* layout, Wiget::State* state) override;
        bool draw_ComboBox(Wiget* wiget, Wiget* base, Wiget::State* layout, Wiget::State* state) override;
        bool draw_ListBox(Wiget* wiget, Wiget* base, Wiget::State* layout, Wiget::State* state) override;
        bool draw_InputField(Wiget* wiget, Wiget* base, Wiget::State* layout, Wiget::State* state) override;
        bool draw_InputSlider(Wiget* wiget, Wiget* base, Wiget::State* layout, Wiget::State* state) override;

        void draw_BeginLayoutState(Wiget* layout, Wiget* base, Wiget::State* state) override;
        void draw_EndLayoutState(Wiget* layout, Wiget* base, Wiget::State* state) override;

        math::Vector2D get_LayoutPadding() const override;
        math::Vector2D get_ItemSpacing() const override;

        math::Vector2D calculate_TextSize(Wiget* wiget, Wiget::State* layout, Wiget::State* state) override;
        math::Vector2D calculate_ButtonSize(Wiget* wiget, Wiget::State* layout, Wiget::State* state) override;
        math::Vector2D calculate_ImageSize(Wiget* wiget, Wiget::State* layout, Wiget::State* state) override;
        math::Vector2D calculate_CheckBoxSize(Wiget* wiget, Wiget::State* layout, Wiget::State* state) override;
        math::Vector2D calculate_RadioButtonGroupSize(Wiget* wiget, Wiget::State* layout, Wiget::State* state) override;
        math::Vector2D calculate_ComboBoxSize(Wiget* wiget, Wiget::State* layout, Wiget::State* state) override;
        math::Vector2D calculate_ListBoxSize(Wiget* wiget, Wiget::State* layout, Wiget::State* state) override;

    private:

        void setupHorizontalAligment(Wiget::State* layout, f32 itemWidth);
        void setupVerticalAligment(Wiget::State* layout, f32 itemHeight);

        ImGuiWigetHandler* m_wigetHandler;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace ui
} //namespace v3d

#endif //USE_IMGUI