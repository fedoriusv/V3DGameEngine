#include "WigetMenu.h"
#include "WigetHandler.h"
#include "Utils/Logger.h"

namespace v3d
{
namespace ui
{

WigetMenuBar::WigetMenuBar() noexcept
    : WigetBase<WigetMenuBar>(V3D_NEW(WigetMenuBar::ContextMenuBar, memory::MemoryLabel::MemoryUI)())
{
}

WigetMenuBar::WigetMenuBar(const WigetMenuBar& other) noexcept
    : WigetBase<WigetMenuBar>(other)
{
    WigetMenuBar::ContextMenuBar* context = V3D_NEW(WigetMenuBar::ContextMenuBar, memory::MemoryLabel::MemoryUI)();
    *context = *static_cast<WigetMenuBar::ContextMenuBar*>(other.m_data);
    m_data = context;
}

WigetMenuBar::~WigetMenuBar()
{
    if (m_data)
    {
        V3D_DELETE(m_data, memory::MemoryLabel::MemoryUI);
        m_data = nullptr;
    }
}

bool WigetMenuBar::update(WigetHandler* handler, WigetLayout* layout, f32 dt)
{
    if (Wiget::update(handler, layout, dt))
    {
        return  handler->draw_MenuBar(this, m_data, dt);
    }

    return false;
}


WigetMenu::WigetMenu(const std::string& text) noexcept
    : WigetBase<WigetMenu>(V3D_NEW(WigetMenu::ContextMenu, memory::MemoryLabel::MemoryUI)())
{
    setText(text);
}

WigetMenu::WigetMenu(const WigetMenu& other) noexcept
    : WigetBase<WigetMenu>(other)
{
    WigetMenu::ContextMenu* context = V3D_NEW(WigetMenu::ContextMenu, memory::MemoryLabel::MemoryUI)();
    *context = *static_cast<WigetMenu::ContextMenu*>(other.m_data);
    m_data = context;
}

WigetMenu::~WigetMenu()
{
    if (m_data)
    {
        V3D_DELETE(m_data, memory::MemoryLabel::MemoryUI);
        m_data = nullptr;
    }
}

bool WigetMenu::update(WigetHandler* handler, WigetLayout* layout, f32 dt)
{
    if (Wiget::update(handler, layout, dt))
    {
        return handler->draw_Menu(this, m_data, dt);
    }

    return false;
}


WigetMenuItem::WigetMenuItem(const std::string& text) noexcept
    : WigetBase<WigetMenuItem>(V3D_NEW(WigetMenuItem::ContextMenuItem, memory::MemoryLabel::MemoryUI)())
{
    setText(text);
}

WigetMenuItem::WigetMenuItem(const WigetMenuItem& other) noexcept
    : WigetBase<WigetMenuItem>(other)
{
    WigetMenuItem::ContextMenuItem* context = V3D_NEW(WigetMenuItem::ContextMenuItem, memory::MemoryLabel::MemoryUI)();
    *context = *static_cast<WigetMenuItem::ContextMenuItem*>(other.m_data);
    m_data = context;
}

WigetMenuItem::~WigetMenuItem()
{
    if (m_data)
    {
        V3D_DELETE(m_data, memory::MemoryLabel::MemoryUI);
        m_data = nullptr;
    }
}

bool WigetMenuItem::update(WigetHandler* handler, WigetLayout* layout, f32 dt)
{
    if (Wiget::update(handler, layout, dt))
    {
        return handler->draw_MenuItem(this, m_data, dt);
    }

    return false;
}

} // namespace ui
} // namespace v3d