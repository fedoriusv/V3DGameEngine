#include "ImGuiDrawer.h"

#if USE_IMGUI
#include "Wigets.h"
#include "ImGuiHandler.h"
#include "ThirdParty/imgui/imgui.h"
#include "ThirdParty/imgui/imgui_internal.h"

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
        if (wndCtx->_position.m_x != 0 || wndCtx->_position.m_y != 0)
        {
            ImGui::SetNextWindowPos(ImVec2(static_cast<f32>(wndCtx->_position.m_x), static_cast<f32>(wndCtx->_position.m_y)), ImGuiCond_Once);
        }

        if (wndCtx->_size.m_width > 0 || wndCtx->_size.m_height > 0)
        {
            ImGui::SetNextWindowSize(ImVec2(static_cast<f32>(wndCtx->_size.m_width), static_cast<f32>(wndCtx->_size.m_height)), ImGuiCond_Once);
        }

        wndCtx->_stateMask &= ~Wiget::State::StateMask::FirstUpdate;
    }

    ImGuiStyle& style = ImGui::GetStyle();
    ImGuiWindowFlags flags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove /*| ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse*/;
    if (wndCtx->_flags & WigetWindow::Moveable)
    {
        flags &= ~ImGuiWindowFlags_NoMove;
    }

    if (wndCtx->_flags & WigetWindow::Resizeable)
    {
        flags &= ~ImGuiWindowFlags_NoResize;
    }

    if (wndCtx->_flags & WigetWindow::Scrollable)
    {
        flags &= ~ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse;
    }

    if (wndCtx->_flags & WigetWindow::AutoResizeByContent)
    {
        flags |= ImGuiWindowFlags_AlwaysAutoResize;
    }

    ImGui::PushID(wndCtx->_uid);
    bool open = true;
    bool active = ImGui::Begin(wndCtx->_title.c_str(), &open, flags);
    if (active)
    {
        ImGuiViewport* viewport = ImGui::GetWindowViewport();
        platform::Window* activeWindow = reinterpret_cast<platform::Window*>(viewport->PlatformUserData);
        wndCtx->_stateMask = (wndCtx->_currentWindow != activeWindow) ? Wiget::State::StateMask::ForceUpdate : 0x0;
        wndCtx->_currentWindow = activeWindow;
        wndCtx->_cachedWindowRect = { ImGui::GetCursorScreenPos().x, ImGui::GetCursorScreenPos().y, ImGui::GetContentRegionAvail().x + ImGui::GetCursorScreenPos().x, ImGui::GetContentRegionAvail().y + ImGui::GetCursorScreenPos().y };
        wndCtx->_cachedWindowOffest = { ImGui::GetWindowContentRegionMin().x, ImGui::GetWindowContentRegionMin().y };

        if (wndCtx->_showToolTip && ImGui::IsItemHovered(ImGuiHoveredFlags_ForTooltip))
        {
            ImGui::SetTooltip(wndCtx->_toolTip.c_str());
        }

        math::Point2D pos(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y);
        bool posChanged = pos != wndCtx->_position;
        if (posChanged || wndCtx->_stateMask & Wiget::State::StateMask::ForceUpdate)
        {
            wndCtx->_position = pos;
            if (wndCtx->_onPositionChanged)
            {
                std::invoke(wndCtx->_onPositionChanged, window, nullptr, pos);
            }
        }

        math::Dimension2D size(ImGui::GetWindowSize().x, ImGui::GetWindowSize().y);
        bool sizeChanged = size != wndCtx->_size;
        if (sizeChanged || wndCtx->_stateMask & Wiget::State::StateMask::ForceUpdate)
        {
            wndCtx->_size = size;
            if (wndCtx->_onSizeChanged)
            {
                std::invoke(wndCtx->_onSizeChanged, window, nullptr, size);
            }
        }

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

        math::Point2D pos(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y);
        bool posChanged = pos != wndCtx->_position;
        if (posChanged || wndCtx->_stateMask & Wiget::State::StateMask::ForceUpdate)
        {
            wndCtx->_position = pos;
            if (wndCtx->_onPositionChanged)
            {
                std::invoke(wndCtx->_onPositionChanged, window, nullptr, pos);
            }
        }

        math::Dimension2D size(ImGui::GetWindowSize().x, ImGui::GetWindowSize().y);
        bool sizeChanged = size != wndCtx->_size;
        if (sizeChanged || wndCtx->_stateMask & Wiget::State::StateMask::ForceUpdate)
        {
            wndCtx->_size = size;
            if (wndCtx->_onSizeChanged)
            {
                std::invoke(wndCtx->_onSizeChanged, window, nullptr, size);
            }
        }
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
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4{ txtCtx->_color.m_x, txtCtx->_color.m_y, txtCtx->_color.m_z, txtCtx->_color.m_w });
        ++pushCount;
    }

    setupHorizontalAligment(layoutCtx, txtCtx->_itemRect.getWidth());
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

math::Vector2D ImGuiWigetDrawer::calculate_TextSize(Wiget* wiget, Wiget::State* layout, Wiget::State* state)
{
    ASSERT(ImGui::GetCurrentContext(), "must be valid");
    WigetText::StateText* txtCtx = static_cast<WigetText::StateText*>(state);
    WigetLayout::StateLayoutBase* layoutCtx = static_cast<WigetLayout::StateLayoutBase*>(layout);

    ASSERT(layoutCtx->_fontSize < m_wigetHandler->m_fonts.size(), "range out");
    ImGui::PushFont(m_wigetHandler->m_fonts[layoutCtx->_fontSize]);
    const ImVec2 alignmentSize = ImGui::CalcTextSize(txtCtx->_text.c_str());
    ImGui::PopFont();

    return math::Vector2D(alignmentSize.x, alignmentSize.y);
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
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ btnCtx->_color.m_x, btnCtx->_color.m_y, btnCtx->_color.m_z, btnCtx->_color.m_w });
        ++pushCount;
    }

    if (btnCtx->_stateMask & Wiget::State::StateMask::HoveredColor)
    {
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ btnCtx->_colorHovered.m_x, btnCtx->_colorHovered.m_y, btnCtx->_colorHovered.m_z, btnCtx->_colorHovered.m_w });
        ++pushCount;
    }

    if (btnCtx->_stateMask & Wiget::State::StateMask::ClickedColor)
    {
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ btnCtx->_colorActive.m_x, btnCtx->_colorActive.m_y, btnCtx->_colorActive.m_z, btnCtx->_colorActive.m_w });
        ++pushCount;
    }

    setupHorizontalAligment(layoutCtx, btnCtx->_itemRect.getWidth());
    //setupVerticalAligment(layoutCtx, btnCtx->_itemRect.getHeight());

    if (layoutCtx->_stateMask & Wiget::State::StateMask::HorizontalLine)
    {
        ImGui::SameLine();
    }

    ImGui::PushID(btnCtx->_uid);
    ImVec2 size = { (f32)btnCtx->_size.m_width, (f32)btnCtx->_size.m_height };
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

math::Vector2D ImGuiWigetDrawer::calculate_ButtonSize(Wiget* wiget, Wiget::State* layout, Wiget::State* state)
{
    ASSERT(ImGui::GetCurrentContext(), "must be valid");
    ImGuiStyle& style = ImGui::GetStyle();

    WigetButton::StateButton* btnCtx = static_cast<WigetButton::StateButton*>(state);
    WigetLayout::StateLayoutBase* layoutCtx = static_cast<WigetLayout::StateLayoutBase*>(layout);

    ImVec2 alignmentSize{ (f32)btnCtx->_size.m_width, (f32)btnCtx->_size.m_height };
    if (btnCtx->_size.m_width == 0 || btnCtx->_size.m_height == 0)
    {
        ASSERT(layoutCtx->_fontSize < m_wigetHandler->m_fonts.size(), "range out");
        ImGui::PushFont(m_wigetHandler->m_fonts[layoutCtx->_fontSize]);
        alignmentSize = ImGui::CalcTextSize(btnCtx->_text.c_str());
        alignmentSize.x += style.FramePadding.x * 2.0f;
        alignmentSize.y += style.FramePadding.y * 2.0f;
        ImGui::PopFont();
    }

    return math::Vector2D(alignmentSize.x, alignmentSize.y);
}

bool ImGuiWigetDrawer::draw_Image(Wiget* image, Wiget* base, Wiget::State* layout, Wiget::State* state)
{
    ASSERT(ImGui::GetCurrentContext(), "must be valid");
    WigetImage::StateImage* imgCtx = static_cast<WigetImage::StateImage*>(state);

    if (imgCtx->_texture)
    {
        u32 id = ~0;
        auto found = std::find(m_wigetHandler->m_activeTextures.begin(), m_wigetHandler->m_activeTextures.end(), imgCtx->_texture);
        if (found == m_wigetHandler->m_activeTextures.end())
        {
            m_wigetHandler->m_activeTextures.push_back(imgCtx->_texture);
            id = m_wigetHandler->m_activeTextures.size() - 1;
        }
        else
        {
            id = std::distance(m_wigetHandler->m_activeTextures.begin(), found);
        }

        ImVec2 size = { (f32)imgCtx->_size.m_width, (f32)imgCtx->_size.m_height };
        ImVec2 uv0 = { imgCtx->_uv.getLeftX(), imgCtx->_uv.getTopY() };
        ImVec2 uv1 = { imgCtx->_uv.getRightX(), imgCtx->_uv.getBottomY() };

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

        s32 a = ImGui::GetItemRectMin().x;
        s32 b = ImGui::GetItemRectMin().y;
        s32 c = ImGui::GetItemRectMax().x;
        s32 d = ImGui::GetItemRectMax().y;
        math::Rect32 rect(a, b, c, d);

        if (imgCtx->_drawRectState != rect || base->isStateMaskActive(Wiget::State::StateMask::ForceUpdate))
        {
            imgCtx->_drawRectState = rect;
            if (imgCtx->_onDrawRectChanged)
            {
                std::invoke(imgCtx->_onDrawRectChanged, image, base, rect);
            }
        }
        return true;
    }

    return false;
}

math::Vector2D ImGuiWigetDrawer::calculate_ImageSize(Wiget* wiget, Wiget::State* layout, Wiget::State* state)
{
    return math::Vector2D();
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

math::Vector2D ImGuiWigetDrawer::calculate_CheckBoxSize(Wiget* wiget, Wiget::State* layout, Wiget::State* state)
{
    return math::Vector2D();
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

math::Vector2D ImGuiWigetDrawer::calculate_RadioButtonGroupSize(Wiget* wiget, Wiget::State* layout, Wiget::State* state)
{
    return math::Vector2D();
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

math::Vector2D ImGuiWigetDrawer::calculate_ComboBoxSize(Wiget* wiget, Wiget::State* layout, Wiget::State* state)
{
    return math::Vector2D();
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

math::Vector2D ImGuiWigetDrawer::calculate_ListBoxSize(Wiget* wiget, Wiget::State* layout, Wiget::State* state)
{
    return math::Vector2D();
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
    ASSERT(ImGui::GetCurrentContext(), "must be valid");
    WigetTabItem::StateTabItem* tabCtx = static_cast<WigetTabItem::StateTabItem*>(state);

    bool clicked = false;
    bool action = ImGui::BeginTabItem(tabCtx->_text.c_str(), &clicked);

    if (tabCtx->_onClickedEvent && clicked)
    {
        std::invoke(tabCtx->_onClickedEvent, wiget);
    }

    tabCtx->_layout.update(m_wigetHandler, wiget, &tabCtx->_layout, 0);
    ImGui::EndTabItem();

    return action;
}

void ImGuiWigetDrawer::draw_BeginLayoutState(Wiget* layout, Wiget* base, Wiget::State* state)
{
    ASSERT(ImGui::GetCurrentContext(), "must be valid");
    WigetLayout::StateLayoutBase* layoutCtx = static_cast<WigetLayout::StateLayoutBase*>(state);

    bool active = true;
    /*if ((layoutCtx->_stateMask & Wiget::State::StateMask::MenuLayout) == 0)
    {
        ImVec2 layoutSize = { (f32)layoutCtx->_size.m_width, (f32)layoutCtx->_size.m_height };
        if (layoutCtx->_size.m_width == 0 || layoutCtx->_size.m_height == 0)
        {
            layoutSize = { layoutCtx->_cachedContentSize.m_x, layoutCtx->_cachedContentSize.m_y };
        }

        if (layoutCtx->_stateMask & Wiget::State::StateMask::WindowLayout || layoutCtx->_flags & WigetLayout::LayoutFlag::Fill)
        {
            layoutSize = { 0.f, 0.f };
        }


    }*/

    ImVec2 layoutSize = { (f32)layoutCtx->_size.m_width, (f32)layoutCtx->_size.m_height };
    if (layoutCtx->_size.m_width > 0 || layoutCtx->_size.m_height > 0)
    {
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

    if (active)
    {
        ASSERT(layoutCtx->_fontSize < m_wigetHandler->m_fonts.size(), "range out");
        ImGui::PushFont(m_wigetHandler->m_fonts[layoutCtx->_fontSize]);

        if (layoutCtx->_stateMask & Wiget::State::StateMask::HorizontalLine)
        {
            ImGui::NewLine();
        }
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

math::Vector2D ImGuiWigetDrawer::get_LayoutPadding() const
{
    ASSERT(ImGui::GetCurrentContext(), "must be valid");
    ImGuiStyle& style = ImGui::GetStyle();

    return math::Vector2D(style.WindowPadding.x, style.WindowPadding.y);
}

math::Vector2D ImGuiWigetDrawer::get_ItemSpacing() const
{
    ASSERT(ImGui::GetCurrentContext(), "must be valid");
    ImGuiStyle& style = ImGui::GetStyle();

    return math::Vector2D(style.ItemSpacing.x, style.ItemSpacing.y);
}

void ImGuiWigetDrawer::setupHorizontalAligment(Wiget::State* layout, f32 itemWidth)
{
    ImGuiStyle& style = ImGui::GetStyle();

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
        f32 offset = (layoutCtx->_stateMask & Wiget::State::StateMask::HorizontalLine) ? layoutCtx->_cachedLayoutOffest.m_x : style.WindowPadding.x;
        ImGui::SetCursorPosX(layoutCtx->_cachedLayoutRect.getWidth() - itemWidth - offset);
        break;
    }

    default:
    case WigetLayout::HorizontalAlignment::AlignmentLeft:
    {
        f32 offset = (layoutCtx->_stateMask & Wiget::State::StateMask::HorizontalLine) ? layoutCtx->_cachedLayoutOffest.m_x : style.WindowPadding.x;
        ImGui::SetCursorPosX(offset);
        layoutCtx->_cachedLayoutOffest.m_x += itemWidth + get_ItemSpacing().m_x;
        break;
    }
    }
}

void ImGuiWigetDrawer::setupVerticalAligment(Wiget::State* layout, f32 itemHeight)
{
    ImGuiStyle& style = ImGui::GetStyle();

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
        f32 offset = (layoutCtx->_stateMask & Wiget::State::StateMask::HorizontalLine) ? style.WindowPadding.y : layoutCtx->_cachedLayoutOffest.m_y;
        ImGui::SetCursorPosY(layoutCtx->_cachedLayoutRect.getHeight() - itemHeight - offset);
        layoutCtx->_cachedLayoutOffest.m_y += itemHeight + get_ItemSpacing().m_y;
        break;
    }

    default:
    case WigetLayout::VerticalAlignment::AlignmentTop:
    {
        f32 offset = (layoutCtx->_stateMask & Wiget::State::StateMask::HorizontalLine) ? style.WindowPadding.y : layoutCtx->_cachedLayoutOffest.m_y;
        ImGui::SetCursorPosY(offset);
        layoutCtx->_cachedLayoutOffest.m_y += itemHeight + get_ItemSpacing().m_y;
        break;
    }
    }
}

} //namespace ui
} //namespace v3d

#endif //USE_IMGUI