#include "ImGuiDrawer.h"
#include "Utils/Logger.h"

#if USE_IMGUI
#include "UI/Widgets.h"
#include "UI/WidgetGizmo.h"
#include "ImGuiHandler.h"
#include "ThirdParty/imgui/imgui.h"
#include "ThirdParty/imgui/imgui_internal.h"
#include "ThirdParty/ImGuizmo/ImGuizmo.h"

#include "Scene/Camera/Camera.h"
#include "Scene/Transform.h"

namespace v3d
{
namespace ui
{

ImGuiWidgetDrawer::ImGuiWidgetDrawer(ImGuiWidgetHandler* handler) noexcept
    : m_widgetHandler(handler)
{
}

bool ImGuiWidgetDrawer::draw_Window(Widget* window, Widget::State* state, f32 dt)
{
    ASSERT(ImGui::GetCurrentContext(), "must be valid");
    WidgetWindow::StateWindow* wndCtx = static_cast<WidgetWindow::StateWindow*>(state);

    if (wndCtx->_stateMask & Widget::State::StateMask::FirstUpdateState)
    {
        if (wndCtx->_position._x != 0 || wndCtx->_position._y != 0)
        {
            ImGui::SetNextWindowPos(ImVec2(static_cast<f32>(wndCtx->_position._x), static_cast<f32>(wndCtx->_position._y)), ImGuiCond_Once);
        }

        if (wndCtx->_size._width > 0 || wndCtx->_size._height > 0)
        {
            ImGui::SetNextWindowSize(ImVec2(static_cast<f32>(wndCtx->_size._width), static_cast<f32>(wndCtx->_size._height)), ImGuiCond_Once);
        }

        wndCtx->_stateMask &= ~Widget::State::StateMask::FirstUpdateState;
    }

    ImGuiStyle& style = ImGui::GetStyle();
    ImGuiWindowFlags flags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove /*| ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse*/;
    if (wndCtx->_createFlags & WidgetWindow::Moveable)
    {
        flags &= ~ImGuiWindowFlags_NoMove;
    }

    if (wndCtx->_createFlags & WidgetWindow::Resizeable)
    {
        flags &= ~ImGuiWindowFlags_NoResize;
    }

    if (wndCtx->_createFlags & WidgetWindow::Scrollable)
    {
        flags &= ~ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse;
    }

    if (wndCtx->_createFlags & WidgetWindow::AutoResizeByContent)
    {
        flags |= ImGuiWindowFlags_AlwaysAutoResize;
    }

    static auto callbackHandler = [this](WidgetWindow* window, WidgetWindow::StateWindow* wndCtx) -> void
        {
            math::Point2D pos(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y);
            const bool posChanged = pos != wndCtx->_position;
            if (posChanged || wndCtx->_stateMask & Widget::State::StateMask::ForceUpdateState)
            {
                wndCtx->_position = pos;
                if (wndCtx->_onPositionChanged)
                {
                    std::invoke(wndCtx->_onPositionChanged, window, nullptr, pos);
                }
            }

            math::Dimension2D size(ImGui::GetWindowSize().x, ImGui::GetWindowSize().y);
            const bool sizeChanged = size != wndCtx->_size;
            if (sizeChanged || wndCtx->_stateMask & Widget::State::StateMask::ForceUpdateState)
            {
                wndCtx->_size = size;
                if (wndCtx->_onSizeChanged)
                {
                    std::invoke(wndCtx->_onSizeChanged, window, nullptr, size);
                }
            }

            bool focused = ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows);
            const bool focusChanged = focused != window->isFocused();
            if (focusChanged)
            {
                wndCtx->_isFocused = focused;
                if (wndCtx->_onFocusChanged)
                {
                    std::invoke(wndCtx->_onFocusChanged, window, focused);
                }
            }
        };

    ImGui::PushID(wndCtx->_uid);
    bool open = true;
    bool active = ImGui::Begin(wndCtx->_title.c_str(), &open, flags);
    if (active)
    {
        ImGuiViewport* viewport = ImGui::GetWindowViewport();
        platform::Window* activeWindow = reinterpret_cast<platform::Window*>(viewport->PlatformUserData);
        wndCtx->_stateMask |= (wndCtx->_currentWindow != activeWindow) ? Widget::State::StateMask::ForceUpdateState : 0x0;
        
        wndCtx->_currentWindow = activeWindow;
        wndCtx->_cachedWindowRect = { ImGui::GetCursorScreenPos().x, ImGui::GetCursorScreenPos().y, ImGui::GetContentRegionAvail().x + ImGui::GetCursorScreenPos().x, ImGui::GetContentRegionAvail().y + ImGui::GetCursorScreenPos().y };
        wndCtx->_cachedWindowOffest = { ImGui::GetWindowContentRegionMin().x, ImGui::GetWindowContentRegionMin().y };

        if (wndCtx->_showToolTip && ImGui::IsItemHovered(ImGuiHoveredFlags_ForTooltip))
        {
            ImGui::SetTooltip(wndCtx->_toolTip.c_str());
        }

        callbackHandler(static_cast<WidgetWindow*>(window), wndCtx);

        wndCtx->_layout.update(m_widgetHandler, window, &wndCtx->_layout, dt);

        ImGuiIO& imguiIO = ImGui::GetIO();
        if (imguiIO.ConfigFlags & ImGuiConfigFlags_DockingEnable && wndCtx->_windowLayout.getWindow())
        {
            static auto dockBuilder = [](const WidgetWindowLayout &windowLayout, ImGuiID dockspaceID, bool force = false) -> bool
                {
                    if (ImGui::DockBuilderGetNode(dockspaceID) == nullptr || force) //recreate
                    {
                        ImGui::DockBuilderRemoveNode(dockspaceID);

                        ImGuiID dockMainID = ImGui::DockBuilderAddNode(dockspaceID);
                        ImGui::DockBuilderDockWindow(windowLayout.getWindow()->getTitle().c_str(), dockMainID);
                        for (const WidgetWindowLayout::LayoutRule& ruleLayout : windowLayout.getRules())
                        {
                            ImGuiID dockID = ImGui::DockBuilderSplitNode(dockMainID, (ImGuiDir)ruleLayout._dir, ruleLayout._ratio, nullptr, &dockMainID);
                            ImGui::DockBuilderDockWindow(ruleLayout._window->getTitle().c_str(), dockID);
                        }

                        ImGui::DockBuilderFinish(dockspaceID);
                    }

                    return false;
                };

            std::string dockspaceName("Dockspace_" + std::to_string(wndCtx->_uid));
            ImGuiID dockspaceID = ImGui::GetID(dockspaceName.c_str());
            dockBuilder(wndCtx->_windowLayout, dockspaceID);

            ImGui::PushStyleColor(ImGuiCol_DockingEmptyBg, ImVec4(1.0, 0, 0, 1));
            ImGui::DockSpace(dockspaceID, ImVec2(0.0f, 0.0f), 0);
            ImGui::PopStyleColor();
        }
    }
    else
    {
        ImGuiViewport* viewport = ImGui::GetWindowViewport();
        platform::Window* activeWindow = reinterpret_cast<platform::Window*>(viewport->PlatformUserData);
        wndCtx->_stateMask = (wndCtx->_currentWindow != activeWindow) ? Widget::State::StateMask::ForceUpdateState : 0x0;
        wndCtx->_currentWindow = activeWindow;
        wndCtx->_cachedWindowRect = { ImGui::GetCursorScreenPos().x, ImGui::GetCursorScreenPos().y, ImGui::GetContentRegionAvail().x + ImGui::GetCursorScreenPos().x, ImGui::GetContentRegionAvail().y + ImGui::GetCursorScreenPos().y };
        wndCtx->_cachedWindowOffest = { ImGui::GetWindowContentRegionMin().x, ImGui::GetWindowContentRegionMin().y };

        callbackHandler(static_cast<WidgetWindow*>(window), wndCtx);
    }
    ImGui::End();

    wndCtx->_stateMask &= ~Widget::State::StateMask::ForceUpdateState;
    ImGui::PopID();

    return active;
}

bool ImGuiWidgetDrawer::draw_Text(Widget* wiget, Widget* baseWidget, Widget::State* layoutBaseState, Widget::State* state)
{
    ASSERT(ImGui::GetCurrentContext(), "must be valid");
    ImGuiStyle& style = ImGui::GetStyle();

    WidgetText::StateText* txtCtx = static_cast<WidgetText::StateText*>(state);
    WidgetLayout::StateLayoutBase* layoutCtx = static_cast<WidgetLayout::StateLayoutBase*>(layoutBaseState);

    u32 pushCount = 0;
    if (txtCtx->_stateMask & Widget::State::StateMask::Color)
    {
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4{ txtCtx->_color._x, txtCtx->_color._y, txtCtx->_color._z, txtCtx->_color._w });
        ++pushCount;
    }

    setupHorizontalAligment(layoutCtx, 0.f, txtCtx->_itemRect.getWidth());
    //setupVerticalAligment(layoutCtx, txtCtx->_itemRect.getHeight());

    ImGui::PushID(txtCtx->_uid);
    if (layoutCtx->_stateMask & Widget::State::StateMask::DefindedSize)
    {
        ImGui::SetNextItemWidth(txtCtx->_itemRect.getWidth());
    }
    ImGui::Text(txtCtx->_text.c_str());
    ImGui::PopID();

    if (pushCount > 0)
    {
        ImGui::PopStyleColor(pushCount);
    }

    return true;
}

math::float2 ImGuiWidgetDrawer::calculate_TextSize(Widget* wiget, Widget::State* layoutBaseState, Widget::State* state)
{
    ASSERT(ImGui::GetCurrentContext(), "must be valid");
    WidgetText::StateText* txtCtx = static_cast<WidgetText::StateText*>(state);
    WidgetLayout::StateLayoutBase* layoutCtx = static_cast<WidgetLayout::StateLayoutBase*>(layoutBaseState);

    ASSERT(layoutCtx->_fontSize < m_widgetHandler->m_fonts.size(), "range out");
    ImGui::PushFont(m_widgetHandler->m_fonts[layoutCtx->_fontSize]);
    const ImVec2 alignmentSize = ImGui::CalcTextSize(txtCtx->_text.c_str());
    ImGui::PopFont();

    return{ alignmentSize.x, alignmentSize.y };
}

bool ImGuiWidgetDrawer::draw_Button(Widget* button, Widget* baseWidget, Widget::State* layoutBaseState, Widget::State* state)
{
    ASSERT(ImGui::GetCurrentContext(), "must be valid");
    ImGuiStyle& style = ImGui::GetStyle();

    WidgetButton::StateButton* btnCtx = static_cast<WidgetButton::StateButton*>(state);
    WidgetLayout::StateLayoutBase* layoutCtx = static_cast<WidgetLayout::StateLayoutBase*>(layoutBaseState);

    u32 pushCount = 0;
    if (btnCtx->_stateMask & Widget::State::StateMask::Color)
    {
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ btnCtx->_color._x, btnCtx->_color._y, btnCtx->_color._z, btnCtx->_color._w });
        ++pushCount;
    }

    if (btnCtx->_stateMask & Widget::State::StateMask::HoveredColor)
    {
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ btnCtx->_colorHovered._x, btnCtx->_colorHovered._y, btnCtx->_colorHovered._z, btnCtx->_colorHovered._w });
        ++pushCount;
    }

    if (btnCtx->_stateMask & Widget::State::StateMask::ClickedColor)
    {
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ btnCtx->_colorActive._x, btnCtx->_colorActive._y, btnCtx->_colorActive._z, btnCtx->_colorActive._w });
        ++pushCount;
    }

    setupHorizontalAligment(layoutCtx, 0.f, btnCtx->_itemRect.getWidth());
    //setupVerticalAligment(layoutCtx, btnCtx->_itemRect.getHeight());

    ImGui::PushID(btnCtx->_uid);
    ImVec2 size = { (f32)btnCtx->_size._width, (f32)btnCtx->_size._height };
    bool action = ImGui::Button(btnCtx->_text.c_str(), size);
    ImGui::PopID();

    if (pushCount > 0)
    {
        ImGui::PopStyleColor(pushCount);
    }

    if (btnCtx->_showToolTip && ImGui::IsItemHovered(ImGuiHoveredFlags_ForTooltip))
    {
        ImGui::SetTooltip(btnCtx->_toolTip.c_str());
    }

    if (btnCtx->_onHoveredEvent && ImGui::IsItemHovered())
    {
        std::invoke(btnCtx->_onHoveredEvent, button);
    }

    if (btnCtx->_onClickedEvent && ImGui::IsItemClicked())
    {
        std::invoke(btnCtx->_onClickedEvent, button);
    }

    return action;
}

math::float2 ImGuiWidgetDrawer::calculate_ButtonSize(Widget* wiget, Widget::State* layoutBaseState, Widget::State* state)
{
    ASSERT(ImGui::GetCurrentContext(), "must be valid");
    ImGuiStyle& style = ImGui::GetStyle();

    WidgetButton::StateButton* btnCtx = static_cast<WidgetButton::StateButton*>(state);
    WidgetLayout::StateLayoutBase* layoutCtx = static_cast<WidgetLayout::StateLayoutBase*>(layoutBaseState);

    ImVec2 alignmentSize{ 0.f, 0.f };
    if (btnCtx->_stateMask & Widget::State::StateMask::DefindedSize)
    {
        alignmentSize = ImVec2{ static_cast<f32>(btnCtx->_size._width), static_cast<f32>(btnCtx->_size._height) };
    }
    else
    {
        ASSERT(layoutCtx->_fontSize < m_widgetHandler->m_fonts.size(), "range out");
        ImGui::PushFont(m_widgetHandler->m_fonts[layoutCtx->_fontSize]);
        alignmentSize = ImGui::CalcTextSize(btnCtx->_text.c_str());
        alignmentSize.x += style.FramePadding.x * 2.0f;
        alignmentSize.y += style.FramePadding.y * 2.0f;
        ImGui::PopFont();
    }

    return { alignmentSize.x, alignmentSize.y };
}

bool ImGuiWidgetDrawer::draw_Image(Widget* image, Widget* baseWidget, Widget::State* layoutBaseState, Widget::State* state)
{
    ASSERT(ImGui::GetCurrentContext(), "must be valid");
    WidgetImage::StateImage* imgCtx = static_cast<WidgetImage::StateImage*>(state);
    WidgetLayout::StateLayoutBase* layoutCtx = static_cast<WidgetLayout::StateLayoutBase*>(layoutBaseState);

    if (imgCtx->_texture)
    {
        ImVec2 size = { (f32)imgCtx->_size._width, (f32)imgCtx->_size._height };
        ImVec2 uv0 = { imgCtx->_uv.getLeftX(), imgCtx->_uv.getTopY() };
        ImVec2 uv1 = { imgCtx->_uv.getRightX(), imgCtx->_uv.getBottomY() };

        size.x = setupHorizontalAligment(layoutCtx, size.x, imgCtx->_itemRect.getWidth());
        //size.y = setupVerticalAligment(layoutCtx, imgCtx->_itemRect.getHeight());

        if (layoutCtx->_aligmentV == WidgetLayout::VerticalAlignment::AlignmentFill)
        {
            ImVec2 avail = ImGui::GetContentRegionAvail();
            size.y = layoutCtx->_cachedLayoutRect.getHeight();
        }

        if (size.x == 0.f || size.y == 0.f)
        {
            return false;
        }

        u32 id = ~0;
        auto found = std::find(m_widgetHandler->m_activeTextures.begin(), m_widgetHandler->m_activeTextures.end(), imgCtx->_texture);
        if (found == m_widgetHandler->m_activeTextures.end())
        {
            m_widgetHandler->m_activeTextures.push_back(nullptr);
            id = m_widgetHandler->m_activeTextures.size() - 1;
        }
        else
        {
            id = std::distance(m_widgetHandler->m_activeTextures.begin(), found);
        }

        ImGui::Image(id, size, uv0, uv1);

        ImGuiViewport* viewport = ImGui::GetWindowViewport();
        platform::Window* activeWindow = reinterpret_cast<platform::Window*>(viewport->PlatformUserData);

        if (imgCtx->_showToolTip && ImGui::IsItemHovered(ImGuiHoveredFlags_ForTooltip))
        {
            ImGui::SetTooltip(imgCtx->_toolTip.c_str());
        }

        if (imgCtx->_onHoveredEvent && ImGui::IsItemHovered())
        {
            std::invoke(imgCtx->_onHoveredEvent, image);
        }

        if (imgCtx->_onClickedEvent && ImGui::IsItemClicked())
        {
            std::invoke(imgCtx->_onClickedEvent, image);
        }

        s32 x = ImGui::GetItemRectMin().x;
        s32 y = ImGui::GetItemRectMin().y;
        s32 width = std::max<s32>(ImGui::GetItemRectSize().x, 1);
        s32 height = std::max<s32>(ImGui::GetItemRectSize().y, 1);
        math::Rect rect(x, y, x + width, y + height);

        if (imgCtx->_drawRectState != rect || baseWidget->isStateMaskActive(Widget::State::StateMask::ForceUpdateState))
        {
            imgCtx->_drawRectState = rect;
            if (imgCtx->_onDrawRectChanged)
            {
                std::invoke(imgCtx->_onDrawRectChanged, image, baseWidget, rect);
            }
        }
        m_widgetHandler->m_activeTextures[id] = imgCtx->_texture;

        return true;
    }

    return false;
}

math::float2 ImGuiWidgetDrawer::calculate_ImageSize(Widget* wiget, Widget::State* layoutBaseState, Widget::State* state)
{
    ASSERT(ImGui::GetCurrentContext(), "must be valid");
    WidgetImage::StateImage* imgCtx = static_cast<WidgetImage::StateImage*>(state);
    ImGuiStyle& style = ImGui::GetStyle();

    return { imgCtx->_size._width + style.FramePadding.x * 2.f, imgCtx->_size._height + style.FramePadding.x * 2.f };
}

bool ImGuiWidgetDrawer::draw_CheckBox(Widget* wiget, Widget* base, Widget::State* layoutBaseState, Widget::State* state)
{
    ASSERT(ImGui::GetCurrentContext(), "must be valid");
    WidgetCheckBox::StateCheckBox* cbCtx = static_cast<WidgetCheckBox::StateCheckBox*>(state);
    WidgetLayout::StateLayoutBase* layoutCtx = static_cast<WidgetLayout::StateLayoutBase*>(layoutBaseState);

    setupHorizontalAligment(layoutCtx, 0.f, cbCtx->_itemRect.getWidth());


    ImGui::PushID(cbCtx->_uid);
    bool value = cbCtx->_value;
    bool active = ImGui::Checkbox(cbCtx->_text.c_str(), &value);
    ImGui::PopID();

    if (cbCtx->_showToolTip && ImGui::IsItemHovered(ImGuiHoveredFlags_ForTooltip))
    {
        ImGui::SetTooltip(cbCtx->_toolTip.c_str());
    }

    if (value != cbCtx->_value)
    {
        cbCtx->_value = value;
        if (cbCtx->_onChangedValueEvent)
        {
            std::invoke(cbCtx->_onChangedValueEvent, wiget, value);
        }
    }
    return active;
}

math::float2 ImGuiWidgetDrawer::calculate_CheckBoxSize(Widget* wiget, Widget::State* layoutBaseState, Widget::State* state)
{
    ASSERT(ImGui::GetCurrentContext(), "must be valid");
    ImGuiStyle& style = ImGui::GetStyle();

    WidgetCheckBox::StateCheckBox* cbCtx = static_cast<WidgetCheckBox::StateCheckBox*>(state);
    WidgetLayout::StateLayoutBase* layoutCtx = static_cast<WidgetLayout::StateLayoutBase*>(layoutBaseState);

    ImVec2 alignmentSize{ 0.f, 0.f };
    ASSERT(layoutCtx->_fontSize < m_widgetHandler->m_fonts.size(), "range out");
    ImGui::PushFont(m_widgetHandler->m_fonts[layoutCtx->_fontSize]);
    alignmentSize = ImGui::CalcTextSize(cbCtx->_text.c_str());
    alignmentSize.x += style.FramePadding.x * 2.0f;
    alignmentSize.y += style.FramePadding.y * 2.0f;
    ImGui::PopFont();

    return { alignmentSize.x, alignmentSize.y };
}

bool ImGuiWidgetDrawer::draw_RadioButtonGroup(Widget* wiget, Widget* baseWidget, Widget::State* layoutBaseState, Widget::State* state)
{
    ASSERT(ImGui::GetCurrentContext(), "must be valid");
    WidgetRadioButtonGroup::StateRadioButtonGroup* rbCtx = static_cast<WidgetRadioButtonGroup::StateRadioButtonGroup*>(state);
    WidgetLayout::StateLayoutBase* layoutCtx = static_cast<WidgetLayout::StateLayoutBase*>(layoutBaseState);

    bool active = false;
    if (!rbCtx->_list.empty())
    {
        setupHorizontalAligment(layoutCtx, 0.f, rbCtx->_itemRect.getWidth());

        ASSERT(rbCtx->_activeIndex < rbCtx->_list.size(), "range out");
        s32 index = rbCtx->_activeIndex;
        for (u32 i = 0; i < rbCtx->_list.size() - 1; ++i)
        {
            active |= ImGui::RadioButton(rbCtx->_list[i].c_str(), &index, i);
            ImGui::SameLine();
        }
        active |= ImGui::RadioButton(rbCtx->_list.back().c_str(), &index, rbCtx->_list.size() - 1);

        if (rbCtx->_showToolTip && ImGui::IsItemHovered(ImGuiHoveredFlags_ForTooltip))
        {
            ImGui::SetTooltip(rbCtx->_toolTip.c_str());
        }

        if (rbCtx->_activeIndex != index)
        {
            rbCtx->_activeIndex = index;
            if (rbCtx->_onChangedIndexEvent)
            {
                std::invoke(rbCtx->_onChangedIndexEvent, wiget, index);
            }
        }
    }
    return active;
}

math::float2 ImGuiWidgetDrawer::calculate_RadioButtonGroupSize(Widget* wiget, Widget::State* layoutBaseState, Widget::State* state)
{
    ASSERT(ImGui::GetCurrentContext(), "must be valid");
    ImGuiStyle& style = ImGui::GetStyle();

    WidgetRadioButtonGroup::StateRadioButtonGroup* rbCtx = static_cast<WidgetRadioButtonGroup::StateRadioButtonGroup*>(state);
    WidgetLayout::StateLayoutBase* layoutCtx = static_cast<WidgetLayout::StateLayoutBase*>(layoutBaseState);

    ImVec2 alignmentSize{ 0.f, 0.f };
    ASSERT(layoutCtx->_fontSize < m_widgetHandler->m_fonts.size(), "range out");
    ImGui::PushFont(m_widgetHandler->m_fonts[layoutCtx->_fontSize]);
    for (auto& item : rbCtx->_list)
    {
        ImVec2 alignmentStr = ImGui::CalcTextSize(item.c_str());
        alignmentSize.x += alignmentStr.x;
    }
    ImGui::PopFont();

    alignmentSize.x += style.FramePadding.x * 2.0f;
    alignmentSize.y += style.FramePadding.y * 2.0f;

    return { alignmentSize.x, alignmentSize.y };
}

bool ImGuiWidgetDrawer::draw_ComboBox(Widget* wiget, Widget* baseWidget, Widget::State* layout, Widget::State* state)
{
    ASSERT(ImGui::GetCurrentContext(), "must be valid");
    WidgetComboBox::StateComboBox* cbCtx = static_cast<WidgetComboBox::StateComboBox*>(state);

    static auto items_ArrayGetter = [](void* user_data, int idx) -> const char*
        {
            std::vector<std::string>& list = *reinterpret_cast<std::vector<std::string>*>(user_data);
            return list[idx].c_str();
        };

    bool active = false;
    if (!cbCtx->_list.empty())
    {
        s32 index = cbCtx->_activeIndex;
        ImGui::PushID(cbCtx->_uid);
        active = ImGui::Combo("", &index, items_ArrayGetter, &cbCtx->_list, cbCtx->_list.size());
        ImGui::PopID();

        if (cbCtx->_showToolTip && ImGui::IsItemHovered(ImGuiHoveredFlags_ForTooltip))
        {
            ImGui::SetTooltip(cbCtx->_toolTip.c_str());
        }

        if (cbCtx->_activeIndex != index)
        {
            cbCtx->_activeIndex = index;
            if (cbCtx->_onChangedIndexEvent)
            {
                std::invoke(cbCtx->_onChangedIndexEvent, wiget, index);
            }
        }
    }
    return active;
}

math::float2 ImGuiWidgetDrawer::calculate_ComboBoxSize(Widget* wiget, Widget::State* layoutBaseState, Widget::State* state)
{
    //TODO
    return {};
}

bool ImGuiWidgetDrawer::draw_ListBox(Widget* wiget, Widget* base, Widget::State* layoutBaseState, Widget::State* state)
{
    ASSERT(ImGui::GetCurrentContext(), "must be valid");
    WidgetListBox::StateListBox* lbCtx = static_cast<WidgetListBox::StateListBox*>(state);

    static auto items_ArrayGetter = [](void* user_data, int idx) -> const char*
        {
            std::vector<std::string>& list = *reinterpret_cast<std::vector<std::string>*>(user_data);
            return list[idx].c_str();
        };

    bool active = false;
    if (!lbCtx->_list.empty())
    {
        s32 index = lbCtx->_activeIndex;
        ImGui::PushID(lbCtx->_uid);
        active = ImGui::ListBox("", &index, items_ArrayGetter, &lbCtx->_list, lbCtx->_list.size(), 4);
        ImGui::PopID();

        if (lbCtx->_showToolTip && ImGui::IsItemHovered(ImGuiHoveredFlags_ForTooltip))
        {
            ImGui::SetTooltip(lbCtx->_toolTip.c_str());
        }

        if (lbCtx->_activeIndex != index)
        {
            lbCtx->_activeIndex = index;
            if (lbCtx->_onChangedIndexEvent)
            {
                std::invoke(lbCtx->_onChangedIndexEvent, wiget, index);
            }
        }
    }
    return active;
}

math::float2 ImGuiWidgetDrawer::calculate_ListBoxSize(Widget* wiget, Widget::State* layoutBaseState, Widget::State* state)
{
    //TODO
    return {};
}

bool ImGuiWidgetDrawer::draw_InputText(Widget* widget, Widget* baseWidget, Widget::State* layoutBaseState, Widget::State* state)
{
    ASSERT(ImGui::GetCurrentContext(), "must be valid");
    WidgetInputText::StateInputText* itCtx = static_cast<WidgetInputText::StateInputText*>(state);
    WidgetLayout::StateLayoutBase* layoutCtx = static_cast<WidgetLayout::StateLayoutBase*>(layoutBaseState);

    setupHorizontalAligment(layoutCtx, 0.f, itCtx->_itemRect.getWidth());

    ImGuiInputTextFlags flags = 0;

    const u32 buffSize = 1024;
    c8 value[buffSize];
    memcpy(value, itCtx->_value[0].c_str(), itCtx->_value[0].length());
    value[itCtx->_value[0].length()] = '\0';

    bool active = false;
    if (itCtx->_multiline)
    {
        ImVec2 size = { static_cast<f32>(itCtx->_size._width), static_cast<f32>(itCtx->_size._height) };
        ImGui::PushID(itCtx->_uid);
        active = ImGui::InputTextMultiline("", value, buffSize, size, flags);
        ImGui::PopID();
    }
    else
    {
        ImGui::PushID(itCtx->_uid);
        active = ImGui::InputText("", value, buffSize, flags);
        ImGui::PopID();
    }

    if (active && ImGui::IsItemEdited())
    {
        u32 strSize = strlen(value);
        if (itCtx->_onChangedValueEvent)
        {
            std::invoke(itCtx->_onChangedValueEvent, widget, std::string(value, strSize));
        }

        itCtx->_value[0].assign(value, strSize);
    }

    return active;
}

bool ImGuiWidgetDrawer::draw_InputValue(Widget* widget, Widget* baseWidget, Widget::State* layoutBaseState, Widget::State* state, bool isRealNumber, u32 array)
{
    ASSERT(ImGui::GetCurrentContext(), "must be valid");
    bool active = false;
    ImGuiInputTextFlags flags = 0;

    if (isRealNumber)
    {
        if (array == 1)
        {
            WidgetInputFloat::StateInputFloat* ifCtx = static_cast<WidgetInputFloat::StateInputFloat*>(state);
            WidgetLayout::StateLayoutBase* layoutCtx = static_cast<WidgetLayout::StateLayoutBase*>(layoutBaseState);

            setupHorizontalAligment(layoutCtx, 0.f, ifCtx->_itemRect.getWidth());

            u32 pushCount = 0;
            if (ifCtx->_stateMask & Widget::State::StateMask::Color)
            {
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4{ ifCtx->_textColor._x, ifCtx->_textColor._y, ifCtx->_textColor._z, ifCtx->_textColor._w });
                ++pushCount;
            }

            if (ifCtx->_stateMask & Widget::State::StateMask::BorderColor)
            {
                ImGui::PushStyleColor(ImGuiCol_Border, ImVec4{ ifCtx->_borderColor._x, ifCtx->_borderColor._y, ifCtx->_borderColor._z, ifCtx->_borderColor._w });
                ++pushCount;
            }

            if (ifCtx->_stateMask & Widget::State::StateMask::BackgroundColor)
            {
                ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4{ ifCtx->_backgroundColor._x, ifCtx->_backgroundColor._y, ifCtx->_backgroundColor._z, ifCtx->_backgroundColor._w });
                ++pushCount;
            }

            f32 value = ifCtx->_value[0];
            ImGui::PushID(ifCtx->_uid);
            active = ImGui::InputFloat("", &value);
            ImGui::PopID();

            if (pushCount > 0)
            {
                ImGui::PopStyleColor(pushCount);
            }

            if (active && ImGui::IsItemEdited())
            {
                if (ifCtx->_onChangedValueEvent)
                {
                    std::invoke(ifCtx->_onChangedValueEvent, widget, value);
                }
                ifCtx->_value[0] = value;
            }
        }
        else
        {
            ASSERT(array == 3, "supporting 3 elements");
            WidgetInputFloat3::StateInputFloat3* ifCtx = static_cast<WidgetInputFloat3::StateInputFloat3*>(state);
            WidgetLayout::StateLayoutBase* layoutCtx = static_cast<WidgetLayout::StateLayoutBase*>(layoutBaseState);

            setupHorizontalAligment(layoutCtx, 0.f, ifCtx->_itemRect.getWidth());

            u32 pushCount = 0;
            if (ifCtx->_stateMask & Widget::State::StateMask::Color)
            {
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4{ ifCtx->_textColor._x, ifCtx->_textColor._y, ifCtx->_textColor._z, ifCtx->_textColor._w });
                ++pushCount;
            }

            if (ifCtx->_stateMask & Widget::State::StateMask::BorderColor)
            {
                ImGui::PushStyleColor(ImGuiCol_Border, ImVec4{ ifCtx->_borderColor._x, ifCtx->_borderColor._y, ifCtx->_borderColor._z, ifCtx->_borderColor._w });
                ++pushCount;
            }

            if (ifCtx->_stateMask & Widget::State::StateMask::BackgroundColor)
            {
                ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4{ ifCtx->_backgroundColor._x, ifCtx->_backgroundColor._y, ifCtx->_backgroundColor._z, ifCtx->_backgroundColor._w });
                ++pushCount;
            }

            auto value = ifCtx->_value;
            ImGui::PushID(ifCtx->_uid);
            active = ImGui::InputScalarN("", ImGuiDataType_Float, value.data(), array, nullptr, nullptr, "%.3f", flags);
            ImGui::PopID();

            if (pushCount > 0)
            {
                ImGui::PopStyleColor(pushCount);
            }

            if (active && ImGui::IsItemEdited())
            {
                if (ifCtx->_onChangedValueEvent)
                {
                    const math::float3 val = { value[0], value[1], value[2] };
                    std::invoke(ifCtx->_onChangedValueEvent, widget, val);
                }
                ifCtx->_value = value;
            }
        }
    }
    else
    {
        if (array == 1)
        {
            WidgetInputInt::StateInputInt* iiCtx = static_cast<WidgetInputInt::StateInputInt*>(state);
            WidgetLayout::StateLayoutBase* layoutCtx = static_cast<WidgetLayout::StateLayoutBase*>(layoutBaseState);

            u32 pushCount = 0;
            if (iiCtx->_stateMask & Widget::State::StateMask::Color)
            {
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4{ iiCtx->_textColor._x, iiCtx->_textColor._y, iiCtx->_textColor._z, iiCtx->_textColor._w });
                ++pushCount;
            }

            if (iiCtx->_stateMask & Widget::State::StateMask::BorderColor)
            {
                ImGui::PushStyleColor(ImGuiCol_Border, ImVec4{ iiCtx->_borderColor._x, iiCtx->_borderColor._y, iiCtx->_borderColor._z, iiCtx->_borderColor._w });
                ++pushCount;
            }

            if (iiCtx->_stateMask & Widget::State::StateMask::BackgroundColor)
            {
                ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4{ iiCtx->_backgroundColor._x, iiCtx->_backgroundColor._y, iiCtx->_backgroundColor._z, iiCtx->_backgroundColor._w });
                ++pushCount;
            }

            setupHorizontalAligment(layoutCtx, 0.f, iiCtx->_itemRect.getWidth());

            auto value = iiCtx->_value;
            ImGui::PushID(iiCtx->_uid);
            active = ImGui::InputInt("", &value[0], 1, 100, flags);
            ImGui::PopID();

            if (pushCount > 0)
            {
                ImGui::PopStyleColor(pushCount);
            }

            if (active && ImGui::IsItemEdited())
            {
                if (iiCtx->_onChangedValueEvent)
                {
                    std::invoke(iiCtx->_onChangedValueEvent, widget, value[0]);
                }

                iiCtx->_value = value;
            }
        }
        else
        {
            ASSERT(false, "impl");
            //TODO

            WidgetInputInt::StateInputInt* iiCtx = static_cast<WidgetInputInt::StateInputInt*>(state);
            WidgetLayout::StateLayoutBase* layoutCtx = static_cast<WidgetLayout::StateLayoutBase*>(layoutBaseState);

            auto value = iiCtx->_value;
            ImGui::PushID(iiCtx->_uid);
            active = ImGui::InputScalarN("", ImGuiDataType_S32, value.data(), array, nullptr, nullptr, "%d", flags);
            ImGui::PopID();
        }
    }

    return active;
}

bool ImGuiWidgetDrawer::draw_InputSliderValue(Widget* widget, Widget* baseWidget, Widget::State* layoutBaseState, Widget::State* state, bool isRealNumber)
{
    ASSERT(ImGui::GetCurrentContext(), "must be valid");
    WidgetInputSlider::StateInputSlider* isCtx = static_cast<WidgetInputSlider::StateInputSlider*>(state);
    WidgetLayout::StateLayoutBase* layoutCtx = static_cast<WidgetLayout::StateLayoutBase*>(layoutBaseState);

    setupHorizontalAligment(layoutCtx, 0.f, isCtx->_itemRect.getWidth());

    //type & dim
    static int i1 = 0;
    ImGui::SliderInt("slider test", &i1, -1, 3);

    return false;
}


bool ImGuiWidgetDrawer::draw_InputDragValue(Widget* widget, Widget* baseWidget, Widget::State* layoutBaseState, Widget::State* state, bool isRealNumber)
{
    ASSERT(ImGui::GetCurrentContext(), "must be valid");

    bool active = false;
    if (isRealNumber)
    {
        WidgetInputDragFloat::StateInputDragFloat* idCtx = static_cast<WidgetInputDragFloat::StateInputDragFloat*>(state);
        WidgetLayout::StateLayoutBase* layoutCtx = static_cast<WidgetLayout::StateLayoutBase*>(layoutBaseState);

        setupHorizontalAligment(layoutCtx, 0.f, idCtx->_itemRect.getWidth());

        u32 pushCount = 0;
        if (idCtx->_stateMask & Widget::State::StateMask::Color)
        {
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4{ idCtx->_textColor._x, idCtx->_textColor._y, idCtx->_textColor._z, idCtx->_textColor._w });
            ++pushCount;
        }

        if (idCtx->_stateMask & Widget::State::StateMask::BorderColor)
        {
            ImGui::PushStyleColor(ImGuiCol_Border, ImVec4{ idCtx->_borderColor._x, idCtx->_borderColor._y, idCtx->_borderColor._z, idCtx->_borderColor._w });
            ++pushCount;
        }

        if (idCtx->_stateMask & Widget::State::StateMask::BackgroundColor)
        {
            ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4{ idCtx->_backgroundColor._x, idCtx->_backgroundColor._y, idCtx->_backgroundColor._z, idCtx->_backgroundColor._w });
            ++pushCount;
        }

        f32 value = idCtx->_value[0];
        f32 step = idCtx->_step;
        ImGui::PushID(idCtx->_uid);
        if (idCtx->_stateMask & Widget::State::StateMask::DefindedSize)
        {
            ImGui::SetNextItemWidth(static_cast<f32>(idCtx->_size._width));
        }
        active = ImGui::DragFloat("", &value, step, idCtx->_range[0], idCtx->_range[1], "%.2f");
        ImGui::PopID();

        if (pushCount > 0)
        {
            ImGui::PopStyleColor(pushCount);
        }

        if (active && ImGui::IsItemEdited())
        {
            if (idCtx->_onChangedValueEvent)
            {
                std::invoke(idCtx->_onChangedValueEvent, widget, value);
            }

            idCtx->_value[0] = value;
        }
    }
    else
    {
        WidgetInputDragInt::StateInputDragInt* idCtx = static_cast<WidgetInputDragInt::StateInputDragInt*>(state);
        WidgetLayout::StateLayoutBase* layoutCtx = static_cast<WidgetLayout::StateLayoutBase*>(layoutBaseState);

        setupHorizontalAligment(layoutCtx, 0.f, idCtx->_itemRect.getWidth());

        u32 pushCount = 0;
        if (idCtx->_stateMask & Widget::State::StateMask::Color)
        {
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4{ idCtx->_textColor._x, idCtx->_textColor._y, idCtx->_textColor._z, idCtx->_textColor._w });
            ++pushCount;
        }

        if (idCtx->_stateMask & Widget::State::StateMask::BorderColor)
        {
            ImGui::PushStyleColor(ImGuiCol_Border, ImVec4{ idCtx->_borderColor._x, idCtx->_borderColor._y, idCtx->_borderColor._z, idCtx->_borderColor._w });
            ++pushCount;
        }

        if (idCtx->_stateMask & Widget::State::StateMask::BackgroundColor)
        {
            ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4{ idCtx->_backgroundColor._x, idCtx->_backgroundColor._y, idCtx->_backgroundColor._z, idCtx->_backgroundColor._w });
            ++pushCount;
        }

        s32 value = idCtx->_value[0];
        s32 step = idCtx->_step;
        ImGui::PushID(idCtx->_uid);
        active = ImGui::DragInt("", &value, step, idCtx->_range[0], idCtx->_range[1], "%d");
        ImGui::PopID();

        if (pushCount > 0)
        {
            ImGui::PopStyleColor(pushCount);
        }

        if (active && ImGui::IsItemEdited())
        {
            if (idCtx->_onChangedValueEvent)
            {
                std::invoke(idCtx->_onChangedValueEvent, widget, value);
            }
            idCtx->_value[0] = value;
        }
    }

    return active;
}

math::float2 ImGuiWidgetDrawer::calculate_InputDragValueSize(Widget* widget, Widget::State* layoutBaseState, Widget::State* state, bool isRealNumber)
{
    ASSERT(ImGui::GetCurrentContext(), "must be valid");

    ImVec2 alignmentSize{ 0.f, 0.f };
    if (isRealNumber)
    {
        WidgetInputDragFloat::StateInputDragFloat* idCtx = static_cast<WidgetInputDragFloat::StateInputDragFloat*>(state);
        WidgetLayout::StateLayoutBase* layoutCtx = static_cast<WidgetLayout::StateLayoutBase*>(layoutBaseState);

        if (idCtx->_stateMask & Widget::State::StateMask::DefindedSize)
        {
            alignmentSize = { static_cast<f32>(idCtx->_size._width), static_cast<f32>(idCtx->_size._height) };
        }
        else
        {
            ASSERT(layoutCtx->_fontSize < m_widgetHandler->m_fonts.size(), "range out");
            ImGui::PushFont(m_widgetHandler->m_fonts[layoutCtx->_fontSize]);
            alignmentSize = ImGui::CalcTextSize(std::format("{:.2f}", idCtx->_value[0]).c_str());
            ImGui::PopFont();
        }
    }
    else
    {
        WidgetInputDragInt::StateInputDragInt* idCtx = static_cast<WidgetInputDragInt::StateInputDragInt*>(state);
        WidgetLayout::StateLayoutBase* layoutCtx = static_cast<WidgetLayout::StateLayoutBase*>(layoutBaseState);

        if (idCtx->_stateMask & Widget::State::StateMask::DefindedSize)
        {
            alignmentSize = { static_cast<f32>(idCtx->_size._width), static_cast<f32>(idCtx->_size._height) };
        }
        else
        {
            ASSERT(layoutCtx->_fontSize < m_widgetHandler->m_fonts.size(), "range out");
            ImGui::PushFont(m_widgetHandler->m_fonts[layoutCtx->_fontSize]);
            alignmentSize = ImGui::CalcTextSize(std::to_string(idCtx->_value[0]).c_str());
            ImGui::PopFont();
        }
    }

    return { alignmentSize.x, alignmentSize.y };
}

bool ImGuiWidgetDrawer::draw_ColorPalette(Widget* widget, Widget* baseWidget, Widget::State* layoutBaseState, Widget::State* state)
{
    ASSERT(ImGui::GetCurrentContext(), "must be valid");
    WidgetColorPalette::StateColorPalette* cpCtx = static_cast<WidgetColorPalette::StateColorPalette*>(state);
    WidgetLayout::StateLayoutBase* layoutCtx = static_cast<WidgetLayout::StateLayoutBase*>(layoutBaseState);

    setupHorizontalAligment(layoutCtx, 0.f, cpCtx->_itemRect.getWidth());

    color::ColorRGBAF color = cpCtx->_color;

    ImGui::PushID(cpCtx->_uid);
    bool active = ImGui::ColorEdit4("", &color[0]);
    ImGui::PopID();

    if (active && ImGui::IsItemEdited())
    {
        if (cpCtx->_onColorChanged)
        {
            std::invoke(cpCtx->_onColorChanged, widget, color);
        }

        cpCtx->_color = color;
    }

    return active;
}

math::float2 ImGuiWidgetDrawer::calculate_ColorPaletteSize(Widget* widget, Widget::State* layoutBaseState, Widget::State* state)
{
    ASSERT(ImGui::GetCurrentContext(), "must be valid");
    WidgetColorPalette::StateColorPalette* cpCtx = static_cast<WidgetColorPalette::StateColorPalette*>(state);
    WidgetLayout::StateLayoutBase* layoutCtx = static_cast<WidgetLayout::StateLayoutBase*>(layoutBaseState);

    ASSERT(layoutCtx->_fontSize < m_widgetHandler->m_fonts.size(), "range out");
    ImGui::PushFont(m_widgetHandler->m_fonts[layoutCtx->_fontSize]);
    ImVec2 alignmentSize = ImGui::CalcTextSize(std::to_string(cpCtx->_color[0]).c_str());
    ImGui::PopFont();

    return { alignmentSize.x, alignmentSize.y };
}

bool ImGuiWidgetDrawer::draw_TreeNode(Widget* widget, Widget* base, Widget::State* layoutBaseState, Widget::State* state, f32 dt)
{
    ASSERT(ImGui::GetCurrentContext(), "must be valid");
    WidgetTreeNode::StateTreeNode* tn = static_cast<WidgetTreeNode::StateTreeNode*>(state);

    ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_None;
    if (tn->_createFlags & WidgetTreeNode::TreeNodeFlag::Framed)
    {
        flags |= ImGuiTreeNodeFlags_Framed;
    }

    if (tn->_createFlags & WidgetTreeNode::TreeNodeFlag::Open)
    {
        flags |= ImGuiTreeNodeFlags_DefaultOpen;
    }

    if (tn->_createFlags & WidgetTreeNode::TreeNodeFlag::NoCollapsed)
    {
        flags |= ImGuiTreeNodeFlags_Leaf;
    }

    if (tn->_isSelected)
    {
        flags |= ImGuiTreeNodeFlags_Selected;
    }

    ImGui::PushID(tn->_uid);
    bool active = ImGui::TreeNodeEx(tn->_text.c_str(), flags);
    if (active)
    {
        tn->_stateMask &= ~Widget::State::StateMask::CollapsedState;
        tn->_layout.update(m_widgetHandler, base, &tn->_layout, dt);
        ImGui::TreePop();
    }
    else
    {
        tn->_stateMask |= Widget::State::StateMask::CollapsedState;
    }
    ImGui::PopID();

    if (tn->_showToolTip && ImGui::IsItemHovered(ImGuiHoveredFlags_ForTooltip))
    {
        ImGui::SetTooltip(tn->_toolTip.c_str());
    }

    if (ImGui::IsItemClicked())
    {
        tn->_isSelected = true;
        if (tn->_onClickEvent)
        {
            std::invoke(tn->_onClickEvent, widget, tn->_index);
        }
    }

    return true;
}

struct Folder {
    std::string name;
    std::string path;
    std::vector<Folder> subfolders;
};

struct Asset {
    std::string name;
    std::string path;
    ImTextureID iconImGuiID; // Thumbnail loaded as ImGui texture
};

void RenderAssetList()
{
    //float thumbnailSize = 64.0f;
    //float padding = 8.0f;
    //float cellSize = thumbnailSize + padding;

    //float panelWidth = ImGui::GetContentRegionAvail().x;
    //int columnCount = (int)(panelWidth / cellSize);
    //if (columnCount < 1) columnCount = 1;

    //ImGui::Columns(columnCount, 0, false);

    //for (const auto& asset : GetAssetsInFolder(selectedFolder)) {
    //    if (!StrContains(asset.name, searchBuffer)) continue;

    //    ImGui::BeginGroup();

    //    ImGui::Image(asset.iconImGuiID, ImVec2(thumbnailSize, thumbnailSize));
    //    if (ImGui::IsItemHovered())
    //        ImGui::SetTooltip(asset.name.c_str());

    //    if (ImGui::IsItemClicked()) {
    //        selectedAsset = asset;
    //    }

    //    ImGui::TextWrapped(asset.name.c_str());

    //    ImGui::EndGroup();
    //    ImGui::NextColumn();
    //}

    //ImGui::Columns(1);
}

void RenderFolderTreeRecursive(const Folder& folder) 
{
    ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow;
    bool open = ImGui::TreeNodeEx(folder.name.c_str(), flags);

    if (ImGui::IsItemClicked()) {
        //selectedFolder = folder.path;
    }

    if (open) {
        for (auto& subFolder : folder.subfolders) {
            RenderFolderTreeRecursive(subFolder);
        }
        ImGui::TreePop();
    }
}

void RenderFolderTree() 
{
    Folder folder;
    folder.name = "root";
    folder.subfolders.push_back({ "folder0", "", {} });
    folder.subfolders.push_back({ "folder1", "", {} });
    folder.subfolders.push_back({ "folder2", "", {} });
    folder.subfolders.push_back({ "folder3", "", {} });

    RenderFolderTreeRecursive(folder);
}

math::float2 ImGuiWidgetDrawer::calculate_TreeNodeSize(Widget* widget, Widget::State* layoutBaseState, Widget::State* state)
{
    ASSERT(ImGui::GetCurrentContext(), "must be valid");
    WidgetTreeNode::StateTreeNode* tn = static_cast<WidgetTreeNode::StateTreeNode*>(state);
    WidgetLayout::StateLayoutBase* layoutCtx = static_cast<WidgetLayout::StateLayoutBase*>(layoutBaseState);

    ASSERT(layoutCtx->_fontSize < m_widgetHandler->m_fonts.size(), "range out");
    ImGui::PushFont(m_widgetHandler->m_fonts[layoutCtx->_fontSize]);
    ImVec2 alignmentSize = ImGui::CalcTextSize(tn->_text.c_str());
    ImGui::PopFont();

    return { alignmentSize.x, alignmentSize.y };
}

bool ImGuiWidgetDrawer::draw_Table(Widget* widget, Widget* base, Widget::State* layout, Widget::State* state)
{
    return false;
}

bool ImGuiWidgetDrawer::draw_TEST(Widget* widget, Widget* baseWidget, Widget::State* layoutBaseState, Widget::State* state)
{
    ASSERT(ImGui::GetCurrentContext(), "must be valid");
    WidgetTEST::StateTEST* ctx = static_cast<WidgetTEST::StateTEST*>(state);
    WidgetLayout::StateLayoutBase* layoutCtx = static_cast<WidgetLayout::StateLayoutBase*>(layoutBaseState);
    setupHorizontalAligment(layoutCtx, 0.f, ctx->_itemRect.getWidth());

        static float leftPanelWidth = 200.0f;
       static float splitterThickness = 4.0f;
   
       ImVec2 contentSize = ImGui::GetContentRegionAvail();
   
       // Left panel
       ImGui::BeginChild("LeftPanel", ImVec2(leftPanelWidth, 0), true);
       RenderFolderTree();
       ImGui::EndChild();
   
       ImGui::SameLine();
   
       //// Splitter
       //ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.4f, 0.4f, 0.4f, 1.0f));
       //ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.6f, 0.6f, 0.6f, 1.0f));
       //ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.8f, 0.8f, 0.8f, 1.0f));
       //ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
       //ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
   
       //ImGui::InvisibleButton("Splitter", ImVec2(splitterThickness, std::max(1.f, contentSize.y)), ImGuiButtonFlags_MouseButtonLeft);
       //if (ImGui::IsItemActive()) {
       //    leftPanelWidth += ImGui::GetIO().MouseDelta.x;
       //    leftPanelWidth = ImClamp(leftPanelWidth, 100.0f, contentSize.x - 100.0f); // Optional clamping
       //}
   
       //ImGui::PopStyleVar(2);
       //ImGui::PopStyleColor(3);
   
       ImGui::SameLine();
   
       // Right panel
       ImGui::BeginChild("RightPanel", ImVec2(0, 0), true);
       RenderAssetList();
       ImGui::EndChild();
   
       return false;
}

bool ImGuiWidgetDrawer::draw_MenuBar(Widget* menubar, Widget::State* state, f32 dt)
{
    ASSERT(ImGui::GetCurrentContext(), "must be valid");
    WidgetMenuBar::StateMenuBar* menuBarCtx = static_cast<WidgetMenuBar::StateMenuBar*>(state);

    bool action = ImGui::BeginMainMenuBar();
    if (action)
    {
        menuBarCtx->_layout.update(m_widgetHandler, menubar, &menuBarCtx->_layout, dt);
        ImGui::EndMainMenuBar();
    }

    return action;
}

math::float2 ImGuiWidgetDrawer::calculate_TableSize(Widget* widget, Widget::State* layout, Widget::State* state)
{
    return math::float2();
}

bool ImGuiWidgetDrawer::draw_Menu(Widget* menu, Widget* base, Widget::State* layout, Widget::State* state, f32 dt)
{
    ASSERT(ImGui::GetCurrentContext(), "must be valid");
    WidgetMenu::StateMenu* menuCtx = static_cast<WidgetMenu::StateMenu*>(state);
    ASSERT(!menuCtx->_text.empty(), "must be filled");

    bool action = ImGui::BeginMenu(menuCtx->_text.c_str(), menuCtx->_isActive);
    if (action)
    {
        if (menuCtx->_showToolTip && ImGui::IsItemHovered(ImGuiHoveredFlags_ForTooltip))
        {
            ImGui::SetTooltip(menuCtx->_toolTip.c_str());
        }

        if (menuCtx->_onClickedEvent && ImGui::IsItemClicked())
        {
            std::invoke(menuCtx->_onClickedEvent, menu);
        }

        menuCtx->_layout.update(m_widgetHandler, base, &menuCtx->_layout, dt);
        ImGui::EndMenu();
    }

    return action;
}

bool ImGuiWidgetDrawer::draw_MenuItem(Widget* item, Widget* base, Widget::State* layout, Widget::State* state)
{
    ASSERT(ImGui::GetCurrentContext(), "must be valid");
    WidgetMenuItem::StateMenuItem* itemCtx = static_cast<WidgetMenuItem::StateMenuItem*>(state);

    bool clicked = false;
    bool action = ImGui::MenuItem(itemCtx->_text.c_str(), nullptr, &clicked, itemCtx->_isActive);
    if (itemCtx->_onClickedEvent && clicked)
    {
        std::invoke(itemCtx->_onClickedEvent, item);
    }

    return action;
}

bool ImGuiWidgetDrawer::draw_TabBar(Widget* item, Widget::State* state, f32 dt)
{
    ImGuiTabBarFlags tab_bar_flags = ImGuiTabBarFlags_AutoSelectNewTabs | ImGuiTabBarFlags_Reorderable | ImGuiTabBarFlags_FittingPolicyResizeDown;
    if (ImGui::BeginTabBar("MyTabBar", tab_bar_flags))
    {
        if (ImGui::BeginTabItem("Avocado"))
        {
            ImGui::Text("This is the Avocado tab!\nblah blah blah blah blah");
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Broccoli"))
        {
            ImGui::Text("This is the Broccoli tab!\nblah blah blah blah blah");
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Cucumber"))
        {
            ImGui::Text("This is the Cucumber tab!\nblah blah blah blah blah");
            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }

    return true;
    //ASSERT(ImGui::GetCurrentContext(), "must be valid");
    //WidgetTabBar::StateTabBar* barCtx = static_cast<WidgetTabBar::StateTabBar*>(state);

    //static ImGuiTabBarFlags tab_bar_flags = 0;//ImGuiTabBarFlags_AutoSelectNewTabs | ImGuiTabBarFlags_Reorderable | ImGuiTabBarFlags_FittingPolicyResizeDown;
    //bool action = ImGui::BeginTabBar("Bar", tab_bar_flags);
    //if (action)
    //{
    //    barCtx->_layout.update(m_wigetHandler, item, &barCtx->_layout, dt);
    //    ImGui::EndTabBar();
    //}

    //return action;
}

bool ImGuiWidgetDrawer::draw_TabItem(Widget* wiget, Widget* base, Widget::State* layout, Widget::State* state)
{
    //ASSERT(ImGui::GetCurrentContext(), "must be valid");
    //WidgetTabItem::StateTabItem* tabCtx = static_cast<WidgetTabItem::StateTabItem*>(state);

    //bool clicked = false;
    //bool action = ImGui::BeginTabItem(tabCtx->_text.c_str(), &clicked);

    //if (tabCtx->_onClickedEvent && clicked)
    //{
    //    std::invoke(tabCtx->_onClickedEvent, wiget);
    //}

    //tabCtx->_layout.update(m_wigetHandler, wiget, &tabCtx->_layout, 0);
    //ImGui::EndTabItem();

    return false;
}

void ImGuiWidgetDrawer::draw_BeginLayoutState(Widget* layout, Widget* base, Widget::State* baseLayoutState, Widget::State* state, const math::float2& size)
{
    ASSERT(ImGui::GetCurrentContext(), "must be valid");
    WidgetLayout::StateLayoutBase* layoutCtx = static_cast<WidgetLayout::StateLayoutBase*>(state);
    WidgetLayout::StateLayoutBase* baseLayoutCtx = static_cast<WidgetLayout::StateLayoutBase*>(baseLayoutState);

    if (!(layoutCtx->_stateMask & Widget::State::StateMask::MainLayout))
    {
        ImGuiChildFlags child_flags = 0;
        ImGuiWindowFlags window_flags = 0;

        ImVec2 layoutSize = { 0, 0 };
        if (baseLayoutCtx->_stateMask & Widget::State::StateMask::DefindedSize)
        {
            layoutSize = { (f32)layoutCtx->_size._width, (f32)layoutCtx->_size._height };
        }
        else
        {
            layoutSize = { size._x, size._y };
        }

        if (layoutCtx->_aligmentH == WidgetLayout::HorizontalAlignment::AlignmentFill)
        {
            layoutSize.x = 0.f;
        }

        if (layoutCtx->_aligmentV == WidgetLayout::VerticalAlignment::AlignmentFill)
        {
            layoutSize.y = 0.f;
        }

        if (layoutCtx->_flags & WidgetLayout::LayoutFlag::Border)
        {
            child_flags |= ImGuiChildFlags_Borders;
        }

        if (baseLayoutCtx->_stateMask & Widget::State::StateMask::HorizontalLine)
        {
            ImGui::SameLine();
        }

        ImGui::BeginChild(layoutCtx->_uid, layoutSize, child_flags, window_flags);
        layoutCtx->_stateMask |= Widget::State::StateMask::ChildLayoutState;
    }

    layoutCtx->_cachedLayoutRect = { ImGui::GetCursorScreenPos().x, ImGui::GetCursorScreenPos().y, ImGui::GetContentRegionAvail().x + ImGui::GetCursorScreenPos().x, ImGui::GetContentRegionAvail().y + ImGui::GetCursorScreenPos().y };
    layoutCtx->_cachedLayoutOffest = get_LayoutPadding();

    ASSERT(layoutCtx->_fontSize < m_widgetHandler->m_fonts.size(), "range out");
    ImGui::PushFont(m_widgetHandler->m_fonts[layoutCtx->_fontSize]);
}

void ImGuiWidgetDrawer::draw_EndLayoutState(Widget* layout, Widget* base, Widget::State* baseLayoutState, Widget::State* state, const math::float2& size)
{
    ASSERT(ImGui::GetCurrentContext(), "must be valid");
    WidgetLayout::StateLayoutBase* layoutCtx = static_cast<WidgetLayout::StateLayoutBase*>(state);
    WidgetLayout::StateLayoutBase* baseLayoutCtx = static_cast<WidgetLayout::StateLayoutBase*>(baseLayoutState);

    ImGui::PopFont();

    if (layoutCtx->_stateMask & Widget::State::StateMask::ChildLayoutState)
    {
        ImGui::EndChild();
        layoutCtx->_stateMask &= ~Widget::State::StateMask::ChildLayoutState;

        if (baseLayoutCtx->_stateMask & Widget::State::StateMask::HorizontalLine)
        {
            ImGui::SameLine();
        }
    }

    layoutCtx->_cachedLayoutRect = {};
    layoutCtx->_cachedLayoutOffest = {};
}

bool ImGuiWidgetDrawer::draw_Separator(Widget* widget, Widget* base, Widget::State* layout, Widget::State* state)
{
    //bool h = false;
    //if (h)
    //{
    //    ImGui::SameLine();
    //}

    //// Splitter
    //ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.4f, 0.4f, 0.4f, 1.0f));
    //ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.6f, 0.6f, 0.6f, 1.0f));
    //ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.8f, 0.8f, 0.8f, 1.0f));
    //ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
    //ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));

    //ImGui::InvisibleButton("Splitter", ImVec2(splitterThickness, std::max(1.f, contentSize.y)), ImGuiButtonFlags_MouseButtonLeft);
    //if (ImGui::IsItemActive()) {
    //    leftPanelWidth += ImGui::GetIO().MouseDelta.x;
    //    leftPanelWidth = ImClamp(leftPanelWidth, 100.0f, contentSize.x - 100.0f); // Optional clamping
    //}

    //ImGui::PopStyleVar(2);
    //ImGui::PopStyleColor(3);

    //if (h)
    //{
    //    ImGui::SameLine();
    //}

    return false;
}

void ImGuiWidgetDrawer::draw_Gizmo(Widget* wiget, Widget* base, Widget::State* layout, Widget::State* state)
{
    ASSERT(ImGui::GetCurrentContext(), "must be valid");
    WidgetGizmo::StateGizmo* gizmoCtx = static_cast<WidgetGizmo::StateGizmo*>(state);
    WidgetLayout::StateLayoutBase* layoutCtx = static_cast<WidgetLayout::StateLayoutBase*>(layout);

    if (gizmoCtx->_isActive && gizmoCtx->_camera)
    {
        const scene::Camera& camera = *gizmoCtx->_camera;
        math::Matrix4D transform = gizmoCtx->_transform;

        static auto convertOp = [](WidgetGizmo::Operation op) -> ImGuizmo::OPERATION
            {
                switch (op)
                {
                case WidgetGizmo::Operation::Translate:
                    return ImGuizmo::TRANSLATE;
                case WidgetGizmo::Operation::Rotate:
                    return  ImGuizmo::ROTATE;
                case WidgetGizmo::Operation::Scale:
                    return ImGuizmo::SCALE;
                default: break;
                }

                return ImGuizmo::UNIVERSAL;
            };

        ImGuizmo::OPERATION gizmoOp = convertOp(gizmoCtx->_operation);
        ImGuizmo::MODE gizmoMode = ImGuizmo::WORLD;
        if (gizmoOp != ImGuizmo::SCALE)
        {
            gizmoMode = (gizmoCtx->_mode == WidgetGizmo::Mode::Local) ? ImGuizmo::LOCAL : ImGuizmo::WORLD;
        }

        ImGuizmo::SetDrawlist();
        ImGuizmo::SetRect(layoutCtx->_cachedLayoutRect.getLeftX(), layoutCtx->_cachedLayoutRect.getTopY(), layoutCtx->_cachedLayoutRect.getWidth(), layoutCtx->_cachedLayoutRect.getHeight());

        f32 rawViewMatrix[16] = {};
        f32 rawProjectionMatrix[16] = {};
        f32 rawTransformMatrix[16] = {};
        camera.getViewMatrix().get(rawViewMatrix);
        camera.getProjectionMatrix().get(rawProjectionMatrix);
        transform.get(rawTransformMatrix);

        ImGuizmo::Manipulate(rawViewMatrix, rawProjectionMatrix, gizmoOp, gizmoMode, rawTransformMatrix, nullptr, nullptr);

        if (ImGuizmo::IsOver() && gizmoCtx->_onHoveredEvent)
        {
            std::invoke(gizmoCtx->_onHoveredEvent, wiget);
        }

        if (ImGuizmo::IsUsing() && gizmoCtx->_onTransformChangedEvent)
        {
            math::Matrix4D matrix;
            matrix.set(rawTransformMatrix);

            std::invoke(gizmoCtx->_onTransformChangedEvent, wiget, base, matrix);
        }
    }
}

void ImGuiWidgetDrawer::draw_ViewManipulator(Widget* wiget, Widget* base, Widget::State* layout, Widget::State* state)
{
    ASSERT(ImGui::GetCurrentContext(), "must be valid");
    WidgetViewManipulator::StateViewManipulator* viewCtx = static_cast<WidgetViewManipulator::StateViewManipulator*>(state);
    WidgetLayout::StateLayoutBase* layoutCtx = static_cast<WidgetLayout::StateLayoutBase*>(layout);

    if (viewCtx->_isActive && viewCtx->_camera)
    {
        const scene::Camera& camera = *viewCtx->_camera;

        float viewManipulateRight = layoutCtx->_cachedLayoutRect.getLeftX() + layoutCtx->_cachedLayoutRect.getWidth();
        float viewManipulateTop = layoutCtx->_cachedLayoutRect.getTopY();

        ImGui::PushStyleColor(ImGuiCol_WindowBg, (ImVec4)ImColor(0.35f, 0.3f, 0.3f));

        ImGuizmo::SetDrawlist();
        ImGuizmo::SetRect(layoutCtx->_cachedLayoutRect.getLeftX(), layoutCtx->_cachedLayoutRect.getTopY(), layoutCtx->_cachedLayoutRect.getWidth(), layoutCtx->_cachedLayoutRect.getHeight());

        f32 rawViewMatrix[16] = {};
        f32 rawProjectionMatrix[16] = {};
        camera.getViewMatrix().get(rawViewMatrix);
        camera.getProjectionMatrix().get(rawProjectionMatrix);

        const bool showGrid = false;
        if (showGrid)
        {
            f32 rawIdentityMatrix[16] = {};
            math::Matrix4D identityMatrix;
            identityMatrix.makeIdentity();
            identityMatrix.get(rawIdentityMatrix);

            ImGuizmo::DrawGrid(rawViewMatrix, rawProjectionMatrix, rawIdentityMatrix, 100.f);
        }
        ImGuizmo::ViewManipulate(rawViewMatrix, 8.f, ImVec2(viewManipulateRight - viewCtx->_size, viewManipulateTop), ImVec2(viewCtx->_size, viewCtx->_size), 0x10101010);

        ImGui::PopStyleColor(1);

        if (ImGuizmo::IsViewManipulateHovered() && viewCtx->_onHoveredEvent)
        {
            std::invoke(viewCtx->_onHoveredEvent, wiget);
        }

        if (ImGuizmo::IsUsingViewManipulate() && viewCtx->_onViewChangedEvent)
        {
            math::Matrix4D viewMatrix;
            viewMatrix.set(rawViewMatrix);

            std::invoke(viewCtx->_onViewChangedEvent, wiget, base, viewMatrix);
        }
    }
}

math::float2 ImGuiWidgetDrawer::get_WindowPadding() const
{
    ASSERT(ImGui::GetCurrentContext(), "must be valid");
    ImGuiStyle& style = ImGui::GetStyle();

    return { style.WindowPadding.x, style.WindowPadding.y };
}

math::float2 ImGuiWidgetDrawer::get_LayoutPadding() const
{
    ASSERT(ImGui::GetCurrentContext(), "must be valid");
    ImGuiStyle& style = ImGui::GetStyle();

    return { style.FramePadding.x, style.FramePadding.y };
}

math::float2 ImGuiWidgetDrawer::get_ItemSpacing() const
{
    ASSERT(ImGui::GetCurrentContext(), "must be valid");
    ImGuiStyle& style = ImGui::GetStyle();

    return { style.ItemSpacing.x, style.ItemSpacing.y };
}

f32 ImGuiWidgetDrawer::setupHorizontalAligment(Widget::State* layout, f32 originalWidth, f32 itemWidth)
{
    ImGuiStyle& style = ImGui::GetStyle();
    f32 width = originalWidth;

    WidgetLayout::StateLayoutBase* layoutCtx = static_cast<WidgetLayout::StateLayoutBase*>(layout);
    switch (layoutCtx->_aligmentH)
    {
    case WidgetLayout::HorizontalAlignment::AlignmentCenter:
    {
        ImGui::SetCursorPosX((layoutCtx->_cachedLayoutRect.getWidth() - itemWidth) * 0.5f);
        break;
    }
    case WidgetLayout::HorizontalAlignment::AlignmentRight:
    {
        f32 offset = (layoutCtx->_stateMask & Widget::State::StateMask::HorizontalLine) ? layoutCtx->_cachedLayoutOffest._x : style.WindowPadding.x;
        ImGui::SetCursorPosX(layoutCtx->_cachedLayoutRect.getWidth() - itemWidth - offset);
        break;
    }

    case WidgetLayout::HorizontalAlignment::AlignmentLeft:
    {
        f32 offset = (layoutCtx->_stateMask & Widget::State::StateMask::HorizontalLine) ? layoutCtx->_cachedLayoutOffest._x : style.WindowPadding.x;
        ImGui::SetCursorPosX(offset);
        layoutCtx->_cachedLayoutOffest._x += itemWidth + get_ItemSpacing()._x;
        break;
    }

    case WidgetLayout::HorizontalAlignment::AlignmentFill:
    {
        ImVec2 avail = ImGui::GetContentRegionAvail();
        width = layoutCtx->_cachedLayoutRect.getWidth();
        break;
    }

    default:
        break;
    }

    if (layoutCtx->_stateMask & Widget::State::StateMask::HorizontalLine)
    {
        ImGui::SameLine();
    }

    return width;
}

f32 ImGuiWidgetDrawer::setupVerticalAligment(Widget::State* layout, f32 originalHeight, f32 itemHeight)
{
    ImGuiStyle& style = ImGui::GetStyle();
    f32 height = originalHeight;

    WidgetLayout::StateLayoutBase* layoutCtx = static_cast<WidgetLayout::StateLayoutBase*>(layout);
    switch (layoutCtx->_aligmentV)
    {
    case WidgetLayout::VerticalAlignment::AlignmentCenter:
    {
        ImGui::SetCursorPosY((layoutCtx->_cachedLayoutRect.getHeight() - itemHeight) * 0.5);
        break;
    }
    case WidgetLayout::VerticalAlignment::AlignmentBottom:
    {
        f32 offset = (layoutCtx->_stateMask & Widget::State::StateMask::HorizontalLine) ? style.WindowPadding.y : layoutCtx->_cachedLayoutOffest._y;
        ImGui::SetCursorPosY(layoutCtx->_cachedLayoutRect.getHeight() - itemHeight - offset);
        layoutCtx->_cachedLayoutOffest._y += itemHeight + get_ItemSpacing()._y;
        break;
    }

    case WidgetLayout::VerticalAlignment::AlignmentTop:
    {
        f32 offset = (layoutCtx->_stateMask & Widget::State::StateMask::HorizontalLine) ? style.WindowPadding.y : layoutCtx->_cachedLayoutOffest._y;
        ImGui::SetCursorPosY(offset);
        layoutCtx->_cachedLayoutOffest._y += itemHeight + get_ItemSpacing()._y;
        break;
    }

    case WidgetLayout::VerticalAlignment::AlignmentFill:
    {
        ImVec2 avail = ImGui::GetContentRegionAvail();
        height = layoutCtx->_cachedLayoutRect.getHeight();
        break;
    }

    default:
        break;
    }

    return height;
}

} //namespace ui
} //namespace v3d

#endif //USE_IMGUI