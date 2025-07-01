#include "WidgetTreeNode.h"
#include "WidgetHandler.h"

namespace v3d
{
namespace ui
{

WidgetTreeNode::WidgetTreeNode(const std::string& text) noexcept
    : WidgetBase<WidgetTreeNode>(V3D_NEW(StateType, memory::MemoryLabel::MemoryUI)())
{
    setText(text);
}

WidgetTreeNode::WidgetTreeNode(const WidgetTreeNode& other) noexcept
    : WidgetBase<WidgetTreeNode>(other)
{
    StateType* state = V3D_NEW(StateType, memory::MemoryLabel::MemoryUI)(*static_cast<const StateType*>(other.m_data));
    m_data = state;
}

WidgetTreeNode::WidgetTreeNode(WidgetTreeNode&& other) noexcept
    : WidgetBase<WidgetTreeNode>(other)
{
    m_data = other.m_data;
    other.m_data = nullptr;
}

WidgetTreeNode::~WidgetTreeNode()
{
    if (m_data)
    {
        V3D_DELETE(m_data, memory::MemoryLabel::MemoryUI);
        m_data = nullptr;
    }
}

bool WidgetTreeNode::update(WidgetHandler* handler, Widget* parent, Widget* layout, f32 dt)
{
    if (Widget::update(handler, parent, layout, dt))
    {
        return handler->getWidgetDrawer()->draw_TreeNode(this, parent, static_cast<WidgetType*>(layout)->m_data, m_data, dt);
    }

    return false;
}

math::float2 WidgetTreeNode::calculateSize(WidgetHandler* handler, Widget* parent, Widget* layout)
{
    m_data->_itemRect = { {0, 0}, handler->getWidgetDrawer()->calculate_TreeNodeSize(this, static_cast<WidgetType*>(layout)->m_data, m_data) };
    return m_data->_itemRect.getSize();
}

Widget* WidgetTreeNode::copy() const
{
    return V3D_NEW(WidgetTreeNode, memory::MemoryLabel::MemoryUI)(*this);
}

} //namespace ui
} //namespace v3d