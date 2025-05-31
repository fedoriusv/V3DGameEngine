#include "CameraArcballHandler.h"
#include "Camera.h"
#include "Renderer/DeviceCaps.h"

#include "Utils/Logger.h"

namespace v3d
{
namespace scene
{

CameraArcballHandler::CameraArcballHandler(std::unique_ptr<Camera> camera, f32 distance) noexcept
    : CameraHandler(std::move(camera))
    , m_distanceLimits({ getNear(), getFar() })

    , m_rotation({ 0.f, 0.f, 0.f })
    , m_deltaRotation({ 0.f, 0.f })

    , m_distance(distance)
    , m_deltaDistance(0.f)
{
}

CameraArcballHandler::CameraArcballHandler(std::unique_ptr<Camera> camera, f32 distance, f32 minDistance, f32 maxDistance) noexcept
    : CameraHandler(std::move(camera))
    , m_distanceLimits({ minDistance, maxDistance })

    , m_distance(distance)
    , m_deltaDistance(0.f)
{
}

CameraArcballHandler::~CameraArcballHandler()
{
}

void CameraArcballHandler::update(f32 deltaTime)
{
    if (m_needUpdate)
    {
        m_rotation.setX(m_rotation.getX() + m_deltaRotation._x * m_rotationSpeed * deltaTime);
        m_rotation.setY(m_rotation.getY() + m_deltaRotation._y * m_rotationSpeed * deltaTime);
        m_deltaRotation = { 0.f, 0.f };


        m_distance += m_deltaDistance * m_zoomSpeed * deltaTime;
        m_distance = std::clamp(m_distance, m_distanceLimits._x, m_distanceLimits._y);
        m_deltaDistance = 0.f;

        //TODO
        math::Matrix4D look = math::SMatrix::lookAtMatrix(math::VectorRegister3D{ 0.0f, 0.0f, 0.0f }, math::VectorRegister3D{ 0.0f, 0.0f, 1.0f }, math::VectorRegister3D{ 0.0f, 1.0f, 0.0f });
        look.makeInverse();
        look.setTranslation({ 0.f, 0.f, -m_distance });

        math::Matrix4D rotate;
        rotate.setRotation(m_rotation);

        math::Matrix4D view = look * rotate;
        m_camera->setTransform(view);

        view.makeInverse();
        setViewMatrix(view);

        CameraHandler::update(deltaTime);
        m_needUpdate = false;
    }
}

void CameraArcballHandler::handleMouseCallback(v3d::event::InputEventHandler* handler, const event::MouseInputEvent* event)
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

void CameraArcballHandler::handleTouchCallback(v3d::event::InputEventHandler* handler, const event::TouchInputEvent* event)
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

} //namespace scene
} //namespace v3d
