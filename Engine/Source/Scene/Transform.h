#pragma once

#include "Common.h"
#include "Object.h"

namespace v3d
{
namespace scene
{
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief Transform class
    */
    class Transform
    {
    public:

        enum TransformState
        {
            TransformState_Translation = 1 << 0,
            TransformState_Rotation = 1 << 1,
            TransformState_Scale = 1 << 2,

            TransformState_All = TransformState_Translation | TransformState_Rotation | TransformState_Scale
        };
        typedef u64 TransformStateFlags;


        Transform() noexcept;
        virtual ~Transform();

        void setPosition(const math::Vector3D& position);
        void setRotation(const math::Vector3D& rotation);
        void setScale(const math::Vector3D& scale);
        void setMatrix(const math::Matrix4D& transform);

        const math::Vector3D& getPosition() const;
        const math::Vector3D& getRotation() const;
        const math::Vector3D& getScale() const;
        const math::Matrix4D& getMatrix() const;

    private:

        void claculateTransform() const;

        mutable TransformStateFlags m_transformFlag;
        mutable math::Matrix4D m_modelMatrix;

        math::Vector3D m_position;
        math::Vector3D m_rotation;
        math::Vector3D m_scale;
    };

    inline const math::Vector3D& Transform::getPosition() const
    {
        return m_position;
    }

    inline const math::Vector3D& Transform::getRotation() const
    {
        return m_rotation;
    }

    inline const math::Vector3D& Transform::getScale() const
    {
        return m_scale;
    }

    inline const math::Matrix4D& Transform::getMatrix() const
    {
        Transform::claculateTransform();
        return m_modelMatrix;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace scene
} //namespace v3d
