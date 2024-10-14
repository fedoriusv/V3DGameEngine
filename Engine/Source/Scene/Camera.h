#pragma once

#include "Common.h"
#include "Component.h"
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
    class Camera : public resource::Resource
    {
    public:

        /**
        * @brief Camera constructor.
        * @param const math::Vector3D& target [optional]
        * @param const math::Vector3D& up [optional]
        * @param bool orthogonal [optional]
        */
        explicit Camera(const math::Vector3D& target = math::Vector3D(0.0f, 0.0f, 1.0f), const math::Vector3D& up = math::Vector3D(0.0f, 1.0f, 0.0f), bool orthogonal = false) noexcept;
        virtual ~Camera();

        void setTarget(const math::Vector3D& target);
        void setUpVector(const math::Vector3D& up);
        void setNear(f32 value);
        void setFar(f32 value);
        void setFOV(f32 value);

        const math::Vector3D& getTarget() const;
        const math::Vector3D& getUpVector() const;
        const math::Matrix4D& getViewMatrix() const;
        const math::Matrix4D& getProjectionMatrix() const;

        void setViewMatrix(const math::Matrix4D& view);
        void setProjectionMatrix(const math::Matrix4D& projection);

        f32 getNear() const;
        f32 getFar() const;
        f32 getFOV() const;

        bool isOrthogonal() const;

    protected:

        enum TransformMatrix
        {
            TransformMatrix_ViewMatrix = 0,
            TransformMatrix_ProjectionMatrix = 1,

            TransformMatrix_Count
        };

        enum CameraState
        {
            CameraState_View = 1 << 0,
            CameraState_Projection = 1 << 1,

            CameraState_All = CameraState_View | CameraState_Projection,
        };

        typedef u16 CameraStateFlags;

        void recalculateProjectionMatrix(const math::Rect32& size) const;
        void recalculateViewMatrix(const math::Vector3D& position) const;

        mutable math::Matrix4D m_transform[TransformMatrix_Count];
        bool m_orthogonal;

        f32 m_zNear;
        f32 m_zFar;
        f32 m_fieldOfView;

    private:

        bool load(const stream::Stream* stream, u32 offset = 0) override;
        bool save(stream::Stream* stream, u32 offset = 0) const override;

        friend class CameraHelper;

        math::Vector3D m_up;
        math::Vector3D m_target;

        mutable CameraStateFlags m_matricesFlag;
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace scene
} //namespace v3d
