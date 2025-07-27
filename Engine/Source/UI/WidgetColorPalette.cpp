#include "WidgetColorPalette.h"
#include "WidgetHandler.h"

namespace v3d
{
namespace ui
{

WidgetColorPalette::WidgetColorPalette() noexcept
    : WidgetBase<WidgetType>(V3D_NEW(StateType, memory::MemoryLabel::MemoryUI)())
{
}

WidgetColorPalette::WidgetColorPalette(const WidgetColorPalette& other) noexcept
    : WidgetBase<WidgetType>(other)
{
    StateType* state = V3D_NEW(StateType, memory::MemoryLabel::MemoryUI)(*static_cast<const StateType*>(other.m_data));
    m_data = state;
}

WidgetColorPalette::WidgetColorPalette(WidgetColorPalette&& other) noexcept
    : WidgetBase<WidgetType>(other)
{
    m_data = other.m_data;
    other.m_data = nullptr;
}

WidgetColorPalette::~WidgetColorPalette()
{
    if (m_data)
    {
        V3D_DELETE(m_data, memory::MemoryLabel::MemoryUI);
        m_data = nullptr;
    }
}

bool WidgetColorPalette::update(WidgetHandler* handler, Widget* parent, Widget* layout, f32 dt)
{
    if (Widget::update(handler, parent, layout, dt))
    {
        return handler->getWidgetDrawer()->draw_ColorPalette(this, parent, static_cast<WidgetType*>(layout)->m_data, m_data);
    }

    return false;
}

Widget* WidgetColorPalette::copy() const
{
    return V3D_NEW(WidgetColorPalette, memory::MemoryLabel::MemoryUI)(*this);
}

} //namespace ui
} //namespace v3d