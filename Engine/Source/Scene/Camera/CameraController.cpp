#include "CameraController.h"

namespace v3d
{
namespace scene
{

CameraController::CameraController(std::unique_ptr<Camera> camera) noexcept
    : m_camera(std::move(camera))
    , m_needUpdate(true)
{
}

CameraController::~CameraController()
{
}

void CameraController::setPerspective(f32 FOV, const math::Dimension2D& size, f32 zNear, f32 zFar)
{
    m_camera->setPerspective(FOV, size, zNear, zFar);
    m_needUpdate = true;
}

void CameraController::setOrtho(const math::Rect& area, f32 zNear, f32 zFar)
{
    m_camera->setOrtho(area, zNear, zFar);
    m_needUpdate = true;
}

Camera& CameraController::getCamera()
{
    return *m_camera;
}

const Camera& CameraController::getCamera() const
{
    return *m_camera;
}

void CameraController::setPosition(const math::Vector3D& position)
{
    m_camera->setPosition(position);
    m_needUpdate = true;
}

void CameraController::setTarget(const math::Vector3D& target)
{
    m_camera->setTarget(target);
    m_needUpdate = true;
}

void CameraController::setNear(f32 value)
{
    m_camera->setNear(value);
    m_needUpdate = true;
}

void CameraController::setFar(f32 value)
{
    m_camera->setFar(value);
    m_needUpdate = true;
}

void CameraController::setFOV(f32 value)
{
    m_camera->setFOV(value);
    m_needUpdate = true;
}

const math::Vector3D& CameraController::getPosition() const
{
    return m_camera->getPosition();
}

const math::Vector3D& CameraController::getTarget() const
{
    return m_camera->getTarget();
}

f32 CameraController::getNear() const
{
    return m_camera->getNear();
}

f32 CameraController::getFar() const
{
    return m_camera->getFar();
}

f32 CameraController::getFOV() const
{
    return m_camera->getFOV();
}

f32 CameraController::getAspectRatio() const
{
    return m_camera->getAspectRatio();
}

bool CameraController::isOrthogonal() const
{
    return m_camera->isOrthogonal();
}

const math::Matrix4D& CameraController::getProjectionMatrix() const
{
    return m_camera->getProjectionMatrix();
}

const math::Matrix4D& CameraController::getViewMatrix() const
{
    return m_camera->getViewMatrix();
}

void CameraController::update(f32 deltaTime)
{
    if (m_needUpdate)
    {
        m_camera->recalculateProjectionMatrix();
        m_camera->recalculateViewMatrix();
    }
}

math::float2 CameraController::calculateJitter(u32 frameID, const math::Dimension2D& viewport)
{
    static constexpr u32 s_sampleCount = 16;
    static auto halton = [](u32 index, u32 base) -> f32
        {
            f32 f = 1.0f;
            f32 invb = 1.f / (f32)base;
            f32 result = 0.0f;

            while (index > 0)
            {
                f *= invb;
                result = result + f * f32(index % base);
                index = u32(std::floorf(index * invb));
            }

            return result;
        };

    const f32 raito = (f32)viewport._width / (f32)viewport._height;
    const u32 phaseCount = s_sampleCount / 2 * raito * raito;
    u32 index = frameID % phaseCount;
    math::float2 jitterOffest = { halton(index, 2) - 0.5f, halton(index, 3) - 0.5f };

    math::float2 jitter =
    {
        jitterOffest._x / (f32)viewport._width * 2.0f,
        jitterOffest._y / (f32)viewport._height * -2.0f
    };

    return jitter;
}

void CameraController::setViewMatrix(const math::Matrix4D& view)
{
    m_camera->m_matrices[Camera::Matrix::Matrix_ViewMatrix] = view;
    m_camera->m_matrices[Camera::Matrix::Matrix_ViewMatrixInverse] = view.getInversed();
    m_camera->m_matricesFlags &= ~Camera::CameraState::CameraState_View;
}

void CameraController::setProjectionMatrix(const math::Matrix4D& proj)
{
    m_camera->m_matrices[Camera::Matrix::Matrix_ProjectionMatrix] = proj;
    m_camera->m_matrices[Camera::Matrix::Matrix_ProjectionMatrixInverse] = proj.getInversed();
    m_camera->m_matricesFlags &= ~Camera::CameraState::CameraState_Projection;
}

} // namespace scene
} //namespace v3d
