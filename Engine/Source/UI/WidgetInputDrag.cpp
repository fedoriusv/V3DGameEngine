#include "WidgetInputDrag.h"
#include "WidgetHandler.h"
#include "WidgetLayout.h"

namespace v3d
{
namespace ui
{

/////////////////////////////////////////////////////////////////////////////////////////////////////

WidgetInputDragInt::WidgetInputDragInt(s32 value) noexcept
    : WidgetInputDragBase(V3D_NEW(StateType, memory::MemoryLabel::MemoryUI)())
{
    setValue(value);
}

WidgetInputDragInt::WidgetInputDragInt(const WidgetInputDragInt& other) noexcept
    : WidgetInputDragBase(other)
{
    StateType* state = V3D_NEW(StateType, memory::MemoryLabel::MemoryUI)();
    *state = *static_cast<StateType*>(other.m_data);
    m_data = state;
}

WidgetInputDragInt::WidgetInputDragInt(WidgetInputDragInt&& other) noexcept
    : WidgetInputDragBase(other)
{
    m_data = other.m_data;
    other.m_data = nullptr;
}

WidgetInputDragInt::~WidgetInputDragInt()
{
    if (m_data)
    {
        V3D_DELETE(m_data, memory::MemoryLabel::MemoryUI);
        m_data = nullptr;
    }
}

bool WidgetInputDragInt::update(WidgetHandler* handler, Widget* parent, Widget* layout, f32 dt)
{
    if (Widget::update(handler, parent, layout, dt))
    {
        ASSERT(Widget::cast_data<StateType>(m_data)._value.size() == 1, "must be 1");
        return handler->getWidgetDrawer()->draw_InputDragValue(this, parent, static_cast<WidgetType*>(layout)->m_data, m_data, false);
    }

    return true;
}

math::float2 WidgetInputDragInt::calculateSize(WidgetHandler* handler, Widget* parent, Widget* layout)
{
    m_data->_itemRect = { {0, 0}, handler->getWidgetDrawer()->calculate_InputDragValueSize(this, static_cast<WidgetType*>(layout)->m_data, m_data, false) };
    return m_data->_itemRect.getSize();
}

Widget* WidgetInputDragInt::copy() const
{
    return V3D_NEW(WidgetInputDragInt, memory::MemoryLabel::MemoryUI)(*this);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////

WidgetInputDragFloat::WidgetInputDragFloat(f32 value) noexcept
    : WidgetInputDragBase(V3D_NEW(StateType, memory::MemoryLabel::MemoryUI)())
{
    setValue(value);
}

WidgetInputDragFloat::WidgetInputDragFloat(const WidgetInputDragFloat& other) noexcept
    : WidgetInputDragBase(other)
{
    StateType* state = V3D_NEW(StateType, memory::MemoryLabel::MemoryUI)();
    *state = *static_cast<StateType*>(other.m_data);
    m_data = state;
}

WidgetInputDragFloat::WidgetInputDragFloat(WidgetInputDragFloat&& other) noexcept
    : WidgetInputDragBase(other)
{
    m_data = other.m_data;
    other.m_data = nullptr;
}

WidgetInputDragFloat::~WidgetInputDragFloat()
{
    if (m_data)
    {
        V3D_DELETE(m_data, memory::MemoryLabel::MemoryUI);
        m_data = nullptr;
    }
}

bool WidgetInputDragFloat::update(WidgetHandler* handler, Widget* parent, Widget* layout, f32 dt)
{
    if (Widget::update(handler, parent, layout, dt))
    {
        return handler->getWidgetDrawer()->draw_InputDragValue(this, parent, static_cast<WidgetType*>(layout)->m_data, m_data, true);
    }

    return true;
}

math::float2 WidgetInputDragFloat::calculateSize(WidgetHandler* handler, Widget* parent, Widget* layout)
{
    m_data->_itemRect = { {0, 0}, handler->getWidgetDrawer()->calculate_InputDragValueSize(this, static_cast<WidgetType*>(layout)->m_data, m_data, true) };
    return m_data->_itemRect.getSize();
}

Widget* WidgetInputDragFloat::copy() const
{
    return V3D_NEW(WidgetInputDragFloat, memory::MemoryLabel::MemoryUI)(*this);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////

} // namespace ui
} // namespace v3d