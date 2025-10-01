#include "EditorGizmo.h"

#include "Events/Input/InputEventMouse.h"
#include "Events/Input/InputEventKeyboard.h"

#include "Utils/Logger.h"

EditorGizmo::EditorGizmo(event::GameEventReceiver* gameEventRecevier) noexcept
    : m_gizmo(nullptr)
    , m_gameEventRecevier(gameEventRecevier)

    , m_sceneData(nullptr)
    , m_selectedNode(nullptr)

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

void EditorGizmo::modify(const math::Matrix4D& matrix)
{
    //LOG_DEBUG("position [%f, %f, %f]", transform.getTranslation().getX(), transform.getTranslation().getY(), transform.getTranslation().getZ());
    //LOG_DEBUG("rotation [%f, %f, %f]", transform.getRotation().getX(), transform.getRotation().getY(), transform.getRotation().getZ());
    //LOG_DEBUG("scale [%f, %f, %f]", transform.getScale().getX(), transform.getScale().getY(), transform.getScale().getZ());
    //m_gizmo->setTransform(transform);
    if (m_selectedNode)
    {
        scene::Transform transform;
        transform.setMatrix(matrix);
        m_gameEventRecevier->sendEvent(new EditorTrasformEvent(m_selectedNode, scene::TransformMode::Local, transform));
    }
}

void EditorGizmo::select()
{
}

void EditorGizmo::setEnable(bool enable)
{
    if (m_gizmo)
    {
        m_currentOp = -1;
        m_gizmo->setActive(enable && m_selectedNode != nullptr);
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

    if (m_selectedNode && m_currentOp > -1)
    {
        modify(m_selectedNode->getTransform(scene::TransformMode::Local).getMatrix());
    }
}

void EditorGizmo::update(f32 dt)
{
}

bool EditorGizmo::handleGameEvent(event::GameEventHandler* handler, const event::GameEvent* event)
{
    if (!m_gizmo)
    {
        return true;
    }

    if (event->_eventType == event::GameEvent::GameEventType::SelectObject)
    {
        const EditorSelectionEvent* selectionEvent = static_cast<const EditorSelectionEvent*>(event);
        m_selectedNode = selectionEvent->_node;
        if (m_selectedNode && m_currentOp > -1)
        {
            m_gizmo->setActive(true);
            m_gizmo->setTransform(m_selectedNode->getTransform(scene::TransformMode::Local).getMatrix());

            return true;
        }

        m_gizmo->setActive(false);
    }
    else if (event->_eventType == event::GameEvent::GameEventType::TransformObject)
    {
        const EditorTrasformEvent* transformEvent = static_cast<const EditorTrasformEvent*>(event);
        if (m_selectedNode && m_currentOp > -1)
        {
            m_gizmo->setTransform(transformEvent->_transform.getMatrix());

            return true;
        }
    }

    return true;
}

bool EditorGizmo::handleInputEvent(event::InputEventHandler* handler, const event::InputEvent* event)
{
    return false;
}
