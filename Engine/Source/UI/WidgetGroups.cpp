#include "WidgetGroups.h"
#include "WidgetHandler.h"
#include "WidgetLayout.h"

namespace v3d
{
namespace ui
{

/////////////////////////////////////////////////////////////////////////////////////////////////////

WidgetRadioButtonGroup::WidgetRadioButtonGroup() noexcept
    : WidgetGroupBase<WidgetRadioButtonGroup>(V3D_NEW(StateType, memory::MemoryLabel::MemoryUI)())
{
}

WidgetRadioButtonGroup::WidgetRadioButtonGroup(std::vector<std::string>& buttons) noexcept
    : WidgetGroupBase<WidgetRadioButtonGroup>(V3D_NEW(StateType, memory::MemoryLabel::MemoryUI)())
{
    for (auto& btn : buttons)
    {
        addElement(btn);
    }
}

WidgetRadioButtonGroup::WidgetRadioButtonGroup(const WidgetRadioButtonGroup& other) noexcept
    : WidgetGroupBase<WidgetRadioButtonGroup>(other)
{
    StateType* state = V3D_NEW(StateType, memory::MemoryLabel::MemoryUI)();
    *state = *static_cast<StateType*>(other.m_data);
    m_data = state;
}

WidgetRadioButtonGroup::~WidgetRadioButtonGroup()
{
    if (m_data)
    {
        V3D_DELETE(m_data, memory::MemoryLabel::MemoryUI);
        m_data = nullptr;
    }
}

bool WidgetRadioButtonGroup::update(WidgetHandler* handler, Widget* parent, Widget* layout, f32 dt)
{
    if (Widget::update(handler, parent, layout, dt))
    {
        return handler->getWidgetDrawer()->draw_RadioButtonGroup(this, parent, static_cast<WidgetType*>(layout)->m_data, m_data);
    }

    return false;
}

Widget* WidgetRadioButtonGroup::copy() const
{
    return V3D_NEW(WidgetRadioButtonGroup, memory::MemoryLabel::MemoryUI)(*this);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////

WidgetComboBox::WidgetComboBox() noexcept
    : WidgetGroupBase<WidgetComboBox>(V3D_NEW(StateType, memory::MemoryLabel::MemoryUI)())
{
}

WidgetComboBox::WidgetComboBox(const WidgetComboBox& other) noexcept
    : WidgetGroupBase<WidgetComboBox>(other)
{
    StateType* state = V3D_NEW(StateType, memory::MemoryLabel::MemoryUI)();
    *state = *static_cast<StateType*>(other.m_data);
    m_data = state;
}

WidgetComboBox::~WidgetComboBox()
{
    if (m_data)
    {
        V3D_DELETE(m_data, memory::MemoryLabel::MemoryUI);
        m_data = nullptr;
    }
}

bool WidgetComboBox::update(WidgetHandler* handler, Widget* parent, Widget* layout, f32 dt)
{
    if (Widget::update(handler, parent, layout, dt))
    {
        return handler->getWidgetDrawer()->draw_ComboBox(this, parent, static_cast<WidgetType*>(layout)->m_data, m_data);
    }

    return false;
}

Widget* WidgetComboBox::copy() const
{
    return V3D_NEW(WidgetComboBox, memory::MemoryLabel::MemoryUI)(*this);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////

WidgetListBox::WidgetListBox() noexcept
    : WidgetGroupBase<WidgetListBox>(V3D_NEW(StateType, memory::MemoryLabel::MemoryUI)())
{
}

WidgetListBox::WidgetListBox(const WidgetListBox& other) noexcept
    : WidgetGroupBase<WidgetListBox>(other)
{
    StateType* state = V3D_NEW(StateType, memory::MemoryLabel::MemoryUI)();
    *state = *static_cast<StateType*>(other.m_data);
    m_data = state;
}

WidgetListBox::~WidgetListBox()
{
    if (m_data)
    {
        V3D_DELETE(m_data, memory::MemoryLabel::MemoryUI);
        m_data = nullptr;
    }
}

bool WidgetListBox::update(WidgetHandler* handler, Widget* parent, Widget* layout, f32 dt)
{
    if (Widget::update(handler, parent, layout, dt))
    {
        return handler->getWidgetDrawer()->draw_ListBox(this, parent, static_cast<WidgetType*>(layout)->m_data, m_data);
    }

    return false;
}

Widget* WidgetListBox::copy() const
{
    return V3D_NEW(WidgetListBox, memory::MemoryLabel::MemoryUI)(*this);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////

} // namespace ui
} // namespace v3d