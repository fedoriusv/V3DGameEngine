#pragma once

#if USE_IMGUI
#include "Common.h"
#include "UI/WigetHandler.h"

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

        void draw_Gizmo(Wiget* wiget, Wiget* base, Wiget::State* layout, Wiget::State* state) override;
        void draw_ViewManipulator(Wiget* wiget, Wiget* base, Wiget::State* layout, Wiget::State* state) override;

        math::TVector2D<f32> get_LayoutPadding() const override;
        math::TVector2D<f32> get_ItemSpacing() const override;

        math::TVector2D<f32> calculate_TextSize(Wiget* wiget, Wiget::State* layout, Wiget::State* state) override;
        math::TVector2D<f32> calculate_ButtonSize(Wiget* wiget, Wiget::State* layout, Wiget::State* state) override;
        math::TVector2D<f32> calculate_ImageSize(Wiget* wiget, Wiget::State* layout, Wiget::State* state) override;
        math::TVector2D<f32> calculate_CheckBoxSize(Wiget* wiget, Wiget::State* layout, Wiget::State* state) override;
        math::TVector2D<f32> calculate_RadioButtonGroupSize(Wiget* wiget, Wiget::State* layout, Wiget::State* state) override;
        math::TVector2D<f32> calculate_ComboBoxSize(Wiget* wiget, Wiget::State* layout, Wiget::State* state) override;
        math::TVector2D<f32> calculate_ListBoxSize(Wiget* wiget, Wiget::State* layout, Wiget::State* state) override;

    private:

        f32 setupHorizontalAligment(Wiget::State* layout, f32 originalWidth, f32 itemWidth);
        f32 setupVerticalAligment(Wiget::State* layout, f32 originalHeight, f32 itemHeight);

        ImGuiWigetHandler* m_wigetHandler;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace ui
} //namespace v3d

#endif //USE_IMGUI