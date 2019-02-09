#pragma once

#include "Common.h"
#include "Component.h"

namespace v3d
{
namespace scene
{
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * Transform class
    */
    class Transform : public Component
    {
    public:

        enum TransformState
        {
            TransformState_Translation = 1 << 0,
            TransformState_Rotation = 1 << 1,
            TransformState_Scale = 1 << 2,

            TransformState_All = TransformState_Translation | TransformState_Rotation | TransformState_Scale
        };
        typedef u16 TransformStateFlags;


        Transform() noexcept;
        virtual ~Transform();

        void setPosition(const core::Vector3D& position);
        void setRotation(const core::Vector3D& rotation);
        void setScale(const core::Vector3D& scale);
        void setTransform(const core::Matrix4D& transform);

        const core::Vector3D& getPosition() const;
        const core::Vector3D& getRotation() const;
        const core::Vector3D& getScale() const;
        const core::Matrix4D& getTransform() const;

    private:

        void updateTransform() const;

        core::Vector3D m_position;
        core::Vector3D m_rotation;
        core::Vector3D m_scale;

        mutable core::Matrix4D m_modelMatrix;
        mutable TransformStateFlags m_transformFlag;
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace scene
} //namespace v3d
