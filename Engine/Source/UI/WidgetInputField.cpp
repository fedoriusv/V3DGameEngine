#include "WidgetInputField.h"
#include "WidgetHandler.h"
#include "WidgetLayout.h"

namespace v3d
{
namespace ui
{

WidgetInputText::WidgetInputText(const std::string& text) noexcept
    : WidgetInputFieldBase(V3D_NEW(StateType, memory::MemoryLabel::MemoryUI)())
{
    setValue(text);
    Widget::cast_data<StateType>(m_data)._multiline = false;
}

WidgetInputText::WidgetInputText(const std::string& text, const math::Dimension2D& size) noexcept
    : WidgetInputFieldBase(V3D_NEW(StateType, memory::MemoryLabel::MemoryUI)())
{
    setValue(text);
    Widget::cast_data<StateType>(m_data)._size = size;
    Widget::cast_data<StateType>(m_data)._multiline = true;
}

WidgetInputText::WidgetInputText(const WidgetInputText& other) noexcept
    : WidgetInputFieldBase(other)
{
    StateType* state = V3D_NEW(StateType, memory::MemoryLabel::MemoryUI)();
    *state = *static_cast<StateType*>(other.m_data);
    m_data = state;
}

WidgetInputText::WidgetInputText(WidgetInputText&& other) noexcept
    : WidgetInputFieldBase(other)
{
    m_data = other.m_data;
    other.m_data = nullptr;
}

WidgetInputText::~WidgetInputText()
{
    if (m_data)
    {
        V3D_DELETE(m_data, memory::MemoryLabel::MemoryUI);
        m_data = nullptr;
    }
}

bool WidgetInputText::update(WidgetHandler* handler, Widget* parent, Widget* layout, f32 dt)
{
    if (Widget::update(handler, parent, layout, dt))
    {
        return handler->getWidgetDrawer()->draw_InputText(this, parent, static_cast<WidgetType*>(layout)->m_data, m_data);
    }

    return true;
}

Widget* WidgetInputText::copy() const
{
    return V3D_NEW(WidgetInputText, memory::MemoryLabel::MemoryUI)(*this);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////

WidgetInputInt::WidgetInputInt(s32 value) noexcept
    : WidgetInputFieldBase(V3D_NEW(StateType, memory::MemoryLabel::MemoryUI)())
{
    setValue(value);
}

WidgetInputInt::WidgetInputInt(const WidgetInputInt& other) noexcept
    : WidgetInputFieldBase(other)
{
    StateType* state = V3D_NEW(StateType, memory::MemoryLabel::MemoryUI)();
    *state = *static_cast<StateType*>(other.m_data);
    m_data = state;
}

WidgetInputInt::WidgetInputInt(WidgetInputInt&& other) noexcept
    : WidgetInputFieldBase(other)
{
    m_data = other.m_data;
    other.m_data = nullptr;
}

WidgetInputInt::~WidgetInputInt()
{
    if (m_data)
    {
        V3D_DELETE(m_data, memory::MemoryLabel::MemoryUI);
        m_data = nullptr;
    }
}

bool WidgetInputInt::update(WidgetHandler* handler, Widget* parent, Widget* layout, f32 dt)
{
    if (Widget::update(handler, parent, layout, dt))
    {
        u32 size = Widget::cast_data<StateType>(m_data)._value.size();
        return handler->getWidgetDrawer()->draw_InputInt(this, parent, static_cast<WidgetType*>(layout)->m_data, m_data, size);
    }

    return true;
}

Widget* WidgetInputInt::copy() const
{
    return V3D_NEW(WidgetInputInt, memory::MemoryLabel::MemoryUI)(*this);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////

WidgetInputFloat::WidgetInputFloat(f32 value) noexcept
    : WidgetInputFieldBase(V3D_NEW(StateType, memory::MemoryLabel::MemoryUI)())
{
    setValue(value);
}

WidgetInputFloat::WidgetInputFloat(const WidgetInputFloat& other) noexcept
    : WidgetInputFieldBase(other)
{
    StateType* state = V3D_NEW(StateType, memory::MemoryLabel::MemoryUI)();
    *state = *static_cast<StateType*>(other.m_data);
    m_data = state;
}

WidgetInputFloat::WidgetInputFloat(WidgetInputFloat&& other) noexcept
    : WidgetInputFieldBase(other)
{
    m_data = other.m_data;
    other.m_data = nullptr;
}

WidgetInputFloat::~WidgetInputFloat()
{
    if (m_data)
    {
        V3D_DELETE(m_data, memory::MemoryLabel::MemoryUI);
        m_data = nullptr;
    }
}

bool WidgetInputFloat::update(WidgetHandler* handler, Widget* parent, Widget* layout, f32 dt)
{
    if (Widget::update(handler, parent, layout, dt))
    {
        u32 size = Widget::cast_data<StateType>(m_data)._value.size();
        return handler->getWidgetDrawer()->draw_InputFloat(this, parent, static_cast<WidgetType*>(layout)->m_data, m_data, size);
    }

    return true;
}

Widget* WidgetInputFloat::copy() const
{
    return V3D_NEW(WidgetInputFloat, memory::MemoryLabel::MemoryUI)(*this);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////

WidgetInputFloat3::WidgetInputFloat3(f32 value0, f32 value1, f32 value2) noexcept
    : WidgetInputFieldBase(V3D_NEW(StateType, memory::MemoryLabel::MemoryUI)())
{
    setValue(value0, value1, value2);
}

WidgetInputFloat3::WidgetInputFloat3(const WidgetInputFloat3& other) noexcept
    : WidgetInputFieldBase(other)
{
    StateType* state = V3D_NEW(StateType, memory::MemoryLabel::MemoryUI)();
    *state = *static_cast<StateType*>(other.m_data);
    m_data = state;
}

WidgetInputFloat3::WidgetInputFloat3(WidgetInputFloat3&& other) noexcept
    : WidgetInputFieldBase(other)
{
    m_data = other.m_data;
    other.m_data = nullptr;
}

WidgetInputFloat3::~WidgetInputFloat3()
{
    if (m_data)
    {
        V3D_DELETE(m_data, memory::MemoryLabel::MemoryUI);
        m_data = nullptr;
    }
}

bool WidgetInputFloat3::update(WidgetHandler* handler, Widget* parent, Widget* layout, f32 dt)
{
    if (Widget::update(handler, parent, layout, dt))
    {
        auto value = Widget::cast_data<StateType>(m_data)._value;
        return handler->getWidgetDrawer()->draw_InputFloat(this, parent, static_cast<WidgetType*>(layout)->m_data, m_data, value.size());
    }

    return true;
}

Widget* WidgetInputFloat3::copy() const
{
    return V3D_NEW(WidgetInputFloat3, memory::MemoryLabel::MemoryUI)(*this);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace ui
} //namespace v3d