#include "CameraFPSHelper.h"
#include "Camera.h"

#include "Platform/Platform.h"
#include "Utils/Logger.h"

namespace v3d
{
namespace scene
{

CameraFPSHelper::CameraFPSHelper(Camera* camera, const math::Vector3D& position) noexcept
    : CameraHelper(camera, position)
    , m_moveSpeed(0.f)

    , m_direction({ false, false, false, false })
{
}

CameraFPSHelper::~CameraFPSHelper()
{
}

void CameraFPSHelper::update(f32 deltaTime)
{
    if (m_needUpdate)
    {
        math::Vector3D frontDirection;
        frontDirection.m_x = cos(CameraFPSHelper::getRotation().m_x * math::k_degToRad) * sin(CameraFPSHelper::getRotation().m_y * math::k_degToRad);
        frontDirection.m_y = sin(CameraFPSHelper::getRotation().m_x * math::k_degToRad);
        frontDirection.m_z = cos(CameraFPSHelper::getRotation().m_x * math::k_degToRad) * cos(CameraFPSHelper::getRotation().m_y * math::k_degToRad);
        frontDirection.normalize();

        if (CameraFPSHelper::isDirectionChange())
        {
            math::Vector3D position = m_transform.getPosition();
            f32 moveSpeed = deltaTime * m_moveSpeed;

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

        getCamera().setTarget(m_transform.getPosition() + frontDirection);
        CameraHelper::update(deltaTime);
    }
}

void CameraFPSHelper::setRotation(const math::Vector3D& rotation)
{
    m_transform.setRotation(rotation);
    m_needUpdate = true;
}

const math::Vector3D& CameraFPSHelper::getRotation() const
{
    return m_transform.getRotation();
}

bool CameraFPSHelper::isDirectionChange() const
{
    return m_direction._forward || m_direction._back || m_direction._left || m_direction._right;
}

void CameraFPSHelper::rotateHandlerCallback(const v3d::event::InputEventHandler* handler, const event::MouseInputEvent* event, bool mouseCapture)
{
    static math::Point2D position = event->_cursorPosition;

    if (handler->isLeftMousePressed() || mouseCapture)
    {
        math::Point2D positionDelta = position - event->_cursorPosition;

        //if (positionDelta.x != 0 && positionDelta.y != 0)
        {
            math::Vector3D rotation = CameraFPSHelper::getRotation();
            rotation.m_x += positionDelta.m_y * k_rotationSpeed;
            rotation.m_y -= positionDelta.m_x * k_rotationSpeed;

            rotation.m_x = math::clamp(rotation.m_x, -k_constrainPitch, k_constrainPitch);
            CameraFPSHelper::setRotation(rotation);
        }
    }

    position = event->_cursorPosition;
}

void CameraFPSHelper::moveHandlerCallback(const v3d::event::InputEventHandler* handler, const event::KeyboardInputEvent* event)
{
    m_direction._forward = handler->isKeyPressed(event::KeyCode::KeyKey_W);
    m_direction._back = handler->isKeyPressed(event::KeyCode::KeyKey_S);
    m_direction._left = handler->isKeyPressed(event::KeyCode::KeyKey_A);
    m_direction._right = handler->isKeyPressed(event::KeyCode::KeyKey_D);

    f32 accelerationSpeed = 1.0f;
    if (handler->isKeyPressed(event::KeyCode::KeyShift))
    {
        accelerationSpeed = k_accelerationSpeed;
    }

    if (CameraFPSHelper::isDirectionChange())
    {
        m_moveSpeed = k_movementSpeed * accelerationSpeed;
        m_needUpdate = true;
    }
}

} //namespace scene
} //namespace v3d
