#include "CameraFPSHelper.h"
#include "Camera.h"

#include "Platform/Platform.h"
#include "Utils/Logger.h"

#define NEW_CAMERA 0

namespace v3d
{
namespace scene
{

CameraFPSHelper::CameraFPSHelper(Camera* camera, const core::Vector3D& position) noexcept
    : CameraHelper(camera, position)
    , m_moveSpeed(0.0f)

    , m_direction( { false, false, false, false })
    , m_rotate(false)

    , m_minBorder(-1000.0f)
    , m_maxBorder(1000.0f)
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
#if NEW_CAMERA
        CameraHelper::update();

        core::Vector3D camFront;
        camFront.x = -cos(m_transform.getRotation().x * core::k_degToRad) * sin(m_transform.getRotation().y * core::k_degToRad);
        camFront.y = sin(m_transform.getRotation().x * core::k_degToRad);
        camFront.z = cos(m_transform.getRotation().x * core::k_degToRad) * cos(m_transform.getRotation().y * core::k_degToRad);
        camFront.normalize();

        f32 moveSpeed = /*deltaTime*/ m_moveSpeed;
        core::Vector3D position = m_transform.getPosition();

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
            position -= (core::crossProduct(camFront, getCamera().getUpVector())).normalize() * moveSpeed;
        }

        if (m_direction._right)
        {
            position += (core::crossProduct(camFront, getCamera().getUpVector())).normalize() * moveSpeed;
        }
        m_transform.setPosition(position);

        core::Matrix4D rotateX;
        rotateX.makeIdentity();
        rotateX.setRotation(core::Vector3D(m_transform.getRotation().x, 0.0f, 0.0f));

        core::Matrix4D rotateY;
        rotateY.makeIdentity();
        rotateY.setRotation(core::Vector3D(0.0f, m_transform.getRotation().y, 0.0f));

        core::Matrix4D rotate = rotateX * rotateY;
        core::Vector3D rotation = rotate.getRotation();
        //LOG_DEBUG("rotation: x= %f, y=%f, z=%f", rotation.x, rotation.y, rotation.z);

       /* core::Matrix4D look = core::buildLookAtMatrix(m_transform.getPosition(), getCamera().getTarget(), getCamera().getUpVector());*/
        core::Matrix4D view = rotate * m_transform.getTransform();

        //core::Vector3D pos = view.getTranslation();
        //LOG_DEBUG("final pos: x= %f, y=%f, z=%f", pos.x, pos.y, pos.z);

        LOG_DEBUG("cam target: x= %f, y=%f, z=%f", camFront.x, camFront.y, camFront.z);
        //getCamera().setTarget(camFront);
        getCamera().setViewMatrix(view);
        
        m_needUpdate = false;
#else //NEW_CAMERA
        //LOG_DEBUG("update deltaTime: %f", deltaTime);

        if (CameraFPSHelper::isRotationChange())
        {
            core::Vector3D angle = m_angle;//core::Vector3D(CameraFPSHelper::getRotation().x, CameraFPSHelper::getRotation().y, 0.f);
            angle *= deltaTime;
            LOG_DEBUG("cam angle: x= %f, y=%f, z=%f", angle.x, angle.y, angle.z);

            core::Vector3D vAxis = core::crossProduct(core::Vector3D(getCamera().getTarget() - CameraFPSHelper::getPosition()), getCamera().getUpVector());
            vAxis.normalize();

            static f32 currentRotX = 0.0f;
            static f32 lastRotX = 0.0f;

            lastRotX = -currentRotX;
            if (currentRotX > 1.0f)
            {
                currentRotX = 1.0f;
                if (lastRotX != 1.0f)
                {
                    CameraFPSHelper::rotate(1.0f - lastRotX, vAxis);
                }
            }
            else if (currentRotX < -1.0f)
            {
                currentRotX = -1.0f;
                if (lastRotX != -1.0f)
                {
                    CameraFPSHelper::rotate(-1.0f - lastRotX, vAxis);
                }
            }
            else
            {
                currentRotX = vAxis.x;
                CameraFPSHelper::rotate(angle.y, vAxis);
            }
            LOG_DEBUG("cam vAxis: x= %f, y=%f, z=%f", vAxis.x, vAxis.y, vAxis.z);
            CameraFPSHelper::rotate(angle.x, core::Vector3D(0.0f, 1.0f, 0.0f));

            m_rotate = false;
        }

        if (CameraFPSHelper::isDirectionChange())
        {
            f32 moveSpeed = deltaTime * m_moveSpeed;
            if (m_direction._forward)
            {
                CameraFPSHelper::move(core::Vector3D(0, 0, moveSpeed));
            }

            if (m_direction._back)
            {
                CameraFPSHelper::move(core::Vector3D(0, 0, -moveSpeed));
            }

            if (m_direction._left)
            {
                CameraFPSHelper::move(core::Vector3D(-moveSpeed, 0, 0));
            }

            if (m_direction._right)
            {
                CameraFPSHelper::move(core::Vector3D(moveSpeed, 0, 0));
            }
        }
        
        CameraHelper::update();
#endif //NEW_CAMERA
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

bool CameraFPSHelper::isDirectionChange() const
{
    return m_direction._forward || m_direction._back || m_direction._left || m_direction._right;
}

bool CameraFPSHelper::isRotationChange() const
{
    return m_rotate;
}

void CameraFPSHelper::rotateHandlerCallback(v3d::event::InputEventHandler* handler, const event::MouseInputEvent* event, bool mouseCapture)
{
    static core::Point2D position = event->_cursorPosition;

    if (handler->isLeftMousePressed() || mouseCapture)
    {
        core::Point2D positionDelta = position - event->_cursorPosition;

#if NEW_CAMERA
        core::Vector3D rotation = CameraFPSHelper::getRotation();
        rotation.x += positionDelta.y * k_rotationSpeed;
        rotation.y -= positionDelta.x * k_rotationSpeed;
#else //NEW_CAMERA
        if (positionDelta.x != 0 && positionDelta.y != 0)
        {
            m_angle = core::Vector3D(positionDelta.x * k_rotationSpeed, positionDelta.y * k_rotationSpeed, 0.f);
            m_rotate = true;
            m_needUpdate = true;
        }
#endif //NEW_CAMERA
     }

    position = event->_cursorPosition;
}

void CameraFPSHelper::moveHandlerCallback(v3d::event::InputEventHandler * handler, const event::KeyboardInputEvent * event)
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
