#include "WigetGizmo.h"
#include "WigetHandler.h"

namespace v3d
{
namespace ui
{

/////////////////////////////////////////////////////////////////////////////////////////////////////

WigetGizmo::WigetGizmo(const scene::Camera* camera) noexcept
    : WigetBase<WigetGizmo>(V3D_NEW(StateType, memory::MemoryLabel::MemoryUI)())
{
    setCamera(camera);
}

WigetGizmo::WigetGizmo(const WigetGizmo& other) noexcept
    : WigetBase<WigetGizmo>(other)
{
    StateType* state = V3D_NEW(StateType, memory::MemoryLabel::MemoryUI)(*static_cast<const StateType*>(other.m_data));
    m_data = state;
}

WigetGizmo::WigetGizmo(WigetGizmo&& other) noexcept
    : WigetBase<WigetGizmo>(other)
{
    m_data = other.m_data;
    other.m_data = nullptr;
}

WigetGizmo::~WigetGizmo()
{
    if (m_data)
    {
        V3D_DELETE(m_data, memory::MemoryLabel::MemoryUI);
        m_data = nullptr;
    }
}

bool WigetGizmo::update(WigetHandler* handler, Wiget* parent, Wiget* layout, f32 dt)
{
    if (Wiget::update(handler, parent, layout, dt))
    {
        handler->getWigetDrawer()->draw_Gizmo(this, parent, static_cast<WigetType*>(layout)->m_data, m_data);
        return true;
    }

    return false;
}

Wiget* WigetGizmo::copy() const
{
    return V3D_NEW(WigetGizmo, memory::MemoryLabel::MemoryUI)(*this);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////

WigetViewManipulator::WigetViewManipulator(const scene::Camera* camera) noexcept
    : WigetBase<WigetViewManipulator>(V3D_NEW(StateType, memory::MemoryLabel::MemoryUI)())
{
    setCamera(camera);
}

WigetViewManipulator::WigetViewManipulator(const WigetViewManipulator& other) noexcept
    : WigetBase<WigetViewManipulator>(other)
{
    StateType* state = V3D_NEW(StateType, memory::MemoryLabel::MemoryUI)(*static_cast<const StateType*>(other.m_data));
    m_data = state;
}

WigetViewManipulator::WigetViewManipulator(WigetViewManipulator&& other) noexcept
    : WigetBase<WigetViewManipulator>(other)
{
    m_data = other.m_data;
    other.m_data = nullptr;
}

WigetViewManipulator::~WigetViewManipulator()
{
    if (m_data)
    {
        V3D_DELETE(m_data, memory::MemoryLabel::MemoryUI);
        m_data = nullptr;
    }
}

bool WigetViewManipulator::update(WigetHandler* handler, Wiget* parent, Wiget* layout, f32 dt)
{
    if (Wiget::update(handler, parent, layout, dt))
    {
        handler->getWigetDrawer()->draw_ViewManipulator(this, parent, static_cast<WigetType*>(layout)->m_data, m_data);
        return true;
    }

    return false;
}

Wiget* WigetViewManipulator::copy() const
{
    return V3D_NEW(WigetViewManipulator, memory::MemoryLabel::MemoryUI)(*this);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////

} // namespace ui
} // namespace v3d