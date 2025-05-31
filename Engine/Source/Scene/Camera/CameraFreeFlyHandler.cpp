#include "CameraFreeFlyHandler.h"
#include "Camera.h"

#include "Platform/Platform.h"
#include "Utils/Logger.h"

namespace v3d
{
namespace scene
{

CameraFreeFlyHandler::CameraFreeFlyHandler(std::unique_ptr<Camera> camera, const math::Vector3D& position) noexcept
    : CameraHandler(std::move(camera))
    , m_moveSpeed(0.f)

    , m_direction({ false, false, false, false })
{
}

CameraFreeFlyHandler::~CameraFreeFlyHandler()
{
}

void CameraFreeFlyHandler::update(f32 deltaTime)
{
    if (m_needUpdate)
    {
        math::Matrix4D transform = m_camera->getTransform();


        //math::Vector3D frontDirection;
        //frontDirection.m_x = cos(CameraFreeFlyHandler::getRotation().m_x * math::k_degToRad) * sin(CameraFreeFlyHandler::getRotation().m_y * math::k_degToRad);
        //frontDirection.m_y = sin(CameraFreeFlyHandler::getRotation().m_x * math::k_degToRad);
        //frontDirection.m_z = cos(CameraFreeFlyHandler::getRotation().m_x * math::k_degToRad) * cos(CameraFreeFlyHandler::getRotation().m_y * math::k_degToRad);
        //frontDirection.normalize();

        //if (CameraFreeFlyHandler::isDirectionChange())
        //{
        //    math::Vector3D position = getPosition();
        //    f32 moveSpeed = deltaTime * m_moveSpeed;

        //    if (m_direction._forward)
        //    {
        //        position += frontDirection * moveSpeed;
        //    }

        //    if (m_direction._back)
        //    {
        //        position -= frontDirection * moveSpeed;
        //    }

        //    //if (m_direction._left)
        //    //{
        //    //    math::Vector3D camRight = math::crossProduct(frontDirection, getCamera().getUpVector());
        //    //    camRight.normalize();

        //    //    position += camRight * moveSpeed;
        //    //}

        //    //if (m_direction._right)
        //    //{
        //    //    math::Vector3D camRight = math::crossProduct(frontDirection, getCamera().getUpVector());
        //    //    camRight.normalize();

        //    //    position -= camRight * moveSpeed;
        //    //}

        //    setPosition(position);
        //}
        //getCamera().setTarget(getPosition() + frontDirection);

        CameraHandler::update(deltaTime);
        m_needUpdate = false;
    }
}

void CameraFreeFlyHandler::setRotation(const math::Vector3D& rotation)
{
    //setRotation(rotation);
    //m_needUpdate = true;
}

const math::Vector3D& CameraFreeFlyHandler::getRotation() const
{
    return  math::Vector3D();
}

bool CameraFreeFlyHandler::isDirectionChange() const
{
    return m_direction._forward || m_direction._back || m_direction._left || m_direction._right;
}

void CameraFreeFlyHandler::rotateHandleCallback(const v3d::event::InputEventHandler* handler, const event::InputEvent* event, bool mouseCapture)
{
    static math::Point2D position = handler->getRelativeCursorPosition();

    if (handler->isLeftMousePressed() || mouseCapture)
    {
        math::Point2D positionDelta = position - handler->getRelativeCursorPosition();
        if (positionDelta._x != 0 && positionDelta._y != 0)
        {
            //math::Vector3D rotation = CameraFreeFlyHandler::getRotation();
            //rotation.m_x += positionDelta._y * k_rotationSpeed;
            //rotation.m_y -= positionDelta._x * k_rotationSpeed;

            //rotation.m_x = std::clamp(rotation.m_x, -k_constrainPitch, k_constrainPitch);
            //CameraFreeFlyHandler::setRotation(rotation);
        }
    }

    position = handler->getRelativeCursorPosition();
}

void CameraFreeFlyHandler::moveHandleCallback(const v3d::event::InputEventHandler* handler, const event::InputEvent* event)
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

    if (CameraFreeFlyHandler::isDirectionChange())
    {
        m_moveSpeed = k_movementSpeed * accelerationSpeed;
        m_needUpdate = true;
    }
}

} //namespace scene
} //namespace v3d
