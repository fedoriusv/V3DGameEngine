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
    class CameraHelper
    {
    public:

        CameraHelper() = delete;
        CameraHelper(const CameraHelper&) = delete;

        explicit CameraHelper(Camera* camera, const core::Vector3D& position) noexcept;
        virtual ~CameraHelper();

        Camera& getCamera();
        void setPosition(const core::Vector3D& position);
        const core::Vector3D& getPosition() const;

        void setPerspective(f32 FOV, const core::Dimension2D& size, f32 zNear, f32 zFar);
        void setOrtho(const core::RectU32& area, f32 zNear, f32 zFar);

        virtual void update();

    private:

        Camera*         m_camera;
        core::RectU32   m_area;

    protected:

        Transform       m_transform;
        bool            m_needUpdate;
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace scene
} //namespace v3d
