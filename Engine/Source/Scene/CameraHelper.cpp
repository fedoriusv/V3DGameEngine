#include "CameraHelper.h"
#include "Renderer/Context.h"

namespace v3d
{
namespace scene
{

CameraHelper::CameraHelper(Camera* camera, const core::Vector3D& position) noexcept
    : m_camera(camera)
    , m_needUpdate(true)
    , m_viewPosition(position)
{
    m_transform.setPosition(position);
}

CameraHelper::~CameraHelper()
{
    if (m_camera)
    {
        delete m_camera;
        m_camera = nullptr;
    }
}

void CameraHelper::setPerspective(f32 FOV, const core::Dimension2D& size, f32 zNear, f32 zFar)
{
    m_area = core::Rect32(0, 0, static_cast<s32>(size.width), static_cast<s32>(size.height));

    ASSERT(m_camera, "nullptr");
    m_camera->setFOV(FOV);
    m_camera->setNear(zNear);
    m_camera->setFar(zFar);
    m_camera->m_orthogonal = false;

    m_needUpdate = true;
}

void CameraHelper::setOrtho(const core::Rect32& area, f32 zNear, f32 zFar)
{
    m_area = area;

    ASSERT(m_camera, "nullptr");
    m_camera->setNear(zNear);
    m_camera->setFar(zFar);
    m_camera->m_orthogonal = true;

    m_needUpdate = true;
}

Camera& CameraHelper::getCamera()
{
    ASSERT(m_camera, "nullptr");
    return *m_camera;
}

const Camera& CameraHelper::getCamera() const
{
    ASSERT(m_camera, "nullptr");
    return *m_camera;
}

void CameraHelper::setPosition(const core::Vector3D& position)
{
    m_transform.setPosition(position);
    m_needUpdate = true;

    ASSERT(m_camera, "nullptr");
    m_camera->m_matricesFlag |= Camera::CameraState::CameraState_View;
}

void CameraHelper::setTarget(const core::Vector3D& target)
{
    ASSERT(m_camera, "nullptr");
    m_camera->setTarget(target);

    m_needUpdate = true;
}

const core::Vector3D& CameraHelper::getPosition() const
{
    return m_transform.getPosition();
}

const core::Vector3D& CameraHelper::getTarget() const
{
    ASSERT(m_camera, "nullptr");
    return m_camera->getTarget();
}

const core::Matrix4D& CameraHelper::getProjectionMatrix() const
{
    ASSERT(m_camera, "nullptr");
    return m_camera->getProjectionMatrix();
}

const core::Matrix4D& CameraHelper::getViewMatrix() const
{
    ASSERT(m_camera, "nullptr");
    return m_camera->getViewMatrix();
}

const core::Vector3D& CameraHelper::getViewPosition() const
{
    return m_viewPosition;
}

void CameraHelper::update(f32 deltaTime)
{
    ASSERT(m_camera, "nullptr");
    if (m_needUpdate)
    {
        m_camera->recalculateProjectionMatrix(m_area);
        m_camera->recalculateViewMatrix(m_transform.getPosition());

        m_needUpdate = false;
    }
}

const core::Rect32& CameraHelper::getArea() const
{
    return m_area;
}

} // namespace scene
} //namespace v3d
