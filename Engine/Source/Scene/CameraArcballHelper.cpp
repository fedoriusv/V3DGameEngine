#include "CameraArcballHelper.h"
#include "Camera.h"
#include "Renderer/DeviceCaps.h"

#include "Utils/Logger.h"

namespace v3d
{
namespace scene
{

CameraArcballHelper::CameraArcballHelper(Camera* camera, f32 distance) noexcept
    : CameraHelper(camera, core::Vector3D(0.0f, 0.0f, k_signZ * distance))
    , m_distanceLimits({ CameraHelper::getCamera().getNear(), CameraHelper::getCamera().getFar() })
{
}

CameraArcballHelper::CameraArcballHelper(Camera* camera, f32 distance, f32 minDistance, f32 maxDistance) noexcept
    : CameraHelper(camera, core::Vector3D(0.0f, 0.0f, k_signZ * distance))
    , m_distanceLimits({ minDistance, maxDistance })
{
}

CameraArcballHelper::~CameraArcballHelper()
{
}

void CameraArcballHelper::setRotation(const core::Vector3D& rotation)
{
    m_transform.setRotation(rotation);
    m_needUpdate = true;
}

const core::Vector3D& CameraArcballHelper::getRotation() const
{
    return m_transform.getRotation();
}

void CameraArcballHelper::update(f32 deltaTime)
{
    if (m_needUpdate)
    {
        CameraHelper::update(deltaTime);

        core::Matrix4D rotate;
        rotate.setRotation(core::Vector3D(m_transform.getRotation().x, m_transform.getRotation().y, 0.0f));
        //LOG_DEBUG("rotation: x= %f, y=%f, z=%f", rotate.getRotation().x, rotate.getRotation().y, rotate.getRotation().z);

        core::Vector4D position = rotate * core::Vector4D(m_transform.getPosition(), 1.0);
        m_viewPosition = { position.x, position.y, position.z };

        rotate.makeTransposed();
        core::Matrix4D look = core::buildLookAtMatrix(m_transform.getPosition(), getCamera().getTarget(), getCamera().getUpVector());
        core::Matrix4D view = look * rotate;

        getCamera().setViewMatrix(view);

        m_needUpdate = false;
    }
}

void CameraArcballHelper::handlerMouseCallback(v3d::event::InputEventHandler* handler, const event::MouseInputEvent* event)
{
    static core::Point2D position = event->_cursorPosition;
    static f32 wheel = event->_wheelValue;

    if (handler->isLeftMousePressed())
    {
        core::Point2D positionDelta = position - event->_cursorPosition;

        core::Vector3D rotation = CameraArcballHelper::getRotation();
        rotation.x -= positionDelta.y * k_rotationSpeed;
        rotation.y -= positionDelta.x * k_rotationSpeed;
        CameraArcballHelper::setRotation(rotation);
    }

    if (handler->isRightMousePressed())
    {
        s32 positionDelta = position.y - event->_cursorPosition.y;
        core::Vector3D postion = CameraHelper::getPosition();
        f32 newZPos = postion.z - (positionDelta * k_zoomSpeed * 0.1f);
        if (k_signZ < 0)
        {
            postion.z = std::clamp(newZPos, k_signZ * m_distanceLimits.y, k_signZ * m_distanceLimits.x);
        }
        else
        {
            postion.z = std::clamp(newZPos, m_distanceLimits.x, m_distanceLimits.y);
        }
        CameraHelper::setPosition(postion);
    }
    else if (event->_event == event::MouseInputEvent::MouseWheel)
    {
        f32 wheelDelta = wheel - event->_wheelValue;

        core::Vector3D postion = CameraHelper::getPosition();
        f32 newZPos = postion.z + (wheelDelta * k_zoomSpeed);
        if (k_signZ < 0)
        {
            postion.z = std::clamp(newZPos, k_signZ * m_distanceLimits.y, k_signZ * m_distanceLimits.x);
        }
        else
        {
            postion.z = std::clamp(newZPos, m_distanceLimits.x, m_distanceLimits.y);
        }
        CameraHelper::setPosition(postion);
    }

    position = event->_cursorPosition;
    wheel = event->_wheelValue;
}

void CameraArcballHelper::handlerTouchCallback(v3d::event::InputEventHandler* handler, const event::TouchInputEvent* event)
{
    static core::Point2D position = event->_position;

    if (event->_event == event::TouchInputEvent::TouchMotion)
    {
        if (event->_motionEvent == event::TouchInputEvent::TouchMotionMove && handler->isScreenTouched())
        {
            LOG_DEBUG("CameraArcballHelper Log: pos %d, %d, _motionEvent %d", event->_position.x, event->_position.y, event->_motionEvent);

            if (!handler->isMultiScreenTouch())
            {
                core::Point2D positionDelta = position - event->_position;

                core::Vector3D rotation = CameraArcballHelper::getRotation();
                rotation.x -= positionDelta.y * k_rotationSpeed;
                rotation.y -= positionDelta.x * k_rotationSpeed;
                CameraArcballHelper::setRotation(rotation);
            }
            else if (handler->isMultiScreenTouch())
            {
                s32 positionDelta = position.y - event->_position.y;
                core::Vector3D postion = CameraHelper::getPosition();
                f32 newZPos = postion.z + (positionDelta * k_zoomSpeed * 0.1f);
                if (k_signZ < 0)
                {
                    postion.z = std::clamp(newZPos, k_signZ * m_distanceLimits.y, k_signZ * m_distanceLimits.x);
                }
                else
                {
                    postion.z = std::clamp(newZPos, m_distanceLimits.x, m_distanceLimits.y);
                }
                CameraHelper::setPosition(postion);
            }
        }
    }

    position = event->_position;
}

} //namespace scene
} //namespace v3d
