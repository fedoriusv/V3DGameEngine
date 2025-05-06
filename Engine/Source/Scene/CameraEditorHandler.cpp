#include "CameraEditorHandler.h"
#include "Camera.h"

#include "Platform/Platform.h"
#include "Utils/Logger.h"

namespace v3d
{
namespace scene
{

CameraEditorHandler::CameraEditorHandler(Camera* camera, const math::Vector3D& position) noexcept
    : CameraHandler(camera, position)
    , m_cameraMode(CameraMode::None)
    , m_moveSpeed(1.f)
    , m_accelerationSpeed(1.f)
    , m_rotationSpeed(1.f)
    , m_direction({ false, false, false, false })
{
}

CameraEditorHandler::~CameraEditorHandler()
{
}

void CameraEditorHandler::update(f32 deltaTime)
{
    if (m_needUpdate)
    {
        if (m_cameraMode == CameraMode::FPSMode || m_cameraMode == CameraMode::None)
        {
            math::Vector3D frontDirection;
            frontDirection.m_x = cos(CameraEditorHandler::getRotation().m_x * math::k_degToRad) * sin(CameraEditorHandler::getRotation().m_y * math::k_degToRad);
            frontDirection.m_y = sin(CameraEditorHandler::getRotation().m_x * math::k_degToRad);
            frontDirection.m_z = cos(CameraEditorHandler::getRotation().m_x * math::k_degToRad) * cos(CameraEditorHandler::getRotation().m_y * math::k_degToRad);
            frontDirection.normalize();

            if (CameraEditorHandler::isDirectionChange())
            {
                math::Vector3D position = m_transform.getPosition();
                f32 moveSpeed = deltaTime * m_moveSpeed * m_accelerationSpeed;

                if (m_direction._forward)
                {
                    position += frontDirection * moveSpeed;
                }

                if (m_direction._back)
                {
                    position -= frontDirection * moveSpeed;
                }

                if (m_direction._left)
                {
                    math::Vector3D camRight = math::crossProduct(frontDirection, getCamera().getUpVector());
                    camRight.normalize();

                    position += camRight * moveSpeed;
                }

                if (m_direction._right)
                {
                    math::Vector3D camRight = math::crossProduct(frontDirection, getCamera().getUpVector());
                    camRight.normalize();

                    position -= camRight * moveSpeed;
                }

                m_transform.setPosition(position);
                m_viewPosition = position;
            }

            CameraHandler::getCamera().setTarget(m_transform.getPosition() + frontDirection);

            CameraHandler::update(deltaTime);
            m_direction = { false, false, false, false };
        }
        else if (m_cameraMode == CameraMode::ArcballMode)
        {
            math::Matrix4D rotate;
            rotate.setRotation(math::Vector3D(m_transform.getRotation().m_x, m_transform.getRotation().m_y, 0.0f));

            math::Vector4D position = rotate * math::Vector4D(m_transform.getPosition(), 1.0);
            m_viewPosition = { position.m_x, position.m_y, position.m_z };

            rotate.makeTransposed();
            math::Matrix4D look = math::buildLookAtMatrix(m_transform.getPosition(), getCamera().getTarget(), getCamera().getUpVector());
            math::Matrix4D view = look * rotate;

            getCamera().setViewMatrix(view);
            CameraHandler::update(deltaTime);
        }

        m_needUpdate = false;
    }
}

void CameraEditorHandler::handleInputEventCallback(const v3d::event::InputEventHandler* handler, const event::InputEvent* event)
{
    bool isArcballMode = (handler->isKeyPressed(event::KeyCode::KeyLAlt) || handler->isKeyPressed(event::KeyCode::KeyRAlt)) && handler->isLeftMousePressed();
    bool isFPSMode = handler->isRightMousePressed();
    if (isArcballMode)
    {
        m_cameraMode = CameraMode::ArcballMode;

        static math::Point2D prevCursorPosition = handler->getRelativeCursorPosition();
        math::Point2D positionDelta = prevCursorPosition - handler->getRelativeCursorPosition();
        math::Vector3D rotation = CameraEditorHandler::getRotation();
        rotation.m_x -= positionDelta.m_y * m_rotationSpeed;
        rotation.m_y -= positionDelta.m_x * m_rotationSpeed;
        CameraEditorHandler::setRotation(rotation);
        prevCursorPosition = handler->getRelativeCursorPosition();
    }
    else if (isFPSMode)
    {
        m_cameraMode = CameraMode::FPSMode;

        static math::Point2D prevCursorPosition = handler->getRelativeCursorPosition();
        math::Point2D positionDelta = prevCursorPosition - handler->getRelativeCursorPosition();
        math::Vector3D rotation = CameraEditorHandler::getRotation();
        rotation.m_x += positionDelta.m_y * m_rotationSpeed;
        rotation.m_x = math::clamp(rotation.m_x, -k_constrainPitch, k_constrainPitch);
        rotation.m_y -= positionDelta.m_x * m_rotationSpeed;
        CameraEditorHandler::setRotation(rotation);
        prevCursorPosition = handler->getRelativeCursorPosition();

        m_direction._forward = handler->isKeyPressed(event::KeyCode::KeyKey_W);
        m_direction._back = handler->isKeyPressed(event::KeyCode::KeyKey_S);
        m_direction._left = handler->isKeyPressed(event::KeyCode::KeyKey_A);
        m_direction._right = handler->isKeyPressed(event::KeyCode::KeyKey_D);
    }

    if (event->_eventType == event::InputEvent::InputEventType::MouseInputEvent)
    {
        const event::MouseInputEvent* mouseEvent = static_cast<const event::MouseInputEvent*>(event);
        if (mouseEvent->_event == event::MouseInputEvent::MouseWheel)
        {
            m_cameraMode = CameraMode::FPSMode;

            f32 wheel = mouseEvent->_wheelValue;
            if (wheel > 0)
            {
                m_direction._forward = true;
                m_direction._back = false;
                m_direction._left = false;
                m_direction._right = false;
            }
            else if (wheel < 0)
            {
                m_direction._forward = false;
                m_direction._back = true;
                m_direction._left = false;
                m_direction._right = false;
            }
        }
    }

    if (CameraEditorHandler::isDirectionChange())
    {
        m_needUpdate = true;
    }
}

void CameraEditorHandler::setRotation(const math::Vector3D& rotation)
{
    m_transform.setRotation(rotation);
    m_needUpdate = true;
}

const math::Vector3D& CameraEditorHandler::getRotation() const
{
    return m_transform.getRotation();
}

void CameraEditorHandler::setMoveSpeed(f32 speed)
{
    m_moveSpeed = speed;
}

f32 CameraEditorHandler::getMoveSpeed() const
{
    return m_moveSpeed;
}

void CameraEditorHandler::setRotationSpeed(f32 speed)
{
    m_rotationSpeed = speed;
}

f32 CameraEditorHandler::getRotationSpeed() const
{
    return m_rotationSpeed;
}

bool CameraEditorHandler::isDirectionChange() const
{
    return m_direction._forward || m_direction._back || m_direction._left || m_direction._right;
}

} //namespace scene
} //namespace v3d
