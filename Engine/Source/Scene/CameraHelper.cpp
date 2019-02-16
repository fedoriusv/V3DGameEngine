#include "CameraHelper.h"
#include "Renderer/Context.h"

namespace v3d
{
namespace scene
{

CameraHelper::CameraHelper(Camera* camera, const core::Vector3D & position) noexcept
    : m_camera(camera)
    , m_needUpdate(true)
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
    m_area = core::RectU32(0, 0, size.width, size.height);

    ASSERT(m_camera, "nullptr");
    m_camera->setFOV(FOV);
    m_camera->setNearValue(zNear);
    m_camera->setFarValue(zFar);
    m_camera->m_orthogonal = false;

    m_needUpdate = true;
}

void CameraHelper::setOrtho(const core::RectU32& area, f32 zNear, f32 zFar)
{
    m_area = area;

    ASSERT(m_camera, "nullptr");
    m_camera->setNearValue(zNear);
    m_camera->setFarValue(zFar);
    m_camera->m_orthogonal = true;

    m_needUpdate = true;
}

Camera& CameraHelper::getCamera()
{
    ASSERT(m_camera, "nullptr");
    return *m_camera;
}

void CameraHelper::setPosition(const core::Vector3D & position)
{
    m_transform.setPosition(position);
    m_needUpdate = true;

    ASSERT(m_camera, "nullptr");
    m_camera->m_matricesFlag |= Camera::CameraState::CameraState_View;
}

const core::Vector3D& CameraHelper::getPosition() const
{
    return m_transform.getPosition();
}

void CameraHelper::update()
{
    ASSERT(m_camera, "nullptr");
    if (m_needUpdate)
    {
        m_camera->recalculateProjectionMatrix(m_area);
        m_camera->recalculateViewMatrix(m_transform.getPosition());

        m_needUpdate = false;
    }
}

} // namespace scene
} //namespace v3d
