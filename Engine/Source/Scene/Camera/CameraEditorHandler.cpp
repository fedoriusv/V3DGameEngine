#include "CameraEditorHandler.h"
#include "Camera.h"

#include "Platform/Platform.h"
#include "Utils/Logger.h"

namespace v3d
{
namespace scene
{

CameraEditorHandler::CameraEditorHandler(std::unique_ptr<Camera> camera) noexcept
    : CameraHandler(std::move(camera))

    , m_distanceLimits({-10, 10})

    , m_moveSpeed(1.f)
    , m_accelerationSpeed(1.f)
    , m_rotationSpeed(1.0f)

    , m_deltaDistance(0.f)

    , m_freeFlyMode(false)
    , m_orbitingMode(false)
{
}

CameraEditorHandler::~CameraEditorHandler()
{
}

void CameraEditorHandler::update(f32 deltaTime)
{
    if (m_needUpdate)
    {
        math::Vector3D rotation;
        rotation.setX(m_camera->getRotation().getX() + m_deltaRotation._x * m_rotationSpeed * deltaTime);
        rotation.setY(m_camera->getRotation().getY() + m_deltaRotation._y * m_rotationSpeed * deltaTime);
        m_deltaRotation = { 0.f, 0.f };

        if (m_orbitingMode)
        {
            f32 distance = (m_camera->getTarget() - m_camera->getPosition()).length();
            distance += m_deltaDistance * m_moveSpeed * deltaTime;
            distance = std::clamp(distance, m_distanceLimits._x, m_distanceLimits._y);
            m_deltaDistance = 0.f;

            math::Matrix4D translation;
            translation.setTranslation({ 0.f, 0.f, -distance });

            math::Matrix4D rotate;
            rotate.setRotation(rotation);
            rotate.setTranslation(m_camera->getTarget());

            math::Matrix4D view = translation * rotate;

            m_camera->setRotation(rotation);
            m_camera->setPosition(view.getTranslation());

            math::Matrix4D transform = m_camera->getTransform();
            transform.makeInverse();
            CameraHandler::setViewMatrix(transform);
        }
        else if (m_freeFlyMode)
        {
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
            CameraHandler::setViewMatrix(transform);
        }

        CameraHandler::update(deltaTime);
        m_needUpdate = false;
    }
}

void CameraEditorHandler::handleInputEventCallback(const v3d::event::InputEventHandler* handler, const event::InputEvent* event)
{
    static math::Point2D prevCursorPosition = {};
    math::Point2D positionDelta = prevCursorPosition - handler->getRelativeCursorPosition();

    if (handler->isKeyPressed(event::KeyCode::KeyLAlt)) //orbit camera mode
    {
        m_orbitingMode = true;
        m_freeFlyMode = false;

        static bool isOrbitingRotation = false;
        if (event->_eventType == event::InputEvent::InputEventType::MouseInputEvent)
        {
            const event::MouseInputEvent* mouseEvent = static_cast<const event::MouseInputEvent*>(event);
            if (mouseEvent->_event == event::MouseInputEvent::MousePressDown && mouseEvent->_key == event::KeyCode::KeyLButton)
            {
                prevCursorPosition = {};
                positionDelta = {};
                isOrbitingRotation = true;
            }
            else if (mouseEvent->_event == event::MouseInputEvent::MousePressUp && mouseEvent->_key == event::KeyCode::KeyLButton)
            {
                prevCursorPosition = {};
                isOrbitingRotation = false;
            }

            if (isOrbitingRotation)
            {
                m_deltaRotation._x = -positionDelta._y;
                m_deltaRotation._y = -positionDelta._x;
                m_needUpdate = true;
            }

            if (handler->isRightMousePressed())
            {
                m_deltaDistance = positionDelta._y * 0.1f;
                m_needUpdate = true;
            }
        }
    }
    else if (handler->isRightMousePressed()) //fps camera mode
    {
        m_freeFlyMode = true;
        m_orbitingMode = false;


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
    else
    {
        m_orbitingMode = false;
        m_freeFlyMode = false;
    }

    prevCursorPosition = handler->getRelativeCursorPosition();
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

} //namespace scene
} //namespace v3d
