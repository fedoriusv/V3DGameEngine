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
    * @brief CameraArcballHelper class. Game side.
    */
    class CameraHelper
    {
    public:

        CameraHelper() = delete;
        CameraHelper(const CameraHelper&) = delete;

        explicit CameraHelper(Camera* camera, const math::Vector3D& position) noexcept;
        virtual ~CameraHelper();

        Camera& getCamera();
        const Camera& getCamera() const;

        void setPosition(const math::Vector3D& position);
        void setTarget(const math::Vector3D& target);
        void setUp(const math::Vector3D& up);

        const math::Vector3D& getPosition() const;
        const math::Vector3D& getTarget() const;

        const math::Matrix4D& getProjectionMatrix() const;
        const math::Matrix4D& getViewMatrix() const;
        const math::Vector3D& getViewPosition() const;

        void setPerspective(f32 FOV, const math::Dimension2D& size, f32 zNear, f32 zFar);
        void setOrtho(const math::Rect32& area, f32 zNear, f32 zFar);

        virtual void update(f32 deltaTime);

    private:

        Camera*         m_camera;
        math::Rect32    m_area;

    protected:

        const math::Rect32& getArea() const;

        Transform       m_transform;
        bool            m_needUpdate;

        math::Vector3D  m_viewPosition;
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace scene
} //namespace v3d
