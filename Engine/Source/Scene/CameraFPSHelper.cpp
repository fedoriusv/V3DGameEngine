#include "CameraFPSHelper.h"
#include "Camera.h"

#include "Platform/Platform.h"
#include "Utils/Logger.h"

#define NEW_CAMERA 1

namespace v3d
{
namespace scene
{

CameraFPSHelper::CameraFPSHelper(Camera* camera, const core::Vector3D& position) noexcept
    : CameraHelper(camera, position)
    , m_moveSpeed(0.0f)

    , m_direction( { false, false, false, false })
{
}

CameraFPSHelper::~CameraFPSHelper()
{
}

void CameraFPSHelper::update(f32 deltaTime)
{
    /*{
        static core::Point2D position;
        core::Point2D positionDelta = position - platform::Platform::getCursorPosition();
        m_angle = core::Vector3D(positionDelta.x * k_rotationSpeed, positionDelta.y * k_rotationSpeed, 0.f);

        if (positionDelta.x != 0 && positionDelta.y != 0)
        {
            m_rotate = true;
            m_needUpdate = true;

            LOG_DEBUG("cam positionDelta: x= %d, y=%d", positionDelta.x, positionDelta.y);
        }

        position = platform::Platform::getCursorPosition();
    }*/


    if (m_needUpdate)
    {
        core::Vector3D camFront;
        camFront.x = -cos(CameraFPSHelper::getRotation().x * core::k_degToRad) * sin(CameraFPSHelper::getRotation().y * core::k_degToRad);
        camFront.y = sin(CameraFPSHelper::getRotation().x * core::k_degToRad);
        camFront.z = cos(CameraFPSHelper::getRotation().x * core::k_degToRad) * cos(CameraFPSHelper::getRotation().y * core::k_degToRad);
        camFront.normalize();
        //LOG_DEBUG("rotation: x= %f, y=%f, z=%f", CameraFPSHelper::getRotation().x, CameraFPSHelper::getRotation().y, CameraFPSHelper::getRotation().z);

        core::Vector3D position = m_transform.getPosition();
        if (CameraFPSHelper::isDirectionChange())
        {
            core::Vector3D camRight = (core::crossProduct(camFront, getCamera().getUpVector())).normalize();
            //core::Vector3D camUp = (core::crossProduct(camRight, camFront)).normalize();

            f32 moveSpeed = deltaTime * m_moveSpeed;

            if (m_direction._forward)
            {
                position += camFront * moveSpeed;
            }

            if (m_direction._back)
            {
                position -= camFront * moveSpeed;
            }

            if (m_direction._left)
            {
                position -= camRight * moveSpeed;
            }

            if (m_direction._right)
            {
                position += camRight * moveSpeed;
            }
            m_transform.setPosition(position);

            //getCamera().setUpVector(camUp);
        }

        getCamera().setTarget(position + camFront);

        CameraHelper::update(deltaTime);
    }
}

void CameraFPSHelper::setRotation(const core::Vector3D & rotation)
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

void CameraFPSHelper::moveHandlerCallback(const v3d::event::InputEventHandler * handler, const event::KeyboardInputEvent * event)
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

#if 0
void CameraFPSHelper::move(const core::Vector3D& direction)
{
    core::Vector3D pos = CameraFPSHelper::getPosition();
    core::Vector3D view = getCamera().getTarget();

    core::Vector3D move = view - pos;
    move.normalize();
    pos += (move * direction.z);
    view += (move * direction.z);

    core::Vector3D strafe = core::crossProduct(move, getCamera().getUpVector());
    strafe.normalize();
    pos += (strafe * direction.x);
    view += (strafe * direction.x);

    pos.y += direction.y;
    view.y += direction.y;

    if (!CameraFPSHelper::isPointOut(pos))
    {
        CameraFPSHelper::setPosition(pos);
        getCamera().setTarget(view);
    }
}

void CameraFPSHelper::rotate(f32 angle, const core::Vector3D& point)
{
    core::Vector3D newView;
    core::Vector3D oldView;

    oldView = getCamera().getTarget() - CameraFPSHelper::getPosition();

    f32 cosTheta = cos(angle);
    f32 sinTheta = sin(angle);

    newView.x = (cosTheta + (1 - cosTheta) * point.x * point.x) * oldView.x;
    newView.x = newView.x + ((1 - cosTheta) * point.x * point.y - point.z * sinTheta) * oldView.y;
    newView.x = newView.x + ((1 - cosTheta) * point.x * point.z + point.y * sinTheta) * oldView.z;

    newView.y = ((1 - cosTheta) * point.x * point.y + point.z * sinTheta) * oldView.x;
    newView.y = newView.y + (cosTheta + (1 - cosTheta) * point.y * point.y) * oldView.y;
    newView.y = newView.y + ((1 - cosTheta) * point.y * point.z - point.x * sinTheta) * oldView.z;

    newView.z = ((1 - cosTheta) * point.x * point.z - point.y * sinTheta) * oldView.x;
    newView.z = newView.z + ((1 - cosTheta) * point.y * point.z + point.x * sinTheta) * oldView.y;
    newView.z = newView.z + (cosTheta + (1 - cosTheta) * point.z * point.z) * oldView.z;

    getCamera().setTarget(CameraFPSHelper::getPosition() + newView);
}

bool CameraFPSHelper::isPointOut(const core::Vector3D& point)
{
    if (point.x <= m_minBorder.x || point.y <= m_minBorder.y || point.z <= m_minBorder.z ||
        point.x >= m_maxBorder.x || point.y >= m_maxBorder.y || point.z >= m_maxBorder.z)
    {
        return true;
    }

    return false;
}
#endif

} //namespace scene
} //namespace v3d
