#include "CameraHandler.h"

namespace v3d
{
namespace scene
{

CameraHandler::CameraHandler(std::unique_ptr<Camera> camera) noexcept
    : m_camera(std::move(camera))
    , m_needUpdate(true)
{
}

CameraHandler::~CameraHandler()
{
}

void CameraHandler::setPerspective(f32 FOV, const math::Dimension2D& size, f32 zNear, f32 zFar)
{
    m_camera->setPerspective(FOV, size, zNear, zFar);
    m_needUpdate = true;
}

void CameraHandler::setOrtho(const math::Rect& area, f32 zNear, f32 zFar)
{
    m_camera->setOrtho(area, zNear, zFar);
    m_needUpdate = true;
}

Camera& CameraHandler::getCamera()
{
    return *m_camera;
}

const Camera& CameraHandler::getCamera() const
{
    return *m_camera;
}

void CameraHandler::setPosition(const math::Vector3D& position)
{
    m_camera->setPosition(position);
    m_needUpdate = true;
}

void CameraHandler::setTarget(const math::Vector3D& target)
{
    m_camera->setTarget(target);
    m_needUpdate = true;
}

void CameraHandler::setNear(f32 value)
{
    m_camera->setNear(value);
    m_needUpdate = true;
}

void CameraHandler::setFar(f32 value)
{
    m_camera->setFar(value);
    m_needUpdate = true;
}

void CameraHandler::setFOV(f32 value)
{
    m_camera->setFOV(value);
    m_needUpdate = true;
}

const math::Vector3D& CameraHandler::getPosition() const
{
    return m_camera->getPosition();
}

const math::Vector3D& CameraHandler::getTarget() const
{
    return m_camera->getTarget();
}

f32 CameraHandler::getNear() const
{
    return m_camera->getNear();
}

f32 CameraHandler::getFar() const
{
    return m_camera->getFar();
}

f32 CameraHandler::getFOV() const
{
    return m_camera->getFOV();
}

f32 CameraHandler::getAspectRatio() const
{
    return m_camera->getAspectRatio();
}

bool CameraHandler::isOrthogonal() const
{
    return m_camera->isOrthogonal();
}

const math::Matrix4D& CameraHandler::getProjectionMatrix() const
{
    return m_camera->getProjectionMatrix();
}

const math::Matrix4D& CameraHandler::getViewMatrix() const
{
    return m_camera->getViewMatrix();
}

void CameraHandler::update(f32 deltaTime)
{
    if (m_needUpdate)
    {
        m_camera->recalculateProjectionMatrix();
        m_camera->recalculateViewMatrix();
    }
}

void CameraHandler::setViewMatrix(const math::Matrix4D& view)
{
    m_camera->m_matrices[Camera::Matrix::Matrix_ViewMatrix] = view;
    m_camera->m_matrices[Camera::Matrix::Matrix_ViewMatrixInverse] = view.getInversed();
    m_camera->m_transform.setTransform(m_camera->m_matrices[Camera::Matrix::Matrix_ViewMatrixInverse]);
    m_camera->m_matricesFlags &= ~Camera::CameraState::CameraState_View;
}

void CameraHandler::setProjectionMatrix(const math::Matrix4D& proj)
{
    m_camera->m_matrices[Camera::Matrix::Matrix_ProjectionMatrix] = proj;
    m_camera->m_matrices[Camera::Matrix::Matrix_ProjectionMatrixInverse] = proj.getInversed();
    m_camera->m_matricesFlags &= ~Camera::CameraState::CameraState_Projection;
}

} // namespace scene
} //namespace v3d
