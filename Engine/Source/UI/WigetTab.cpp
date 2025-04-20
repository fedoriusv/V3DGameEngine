#include "WigetTab.h"
#include "WigetHandler.h"
#include "Utils/Logger.h"

namespace v3d
{
namespace ui
{

/////////////////////////////////////////////////////////////////////////////////////////////////////

WigetTabBar::WigetTabBar() noexcept
    : WigetBase<WigetTabBar>(V3D_NEW(StateType, memory::MemoryLabel::MemoryUI)())
{
    Wiget::cast_data<StateType>(m_data)._layout.m_data->_stateMask |= Wiget::State::StateMask::MenuLayout;
}

WigetTabBar::WigetTabBar(const WigetTabBar& other) noexcept
    : WigetBase<WigetTabBar>(other)
{
    StateType* state = V3D_NEW(StateType, memory::MemoryLabel::MemoryUI)(*static_cast<const StateType*>(other.m_data));
    m_data = state;
}

WigetTabBar::~WigetTabBar()
{
    if (m_data)
    {
        V3D_DELETE(m_data, memory::MemoryLabel::MemoryUI);
        m_data = nullptr;
    }
}

bool WigetTabBar::update(WigetHandler* handler, Wiget* parent, Wiget* layout, f32 dt)
{
    if (Wiget::update(handler, parent, layout, dt))
    {
        return  handler->getWigetDrawer()->draw_TabBar(this, m_data, dt);
    }

    return false;
}

Wiget* WigetTabBar::copy() const
{
    return V3D_NEW(WigetTabBar, memory::MemoryLabel::MemoryUI)(*this);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////

WigetTabItem::WigetTabItem(const std::string& text) noexcept
    : WigetBase<WigetTabItem>(V3D_NEW(StateType, memory::MemoryLabel::MemoryUI)())
{
    setText(text);
}

WigetTabItem::WigetTabItem(const WigetTabItem& other) noexcept
    : WigetBase<WigetTabItem>(other)
{
    StateType* state = V3D_NEW(StateType, memory::MemoryLabel::MemoryUI)(*static_cast<const StateType*>(other.m_data));
    m_data = state;
}

WigetTabItem::~WigetTabItem()
{
    if (m_data)
    {
        V3D_DELETE(m_data, memory::MemoryLabel::MemoryUI);
        m_data = nullptr;
    }
}

bool WigetTabItem::update(WigetHandler* handler, Wiget* parent, Wiget* layout, f32 dt)
{
    if (Wiget::update(handler, parent, layout, dt))
    {
        return  handler->getWigetDrawer()->draw_TabItem(this, parent, static_cast<WigetType*>(layout)->m_data, m_data);
    }

    return false;
}

Wiget* WigetTabItem::copy() const
{
    return V3D_NEW(WigetTabItem, memory::MemoryLabel::MemoryUI)(*this);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace scene
} //namespace v3d