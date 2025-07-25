#pragma once

#include "Common.h"
#include "Scene/Renderable.h"
#include "Scene/Component.h"
#include "Resource/Resource.h"

namespace v3d
{
namespace scene
{
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief Camera class. Component, Resource. Game side.
    * Calculates View and Projection matrices
    */
    class Camera : public Renderable, public resource::Resource
    {
    public:

        /**
        * @brief Camera constructor.
        * @param const math::Vector3D& target [optional]
        * @param const math::Vector3D& up [optional]
        * @param bool orthogonal [optional]
        */
        explicit Camera(const math::Vector3D& position = { 0.0f, 0.0f, 0.0f }, const math::Vector3D& target = { 0.0f, 0.0f, 1.0f }, bool orthogonal = false) noexcept;
        virtual ~Camera();

        void setPerspective(f32 FOV, const math::Dimension2D& size, f32 zNear, f32 zFar);
        void setOrtho(const math::Rect& area, f32 zNear, f32 zFar);

        void setTarget(const math::Vector3D& target);
        const math::Vector3D& getTarget() const;
        const math::Vector3D getForwardVector() const;
        const math::Vector3D getUpVector() const;

        void setPosition(const math::Vector3D& position) override;
        void setRotation(const math::Vector3D& rotation) override;
        void setScale(const math::Vector3D& scale) override;
        void setTransform(const math::Matrix4D& transform) override;

        void setNear(f32 value);
        f32 getNear() const;

        void setFar(f32 value);
        f32 getFar() const;

        void setFOV(f32 value);
        f32 getFOV() const;

        const math::Matrix4D& getViewMatrix() const;
        const math::Matrix4D& getViewMatrixInverse() const;

        const math::Matrix4D& getProjectionMatrix() const;
        const math::Matrix4D& getProjectionMatrixInverse() const;

        f32 getAspectRatio() const;
        bool isOrthogonal() const;

    protected:

        enum Matrix
        {
            Matrix_ViewMatrix = 0,
            Matrix_ViewMatrixInverse = 1,
            Matrix_ProjectionMatrix = 2,
            Matrix_ProjectionMatrixInverse = 3,

            Matrix_Count
        };

        enum CameraState
        {
            CameraState_View = 1 << 0,
            CameraState_Projection = 1 << 1,

            CameraState_All = CameraState_View | CameraState_Projection,
        };

        typedef u16 CameraStateFlags;

        void recalculateProjectionMatrix() const;
        void recalculateViewMatrix() const;

        mutable math::Matrix4D   m_matrices[Matrix_Count];
        mutable f32              m_aspectRatio;
        mutable math::Vector3D   m_target;
        mutable math::Vector3D   m_up;
        math::Rect               m_area;
        f32                      m_zNear;
        f32                      m_zFar;
        f32                      m_fieldOfView;
        bool                     m_orthogonal;
        mutable CameraStateFlags m_matricesFlags;

        friend class CameraHandler;

    private:

        bool load(const stream::Stream* stream, u32 offset = 0) override;
        bool save(stream::Stream* stream, u32 offset = 0) const override;
    };

    inline bool Camera::isOrthogonal() const
    {
        return m_orthogonal;
    }

    inline f32 Camera::getNear() const
    {
        return m_zNear;
    }

    inline f32 Camera::getFar() const
    {
        return m_zFar;
    }

    inline f32 Camera::getFOV() const
    {
        return m_fieldOfView;
    }

    inline f32 Camera::getAspectRatio() const
    {
        return m_aspectRatio;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace scene
} //namespace v3d
