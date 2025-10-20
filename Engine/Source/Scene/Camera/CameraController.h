#pragma once

#include "Common.h"
#include "Camera.h"
#include "Scene/Transform.h"

namespace v3d
{
namespace scene
{
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief CameraController class. Game side.
    */
    class CameraController : public Object
    {
    public:

        CameraController() = delete;
        CameraController(const CameraController&) = delete;

        explicit CameraController(std::unique_ptr<Camera> camera) noexcept;
        virtual ~CameraController();

        Camera& getCamera();
        const Camera& getCamera() const;

        void setPosition(const math::Vector3D& position);
        void setTarget(const math::Vector3D& target);
        void setNear(f32 value);
        void setFar(f32 value);
        void setFOV(f32 value);

        const math::Vector3D& getPosition() const;
        const math::Vector3D& getTarget() const;
        f32 getNear() const;
        f32 getFar() const;
        f32 getFOV() const;
        f32 getAspectRatio() const;
        bool isOrthogonal() const;

        void setViewMatrix(const math::Matrix4D& view);
        const math::Matrix4D& getViewMatrix() const;

        void setProjectionMatrix(const math::Matrix4D& projection);
        const math::Matrix4D& getProjectionMatrix() const;

        void setPerspective(f32 FOV, const math::Dimension2D& size, f32 zNear, f32 zFar);
        void setOrtho(const math::Rect& area, f32 zNear, f32 zFar);

        virtual void update(f32 deltaTime);

        static math::float2 calculateJitter(u32 frameID, const math::Dimension2D& viewport);

    protected:

        std::unique_ptr<Camera> m_camera;
        bool m_needUpdate;
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace scene
} //namespace v3d
