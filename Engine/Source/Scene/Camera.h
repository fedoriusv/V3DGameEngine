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
    * Camera class
    */
    class Camera : public Component, public resource::Resource
    {
    public:

        enum TransformMatrix
        {
            TransformMatrix_ViewMatrix = 0,
            TransformMatrix_ProjectionMatrix = 1,

            TransformMatrix_Count
        };

        enum CameraState
        {
            CameraState_View       = 1 << 0,
            CameraState_Projection = 1 << 1,

            CameraState_All = CameraState_View | CameraState_Projection,
        };

        typedef u16 CameraStateFlags;

        explicit Camera(const core::Vector3D& target = core::Vector3D(0.0f, 0.0f, 1.0f), const core::Vector3D& up = core::Vector3D(0.0f, -1.0f, 0.0f), bool orthogonal = false) noexcept;
        virtual ~Camera();

        void setTarget(const core::Vector3D& target);
        void setUpVector(const core::Vector3D& up);
        void setNearValue(f32 value);
        void setFarValue(f32 value);
        void setFOV(f32 value);

        const core::Vector3D& getTarget() const;
        const core::Vector3D& getUpVector() const;
        const core::Matrix4D& getViewMatrix() const;
        const core::Matrix4D& getProjectionMatrix() const;

        f32 getNearValue() const;
        f32 getFarValue() const;
        f32 getFOV() const;

        bool isOrthogonal() const;

        void init(stream::Stream* stream) override;
        bool load() override;

    protected:

        void recalculateProjectionMatrix(const core::Dimension2D& size) const;
        void recalculateViewMatrix(const core::Vector3D& position) const;

        mutable core::Matrix4D m_transform[TransformMatrix_Count];
        bool m_orthogonal;

        f32 m_zNear;
        f32 m_zFar;
        f32 m_fieldOfView;

    private:

        friend class CameraHelper;

        core::Vector3D m_up;
        core::Vector3D m_target;

        mutable CameraStateFlags m_matricesFlag;
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace scene
} //namespace v3d
