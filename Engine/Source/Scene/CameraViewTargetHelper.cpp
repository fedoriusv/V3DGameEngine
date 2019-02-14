#include "CameraViewTargetHelper.h"

namespace v3d
{
namespace scene
{

CameraViewTargetHelper::CameraViewTargetHelper(Camera * camera, const core::Vector3D & position) noexcept
    : CameraHelper(camera, position)
    , m_needUpdate(false)
{
}

CameraViewTargetHelper::~CameraViewTargetHelper()
{
}

void CameraViewTargetHelper::setRotation(const core::Vector3D & rotation)
{
    m_rotate = rotation;
    m_needUpdate = true;
}

const core::Vector3D& CameraViewTargetHelper::getRotation() const
{
    return m_transform.getRotation();
}

void CameraViewTargetHelper::update()
{
    if (m_needUpdate)
    {
        core::Matrix4D tranformCamera;
        tranformCamera.makeIdentity();
        tranformCamera.setTranslation(m_transform.getPosition());

        core::Matrix4D rotate;
        rotate.makeIdentity();
        rotate.setRotation(m_rotate);

        core::Matrix4D view = rotate * tranformCamera;
        CameraHelper::setPosition(view.getTranslation());
        //getCamera().setViewMatrix(view);

        m_needUpdate = false;
    }

    CameraHelper::update();
}

void CameraViewTargetHelper::rotateHandler(const event::MouseInputEvent * event)
{
    static core::Point2D position = event->_cursorPosition;
    static f32 wheel = event->_wheelValue;
    position = event->_cursorPosition;
    wheel = event->_wheelValue;
}

} //namespace scene
} //namespace v3d
