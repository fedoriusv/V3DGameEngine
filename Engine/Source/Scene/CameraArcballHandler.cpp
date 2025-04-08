#include "CameraArcballHandler.h"
#include "Camera.h"
#include "Renderer/DeviceCaps.h"

#include "Utils/Logger.h"

namespace v3d
{
namespace scene
{

CameraArcballHandler::CameraArcballHandler(Camera* camera, f32 distance) noexcept
    : CameraHandler(camera, math::Vector3D(0.0f, 0.0f, k_signZ * distance))
    , m_distanceLimits({ CameraHandler::getCamera().getNear(), CameraHandler::getCamera().getFar() })
{
}

CameraArcballHandler::CameraArcballHandler(Camera* camera, f32 distance, f32 minDistance, f32 maxDistance) noexcept
    : CameraHandler(camera, math::Vector3D(0.0f, 0.0f, k_signZ * distance))
    , m_distanceLimits({ minDistance, maxDistance })
{
}

CameraArcballHandler::~CameraArcballHandler()
{
}

void CameraArcballHandler::setRotation(const math::Vector3D& rotation)
{
    m_transform.setRotation(rotation);
    m_needUpdate = true;
}

const math::Vector3D& CameraArcballHandler::getRotation() const
{
    return m_transform.getRotation();
}

void CameraArcballHandler::update(f32 deltaTime)
{
    if (m_needUpdate)
    {
        CameraHandler::update(deltaTime);

        math::Matrix4D rotate;
        rotate.setRotation(math::Vector3D(m_transform.getRotation().m_x, m_transform.getRotation().m_y, 0.0f));
        //LOG_DEBUG("rotation: x= %f, y=%f, z=%f", rotate.getRotation().x, rotate.getRotation().y, rotate.getRotation().z);

        math::Vector4D position = rotate * math::Vector4D(m_transform.getPosition(), 1.0);
        m_viewPosition = { position.m_x, position.m_y, position.m_z };

        rotate.makeTransposed();
        math::Matrix4D look = math::buildLookAtMatrix(m_transform.getPosition(), getCamera().getTarget(), getCamera().getUpVector());
        math::Matrix4D view = look * rotate;

        getCamera().setViewMatrix(view);

        m_needUpdate = false;
    }
}

void CameraArcballHandler::handleMouseCallback(v3d::event::InputEventHandler* handler, const event::MouseInputEvent* event)
{
    static math::Point2D position = event->_clientCoordinates;
    static f32 wheel = event->_wheelValue;

    if (handler->isLeftMousePressed())
    {
        math::Point2D positionDelta = position - event->_clientCoordinates;

        math::Vector3D rotation = CameraArcballHandler::getRotation();
        rotation.m_x -= positionDelta.m_y * k_rotationSpeed;
        rotation.m_y -= positionDelta.m_x * k_rotationSpeed;
        CameraArcballHandler::setRotation(rotation);
    }

    if (handler->isRightMousePressed())
    {
        s32 positionDelta = position.m_y - event->_clientCoordinates.m_y;
        math::Vector3D postion = CameraHandler::getPosition();
        f32 newZPos = postion.m_z - (positionDelta * k_zoomSpeed * 0.1f);
        if (k_signZ < 0)
        {
            postion.m_z = math::clamp(newZPos, k_signZ * m_distanceLimits.m_y, k_signZ * m_distanceLimits.m_x);
        }
        else
        {
            postion.m_z = math::clamp(newZPos, m_distanceLimits.m_x, m_distanceLimits.m_y);
        }
        CameraHandler::setPosition(postion);
    }
    else if (event->_event == event::MouseInputEvent::MouseWheel)
    {
        f32 wheelDelta = /*wheel - */event->_wheelValue;

        math::Vector3D postion = CameraHandler::getPosition();
        f32 newZPos = postion.m_z + (wheelDelta * k_zoomSpeed);
        if (k_signZ < 0)
        {
            postion.m_z = math::clamp(newZPos, k_signZ * m_distanceLimits.m_y, k_signZ * m_distanceLimits.m_x);
        }
        else
        {
            postion.m_z = math::clamp(newZPos, m_distanceLimits.m_x, m_distanceLimits.m_y);
        }
        CameraHandler::setPosition(postion);
    }

    position = event->_clientCoordinates;
    wheel = event->_wheelValue;
}

void CameraArcballHandler::handleTouchCallback(v3d::event::InputEventHandler* handler, const event::TouchInputEvent* event)
{
    static math::Point2D position = event->_position;

    if (event->_event == event::TouchInputEvent::TouchMotion)
    {
        if (event->_motionEvent == event::TouchInputEvent::TouchMotionMove && handler->isScreenTouched())
        {
            LOG_DEBUG("CameraArcballHandler Log: pos %d, %d, _motionEvent %d", event->_position.m_x, event->_position.m_y, event->_motionEvent);

            if (!handler->isMultiScreenTouch())
            {
                math::Point2D positionDelta = position - event->_position;

                math::Vector3D rotation = CameraArcballHandler::getRotation();
                rotation.m_x -= positionDelta.m_y * k_rotationSpeed;
                rotation.m_y -= positionDelta.m_x * k_rotationSpeed;
                CameraArcballHandler::setRotation(rotation);
            }
            else if (handler->isMultiScreenTouch())
            {
                s32 positionDelta = position.m_y - event->_position.m_y;
                math::Vector3D postion = CameraHandler::getPosition();
                f32 newZPos = postion.m_z + (positionDelta * k_zoomSpeed * 0.1f);
                if (k_signZ < 0)
                {
                    postion.m_z = math::clamp(newZPos, k_signZ * m_distanceLimits.m_y, k_signZ * m_distanceLimits.m_x);
                }
                else
                {
                    postion.m_z = math::clamp(newZPos, m_distanceLimits.m_x, m_distanceLimits.m_y);
                }
                CameraHandler::setPosition(postion);
            }
        }
    }

    position = event->_position;
}

} //namespace scene
} //namespace v3d
