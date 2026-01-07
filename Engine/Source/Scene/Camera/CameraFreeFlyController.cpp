#include "CameraFreeFlyController.h"
#include "Camera.h"

#include "Platform/Platform.h"
#include "Utils/Logger.h"

namespace v3d
{
namespace scene
{

CameraFreeFlyController::CameraFreeFlyController(std::unique_ptr<Camera> camera) noexcept
    : CameraController(std::move(camera))
    , m_moveSpeed(1.f)
    , m_accelerationSpeed(1.f)
    , m_rotationSpeed(1.0f)

    , m_deltaRotation({ 0.f, 0.f })
    , m_deltaDistance(0.f)
{
}

CameraFreeFlyController::~CameraFreeFlyController()
{
}

void CameraFreeFlyController::update(f32 deltaTime)
{
    if (m_needUpdate)
    {
        math::Vector3D rotation;
        rotation.setX(m_camera->getRotation().getX() + m_deltaRotation._x * m_rotationSpeed * deltaTime);
        rotation.setY(m_camera->getRotation().getY() + m_deltaRotation._y * m_rotationSpeed * deltaTime);
        m_deltaRotation = { 0.f, 0.f };

        math::Matrix4D rotate;
        rotate.setRotation(rotation);

        m_accelerationSpeed = std::clamp(m_accelerationSpeed, 0.1f, 1.f);
        math::Vector4D forward = rotate * (m_direction * m_moveSpeed * m_accelerationSpeed * deltaTime);
        m_direction = { 0.f, 0.f, 0.f, 0.f };

        m_camera->setRotation(rotation);
        m_camera->setPosition(m_camera->getPosition() + math::Vector3D{ forward.getX(), forward.getY(), forward.getZ() });

        math::Matrix4D transform = m_camera->getTransform();
        m_camera->setTarget(m_camera->getPosition() + m_camera->getForwardVector() * 2.f);

        transform.makeInverse();
        CameraController::setViewMatrix(transform);

        CameraController::update(deltaTime);
        m_needUpdate = false;
    }
}

void CameraFreeFlyController::handleInputEventCallback(const v3d::event::InputEventHandler* handler, const event::InputEvent* event)
{
    static math::Point2D prevCursorPosition = {};
    math::Point2D positionDelta = prevCursorPosition - handler->getRelativeCursorPosition();

    if (handler->isRightMousePressed())
    {
        m_deltaRotation._x = -positionDelta._y;
        m_deltaRotation._y = -positionDelta._x;

        f32 directionFwd = 0.f;
        if (handler->isKeyPressed(event::KeyCode::KeyKey_W))
        {
            directionFwd = 1.0f;
        }
        else if (handler->isKeyPressed(event::KeyCode::KeyKey_S))
        {
            directionFwd = -1.0f;
        }

        f32 directionSide = 0.f;
        if (handler->isKeyPressed(event::KeyCode::KeyKey_A))
        {
            directionSide = -1.0f;
        }
        else if (handler->isKeyPressed(event::KeyCode::KeyKey_D))
        {
            directionSide = 1.0f;
        }

        m_direction = { directionSide, 0.f, directionFwd, 0.f };
        m_needUpdate = true;
    }

    prevCursorPosition = handler->getRelativeCursorPosition();
}

void CameraFreeFlyController::setMoveSpeed(f32 speed)
{
    m_moveSpeed = speed;
}

f32 CameraFreeFlyController::getMoveSpeed() const
{
    return m_moveSpeed;
}

void CameraFreeFlyController::setRotationSpeed(f32 speed)
{
    m_rotationSpeed = speed;
}

f32 CameraFreeFlyController::getRotationSpeed() const
{
    return m_rotationSpeed;
}

} //namespace scene
} //namespace v3d
