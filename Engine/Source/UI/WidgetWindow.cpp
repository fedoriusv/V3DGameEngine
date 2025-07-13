#include "WidgetWindow.h"
#include "WidgetHandler.h"
#include "Utils/Logger.h"

namespace v3d
{
namespace ui
{

WidgetWindow::WidgetWindow(const std::string& title, WindowFlags flags) noexcept
    : WidgetBase<WidgetWindow>(V3D_NEW(StateType, memory::MemoryLabel::MemoryUI)())
{
    setTitle(title);

    Widget::cast_data<StateType>(m_data)._createFlags = flags;
    Widget::cast_data<StateType>(m_data)._layout.m_data->_stateMask |= Widget::State::StateMask::MainLayout | Widget::State::StateMask::FirstUpdateState;
}

WidgetWindow::WidgetWindow(const std::string& title, const math::Dimension2D& size, const math::Point2D& pos, WindowFlags flags) noexcept
    : WidgetBase<WidgetWindow>(V3D_NEW(StateType, memory::MemoryLabel::MemoryUI)())
{
    setTitle(title);
    setSize(size);
    setPosition(pos);

    Widget::cast_data<StateType>(m_data)._createFlags = flags;
    Widget::cast_data<StateType>(m_data)._layout.m_data->_stateMask |= Widget::State::StateMask::MainLayout | Widget::State::StateMask::FirstUpdateState;
}

WidgetWindow::WidgetWindow(const WidgetWindow& other) noexcept
    : WidgetBase<WidgetWindow>(other)
{
    StateType* state = V3D_NEW(StateType, memory::MemoryLabel::MemoryUI)(*static_cast<const StateType*>(other.m_data));
    m_data = state;
}

WidgetWindow::WidgetWindow(WidgetWindow&& other) noexcept
    : WidgetBase<WidgetWindow>(other)
{
    m_data = other.m_data;
    other.m_data = nullptr;
}

WidgetWindow::~WidgetWindow()
{
    if (m_data)
    {
        V3D_DELETE(m_data, memory::MemoryLabel::MemoryUI);
        m_data = nullptr;
    }
}

Widget* WidgetWindow::findWidgetByID(u64 id)
{
    for (auto& wiget : Widget::cast_data<StateType>(m_data)._layout.m_wigets)
    {
        if (wiget->getID() == id)
        {
            return wiget;
        }

        if (wiget->getType() == typeOf<WidgetLayout>())
        {
            Widget* child = static_cast<WidgetLayout*>(wiget)->findWidgetByID(id);
            if (child)
            {
                return child;
            }
        }
        else if (wiget->getType() == typeOf<WidgetHorizontalLayout>())
        {
            Widget* child = static_cast<WidgetHorizontalLayout*>(wiget)->findWidgetByID(id);
            if (child)
            {
                return child;
            }
        }
    }

    return nullptr;
}

bool WidgetWindow::update(WidgetHandler* handler, Widget* parent, Widget* layout, f32 dt)
{
    if (Widget::update(handler, parent, layout, dt))
    {
        return handler->getWidgetDrawer()->draw_Window(this, m_data, dt);
    }

    return false;
}

Widget* WidgetWindow::copy() const
{
    return V3D_NEW(WidgetWindow, memory::MemoryLabel::MemoryUI)(*this);
}

} // namespace ui
} // namespace v3d