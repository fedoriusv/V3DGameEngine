#include "WidgetTable.h"
#include "WidgetHandler.h"

namespace v3d
{
namespace ui
{

WidgetTable::WidgetTable() noexcept
    : WidgetBase<WidgetTable>(V3D_NEW(StateType, memory::MemoryLabel::MemoryUI)())
{
}

WidgetTable::WidgetTable(const WidgetTable& other) noexcept
    : WidgetBase<WidgetTable>(other)
{
    StateType* state = V3D_NEW(StateType, memory::MemoryLabel::MemoryUI)(*static_cast<const StateType*>(other.m_data));
    m_data = state;
}

WidgetTable::WidgetTable(WidgetTable&& other) noexcept
    : WidgetBase<WidgetTable>(other)
{
    m_data = other.m_data;
    other.m_data = nullptr;
}

WidgetTable::~WidgetTable()
{
    if (m_data)
    {
        V3D_DELETE(m_data, memory::MemoryLabel::MemoryUI);
        m_data = nullptr;
    }
}

bool WidgetTable::update(WidgetHandler* handler, Widget* parent, Widget* layout, f32 dt)
{
    return false;
}

math::float2 WidgetTable::calculateSize(WidgetHandler* handler, Widget* parent, Widget* layout)
{
    m_data->_itemRect = { {0, 0}, handler->getWidgetDrawer()->calculate_TableSize(this, static_cast<WidgetType*>(layout)->m_data, m_data) };
    return m_data->_itemRect.getSize();
}

Widget* WidgetTable::copy() const
{
    return V3D_NEW(WidgetTable, memory::MemoryLabel::MemoryUI)(*this);
}

} //namespace ui
} //namespace v3d