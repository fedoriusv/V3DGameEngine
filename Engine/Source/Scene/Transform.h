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
    class Transform final
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

        void setPosition(const math::Vector3D& position);
        void setRotation(const math::Vector3D& rotation);
        void setScale(const math::Vector3D& scale);
        void setTransform(const math::Matrix4D& transform);

        const math::Vector3D& getPosition() const;
        const math::Vector3D& getRotation() const;
        const math::Vector3D& getScale() const;
        const math::Matrix4D& getTransform() const;

    private:

        void updateTransform() const;

        math::Vector3D m_position;
        math::Vector3D m_rotation;
        math::Vector3D m_scale;

        mutable math::Matrix4D m_modelMatrix;
        mutable TransformStateFlags m_transformFlag;
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace scene
} //namespace v3d
