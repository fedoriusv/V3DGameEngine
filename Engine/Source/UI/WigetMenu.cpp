#include "WigetMenu.h"
#include "WigetHandler.h"
#include "Utils/Logger.h"

namespace v3d
{
namespace ui
{

/////////////////////////////////////////////////////////////////////////////////////////////////////

WigetMenuBar::WigetMenuBar(MenuFlags flags) noexcept
    : WigetBase<WigetMenuBar>(V3D_NEW(StateType, memory::MemoryLabel::MemoryUI)())
{
    Wiget::cast_data<StateType>(m_data)._flags = flags;
}

WigetMenuBar::WigetMenuBar(const WigetMenuBar& other) noexcept
    : WigetBase<WigetMenuBar>(other)
{
    StateType* state = V3D_NEW(StateType, memory::MemoryLabel::MemoryUI)(*static_cast<const StateType*>(other.m_data));
    m_data = state;
}

WigetMenuBar::WigetMenuBar(WigetMenuBar&& other) noexcept
    : WigetBase<WigetMenuBar>(other)
{
    m_data = other.m_data;
    other.m_data = nullptr;
}

WigetMenuBar::~WigetMenuBar()
{
    if (m_data)
    {
        V3D_DELETE(m_data, memory::MemoryLabel::MemoryUI);
        m_data = nullptr;
    }
}

bool WigetMenuBar::update(WigetHandler* handler, Wiget* parent, Wiget* layout, f32 dt)
{
    if (Wiget::update(handler, parent, layout, dt))
    {
        return  handler->getWigetDrawer()->draw_MenuBar(this, m_data, dt);
    }

    return false;
}

Wiget* WigetMenuBar::copy() const
{
    return V3D_NEW(WigetMenuBar, memory::MemoryLabel::MemoryUI)(*this);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////

WigetMenu::WigetMenu(const std::string& text) noexcept
    : WigetBase<WigetMenu>(V3D_NEW(StateType, memory::MemoryLabel::MemoryUI)())
{
    setText(text);
}

WigetMenu::WigetMenu(const WigetMenu& other) noexcept
    : WigetBase<WigetMenu>(other)
{
    StateType* state = V3D_NEW(StateType, memory::MemoryLabel::MemoryUI)(*static_cast<const StateType*>(other.m_data));
    m_data = state;
}

WigetMenu::WigetMenu(WigetMenu&& other) noexcept
    : WigetBase<WigetMenu>(other)
{
    m_data = other.m_data;
    other.m_data = nullptr;
}

WigetMenu::~WigetMenu()
{
    if (m_data)
    {
        V3D_DELETE(m_data, memory::MemoryLabel::MemoryUI);
        m_data = nullptr;
    }
}

bool WigetMenu::update(WigetHandler* handler, Wiget* parent, Wiget* layout, f32 dt)
{
    if (Wiget::update(handler, parent, layout, dt))
    {
        return handler->getWigetDrawer()->draw_Menu(this, parent, static_cast<WigetType*>(layout)->m_data, m_data, dt);
    }

    return false;
}

Wiget* WigetMenu::copy() const
{
    return V3D_NEW(WigetMenu, memory::MemoryLabel::MemoryUI)(*this);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////

WigetMenuItem::WigetMenuItem(const std::string& text) noexcept
    : WigetBase<WigetMenuItem>(V3D_NEW(StateType, memory::MemoryLabel::MemoryUI)())
{
    setText(text);
}

WigetMenuItem::WigetMenuItem(const WigetMenuItem& other) noexcept
    : WigetBase<WigetMenuItem>(other)
{
    StateType* state = V3D_NEW(StateType, memory::MemoryLabel::MemoryUI)(*static_cast<const StateType*>(other.m_data));
    m_data = state;
}

WigetMenuItem::WigetMenuItem(WigetMenuItem&& other) noexcept
    : WigetBase<WigetMenuItem>(other)
{
    m_data = other.m_data;
    other.m_data = nullptr;
}

WigetMenuItem::~WigetMenuItem()
{
    if (m_data)
    {
        V3D_DELETE(m_data, memory::MemoryLabel::MemoryUI);
        m_data = nullptr;
    }
}

bool WigetMenuItem::update(WigetHandler* handler, Wiget* parent, Wiget* layout, f32 dt)
{
    if (Wiget::update(handler, parent, layout, dt))
    {
        return handler->getWigetDrawer()->draw_MenuItem(this, parent, static_cast<WigetType*>(layout)->m_data, m_data);
    }

    return false;
}

Wiget* WigetMenuItem::copy() const
{
    return V3D_NEW(WigetMenuItem, memory::MemoryLabel::MemoryUI)(*this);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////

} // namespace ui
} // namespace v3d