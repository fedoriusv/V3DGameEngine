#include "EditorGizmo.h"

#include "Events/Input/InputEventMouse.h"
#include "Events/Input/InputEventKeyboard.h"

#include "Utils/Logger.h"

namespace v3d
{
    extern scene::Transform g_modelTransform;
}

EditorGizmo::EditorGizmo()
    : m_wiget(nullptr)
{
    InputEventHandler::bind([this](const event::MouseInputEvent* event)
        {
            this->handleInputEvent(this, event);
        }
    );

    InputEventHandler::bind([this](const event::KeyboardInputEvent* event)
        {
            this->handleInputEvent(this, event);
        }
    );
}

EditorGizmo::~EditorGizmo()
{
}

void EditorGizmo::init(ui::WigetGizmo* wiget)
{
    m_wiget = wiget;
}

void EditorGizmo::modify(const scene::Transform& transform)
{
    LOG_DEBUG("position [%f, %f, %f]", transform.getPosition().m_x, transform.getPosition().m_y, transform.getPosition().m_z);
    LOG_DEBUG("rotation [%f, %f, %f]", transform.getRotation().m_x, transform.getRotation().m_y, transform.getRotation().m_z);
    LOG_DEBUG("scale [%f, %f, %f]", transform.getScale().m_x, transform.getScale().m_y, transform.getScale().m_z);
    g_modelTransform = transform;
    m_wiget->setTransform(transform);
}

void EditorGizmo::select()
{
}

void EditorGizmo::setEnable(bool enable)
{
    if (m_wiget)
    {
        m_wiget->setActive(enable);
    }
}

void EditorGizmo::setOperation(u32 index)
{
    static std::array<ui::WigetGizmo::Operation, 3> op = {
        ui::WigetGizmo::Operation::Translate,
        ui::WigetGizmo::Operation::Rotate,
        ui::WigetGizmo::Operation::Scale
    };

    if (m_wiget)
    {
        m_wiget->setOperation(op[index]);
    }
}

void EditorGizmo::update(f32 dt)
{
}

bool EditorGizmo::handleInputEvent(v3d::event::InputEventHandler* handler, const v3d::event::InputEvent* event)
{
    return false;
}
