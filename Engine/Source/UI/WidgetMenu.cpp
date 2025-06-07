#include "WidgetMenu.h"
#include "WidgetHandler.h"
#include "Utils/Logger.h"

namespace v3d
{
namespace ui
{

/////////////////////////////////////////////////////////////////////////////////////////////////////

WidgetMenuBar::WidgetMenuBar(MenuFlags flags) noexcept
    : WidgetBase<WidgetMenuBar>(V3D_NEW(StateType, memory::MemoryLabel::MemoryUI)())
{
    Widget::cast_data<StateType>(m_data)._flags = flags;
}

WidgetMenuBar::WidgetMenuBar(const WidgetMenuBar& other) noexcept
    : WidgetBase<WidgetMenuBar>(other)
{
    StateType* state = V3D_NEW(StateType, memory::MemoryLabel::MemoryUI)(*static_cast<const StateType*>(other.m_data));
    m_data = state;
}

WidgetMenuBar::WidgetMenuBar(WidgetMenuBar&& other) noexcept
    : WidgetBase<WidgetMenuBar>(other)
{
    m_data = other.m_data;
    other.m_data = nullptr;
}

WidgetMenuBar::~WidgetMenuBar()
{
    if (m_data)
    {
        V3D_DELETE(m_data, memory::MemoryLabel::MemoryUI);
        m_data = nullptr;
    }
}

bool WidgetMenuBar::update(WidgetHandler* handler, Widget* parent, Widget* layout, f32 dt)
{
    if (Widget::update(handler, parent, layout, dt))
    {
        return  handler->getWidgetDrawer()->draw_MenuBar(this, m_data, dt);
    }

    return false;
}

Widget* WidgetMenuBar::copy() const
{
    return V3D_NEW(WidgetMenuBar, memory::MemoryLabel::MemoryUI)(*this);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////

WidgetMenu::WidgetMenu(const std::string& text) noexcept
    : WidgetBase<WidgetMenu>(V3D_NEW(StateType, memory::MemoryLabel::MemoryUI)())
{
    setText(text);
}

WidgetMenu::WidgetMenu(const WidgetMenu& other) noexcept
    : WidgetBase<WidgetMenu>(other)
{
    StateType* state = V3D_NEW(StateType, memory::MemoryLabel::MemoryUI)(*static_cast<const StateType*>(other.m_data));
    m_data = state;
}

WidgetMenu::WidgetMenu(WidgetMenu&& other) noexcept
    : WidgetBase<WidgetMenu>(other)
{
    m_data = other.m_data;
    other.m_data = nullptr;
}

WidgetMenu::~WidgetMenu()
{
    if (m_data)
    {
        V3D_DELETE(m_data, memory::MemoryLabel::MemoryUI);
        m_data = nullptr;
    }
}

bool WidgetMenu::update(WidgetHandler* handler, Widget* parent, Widget* layout, f32 dt)
{
    if (Widget::update(handler, parent, layout, dt))
    {
        return handler->getWidgetDrawer()->draw_Menu(this, parent, static_cast<WidgetType*>(layout)->m_data, m_data, dt);
    }

    return false;
}

Widget* WidgetMenu::copy() const
{
    return V3D_NEW(WidgetMenu, memory::MemoryLabel::MemoryUI)(*this);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////

WidgetMenuItem::WidgetMenuItem(const std::string& text) noexcept
    : WidgetBase<WidgetMenuItem>(V3D_NEW(StateType, memory::MemoryLabel::MemoryUI)())
{
    setText(text);
}

WidgetMenuItem::WidgetMenuItem(const WidgetMenuItem& other) noexcept
    : WidgetBase<WidgetMenuItem>(other)
{
    StateType* state = V3D_NEW(StateType, memory::MemoryLabel::MemoryUI)(*static_cast<const StateType*>(other.m_data));
    m_data = state;
}

WidgetMenuItem::WidgetMenuItem(WidgetMenuItem&& other) noexcept
    : WidgetBase<WidgetMenuItem>(other)
{
    m_data = other.m_data;
    other.m_data = nullptr;
}

WidgetMenuItem::~WidgetMenuItem()
{
    if (m_data)
    {
        V3D_DELETE(m_data, memory::MemoryLabel::MemoryUI);
        m_data = nullptr;
    }
}

bool WidgetMenuItem::update(WidgetHandler* handler, Widget* parent, Widget* layout, f32 dt)
{
    if (Widget::update(handler, parent, layout, dt))
    {
        return handler->getWidgetDrawer()->draw_MenuItem(this, parent, static_cast<WidgetType*>(layout)->m_data, m_data);
    }

    return false;
}

Widget* WidgetMenuItem::copy() const
{
    return V3D_NEW(WidgetMenuItem, memory::MemoryLabel::MemoryUI)(*this);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////

} // namespace ui
} // namespace v3d