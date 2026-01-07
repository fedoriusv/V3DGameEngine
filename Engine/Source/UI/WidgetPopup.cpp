#include "WidgetPopup.h"
#include "WidgetHandler.h"
#include "Utils/Logger.h"

namespace v3d
{
namespace ui
{

WidgetPopup::WidgetPopup(const std::string& title, PopupFlags flags) noexcept
    : WidgetBase<WidgetPopup>(V3D_NEW(StateType, memory::MemoryLabel::MemoryUI)())
{
    setTitle(title);

    Widget::cast_data<StateType>(m_data)._isVisible = false;
    Widget::cast_data<StateType>(m_data)._createFlags = flags;
    Widget::cast_data<StateType>(m_data)._stateMask |= Widget::State::StateMask::MainLayout | Widget::State::StateMask::FirstUpdateState;
}

WidgetPopup::WidgetPopup(const std::string& title, const math::Dimension2D& size, const math::Point2D& pos, PopupFlags flags) noexcept
    : WidgetBase<WidgetPopup>(V3D_NEW(StateType, memory::MemoryLabel::MemoryUI)())
{
    setTitle(title);
    setSize(size);
    setPosition(pos);

    Widget::cast_data<StateType>(m_data)._isVisible = false;
    Widget::cast_data<StateType>(m_data)._createFlags = flags;
    Widget::cast_data<StateType>(m_data)._stateMask |= Widget::State::StateMask::MainLayout | Widget::State::StateMask::FirstUpdateState;
}

WidgetPopup::WidgetPopup(const WidgetPopup& other) noexcept
    : WidgetBase<WidgetPopup>(other)
{
    StateType* state = V3D_NEW(StateType, memory::MemoryLabel::MemoryUI)(*static_cast<const StateType*>(other.m_data));
    m_data = state;
}

WidgetPopup::WidgetPopup(WidgetPopup&& other) noexcept
    : WidgetBase<WidgetPopup>(other)
{
    m_data = other.m_data;
    other.m_data = nullptr;
}

WidgetPopup::~WidgetPopup()
{
    if (m_data)
    {
        V3D_DELETE(m_data, memory::MemoryLabel::MemoryUI);
        m_data = nullptr;
    }
}

bool WidgetPopup::open(const std::string& args)
{
    if (!Widget::cast_data<StateType>(m_data)._isVisible)
    {
        Widget::cast_data<StateType>(m_data)._isVisible = true;
        Widget::cast_data<StateType>(m_data)._stateMask |= Widget::State::StateMask::FirstUpdateState;

        //onOpen callback 
    }

    return false;
}

bool WidgetPopup::close()
{
    return false;
}

bool WidgetPopup::update(WidgetHandler* handler, Widget* parent, Widget* layout, f32 dt)
{
    if (Widget::update(handler, parent, layout, dt))
    {
        return handler->getWidgetDrawer()->draw_Popup(this, m_data, dt);
    }

    return false;
}

math::float2 WidgetPopup::calculateSize(WidgetHandler* handler, Widget* parent, Widget* layout)
{
    return math::float2();
}

Widget* WidgetPopup::copy() const
{
    return V3D_NEW(WidgetPopup, memory::MemoryLabel::MemoryUI)(*this);
}

} //namespace ui
} //namespace v3d