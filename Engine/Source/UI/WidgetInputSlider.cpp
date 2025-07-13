#include "WidgetInputSlider.h"
#include "WidgetHandler.h"
#include "WidgetLayout.h"

namespace v3d
{
namespace ui
{

WidgetInputSlider::WidgetInputSlider() noexcept
    : WidgetBase<WidgetInputSlider>(V3D_NEW(StateType, memory::MemoryLabel::MemoryUI)())
{
}

WidgetInputSlider::WidgetInputSlider(const WidgetInputSlider& other) noexcept
    : WidgetBase<WidgetInputSlider>(other)
{
    StateType* state = V3D_NEW(StateType, memory::MemoryLabel::MemoryUI)();
    *state = *static_cast<StateType*>(other.m_data);
    m_data = state;
}

WidgetInputSlider::~WidgetInputSlider()
{
    if (m_data)
    {
        V3D_DELETE(m_data, memory::MemoryLabel::MemoryUI);
        m_data = nullptr;
    }
}

bool WidgetInputSlider::update(WidgetHandler* handler, Widget* parent, Widget* layout, f32 dt)
{
    if (Widget::update(handler, parent, layout, dt))
    {
        //return handler->getWidgetDrawer()->draw_InputSliderFloat(this, parent, static_cast<WidgetType*>(layout)->m_data, m_data);
    }

    return false;
}

Widget* WidgetInputSlider::copy() const
{
    return V3D_NEW(WidgetInputSlider, memory::MemoryLabel::MemoryUI)(*this);
}

} // namespace ui
} // namespace v3d