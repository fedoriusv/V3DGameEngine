#include "Camera.h"
#include "Utils/Logger.h"

namespace v3d
{
namespace scene
{

Camera::Camera(const core::Vector3D& target, const core::Vector3D& up, bool orthogonal) noexcept
    : m_orthogonal(orthogonal)
    , m_zNear(0.1f)
    , m_zFar(256.0f)
    , m_fieldOfView(60.0f)

    , m_up(up)
    , m_target(target)

    , m_matricesFlag(CameraState::CameraState_Projection | CameraState::CameraState_View)
{
    LOG_DEBUG("Camera constructor %xll", this);
}

Camera::~Camera()
{
    LOG_DEBUG("Camera destructor %xll", this);
}

void Camera::setTarget(const core::Vector3D& target)
{
    m_target = target;
    m_matricesFlag |= CameraState::CameraState_View;
}

void Camera::setUpVector(const core::Vector3D& up)
{
    m_up = up;
    m_matricesFlag |= CameraState::CameraState_View;
}

const core::Vector3D& Camera::getTarget() const
{
    return m_target;
}

const core::Vector3D& Camera::getUpVector() const
{
    return m_up;
}

const core::Matrix4D& Camera::getViewMatrix() const
{
    if (m_matricesFlag & CameraState::CameraState_View)
    {
        ASSERT(false, "not updated");
        //Camera::recalculateViewMatrix();
    }

    return m_transform[TransformMatrix::TransformMatrix_ViewMatrix];
}

const core::Matrix4D& Camera::getProjectionMatrix() const
{
    if (m_matricesFlag & CameraState::CameraState_Projection)
    {
        ASSERT(false, "not updated");
        //Camera::recalculateProjectionMatrix();
    }

    return m_transform[TransformMatrix::TransformMatrix_ProjectionMatrix];
}

void Camera::setViewMatrix(const core::Matrix4D & view)
{
    m_transform[TransformMatrix::TransformMatrix_ViewMatrix] = view;
    m_matricesFlag &= ~CameraState::CameraState_View;
}

void Camera::setProjectionMatrix(const core::Matrix4D & proj)
{
    m_transform[TransformMatrix::TransformMatrix_ProjectionMatrix] = proj;
    m_matricesFlag &= ~CameraState::CameraState_Projection;
}

bool Camera::isOrthogonal() const
{
    return m_orthogonal;
}

f32 Camera::getNearValue() const
{
    return m_zNear;
}

f32 Camera::getFarValue() const
{
    return m_zFar;
}

f32 Camera::getFOV() const
{
    return m_fieldOfView;
}

void Camera::setNearValue(f32 value)
{
    m_zNear = value;
    m_matricesFlag |= CameraState::CameraState_Projection;
}

void Camera::setFarValue(f32 value)
{
    m_zFar = value;
    m_matricesFlag |= CameraState::CameraState_Projection;
}

void Camera::setFOV(f32 value)
{
    m_fieldOfView = value;
    m_matricesFlag |= CameraState::CameraState_Projection;
}

void Camera::recalculateProjectionMatrix(const core::RectU32& size) const
{
    if (m_matricesFlag & CameraState::CameraState_Projection)
    {
        if (Camera::isOrthogonal())
        {
            m_transform[TransformMatrix::TransformMatrix_ProjectionMatrix] = core::buildProjectionMatrixOrtho(static_cast<f32>(size.getLeftX()), static_cast<f32>(size.getRightX()), static_cast<f32>(size.getTopY()), static_cast<f32>(size.getBottomY()), m_zNear, m_zFar);
        }
        else
        {
            m_transform[TransformMatrix::TransformMatrix_ProjectionMatrix] = core::buildProjectionMatrixPerspective(m_fieldOfView, static_cast<f32>(size.getWidth()) / static_cast<f32>(size.getHeight()), m_zNear, m_zFar);
        }
        m_matricesFlag &= ~CameraState::CameraState_Projection;
    }
}

void Camera::recalculateViewMatrix(const core::Vector3D& position) const
{
    if (m_matricesFlag & CameraState::CameraState_View)
    {
        /*core::Vector3D tgtv = m_target - position;
        core::Vector3D up = m_up;
        f32 dp = core::dotProduct(tgtv.normalize(), up.normalize());
        if (core::isEquals(fabs(dp), 1.f))
        {
            up.x = (up.x + 0.5f);
        }*/

        m_transform[TransformMatrix::TransformMatrix_ViewMatrix] = core::buildLookAtMatrix(position, m_target, m_up);
        m_matricesFlag &= ~CameraState::CameraState_View;
    }
}

void Camera::init(stream::Stream* stream)
{
    ASSERT(stream, "nullptr");
    m_stream = stream;
}

bool Camera::load()
{
    if (m_loaded)
    {
        return true;
    }
    ASSERT(m_stream, "nullptr");
    
    //TODO:
    ASSERT(false, "not implemented");

    m_loaded = true;
    return false;
}

} //namespace scene
} //namespace v3d
