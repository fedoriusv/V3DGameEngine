#include "WigetGroups.h"
#include "WigetHandler.h"

namespace v3d
{
namespace ui
{

/////////////////////////////////////////////////////////////////////////////////////////////////////

WigetRadioButtonGroup::WigetRadioButtonGroup() noexcept
    : WigetGroup<WigetRadioButtonGroup>(V3D_NEW(WigetRadioButtonGroup::ContextRadioButtonGroup, memory::MemoryLabel::MemoryUI)())
{
}

WigetRadioButtonGroup::WigetRadioButtonGroup(std::vector<std::string>& buttons) noexcept
    : WigetGroup<WigetRadioButtonGroup>(V3D_NEW(WigetRadioButtonGroup::ContextRadioButtonGroup, memory::MemoryLabel::MemoryUI)())
{
    for (auto& btn : buttons)
    {
        addElement(btn);
    }
}

WigetRadioButtonGroup::WigetRadioButtonGroup(const WigetRadioButtonGroup& other) noexcept
    : WigetGroup<WigetRadioButtonGroup>(other)
{
    WigetRadioButtonGroup::ContextRadioButtonGroup* context = V3D_NEW(WigetRadioButtonGroup::ContextRadioButtonGroup, memory::MemoryLabel::MemoryUI)();
    *context = *static_cast<WigetRadioButtonGroup::ContextRadioButtonGroup*>(other.m_data);
    m_data = context;
}

WigetRadioButtonGroup::~WigetRadioButtonGroup()
{
    if (m_data)
    {
        V3D_DELETE(m_data, memory::MemoryLabel::MemoryUI);
        m_data = nullptr;
    }
}

bool WigetRadioButtonGroup::update(WigetHandler* handler, WigetLayout* layout, f32 dt)
{
    if (Wiget::update(handler, layout, dt))
    {
        return handler->draw_RadioButtonGroup(this, m_data, dt);
    }

    return false;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////

WigetComboBox::WigetComboBox() noexcept
    : WigetGroup<WigetComboBox>(V3D_NEW(WigetComboBox::ContextComboBox, memory::MemoryLabel::MemoryUI)())
{
}

WigetComboBox::WigetComboBox(const WigetComboBox& other) noexcept
    : WigetGroup<WigetComboBox>(other)
{
    WigetComboBox::ContextComboBox* context = V3D_NEW(WigetComboBox::ContextComboBox, memory::MemoryLabel::MemoryUI)();
    *context = *static_cast<WigetComboBox::ContextComboBox*>(other.m_data);
    m_data = context;
}

WigetComboBox::~WigetComboBox()
{
    if (m_data)
    {
        V3D_DELETE(m_data, memory::MemoryLabel::MemoryUI);
        m_data = nullptr;
    }
}

bool WigetComboBox::update(WigetHandler* handler, WigetLayout* layout, f32 dt)
{
    if (Wiget::update(handler, layout, dt))
    {
        return handler->draw_ComboBox(this, m_data, dt);
    }

    return false;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////

WigetListBox::WigetListBox() noexcept
    : WigetGroup<WigetListBox>(V3D_NEW(WigetListBox::ContextListBox, memory::MemoryLabel::MemoryUI)())
{
}

WigetListBox::WigetListBox(const WigetListBox& other) noexcept
    : WigetGroup<WigetListBox>(other)
{
    WigetListBox::ContextListBox* context = V3D_NEW(WigetListBox::ContextListBox, memory::MemoryLabel::MemoryUI)();
    *context = *static_cast<WigetListBox::ContextListBox*>(other.m_data);
    m_data = context;
}

WigetListBox::~WigetListBox()
{
    if (m_data)
    {
        V3D_DELETE(m_data, memory::MemoryLabel::MemoryUI);
        m_data = nullptr;
    }
}

bool WigetListBox::update(WigetHandler* handler, WigetLayout* layout, f32 dt)
{
    if (Wiget::update(handler, layout, dt))
    {
        return handler->draw_ListBox(this, m_data, dt);
    }

    return false;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////

} // namespace ui
} // namespace v3d