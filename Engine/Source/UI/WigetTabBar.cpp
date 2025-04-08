#include "WigetTabBar.h"
#include "WigetHandler.h"
#include "Utils/Logger.h"

namespace v3d
{
namespace ui
{

WigetTabBar::WigetTabBar() noexcept
    : WigetBase<WigetTabBar>(V3D_NEW(WigetTabBar::ContextTabBar, memory::MemoryLabel::MemoryUI)())
{
}

WigetTabBar::WigetTabBar(const WigetTabBar& other) noexcept
    : WigetBase<WigetTabBar>(other)
{
    WigetTabBar::ContextTabBar* context = V3D_NEW(WigetTabBar::ContextTabBar, memory::MemoryLabel::MemoryUI)();
    *context = *static_cast<WigetTabBar::ContextTabBar*>(other.m_data);
    m_data = context;
}

WigetTabBar::~WigetTabBar()
{
    if (m_data)
    {
        V3D_DELETE(m_data, memory::MemoryLabel::MemoryUI);
        m_data = nullptr;
    }
}

bool WigetTabBar::update(WigetHandler* handler, WigetLayout* layout, f32 dt)
{
    ContextTabBar& context = Wiget::cast_data<ContextTabBar>(m_data);
    if (context._onUpdate)
    {
        std::invoke(context._onUpdate, this, dt);
    }

    return  handler->draw_TabBar(this, m_data, dt);
}

} //namespace scene
} //namespace v3d