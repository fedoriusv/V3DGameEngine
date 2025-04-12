#include "WigetWindow.h"
#include "WigetHandler.h"
#include "Utils/Logger.h"

namespace v3d
{
namespace ui
{

WigetWindow::WigetWindow(const std::string& title, const math::Dimension2D& size, const math::Point2D& pos, WindowFlags flags) noexcept
    : WigetBase<WigetWindow>(V3D_NEW(WigetWindow::ContextWindow, memory::MemoryLabel::MemoryUI)())
{
    setTitle(title);
    setSize(size);
    setPosition(pos);

    Wiget::cast_data<ContextWindow>(m_data)._flags = flags;
}

WigetWindow::WigetWindow(const WigetWindow& other) noexcept
    : WigetBase<WigetWindow>(other)
{
    WigetWindow::ContextWindow* context = V3D_NEW(WigetWindow::ContextWindow, memory::MemoryLabel::MemoryUI)();
    *context = *static_cast<WigetWindow::ContextWindow*>(other.m_data);
    m_data = context;
}

WigetWindow::~WigetWindow()
{
    if (m_data)
    {
        V3D_DELETE(m_data, memory::MemoryLabel::MemoryUI);
        m_data = nullptr;
    }
}

bool WigetWindow::update(WigetHandler* handler, WigetLayout* layout, f32 dt)
{
    if (Wiget::update(handler, layout, dt))
    {
        return handler->draw_Window(this, m_data, dt);
    }

    return false;
}

} // namespace ui
} // namespace v3d