#include "CameraOrbitController.h"
#include "Camera.h"
#include "Renderer/DeviceCaps.h"

#include "Utils/Logger.h"

namespace v3d
{
namespace scene
{

CameraOrbitController::CameraOrbitController(std::unique_ptr<Camera> camera, f32 distance) noexcept
    : CameraController(std::move(camera))
    , m_distanceLimits({ getNear(), getFar() })

    , m_deltaRotation({ 0.f, 0.f })

    , m_distance(distance)
    , m_deltaDistance(0.f)
{
}

CameraOrbitController::CameraOrbitController(std::unique_ptr<Camera> camera, f32 distance, f32 minDistance, f32 maxDistance) noexcept
    : CameraController(std::move(camera))
    , m_distanceLimits({ minDistance, maxDistance })

    , m_distance(distance)
    , m_deltaDistance(0.f)
{
}

CameraOrbitController::~CameraOrbitController()
{
}

void CameraOrbitController::update(f32 deltaTime)
{
    if (m_needUpdate)
    {
        math::Vector3D rotation;
        rotation.setX(m_camera->getRotation().getX() + m_deltaRotation._x * m_rotationSpeed * deltaTime);
        rotation.setY(m_camera->getRotation().getY() + m_deltaRotation._y * m_rotationSpeed * deltaTime);
        m_deltaRotation = { 0.f, 0.f };

        f32 distance = (m_camera->getTarget() - m_camera->getPosition()).length();
        distance += m_deltaDistance * m_zoomSpeed * deltaTime;
        distance = std::clamp(distance, m_distanceLimits._x, m_distanceLimits._y);
        m_deltaDistance = 0.f;

        math::Matrix4D translation;
        translation.setTranslation({ 0.f, 0.f, -distance });

        math::Matrix4D rotate;
        rotate.setRotation(rotation);
        rotate.setTranslation(m_camera->getTarget());

        math::Matrix4D view = rotate * translation;

        m_camera->setRotation(rotation);
        m_camera->setPosition(view.getTranslation());

        math::Matrix4D transform = m_camera->getTransform();
        transform.makeInverse();
        CameraController::setViewMatrix(transform);

        CameraController::update(deltaTime);
        m_needUpdate = false;
    }
}

void CameraOrbitController::handleMouseCallback(const event::InputEventHandler* handler, const event::MouseInputEvent* event)
{
    static math::Point2D prevPosition = event->_clientCoordinates;
    math::Point2D positionDelta = prevPosition - event->_clientCoordinates;

    static bool isOrbiting = false;

    const event::MouseInputEvent* mouseEvent = static_cast<const event::MouseInputEvent*>(event);
    if (mouseEvent->_event == event::MouseInputEvent::MousePressDown && mouseEvent->_key == event::KeyCode::KeyLButton)
    {
        prevPosition = {};
        positionDelta = {};
        isOrbiting = true;
    }
    else if (mouseEvent->_event == event::MouseInputEvent::MousePressUp && mouseEvent->_key == event::KeyCode::KeyLButton)
    {
        prevPosition = {};
        isOrbiting = false;
    }

    if (isOrbiting)
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
    else if (event->_event == event::MouseInputEvent::MouseWheel)
    {
        m_deltaDistance = event->_wheelValue;
        m_needUpdate = true;
    }

    prevPosition = event->_clientCoordinates;
}

void CameraOrbitController::handleTouchCallback(const event::InputEventHandler* handler, const event::TouchInputEvent* event)
{
    static math::Point2D position = event->_position;

    if (event->_event == event::TouchInputEvent::TouchMotion)
    {
        if (event->_motionEvent == event::TouchInputEvent::TouchMotionMove && handler->isScreenTouched())
        {
            LOG_DEBUG("CameraArcballHandler Log: pos %d, %d, _motionEvent %d", event->_position._x, event->_position._y, event->_motionEvent);

            if (!handler->isMultiScreenTouch())
            {
                math::Point2D positionDelta = position - event->_position;

                //m_cachedRotation.m_x += positionDelta.m_y;
                //m_cachedRotation.m_y -= positionDelta.m_x;
                //m_needUpdate = true;
            }
            else if (handler->isMultiScreenTouch())
            {
                //s32 positionDelta = position.m_y - event->_position.m_y;
                //math::Vector3D postion = CameraHandler::getPosition();
                //f32 newZPos = postion.m_z + (positionDelta * m_zoomSpeed * 0.1f);
                //if (k_signZ < 0)
                //{
                //    postion.m_z = math::clamp(newZPos, k_signZ * m_distanceLimits.m_y, k_signZ * m_distanceLimits.m_x);
                //}
                //else
                //{
                //    postion.m_z = math::clamp(newZPos, m_distanceLimits.m_x, m_distanceLimits.m_y);
                //}
                //CameraHandler::setPosition(postion);
            }
        }
    }

    position = event->_position;
}

void CameraOrbitController::handleInputEventCallback(const v3d::event::InputEventHandler* handler, const event::InputEvent* event)
{
    if (event->_eventType == event::InputEvent::InputEventType::MouseInputEvent)
    {
        const event::MouseInputEvent* mouseEvent = static_cast<const event::MouseInputEvent*>(event);
        handleMouseCallback(handler, mouseEvent);
    }
    else if (event->_eventType == event::InputEvent::InputEventType::TouchInputEvent)
    {
        const event::TouchInputEvent* touchEvent = static_cast<const event::TouchInputEvent*>(event);
        handleTouchCallback(handler, touchEvent);
    }
}

} //namespace scene
} //namespace v3d
