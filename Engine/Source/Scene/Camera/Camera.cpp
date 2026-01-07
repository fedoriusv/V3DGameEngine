#include "Camera.h"
#include "Utils/Logger.h"
#include "Stream/Stream.h"

namespace v3d
{
namespace scene
{

Camera::Camera(const math::Vector3D& position, const math::Vector3D& target, bool orthogonal) noexcept
    : m_header()
    , m_clipNear(0.1f)
    , m_clipFar(256.0f)
    , m_fieldOfView(60.0f)
    , m_aspectRatio(1.f)
    , m_orthogonal(orthogonal)

    , m_target(target)
    , m_up({ 0.0f, 1.0f, 0.0f })
    , m_matricesFlags(CameraState::CameraState_Projection | CameraState::CameraState_View)
{
    LOG_DEBUG("Camera constructor %llx", this);
    math::Matrix4D view = math::SMatrix::lookAtMatrix(position, m_target, m_up);
    if (std::abs(view.getDeterminant()) > 1e-6f)
    {
        view.makeInverse();
        m_transform.setMatrix(view);
    }
}

Camera::Camera(const CameraHeader& header) noexcept
    : m_header(header)
    , m_clipNear(0.0f)
    , m_clipFar(0.0f)
    , m_fieldOfView(0.0f)
    , m_aspectRatio(1.f)
    , m_orthogonal(false)

    , m_target({0.f, 0.f, 0.f })
    , m_up({ 0.0f, 1.0f, 0.0f })
    , m_matricesFlags(CameraState::CameraState_Projection | CameraState::CameraState_View)
{
    LOG_DEBUG("Camera constructor %llx", this);
}

Camera::~Camera()
{
    LOG_DEBUG("Camera destructor %llx", this);
}

void Camera::setPerspective(f32 FOV, const math::Dimension2D& size, f32 zNear, f32 zFar)
{
    m_fieldOfView = FOV;
    m_clipNear = zNear;
    m_clipFar = zFar;
    m_orthogonal = false;
    m_aspectRatio = static_cast<f32>(size._width) / static_cast<f32>(size._height);

    m_area = math::Rect(0.f, 0.f, static_cast<f32>(size._width), static_cast<f32>(size._height));
    m_matricesFlags |= CameraState::CameraState_Projection;
}

void Camera::setOrtho(const math::Rect& area, f32 zNear, f32 zFar)
{
    m_clipNear = zNear;
    m_clipFar = zFar;
    m_orthogonal = true;
    m_aspectRatio = m_area.getWidth() / m_area.getHeight();

    m_area = area;
    m_matricesFlags |= CameraState::CameraState_Projection;
}

void Camera::setTarget(const math::Vector3D& target)
{
    m_target = target;
    m_matricesFlags |= CameraState::CameraState_View;
}

void Camera::setPosition(const math::Vector3D& position)
{
    m_transform.setPosition(position);
    m_matricesFlags |= CameraState::CameraState_View;
}

void Camera::setRotation(const math::Vector3D& rotation)
{
    m_transform.setRotation(rotation);
    m_matricesFlags |= CameraState::CameraState_View;
}

void Camera::setTransform(const math::Matrix4D& transform)
{
    m_transform.setMatrix(transform);
    m_matricesFlags |= CameraState::CameraState_View;
}

const math::Vector3D& Camera::getTarget() const
{
    return m_target;
}

const math::Vector3D Camera::getForwardVector() const
{
    return math::Vector3D(m_transform.getMatrix()[8], m_transform.getMatrix()[9], m_transform.getMatrix()[10]).normalize();
}

const math::Vector3D Camera::getUpVector() const
{
    return m_up;
}

const math::Matrix4D& Camera::getViewMatrix() const
{
    ASSERT(!(m_matricesFlags & CameraState::CameraState_View), "not updated");
    return m_matrices[Matrix::Matrix_ViewMatrix];
}

const math::Matrix4D& Camera::getViewMatrixInverse() const
{
    ASSERT(!(m_matricesFlags & CameraState::CameraState_View), "not updated");
    return m_matrices[Matrix::Matrix_ViewMatrixInverse];
}

const math::Matrix4D& Camera::getProjectionMatrix() const
{
    ASSERT(!(m_matricesFlags & CameraState::CameraState_Projection), "not updated");
    return m_matrices[Matrix::Matrix_ProjectionMatrix];
}

const math::Matrix4D& Camera::getProjectionMatrixInverse() const
{
    ASSERT(!(m_matricesFlags & CameraState::CameraState_Projection), "not updated");
    return m_matrices[Matrix::Matrix_ProjectionMatrixInverse];
}

void Camera::setNear(f32 value)
{
    m_clipNear = value;
    m_matricesFlags |= CameraState::CameraState_Projection;
}

void Camera::setFar(f32 value)
{
    m_clipFar = value;
    m_matricesFlags |= CameraState::CameraState_Projection;
}

void Camera::setFOV(f32 value)
{
    m_fieldOfView = value;
    m_matricesFlags |= CameraState::CameraState_Projection;
}

void Camera::recalculateProjectionMatrix() const
{
    if (m_matricesFlags & CameraState::CameraState_Projection)
    {
        if (m_orthogonal)
        {
            m_matrices[Matrix::Matrix_ProjectionMatrix] = math::SMatrix::projectionMatrixOrtho(m_area.getLeftX(), m_area.getRightX(), m_area.getBottomY(), m_area.getTopY(), m_clipNear, m_clipFar);
        }
        else
        {
            f32 fovY = m_fieldOfView * math::k_degToRad;
            m_matrices[Matrix::Matrix_ProjectionMatrix] = math::SMatrix::projectionMatrixPerspective(fovY, m_area.getWidth() / m_area.getHeight(), m_clipNear, m_clipFar);
        }
        m_matrices[Matrix::Matrix_ProjectionMatrixInverse] = m_matrices[Matrix::Matrix_ProjectionMatrix].getInversed();
        m_matricesFlags &= ~CameraState::CameraState_Projection;
    }
}

void Camera::recalculateViewMatrix() const
{
    if (m_matricesFlags & CameraState::CameraState_View)
    {
        m_matrices[Matrix::Matrix_ViewMatrix] = math::SMatrix::lookAtMatrix(m_transform.getPosition(), m_target, m_up);
        m_matrices[Matrix::Matrix_ViewMatrixInverse] = m_matrices[Matrix::Matrix_ViewMatrix].getInversed();
        m_matricesFlags &= ~CameraState::CameraState_View;
    }
}

bool Camera::load(const stream::Stream* stream, u32 offset)
{
    if (m_loaded)
    {
        LOG_WARNING("Camera::load: the camera %llx is already loaded", this);
        return true;
    }

    ASSERT(stream, "nullptr");
    stream->seekBeg(offset);
    ASSERT(offset == m_header._offset, "wrong offset");

    stream->read<f32>(m_clipNear);
    stream->read<f32>(m_clipFar);
    stream->read<f32>(m_fieldOfView);
    stream->read<bool>(m_orthogonal);

    m_matricesFlags = CameraState::CameraState_All;
    m_loaded = true;
    return true;
}

bool Camera::save(stream::Stream* stream, u32 offset) const
{
    ASSERT(false, "not impl");
    return false;
}

} //namespace scene
} //namespace v3d
