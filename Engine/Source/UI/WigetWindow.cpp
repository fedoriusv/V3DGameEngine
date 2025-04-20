#include "WigetWindow.h"
#include "WigetHandler.h"
#include "Utils/Logger.h"

namespace v3d
{
namespace ui
{

WigetWindow::WigetWindow(const std::string& title, WindowFlags flags) noexcept
    : WigetBase<WigetWindow>(V3D_NEW(StateType, memory::MemoryLabel::MemoryUI)())
{
    setTitle(title);

    Wiget::cast_data<StateType>(m_data)._flags = flags;
    Wiget::cast_data<StateType>(m_data)._layout.m_data->_stateMask |= Wiget::State::StateMask::WindowLayout | Wiget::State::StateMask::FirstUpdate;
}

WigetWindow::WigetWindow(const std::string& title, const math::Dimension2D& size, const math::Point2D& pos, WindowFlags flags) noexcept
    : WigetBase<WigetWindow>(V3D_NEW(StateType, memory::MemoryLabel::MemoryUI)())
{
    setTitle(title);
    setSize(size);
    setPosition(pos);

    Wiget::cast_data<StateType>(m_data)._flags = flags;
    Wiget::cast_data<StateType>(m_data)._layout.m_data->_stateMask |= Wiget::State::StateMask::WindowLayout | Wiget::State::StateMask::FirstUpdate;
}

WigetWindow::WigetWindow(const WigetWindow& other) noexcept
    : WigetBase<WigetWindow>(other)
{
    StateType* state = V3D_NEW(StateType, memory::MemoryLabel::MemoryUI)(*static_cast<const StateType*>(other.m_data));
    m_data = state;
}

WigetWindow::WigetWindow(WigetWindow&& other) noexcept
    : WigetBase<WigetWindow>(other)
{
    m_data = other.m_data;
    other.m_data = nullptr;
}

WigetWindow::~WigetWindow()
{
    if (m_data)
    {
        V3D_DELETE(m_data, memory::MemoryLabel::MemoryUI);
        m_data = nullptr;
    }
}

bool WigetWindow::update(WigetHandler* handler, Wiget* parent, Wiget* layout, f32 dt)
{
    if (Wiget::update(handler, parent, layout, dt))
    {
        return handler->getWigetDrawer()->draw_Window(this, m_data, dt);
    }

    return false;
}

Wiget* WigetWindow::copy() const
{
    return V3D_NEW(WigetWindow, memory::MemoryLabel::MemoryUI)(*this);
}

} // namespace ui
} // namespace v3d