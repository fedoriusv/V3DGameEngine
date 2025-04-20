#include "WigetGroups.h"
#include "WigetHandler.h"
#include "WigetLayout.h"

namespace v3d
{
namespace ui
{

/////////////////////////////////////////////////////////////////////////////////////////////////////

WigetRadioButtonGroup::WigetRadioButtonGroup() noexcept
    : WigetGroupBase<WigetRadioButtonGroup>(V3D_NEW(StateType, memory::MemoryLabel::MemoryUI)())
{
}

WigetRadioButtonGroup::WigetRadioButtonGroup(std::vector<std::string>& buttons) noexcept
    : WigetGroupBase<WigetRadioButtonGroup>(V3D_NEW(StateType, memory::MemoryLabel::MemoryUI)())
{
    for (auto& btn : buttons)
    {
        addElement(btn);
    }
}

WigetRadioButtonGroup::WigetRadioButtonGroup(const WigetRadioButtonGroup& other) noexcept
    : WigetGroupBase<WigetRadioButtonGroup>(other)
{
    StateType* state = V3D_NEW(StateType, memory::MemoryLabel::MemoryUI)();
    *state = *static_cast<StateType*>(other.m_data);
    m_data = state;
}

WigetRadioButtonGroup::~WigetRadioButtonGroup()
{
    if (m_data)
    {
        V3D_DELETE(m_data, memory::MemoryLabel::MemoryUI);
        m_data = nullptr;
    }
}

bool WigetRadioButtonGroup::update(WigetHandler* handler, Wiget* parent, Wiget* layout, f32 dt)
{
    if (Wiget::update(handler, parent, layout, dt))
    {
        return handler->getWigetDrawer()->draw_RadioButtonGroup(this, parent, static_cast<WigetType*>(layout)->m_data, m_data);
    }

    return false;
}

Wiget* WigetRadioButtonGroup::copy() const
{
    return V3D_NEW(WigetRadioButtonGroup, memory::MemoryLabel::MemoryUI)(*this);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////

WigetComboBox::WigetComboBox() noexcept
    : WigetGroupBase<WigetComboBox>(V3D_NEW(StateType, memory::MemoryLabel::MemoryUI)())
{
}

WigetComboBox::WigetComboBox(const WigetComboBox& other) noexcept
    : WigetGroupBase<WigetComboBox>(other)
{
    StateType* state = V3D_NEW(StateType, memory::MemoryLabel::MemoryUI)();
    *state = *static_cast<StateType*>(other.m_data);
    m_data = state;
}

WigetComboBox::~WigetComboBox()
{
    if (m_data)
    {
        V3D_DELETE(m_data, memory::MemoryLabel::MemoryUI);
        m_data = nullptr;
    }
}

bool WigetComboBox::update(WigetHandler* handler, Wiget* parent, Wiget* layout, f32 dt)
{
    if (Wiget::update(handler, parent, layout, dt))
    {
        return handler->getWigetDrawer()->draw_ComboBox(this, parent, static_cast<WigetType*>(layout)->m_data, m_data);
    }

    return false;
}

Wiget* WigetComboBox::copy() const
{
    return V3D_NEW(WigetComboBox, memory::MemoryLabel::MemoryUI)(*this);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////

WigetListBox::WigetListBox() noexcept
    : WigetGroupBase<WigetListBox>(V3D_NEW(StateType, memory::MemoryLabel::MemoryUI)())
{
}

WigetListBox::WigetListBox(const WigetListBox& other) noexcept
    : WigetGroupBase<WigetListBox>(other)
{
    StateType* state = V3D_NEW(StateType, memory::MemoryLabel::MemoryUI)();
    *state = *static_cast<StateType*>(other.m_data);
    m_data = state;
}

WigetListBox::~WigetListBox()
{
    if (m_data)
    {
        V3D_DELETE(m_data, memory::MemoryLabel::MemoryUI);
        m_data = nullptr;
    }
}

bool WigetListBox::update(WigetHandler* handler, Wiget* parent, Wiget* layout, f32 dt)
{
    if (Wiget::update(handler, parent, layout, dt))
    {
        return handler->getWigetDrawer()->draw_ListBox(this, parent, static_cast<WigetType*>(layout)->m_data, m_data);
    }

    return false;
}

Wiget* WigetListBox::copy() const
{
    return V3D_NEW(WigetListBox, memory::MemoryLabel::MemoryUI)(*this);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////

} // namespace ui
} // namespace v3d