#include "WidgetGizmo.h"
#include "WidgetHandler.h"

namespace v3d
{
namespace ui
{

/////////////////////////////////////////////////////////////////////////////////////////////////////

WidgetGizmo::WidgetGizmo(const scene::Camera* camera) noexcept
    : WidgetBase<WidgetGizmo>(V3D_NEW(StateType, memory::MemoryLabel::MemoryUI)())
{
    setCamera(camera);
}

WidgetGizmo::WidgetGizmo(const WidgetGizmo& other) noexcept
    : WidgetBase<WidgetGizmo>(other)
{
    StateType* state = V3D_NEW(StateType, memory::MemoryLabel::MemoryUI)(*static_cast<const StateType*>(other.m_data));
    m_data = state;
}

WidgetGizmo::WidgetGizmo(WidgetGizmo&& other) noexcept
    : WidgetBase<WidgetGizmo>(other)
{
    m_data = other.m_data;
    other.m_data = nullptr;
}

WidgetGizmo::~WidgetGizmo()
{
    if (m_data)
    {
        V3D_DELETE(m_data, memory::MemoryLabel::MemoryUI);
        m_data = nullptr;
    }
}

bool WidgetGizmo::update(WidgetHandler* handler, Widget* parent, Widget* layout, f32 dt)
{
    if (Widget::update(handler, parent, layout, dt))
    {
        handler->getWidgetDrawer()->draw_Gizmo(this, parent, static_cast<WidgetType*>(layout)->m_data, m_data);
        return true;
    }

    return false;
}

Widget* WidgetGizmo::copy() const
{
    return V3D_NEW(WidgetGizmo, memory::MemoryLabel::MemoryUI)(*this);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////

WidgetViewManipulator::WidgetViewManipulator(const scene::Camera* camera) noexcept
    : WidgetBase<WidgetViewManipulator>(V3D_NEW(StateType, memory::MemoryLabel::MemoryUI)())
{
    setCamera(camera);
}

WidgetViewManipulator::WidgetViewManipulator(const WidgetViewManipulator& other) noexcept
    : WidgetBase<WidgetViewManipulator>(other)
{
    StateType* state = V3D_NEW(StateType, memory::MemoryLabel::MemoryUI)(*static_cast<const StateType*>(other.m_data));
    m_data = state;
}

WidgetViewManipulator::WidgetViewManipulator(WidgetViewManipulator&& other) noexcept
    : WidgetBase<WidgetViewManipulator>(other)
{
    m_data = other.m_data;
    other.m_data = nullptr;
}

WidgetViewManipulator::~WidgetViewManipulator()
{
    if (m_data)
    {
        V3D_DELETE(m_data, memory::MemoryLabel::MemoryUI);
        m_data = nullptr;
    }
}

bool WidgetViewManipulator::update(WidgetHandler* handler, Widget* parent, Widget* layout, f32 dt)
{
    if (Widget::update(handler, parent, layout, dt))
    {
        handler->getWidgetDrawer()->draw_ViewManipulator(this, parent, static_cast<WidgetType*>(layout)->m_data, m_data);
        return true;
    }

    return false;
}

Widget* WidgetViewManipulator::copy() const
{
    return V3D_NEW(WidgetViewManipulator, memory::MemoryLabel::MemoryUI)(*this);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////

} // namespace ui
} // namespace v3d