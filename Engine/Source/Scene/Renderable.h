#pragma once

#include "Common.h"
#include "Transform.h"

namespace v3d
{
namespace scene
{
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////

    class Renderable
    {
    public:

        virtual ~Renderable() = default;

        virtual void setPosition(const math::Vector3D& position);
        virtual const math::Vector3D& getPosition() const;

        virtual void setRotation(const math::Vector3D& rotation);
        virtual const math::Vector3D& getRotation() const;

        virtual void setScale(const math::Vector3D& scale);
        virtual const math::Vector3D& getScale() const;

        virtual void setTransform(const math::Matrix4D& transform);
        virtual const math::Matrix4D& getTransform() const;

    protected:

        Renderable() = default;

        mutable Transform m_transform;
    };

    inline void Renderable::setPosition(const math::Vector3D& position)
    {
        m_transform.setPosition(position);
    }

    inline const math::Vector3D& Renderable::getPosition() const
    {
        return m_transform.getPosition();
    }

    inline void Renderable::setRotation(const math::Vector3D& rotation)
    {
        m_transform.setRotation(rotation);
    }

    inline const math::Vector3D& Renderable::getRotation() const
    {
        return m_transform.getRotation();
    }

    inline void Renderable::setScale(const math::Vector3D& scale)
    {
        m_transform.setScale(scale);
    }

    inline const math::Vector3D& Renderable::getScale() const
    {
        return m_transform.getScale();
    }

    inline void Renderable::setTransform(const math::Matrix4D& transform)
    {
        m_transform.setTransform(transform);
    }

    inline const math::Matrix4D& Renderable::getTransform() const
    {
        return m_transform.getTransform();
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace scene
} //namespace v3d