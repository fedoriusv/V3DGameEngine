#pragma once

#include "Common.h"
#include "Camera.h"
#include "Transform.h"

namespace v3d
{
namespace scene
{
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * CameraHelper
    */
    class CameraHelper final
    {
    public:

        CameraHelper(Camera* camera, const core::Vector3D& position) noexcept;
        ~CameraHelper();

        Camera& getCamera();
        void setPosition(const core::Vector3D& position);

        void update(const core::Dimension2D& size);

    private:

        Camera*     m_camera;
        Transform   m_transform;
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace scene
} //namespace v3d
