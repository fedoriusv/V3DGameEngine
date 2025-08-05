#pragma once

#include "Common.h"
#include "Transform.h"

namespace v3d
{
namespace scene
{
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////

    class ModelHandler;

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief Renderable class property
    */
    class Renderable
    {
    public:

        virtual ~Renderable() = default;

        virtual void setPosition(const math::Vector3D& position);
        virtual void setRotation(const math::Vector3D& rotation);
        virtual void setScale(const math::Vector3D& scale);
        virtual void setTransform(const math::Matrix4D& transform);

        const math::Vector3D& getPosition() const;
        const math::Vector3D& getRotation() const;
        const math::Vector3D& getScale() const;
        const math::Matrix4D& getTransform() const;
        const math::Matrix4D& getPrevTransform() const;

        virtual TypePtr getType() const = 0;

    protected:

        friend ModelHandler;

        Renderable() = default;

        Transform m_transform;
        Transform m_prevTransform;
    };

    inline void Renderable::setPosition(const math::Vector3D& position)
    {
        m_transform.setPosition(position);
    }

    inline void Renderable::setRotation(const math::Vector3D& rotation)
    {
        m_transform.setRotation(rotation);
    }

    inline void Renderable::setScale(const math::Vector3D& scale)
    {
        m_transform.setScale(scale);
    }

    inline void Renderable::setTransform(const math::Matrix4D& transform)
    {
        m_transform.setTransform(transform);
    }

    inline const math::Vector3D& Renderable::getPosition() const
    {
        return m_transform.getPosition();
    }

    inline const math::Vector3D& Renderable::getRotation() const
    {
        return m_transform.getRotation();
    }

    inline const math::Vector3D& Renderable::getScale() const
    {
        return m_transform.getScale();
    }

    inline const math::Matrix4D& Renderable::getTransform() const
    {
        return m_transform.getTransform();
    }

    inline const math::Matrix4D& Renderable::getPrevTransform() const
    {
        return m_prevTransform.getTransform();
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace scene
} //namespace v3d