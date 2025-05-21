#include "ImGuiDrawer.h"
#include "Utils/Logger.h"

#if USE_IMGUI
#include "Wigets.h"
#include "WigetGizmo.h"
#include "ImGuiHandler.h"
#include "ThirdParty/imgui/imgui.h"
#include "ThirdParty/imgui/imgui_internal.h"
#include "ThirdParty/ImGuizmo/ImGuizmo.h"

#include "Scene/Camera.h"
#include "Scene/Transform.h"

namespace v3d
{
namespace ui
{

ImGuiWigetDrawer::ImGuiWigetDrawer(ImGuiWigetHandler* handler) noexcept
    : m_wigetHandler(handler)
{
}

bool ImGuiWigetDrawer::draw_Window(Wiget* window, Wiget::State* state, f32 dt)
{
    ASSERT(ImGui::GetCurrentContext(), "must be valid");
    WigetWindow::StateWindow* wndCtx = static_cast<WigetWindow::StateWindow*>(state);

    if (wndCtx->_stateMask & Wiget::State::StateMask::FirstUpdate)
    {
        if (wndCtx->_position._x != 0 || wndCtx->_position._y != 0)
        {
            ImGui::SetNextWindowPos(ImVec2(static_cast<f32>(wndCtx->_position._x), static_cast<f32>(wndCtx->_position._y)), ImGuiCond_Once);
        }

        if (wndCtx->_size._width > 0 || wndCtx->_size._height > 0)
        {
            ImGui::SetNextWindowSize(ImVec2(static_cast<f32>(wndCtx->_size._width), static_cast<f32>(wndCtx->_size._height)), ImGuiCond_Once);
        }

        wndCtx->_stateMask &= ~Wiget::State::StateMask::FirstUpdate;
    }

    ImGuiStyle& style = ImGui::GetStyle();
    ImGuiWindowFlags flags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove /*| ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse*/;
    if (wndCtx->_createFlags & WigetWindow::Moveable)
    {
        flags &= ~ImGuiWindowFlags_NoMove;
    }

    if (wndCtx->_createFlags & WigetWindow::Resizeable)
    {
        flags &= ~ImGuiWindowFlags_NoResize;
    }

    if (wndCtx->_createFlags & WigetWindow::Scrollable)
    {
        flags &= ~ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse;
    }

    if (wndCtx->_createFlags & WigetWindow::AutoResizeByContent)
    {
        flags |= ImGuiWindowFlags_AlwaysAutoResize;
    }

    static auto callbackHandler = [this](WigetWindow* window, WigetWindow::StateWindow* wndCtx) -> void
        {
            math::Point2D pos(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y);
            const bool posChanged = pos != wndCtx->_position;
            if (posChanged || wndCtx->_stateMask & Wiget::State::StateMask::ForceUpdate)
            {
                wndCtx->_position = pos;
                if (wndCtx->_onPositionChanged)
                {
                    std::invoke(wndCtx->_onPositionChanged, window, nullptr, pos);
                }
            }

            math::Dimension2D size(ImGui::GetWindowSize().x, ImGui::GetWindowSize().y);
            const bool sizeChanged = size != wndCtx->_size;
            if (sizeChanged || wndCtx->_stateMask & Wiget::State::StateMask::ForceUpdate)
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
        wndCtx->_stateMask |= (wndCtx->_currentWindow != activeWindow) ? Wiget::State::StateMask::ForceUpdate : 0x0;
        
        wndCtx->_currentWindow = activeWindow;
        wndCtx->_cachedWindowRect = { ImGui::GetCursorScreenPos().x, ImGui::GetCursorScreenPos().y, ImGui::GetContentRegionAvail().x + ImGui::GetCursorScreenPos().x, ImGui::GetContentRegionAvail().y + ImGui::GetCursorScreenPos().y };
        wndCtx->_cachedWindowOffest = { ImGui::GetWindowContentRegionMin().x, ImGui::GetWindowContentRegionMin().y };

        if (wndCtx->_showToolTip && ImGui::IsItemHovered(ImGuiHoveredFlags_ForTooltip))
        {
            ImGui::SetTooltip(wndCtx->_toolTip.c_str());
        }

        callbackHandler(static_cast<WigetWindow*>(window), wndCtx);

        wndCtx->_layout.update(m_wigetHandler, window, &wndCtx->_layout, dt);

        ImGuiIO& imguiIO = ImGui::GetIO();
        if (imguiIO.ConfigFlags & ImGuiConfigFlags_DockingEnable && wndCtx->_windowLayout.getWindow())
        {
            static auto dockBuilder = [](const WigetWindowLayout &windowLayout, ImGuiID dockspaceID, bool force = false) -> bool
                {
                    if (ImGui::DockBuilderGetNode(dockspaceID) == nullptr || force) //recreate
                    {
                        ImGui::DockBuilderRemoveNode(dockspaceID);

                        ImGuiID dockMainID = ImGui::DockBuilderAddNode(dockspaceID);
                        ImGui::DockBuilderDockWindow(windowLayout.getWindow()->getTitle().c_str(), dockMainID);
                        for (const WigetWindowLayout::LayoutRule& ruleLayout : windowLayout.getRules())
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
        wndCtx->_stateMask = (wndCtx->_currentWindow != activeWindow) ? Wiget::State::StateMask::ForceUpdate : 0x0;
        wndCtx->_currentWindow = activeWindow;
        wndCtx->_cachedWindowRect = { ImGui::GetCursorScreenPos().x, ImGui::GetCursorScreenPos().y, ImGui::GetContentRegionAvail().x + ImGui::GetCursorScreenPos().x, ImGui::GetContentRegionAvail().y + ImGui::GetCursorScreenPos().y };
        wndCtx->_cachedWindowOffest = { ImGui::GetWindowContentRegionMin().x, ImGui::GetWindowContentRegionMin().y };

        callbackHandler(static_cast<WigetWindow*>(window), wndCtx);
    }
    ImGui::End();

    wndCtx->_stateMask &= ~Wiget::State::StateMask::ForceUpdate;
    ImGui::PopID();

    return active;
}

bool ImGuiWigetDrawer::draw_Text(Wiget* wiget, Wiget* base, Wiget::State* layout, Wiget::State* state)
{
    ASSERT(ImGui::GetCurrentContext(), "must be valid");
    ImGuiStyle& style = ImGui::GetStyle();

    WigetText::StateText* txtCtx = static_cast<WigetText::StateText*>(state);
    WigetLayout::StateLayoutBase* layoutCtx = static_cast<WigetLayout::StateLayoutBase*>(layout);

    u32 pushCount = 0;
    if (txtCtx->_stateMask & Wiget::State::StateMask::Color)
    {
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4{ txtCtx->_color._x, txtCtx->_color._y, txtCtx->_color._z, txtCtx->_color._w });
        ++pushCount;
    }

    setupHorizontalAligment(layoutCtx, 0.f, txtCtx->_itemRect.getWidth());
    //setupVerticalAligment(layoutCtx, txtCtx->_itemRect.getHeight());

    if (layoutCtx->_stateMask & Wiget::State::StateMask::HorizontalLine)
    {
        ImGui::SameLine();
    }

    ImGui::PushID(txtCtx->_uid);
    ImGui::Text(txtCtx->_text.c_str());
    ImGui::PopID();

    if (pushCount > 0)
    {
        ImGui::PopStyleColor(pushCount);
    }

    return true;
}

math::TVector2D<f32> ImGuiWigetDrawer::calculate_TextSize(Wiget* wiget, Wiget::State* layout, Wiget::State* state)
{
    ASSERT(ImGui::GetCurrentContext(), "must be valid");
    WigetText::StateText* txtCtx = static_cast<WigetText::StateText*>(state);
    WigetLayout::StateLayoutBase* layoutCtx = static_cast<WigetLayout::StateLayoutBase*>(layout);

    ASSERT(layoutCtx->_fontSize < m_wigetHandler->m_fonts.size(), "range out");
    ImGui::PushFont(m_wigetHandler->m_fonts[layoutCtx->_fontSize]);
    const ImVec2 alignmentSize = ImGui::CalcTextSize(txtCtx->_text.c_str());
    ImGui::PopFont();

    return{ alignmentSize.x, alignmentSize.y };
}

bool ImGuiWigetDrawer::draw_Button(Wiget* button, Wiget* base, Wiget::State* layout, Wiget::State* state)
{
    ASSERT(ImGui::GetCurrentContext(), "must be valid");
    ImGuiStyle& style = ImGui::GetStyle();

    WigetButton::StateButton* btnCtx = static_cast<WigetButton::StateButton*>(state);
    WigetLayout::StateLayoutBase* layoutCtx = static_cast<WigetLayout::StateLayoutBase*>(layout);

    u32 pushCount = 0;
    if (btnCtx->_stateMask & Wiget::State::StateMask::Color)
    {
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ btnCtx->_color._x, btnCtx->_color._y, btnCtx->_color._z, btnCtx->_color._w });
        ++pushCount;
    }

    if (btnCtx->_stateMask & Wiget::State::StateMask::HoveredColor)
    {
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ btnCtx->_colorHovered._x, btnCtx->_colorHovered._y, btnCtx->_colorHovered._z, btnCtx->_colorHovered._w });
        ++pushCount;
    }

    if (btnCtx->_stateMask & Wiget::State::StateMask::ClickedColor)
    {
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ btnCtx->_colorActive._x, btnCtx->_colorActive._y, btnCtx->_colorActive._z, btnCtx->_colorActive._w });
        ++pushCount;
    }

    setupHorizontalAligment(layoutCtx, 0.f, btnCtx->_itemRect.getWidth());
    //setupVerticalAligment(layoutCtx, btnCtx->_itemRect.getHeight());

    if (layoutCtx->_stateMask & Wiget::State::StateMask::HorizontalLine)
    {
        ImGui::SameLine();
    }

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

math::TVector2D<f32> ImGuiWigetDrawer::calculate_ButtonSize(Wiget* wiget, Wiget::State* layout, Wiget::State* state)
{
    ASSERT(ImGui::GetCurrentContext(), "must be valid");
    ImGuiStyle& style = ImGui::GetStyle();

    WigetButton::StateButton* btnCtx = static_cast<WigetButton::StateButton*>(state);
    WigetLayout::StateLayoutBase* layoutCtx = static_cast<WigetLayout::StateLayoutBase*>(layout);

    ImVec2 alignmentSize{ (f32)btnCtx->_size._width, (f32)btnCtx->_size._height };
    if (btnCtx->_size._width == 0 || btnCtx->_size._height == 0)
    {
        ASSERT(layoutCtx->_fontSize < m_wigetHandler->m_fonts.size(), "range out");
        ImGui::PushFont(m_wigetHandler->m_fonts[layoutCtx->_fontSize]);
        alignmentSize = ImGui::CalcTextSize(btnCtx->_text.c_str());
        alignmentSize.x += style.FramePadding.x * 2.0f;
        alignmentSize.y += style.FramePadding.y * 2.0f;
        ImGui::PopFont();
    }

    return { alignmentSize.x, alignmentSize.y };
}

bool ImGuiWigetDrawer::draw_Image(Wiget* image, Wiget* base, Wiget::State* layout, Wiget::State* state)
{
    ASSERT(ImGui::GetCurrentContext(), "must be valid");
    WigetImage::StateImage* imgCtx = static_cast<WigetImage::StateImage*>(state);
    WigetLayout::StateLayoutBase* layoutCtx = static_cast<WigetLayout::StateLayoutBase*>(layout);

    if (imgCtx->_texture)
    {
        ImVec2 size = { (f32)imgCtx->_size._width, (f32)imgCtx->_size._height };
        ImVec2 uv0 = { imgCtx->_uv.getLeftX(), imgCtx->_uv.getTopY() };
        ImVec2 uv1 = { imgCtx->_uv.getRightX(), imgCtx->_uv.getBottomY() };

        size.x = setupHorizontalAligment(layoutCtx, size.x, imgCtx->_itemRect.getWidth());
        //size.y = setupVerticalAligment(layoutCtx, imgCtx->_itemRect.getHeight());

        if (layoutCtx->_aligmentV == WigetLayout::VerticalAlignment::AlignmentFill)
        {
            ImVec2 avail = ImGui::GetContentRegionAvail();
            size.y = layoutCtx->_cachedLayoutRect.getHeight();
        }

        if (size.x == 0.f || size.y == 0.f)
        {
            return false;
        }

        u32 id = ~0;
        auto found = std::find(m_wigetHandler->m_activeTextures.begin(), m_wigetHandler->m_activeTextures.end(), imgCtx->_texture);
        if (found == m_wigetHandler->m_activeTextures.end())
        {
            m_wigetHandler->m_activeTextures.push_back(nullptr);
            id = m_wigetHandler->m_activeTextures.size() - 1;
        }
        else
        {
            id = std::distance(m_wigetHandler->m_activeTextures.begin(), found);
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

        if (imgCtx->_drawRectState != rect || base->isStateMaskActive(Wiget::State::StateMask::ForceUpdate))
        {
            imgCtx->_drawRectState = rect;
            if (imgCtx->_onDrawRectChanged)
            {
                std::invoke(imgCtx->_onDrawRectChanged, image, base, rect);
            }
        }
        m_wigetHandler->m_activeTextures[id] = imgCtx->_texture;

        return true;
    }

    return false;
}

math::TVector2D<f32> ImGuiWigetDrawer::calculate_ImageSize(Wiget* wiget, Wiget::State* layout, Wiget::State* state)
{
    ASSERT(ImGui::GetCurrentContext(), "must be valid");
    WigetImage::StateImage* imgCtx = static_cast<WigetImage::StateImage*>(state);
    ImGuiStyle& style = ImGui::GetStyle();

    return { imgCtx->_size._width + style.FramePadding.x * 2.f, imgCtx->_size._height + style.FramePadding.x * 2.f };
}

bool ImGuiWigetDrawer::draw_CheckBox(Wiget* wiget, Wiget* base, Wiget::State* layout, Wiget::State* state)
{
    ASSERT(ImGui::GetCurrentContext(), "must be valid");
    WigetCheckBox::StateCheckBox* cbCtx = static_cast<WigetCheckBox::StateCheckBox*>(state);

    bool value = cbCtx->_value;

    ImGui::PushID(cbCtx->_uid);
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

math::TVector2D<f32> ImGuiWigetDrawer::calculate_CheckBoxSize(Wiget* wiget, Wiget::State* layout, Wiget::State* state)
{
    return {};
}

bool ImGuiWigetDrawer::draw_RadioButtonGroup(Wiget* wiget, Wiget* base, Wiget::State* layout, Wiget::State* state)
{
    ASSERT(ImGui::GetCurrentContext(), "must be valid");
    WigetRadioButtonGroup::StateRadioButtonGroup* rbCtx = static_cast<WigetRadioButtonGroup::StateRadioButtonGroup*>(state);

    bool active = false;
    if (!rbCtx->_list.empty())
    {
        ASSERT(rbCtx->_activeIndex < rbCtx->_list.size(), "range out");
        s32 index = rbCtx->_activeIndex;
        for (u32 i = 0; i < rbCtx->_list.size() - 1; ++i)
        {
            active |= ImGui::RadioButton(rbCtx->_list[i].c_str(), &index, i); ImGui::SameLine();
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

math::TVector2D<f32> ImGuiWigetDrawer::calculate_RadioButtonGroupSize(Wiget* wiget, Wiget::State* layout, Wiget::State* state)
{
    return {};
}

bool ImGuiWigetDrawer::draw_ComboBox(Wiget* wiget, Wiget* base, Wiget::State* layout, Wiget::State* state)
{
    ASSERT(ImGui::GetCurrentContext(), "must be valid");
    WigetComboBox::StateComboBox* cbCtx = static_cast<WigetComboBox::StateComboBox*>(state);

    static auto items_ArrayGetter = [](void* user_data, int idx) -> const char*
        {
            std::vector<std::string>& list = *reinterpret_cast<std::vector<std::string>*>(user_data);
            return list[idx].c_str();
        };

    bool active = false;
    if (!cbCtx->_list.empty())
    {
        s32 index = cbCtx->_activeIndex;
        active = ImGui::Combo("combo", &index, items_ArrayGetter, &cbCtx->_list, cbCtx->_list.size());

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

math::TVector2D<f32> ImGuiWigetDrawer::calculate_ComboBoxSize(Wiget* wiget, Wiget::State* layout, Wiget::State* state)
{
    return {};
}

bool ImGuiWigetDrawer::draw_ListBox(Wiget* wiget, Wiget* base, Wiget::State* layout, Wiget::State* state)
{
    ASSERT(ImGui::GetCurrentContext(), "must be valid");
    WigetListBox::StateListBox* lbCtx = static_cast<WigetListBox::StateListBox*>(state);

    static auto items_ArrayGetter = [](void* user_data, int idx) -> const char*
        {
            std::vector<std::string>& list = *reinterpret_cast<std::vector<std::string>*>(user_data);
            return list[idx].c_str();
        };

    bool active = false;
    if (!lbCtx->_list.empty())
    {
        s32 index = lbCtx->_activeIndex;
        active = ImGui::ListBox("listbox", &index, items_ArrayGetter, &lbCtx->_list, lbCtx->_list.size(), 4);

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

math::TVector2D<f32> ImGuiWigetDrawer::calculate_ListBoxSize(Wiget* wiget, Wiget::State* layout, Wiget::State* state)
{
    return {};
}

bool ImGuiWigetDrawer::draw_InputField(Wiget* wiget, Wiget* base, Wiget::State* layout, Wiget::State* state)
{
    static int i0 = 123;
    ImGui::InputInt("input int", &i0); 

    return false;
}

bool ImGuiWigetDrawer::draw_InputSlider(Wiget* wiget, Wiget* base, Wiget::State* layout, Wiget::State* state)
{
    static int i1 = 0;
    ImGui::SliderInt("slider int", &i1, -1, 3);

    return false;
}

bool ImGuiWigetDrawer::draw_MenuBar(Wiget* menubar, Wiget::State* state, f32 dt)
{
    ASSERT(ImGui::GetCurrentContext(), "must be valid");
    WigetMenuBar::StateMenuBar* menuBarCtx = static_cast<WigetMenuBar::StateMenuBar*>(state);

    bool action = ImGui::BeginMainMenuBar();
    if (action)
    {
        menuBarCtx->_layout.update(m_wigetHandler, menubar, &menuBarCtx->_layout, dt);
        ImGui::EndMainMenuBar();
    }

    return action;
}

bool ImGuiWigetDrawer::draw_Menu(Wiget* menu, Wiget* base, Wiget::State* layout, Wiget::State* state, f32 dt)
{
    ASSERT(ImGui::GetCurrentContext(), "must be valid");
    WigetMenu::StateMenu* menuCtx = static_cast<WigetMenu::StateMenu*>(state);
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

        menuCtx->_layout.update(m_wigetHandler, base, &menuCtx->_layout, dt);
        ImGui::EndMenu();
    }

    return action;
}

bool ImGuiWigetDrawer::draw_MenuItem(Wiget* item, Wiget* base, Wiget::State* layout, Wiget::State* state)
{
    ASSERT(ImGui::GetCurrentContext(), "must be valid");
    WigetMenuItem::StateMenuItem* itemCtx = static_cast<WigetMenuItem::StateMenuItem*>(state);

    bool clicked = false;
    bool action = ImGui::MenuItem(itemCtx->_text.c_str(), nullptr, &clicked, itemCtx->_isActive);
    if (itemCtx->_onClickedEvent && clicked)
    {
        std::invoke(itemCtx->_onClickedEvent, item);
    }

    return action;
}

bool ImGuiWigetDrawer::draw_TabBar(Wiget* item, Wiget::State* state, f32 dt)
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
    //WigetTabBar::StateTabBar* barCtx = static_cast<WigetTabBar::StateTabBar*>(state);

    //static ImGuiTabBarFlags tab_bar_flags = 0;//ImGuiTabBarFlags_AutoSelectNewTabs | ImGuiTabBarFlags_Reorderable | ImGuiTabBarFlags_FittingPolicyResizeDown;
    //bool action = ImGui::BeginTabBar("Bar", tab_bar_flags);
    //if (action)
    //{
    //    barCtx->_layout.update(m_wigetHandler, item, &barCtx->_layout, dt);
    //    ImGui::EndTabBar();
    //}

    //return action;
}

bool ImGuiWigetDrawer::draw_TabItem(Wiget* wiget, Wiget* base, Wiget::State* layout, Wiget::State* state)
{
    //ASSERT(ImGui::GetCurrentContext(), "must be valid");
    //WigetTabItem::StateTabItem* tabCtx = static_cast<WigetTabItem::StateTabItem*>(state);

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

void ImGuiWigetDrawer::draw_BeginLayoutState(Wiget* layout, Wiget* base, Wiget::State* state)
{
    ASSERT(ImGui::GetCurrentContext(), "must be valid");
    WigetLayout::StateLayoutBase* layoutCtx = static_cast<WigetLayout::StateLayoutBase*>(state);

    bool active = true;

    ImVec2 layoutSize = { (f32)layoutCtx->_size._width, (f32)layoutCtx->_size._height };
    if (layoutCtx->_aligmentH == WigetLayout::HorizontalAlignment::AlignmentFill || (layoutCtx->_size._width > 0 || layoutCtx->_size._height > 0))
    {
        if (layoutCtx->_aligmentH == WigetLayout::HorizontalAlignment::AlignmentFill)
        {
            layoutSize.x = 0.f;
        }

        if (layoutCtx->_aligmentH == WigetLayout::HorizontalAlignment::AlignmentFill)
        {
            layoutSize.y = 0.f;
        }

        ImGuiChildFlags child_flags = 0;
        ImGuiWindowFlags window_flags = 0;
        if (layoutCtx->_flags & WigetLayout::LayoutFlag::Border)
        {
            child_flags |= ImGuiChildFlags_Borders;
        }
        active = ImGui::BeginChild(layoutCtx->_uid, layoutSize, child_flags, window_flags);
        layoutCtx->_stateMask |= Wiget::State::StateMask::ChildLayout;
    }

    layoutCtx->_cachedLayoutRect = { ImGui::GetCursorScreenPos().x, ImGui::GetCursorScreenPos().y, ImGui::GetContentRegionAvail().x + ImGui::GetCursorScreenPos().x, ImGui::GetContentRegionAvail().y + ImGui::GetCursorScreenPos().y };
    layoutCtx->_cachedLayoutOffest = get_LayoutPadding();

    ASSERT(layoutCtx->_fontSize < m_wigetHandler->m_fonts.size(), "range out");
    ImGui::PushFont(m_wigetHandler->m_fonts[layoutCtx->_fontSize]);

    if (layoutCtx->_stateMask & Wiget::State::StateMask::HorizontalLine)
    {
        ImGui::NewLine();
    }
}

void ImGuiWigetDrawer::draw_EndLayoutState(Wiget* layout, Wiget* base, Wiget::State* state)
{
    ASSERT(ImGui::GetCurrentContext(), "must be valid");
    WigetLayout::StateLayoutBase* layoutCtx = static_cast<WigetLayout::StateLayoutBase*>(state);

    ImGui::PopFont();

    if (layoutCtx->_stateMask & Wiget::State::StateMask::ChildLayout)
    {
        ImGui::EndChild();
        layoutCtx->_stateMask &= ~Wiget::State::StateMask::ChildLayout;
    }

    layoutCtx->_cachedLayoutRect = {};
    layoutCtx->_cachedLayoutOffest = {};
}

void ImGuiWigetDrawer::draw_Gizmo(Wiget* wiget, Wiget* base, Wiget::State* layout, Wiget::State* state)
{
    ASSERT(ImGui::GetCurrentContext(), "must be valid");
    WigetGizmo::StateGizmo* gizmoCtx = static_cast<WigetGizmo::StateGizmo*>(state);
    WigetLayout::StateLayoutBase* layoutCtx = static_cast<WigetLayout::StateLayoutBase*>(layout);

    if (gizmoCtx->_isActive && gizmoCtx->_camera)
    {
        const scene::Camera& camera = *gizmoCtx->_camera;
        scene::Transform transform = gizmoCtx->_transform;

        static auto convertOp = [](WigetGizmo::Operation op) -> ImGuizmo::OPERATION
            {
                switch (op)
                {
                case WigetGizmo::Operation::Translate:
                    return ImGuizmo::TRANSLATE;
                case WigetGizmo::Operation::Rotate:
                    return  ImGuizmo::ROTATE;
                case WigetGizmo::Operation::Scale:
                    return ImGuizmo::SCALE;
                default: break;
                }

                return ImGuizmo::UNIVERSAL;
            };

        ImGuizmo::OPERATION gizmoOp = convertOp(gizmoCtx->_operation);
        ImGuizmo::MODE gizmoMode = ImGuizmo::WORLD;
        if (gizmoOp != ImGuizmo::SCALE)
        {
            gizmoMode = (gizmoCtx->_mode == WigetGizmo::Mode::Local) ? ImGuizmo::LOCAL : ImGuizmo::WORLD;
        }

        ImGuizmo::SetDrawlist();
        ImGuizmo::SetRect(layoutCtx->_cachedLayoutRect.getLeftX(), layoutCtx->_cachedLayoutRect.getTopY(), layoutCtx->_cachedLayoutRect.getWidth(), layoutCtx->_cachedLayoutRect.getHeight());

        f32 rawViewMatrix[16] = {};
        f32 rawProjectionMatrix[16] = {};
        f32 rawTransformMatrix[16] = {};
        camera.getViewMatrix().get(rawViewMatrix);
        camera.getProjectionMatrix().get(rawProjectionMatrix);
        transform.getTransform().get(rawTransformMatrix);

        ImGuizmo::Manipulate(rawViewMatrix, rawProjectionMatrix, gizmoOp, gizmoMode, rawTransformMatrix, nullptr, nullptr);

        if (ImGuizmo::IsOver() && gizmoCtx->_onHoveredEvent)
        {
            std::invoke(gizmoCtx->_onHoveredEvent, wiget);
        }

        if (ImGuizmo::IsUsing() && gizmoCtx->_onTransformChangedEvent)
        {
            math::Matrix4D matrix;
            matrix.set(rawTransformMatrix);

            scene::Transform modifyTransform;
            modifyTransform.setTransform(matrix);

            std::invoke(gizmoCtx->_onTransformChangedEvent, wiget, base, modifyTransform);
        }
    }
}

void ImGuiWigetDrawer::draw_ViewManipulator(Wiget* wiget, Wiget* base, Wiget::State* layout, Wiget::State* state)
{
    ASSERT(ImGui::GetCurrentContext(), "must be valid");
    WigetViewManipulator::StateViewManipulator* viewCtx = static_cast<WigetViewManipulator::StateViewManipulator*>(state);
    WigetLayout::StateLayoutBase* layoutCtx = static_cast<WigetLayout::StateLayoutBase*>(layout);

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

        const bool showGrid = true;
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

math::TVector2D<f32> ImGuiWigetDrawer::get_LayoutPadding() const
{
    ASSERT(ImGui::GetCurrentContext(), "must be valid");
    ImGuiStyle& style = ImGui::GetStyle();

    return { style.WindowPadding.x, style.WindowPadding.y };
}

math::TVector2D<f32> ImGuiWigetDrawer::get_ItemSpacing() const
{
    ASSERT(ImGui::GetCurrentContext(), "must be valid");
    ImGuiStyle& style = ImGui::GetStyle();

    return { style.ItemSpacing.x, style.ItemSpacing.y };
}

f32 ImGuiWigetDrawer::setupHorizontalAligment(Wiget::State* layout, f32 originalWidth, f32 itemWidth)
{
    ImGuiStyle& style = ImGui::GetStyle();
    f32 width = originalWidth;

    WigetLayout::StateLayoutBase* layoutCtx = static_cast<WigetLayout::StateLayoutBase*>(layout);
    switch (layoutCtx->_aligmentH)
    {
    case WigetLayout::HorizontalAlignment::AlignmentCenter:
    {
        ImGui::SetCursorPosX((layoutCtx->_cachedLayoutRect.getWidth() - itemWidth) * 0.5f);
        break;
    }
    case WigetLayout::HorizontalAlignment::AlignmentRight:
    {
        f32 offset = (layoutCtx->_stateMask & Wiget::State::StateMask::HorizontalLine) ? layoutCtx->_cachedLayoutOffest._x : style.WindowPadding.x;
        ImGui::SetCursorPosX(layoutCtx->_cachedLayoutRect.getWidth() - itemWidth - offset);
        break;
    }

    case WigetLayout::HorizontalAlignment::AlignmentLeft:
    {
        f32 offset = (layoutCtx->_stateMask & Wiget::State::StateMask::HorizontalLine) ? layoutCtx->_cachedLayoutOffest._x : style.WindowPadding.x;
        ImGui::SetCursorPosX(offset);
        layoutCtx->_cachedLayoutOffest._x += itemWidth + get_ItemSpacing()._x;
        break;
    }

    case WigetLayout::HorizontalAlignment::AlignmentFill:
    {
        ImVec2 avail = ImGui::GetContentRegionAvail();
        width = layoutCtx->_cachedLayoutRect.getWidth();
        break;
    }

    default:
        break;
    }

    return width;
}

f32 ImGuiWigetDrawer::setupVerticalAligment(Wiget::State* layout, f32 originalHeight, f32 itemHeight)
{
    ImGuiStyle& style = ImGui::GetStyle();
    f32 height = originalHeight;

    WigetLayout::StateLayoutBase* layoutCtx = static_cast<WigetLayout::StateLayoutBase*>(layout);
    switch (layoutCtx->_aligmentV)
    {
    case WigetLayout::VerticalAlignment::AlignmentCenter:
    {
        ImGui::SetCursorPosY((layoutCtx->_cachedLayoutRect.getHeight() - itemHeight) * 0.5);
        break;
    }
    case WigetLayout::VerticalAlignment::AlignmentBottom:
    {
        f32 offset = (layoutCtx->_stateMask & Wiget::State::StateMask::HorizontalLine) ? style.WindowPadding.y : layoutCtx->_cachedLayoutOffest._y;
        ImGui::SetCursorPosY(layoutCtx->_cachedLayoutRect.getHeight() - itemHeight - offset);
        layoutCtx->_cachedLayoutOffest._y += itemHeight + get_ItemSpacing()._y;
        break;
    }

    case WigetLayout::VerticalAlignment::AlignmentTop:
    {
        f32 offset = (layoutCtx->_stateMask & Wiget::State::StateMask::HorizontalLine) ? style.WindowPadding.y : layoutCtx->_cachedLayoutOffest._y;
        ImGui::SetCursorPosY(offset);
        layoutCtx->_cachedLayoutOffest._y += itemHeight + get_ItemSpacing()._y;
        break;
    }

    case WigetLayout::VerticalAlignment::AlignmentFill:
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