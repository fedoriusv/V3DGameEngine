#include "CameraFPSHelper.h"
#include "Camera.h"

#include "Platform/Platform.h"
#include "Utils/Logger.h"

namespace v3d
{
namespace scene
{

CameraFPSHelper::CameraFPSHelper(Camera* camera, const core::Vector3D& position) noexcept
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
        core::Vector3D frontDirection;
        frontDirection.x = -cos(CameraFPSHelper::getRotation().x * core::k_degToRad) * sin(CameraFPSHelper::getRotation().y * core::k_degToRad);
        frontDirection.y = sin(CameraFPSHelper::getRotation().x * core::k_degToRad);
        frontDirection.z = cos(CameraFPSHelper::getRotation().x * core::k_degToRad) * cos(CameraFPSHelper::getRotation().y * core::k_degToRad);
        frontDirection.normalize();

        if (CameraFPSHelper::isDirectionChange())
        {
            core::Vector3D position = m_transform.getPosition();
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
                core::Vector3D camRight = core::crossProduct(frontDirection, getCamera().getUpVector());
                camRight.normalize();

                position += camRight * moveSpeed;
            }

            if (m_direction._right)
            {
                core::Vector3D camRight = core::crossProduct(frontDirection, getCamera().getUpVector());
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

void CameraFPSHelper::setRotation(const core::Vector3D& rotation)
{
    m_transform.setRotation(rotation);
    m_needUpdate = true;
}

const core::Vector3D& CameraFPSHelper::getRotation() const
{
    return m_transform.getRotation();
}

bool CameraFPSHelper::isDirectionChange() const
{
    return m_direction._forward || m_direction._back || m_direction._left || m_direction._right;
}

void CameraFPSHelper::rotateHandlerCallback(const v3d::event::InputEventHandler* handler, const event::MouseInputEvent* event, bool mouseCapture)
{
    static core::Point2D position = event->_cursorPosition;

    if (handler->isLeftMousePressed() || mouseCapture)
    {
        core::Point2D positionDelta = position - event->_cursorPosition;

        //if (positionDelta.x != 0 && positionDelta.y != 0)
        {
            core::Vector3D rotation = CameraFPSHelper::getRotation();
            rotation.x += positionDelta.y * k_rotationSpeed;
            rotation.y += positionDelta.x * k_rotationSpeed;

            rotation.x = std::clamp(rotation.x, -k_constrainPitch, k_constrainPitch);
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
