#include "EditorGizmo.h"

#include "Events/Input/InputEventMouse.h"
#include "Events/Input/InputEventKeyboard.h"

#include "Utils/Logger.h"

EditorGizmo::EditorGizmo(event::GameEventReceiver* gameEventRecevier) noexcept
    : m_gizmo(nullptr)

    , m_sceneData(nullptr)
    , m_selectedObject(nullptr)

    , m_currentOp(-1)
{
}

EditorGizmo::~EditorGizmo()
{
}

void EditorGizmo::registerWiget(ui::WidgetGizmo* widget, scene::SceneData& sceneData)
{
    ASSERT(widget, "must be valid");
    m_gizmo = widget;
    m_sceneData = &sceneData;
}

void EditorGizmo::modify(const math::Matrix4D& transform)
{
    LOG_DEBUG("position [%f, %f, %f]", transform.getTranslation().getX(), transform.getTranslation().getY(), transform.getTranslation().getZ());
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
        m_currentOp = -1;
        m_gizmo->setActive(enable && m_selectedObject != nullptr);
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
        m_currentOp = index;
        m_gizmo->setOperation(op[index]);
    }
}

void EditorGizmo::update(f32 dt)
{
    if (m_selectedObject && m_currentOp > -1)
    {
        modify(m_selectedObject->_object->getTransform());
    }
}

bool EditorGizmo::handleGameEvent(event::GameEventHandler* handler, const event::GameEvent* event)
{
    if (event->_eventType == event::GameEvent::GameEventType::SelectObject)
    {
        const EditorSelectionEvent* selectionEvent = static_cast<const EditorSelectionEvent*>(event);
        m_selectedObject = (selectionEvent->_selectedIndex != k_emptyIndex) ? m_sceneData->m_generalList[selectionEvent->_selectedIndex] : nullptr;
        if (m_gizmo)
        {
            if (m_selectedObject && m_currentOp > -1)
            {
                m_gizmo->setActive(true);
                modify(m_sceneData->m_generalList[selectionEvent->_selectedIndex]->_object->getTransform());
            }
            else
            {
                m_gizmo->setActive(false);
            }
        }
    }

    return true;
}

bool EditorGizmo::handleInputEvent(event::InputEventHandler* handler, const event::InputEvent* event)
{
    return false;
}
