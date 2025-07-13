#include "WidgetTreeNode.h"
#include "WidgetHandler.h"

namespace v3d
{
namespace ui
{

WidgetTreeNode::WidgetTreeNode(const std::string& text, TreeNodeFlags flags) noexcept
    : WidgetBase<WidgetTreeNode>(V3D_NEW(StateType, memory::MemoryLabel::MemoryUI)())
{
    setText(text);
    Widget::cast_data<StateType>(m_data)._createFlags = flags;
    Widget::cast_data<StateType>(m_data)._layout.m_data->_stateMask |= Widget::State::StateMask::MainLayout;
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
    math::float2 layoutSize = { 0.0f , 0.0f };
    WidgetLayout& treeNodeLayout = Widget::cast_data<StateType>(m_data)._layout;

    WidgetDrawer* drawer = handler->getWidgetDrawer();
    if (!(Widget::cast_data<StateTreeNode>(m_data)._stateMask & Widget::State::StateMask::CollapsedState))
    {
        layoutSize += treeNodeLayout.calculateSize(handler, parent, this);
        if (cast_data<StateType>(m_data)._stateMask & Widget::State::StateMask::HorizontalLine)
        {
            layoutSize._x += drawer->get_WindowPadding()._x * 2.0f;
        }
        else
        {
            layoutSize._y += drawer->get_WindowPadding()._y * 2.0f;
        }
    }
    layoutSize += drawer->calculate_TreeNodeSize(this, static_cast<WidgetType*>(layout)->m_data, m_data);

    m_data->_itemRect = { {0, 0}, layoutSize };
    return m_data->_itemRect.getSize();
}

Widget* WidgetTreeNode::copy() const
{
    return V3D_NEW(WidgetTreeNode, memory::MemoryLabel::MemoryUI)(*this);
}

} // namespace ui
} // namespace v3d