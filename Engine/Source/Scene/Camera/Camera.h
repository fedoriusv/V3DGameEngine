#pragma once

#include "Common.h"
#include "Scene/Component.h"
#include "Scene/Transform.h"
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
    class Camera : public Object, public resource::Resource, public ComponentBase<Camera, Component>
    {
    public:

        /**
        * @brief CameraHeader struct.
        */
        struct CameraHeader : resource::ResourceHeader
        {
            CameraHeader() noexcept
                : resource::ResourceHeader(resource::ResourceType::Camera)
            {
            }
        };

        /**
        * @brief Camera constructor.
        * @param const math::Vector3D& target [optional]
        * @param const math::Vector3D& up [optional]
        * @param bool orthogonal [optional]
        */
        explicit Camera(const math::Vector3D& position = { 0.0f, 0.0f, 0.0f }, const math::Vector3D& target = { 0.0f, 0.0f, 1.0f }, bool orthogonal = false) noexcept;
        explicit Camera(const CameraHeader& header) noexcept;
        virtual ~Camera();

        void setPerspective(f32 FOV, const math::Dimension2D& size, f32 zNear, f32 zFar);
        void setOrtho(const math::Rect& area, f32 zNear, f32 zFar);

        void setTarget(const math::Vector3D& target);
        const math::Vector3D& getTarget() const;
        const math::Vector3D getForwardVector() const;
        const math::Vector3D getUpVector() const;

        void setPosition(const math::Vector3D& position);
        void setRotation(const math::Vector3D& rotation);
        void setScale(const math::Vector3D& scale);
        void setTransform(const math::Matrix4D& transform);

        const math::Vector3D& getPosition() const;
        const math::Vector3D& getRotation() const;
        const math::Vector3D& getScale() const;
        const math::Matrix4D& getTransform() const;

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

        typedef u8 CameraStateFlags;

        void recalculateProjectionMatrix() const;
        void recalculateViewMatrix() const;

        CameraHeader             m_header;
        Transform                m_transform;
        mutable math::Matrix4D   m_matrices[Matrix_Count];
        mutable math::Vector3D   m_target;
        mutable math::Vector3D   m_up;
        math::Rect               m_area;
        mutable f32              m_aspectRatio;
        f32                      m_clipNear;
        f32                      m_clipFar;
        f32                      m_fieldOfView;
        mutable CameraStateFlags m_matricesFlags;
        bool                     m_orthogonal;

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
        return m_clipNear;
    }

    inline f32 Camera::getFar() const
    {
        return m_clipFar;
    }

    inline f32 Camera::getFOV() const
    {
        return m_fieldOfView;
    }

    inline f32 Camera::getAspectRatio() const
    {
        return m_aspectRatio;
    }

    inline const math::Vector3D& Camera::getPosition() const
    {
        return m_transform.getPosition();
    }

    inline const math::Vector3D& Camera::getRotation() const
    {
        return m_transform.getRotation();
    }

    inline const math::Vector3D& Camera::getScale() const
    {
        return m_transform.getScale();
    }

    inline const math::Matrix4D& Camera::getTransform() const
    {
        return m_transform.getMatrix();
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace scene

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////
    
    template<>
    struct TypeOf<scene::Camera>
    {
        static TypePtr get()
        {
            static TypePtr ptr = nullptr;
            return (TypePtr)&ptr;
        }
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace v3d
