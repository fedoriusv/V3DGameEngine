#include "CameraHelper.h"
#include "Renderer/Context.h"

namespace v3d
{
namespace scene
{

CameraHelper::CameraHelper(Camera* camera, const core::Vector3D & position) noexcept
    : m_camera(camera)
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

Camera& CameraHelper::getCamera()
{
    ASSERT(m_camera, "nullptr");
    return *m_camera;
}

void CameraHelper::setPosition(const core::Vector3D & position)
{
    m_transform.setPosition(position);
}

void CameraHelper::update(const core::Dimension2D& size)
{
    ASSERT(m_camera, "nullptr");
    m_camera->recalculateProjectionMatrix(size);
    m_camera->recalculateViewMatrix(m_transform.getPosition());
}

} // namespace scene
} //namespace v3d
