#include "EditorGizmo.h"

#include "Events/Input/InputEventMouse.h"
#include "Events/Input/InputEventKeyboard.h"

#include "Utils/Logger.h"

EditorGizmo::EditorGizmo() noexcept
    : m_gizmo(nullptr)
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

void EditorGizmo::init(ui::WidgetGizmo* widget)
{
    m_gizmo = widget;
}

void EditorGizmo::modify(const scene::Transform& transform)
{
    LOG_DEBUG("position [%f, %f, %f]", transform.getPosition().getX(), transform.getPosition().getY(), transform.getPosition().getZ());
    LOG_DEBUG("rotation [%f, %f, %f]", transform.getRotation().getX(), transform.getRotation().getY(), transform.getRotation().getZ());
    LOG_DEBUG("scale [%f, %f, %f]", transform.getScale().getX(), transform.getScale().getY(), transform.getScale().getZ());
    m_gizmo->setTransform(transform);
}

void EditorGizmo::select()
{
}

void EditorGizmo::setEnable(bool enable)
{
    if (m_gizmo)
    {
        m_gizmo->setActive(enable);
    }
}

void EditorGizmo::setOperation(u32 index)
{
    static std::array<ui::WidgetGizmo::Operation, 3> op = {
        ui::WidgetGizmo::Operation::Translate,
        ui::WidgetGizmo::Operation::Rotate,
        ui::WidgetGizmo::Operation::Scale
    };

    if (m_gizmo)
    {
        m_gizmo->setOperation(op[index]);
    }
}

void EditorGizmo::update(f32 dt)
{
    if (m_selectedObject)
    {
        modify(m_selectedObject->m_transform);
    }
}

void EditorGizmo::handleNotify(const utils::Reporter<EditorReport>* reporter, const EditorReport& data)
{
    modify(data.instanceObject->m_transform);

    m_selectedObject = data.instanceObject;
}

bool EditorGizmo::handleGameEvent(event::GameEventHandler* handler, const event::GameEvent* event)
{
    if (event->_eventType == event::GameEvent::GameEventType::SelectObject)
    {
        //modify(data.transform);
    }

    return false;
}

bool EditorGizmo::handleInputEvent(event::InputEventHandler* handler, const event::InputEvent* event)
{
    return false;
}
