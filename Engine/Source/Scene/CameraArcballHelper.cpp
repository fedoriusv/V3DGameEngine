#include "CameraArcballHelper.h"
#include "Camera.h"

#include "Utils/Logger.h"

namespace v3d
{
namespace scene
{

CameraArcballHelper::CameraArcballHelper(Camera* camera, f32 distance) noexcept
    : CameraHelper(camera, core::Vector3D(0.0f, 0.0f, k_signZ * distance))
{
}

CameraArcballHelper::~CameraArcballHelper()
{
}

void CameraArcballHelper::setRotation(const core::Vector3D & rotation)
{
    m_transform.setRotation(rotation);
    m_needUpdate = true;
}

const core::Vector3D& CameraArcballHelper::getRotation() const
{
    return m_transform.getRotation();
}

void CameraArcballHelper::update()
{
    if (m_needUpdate)
    {
        CameraHelper::update();

        core::Matrix4D rotateX;
        rotateX.makeIdentity();
        rotateX.setRotation(core::Vector3D(m_transform.getRotation().x, 0.0f, 0.0f));

        core::Matrix4D rotateY;
        rotateY.makeIdentity();
        rotateY.setRotation(core::Vector3D(0.0f, m_transform.getRotation().y, 0.0f));

        core::Matrix4D rotate = rotateX * rotateY;

        core::Matrix4D look = core::buildLookAtMatrix(m_transform.getPosition(), getCamera().getTarget(), getCamera().getUpVector());
        core::Matrix4D view = look * rotate;

        /*core::Matrix4D position;
        position.makeIdentity();
        position.setTranslation(-m_transform.getPosition());
        core::Matrix4D view = look * rotate;*/

        getCamera().setViewMatrix(view);
        m_needUpdate = false;
    }
}

void CameraArcballHelper::handlerCallback(v3d::event::InputEventHandler* handler, const event::MouseInputEvent* event)
{
    static core::Point2D position = event->_cursorPosition;
    static f32 wheel = event->_wheelValue;

    if (handler->isLeftMousePressed())
    {
        core::Point2D positionDelta = position - event->_cursorPosition;

        core::Vector3D rotation = CameraArcballHelper::getRotation();
        rotation.x += positionDelta.y * k_rotationSpeed;
        rotation.y += positionDelta.x * k_rotationSpeed;
        CameraArcballHelper::setRotation(rotation);
    }

    if (handler->isRightMousePressed())
    {
        s32 positionDelta = position.y - event->_cursorPosition.y;
        core::Vector3D postion = CameraHelper::getPosition();
        f32 newZPos = postion.z + (positionDelta * k_zoomSpeed * 0.1f);
        if (k_signZ < 0)
        {
            postion.z = std::clamp(newZPos, k_signZ * CameraHelper::getCamera().getFarValue(), k_signZ * CameraHelper::getCamera().getNearValue());
        }
        else
        {
            postion.z = std::clamp(newZPos, CameraHelper::getCamera().getNearValue(), CameraHelper::getCamera().getFarValue());
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
            postion.z = std::clamp(newZPos, k_signZ * CameraHelper::getCamera().getFarValue(), k_signZ * CameraHelper::getCamera().getNearValue());
        }
        else
        {
            postion.z = std::clamp(newZPos, CameraHelper::getCamera().getNearValue(), CameraHelper::getCamera().getFarValue());
        }
        CameraHelper::setPosition(postion);
    }

    position = event->_cursorPosition;
    wheel = event->_wheelValue;
}

} //namespace scene
} //namespace v3d