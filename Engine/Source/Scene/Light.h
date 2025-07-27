#pragma once

#include "Common.h"
#include "Renderable.h"
#include "Resource/Resource.h"
#include "Transform.h"

namespace v3d
{
namespace scene
{
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief Light interface
    */
    class Light : public Renderable, public resource::Resource
    {
    public:

        void setColor(const color::ColorRGBF& color);
        void setIntensity(f32 intensity);
        void setTemperature(f32 temperature);

        void setPosition(const math::Vector3D& position);
        void setRotation(const math::Vector3D& rotation);
        void setTransform(const math::Matrix4D& transform);

        f32 getIntensity() const;
        f32 getTemperature() const;
        const color::ColorRGBF& getColor() const;
        const scene::Transform& getTransform() const;

    protected:

        enum class Type
        {
            DirectionalLight,
            PointLight,
            SpotLight
        };

        explicit Light(Type type) noexcept;
        virtual ~Light();

        scene::Transform m_transform;
        color::ColorRGBF m_color;
        f32              m_intensity;
        f32              m_temperature;
        Type             m_type;

    private:

        bool load(const stream::Stream* stream, u32 offset = 0) override;
        bool save(stream::Stream* stream, u32 offset = 0) const override;
    };

    inline void Light::setColor(const color::ColorRGBF& color)
    {
        m_color = color;
    }

    inline void Light::setIntensity(f32 intensity)
    {
        m_intensity = intensity;
    }

    inline void Light::setTemperature(f32 temperature)
    {
        m_temperature = temperature;
    }

    inline f32 Light::getIntensity() const
    {
        return m_intensity;
    }

    inline f32 Light::getTemperature() const
    {
        return m_temperature;
    }

    inline const color::ColorRGBF& Light::getColor() const
    {
        return m_color;
    }

    inline const scene::Transform& Light::getTransform() const
    {
        return m_transform;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief DirectionalLight class
    */
    class DirectionalLight : public Light
    {
    public:

        DirectionalLight() noexcept;
        ~DirectionalLight();

        const math::Vector3D getDirection() const;

        TypePtr getType() const final;

    private:

        const math::Vector4D k_forwardVector;
    };

    inline TypePtr DirectionalLight::getType() const
    {
        return typeOf<DirectionalLight>();
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////

        /**
    * @brief PointLight class
    */
    class PointLight : public Light
    {
    public:

        PointLight() noexcept;
        ~PointLight();

        TypePtr getType() const final;

    private:

        math::Vector4D  m_position;
        f32             m_attenuation;
        f32             m_radius;
    };

    inline TypePtr PointLight::getType() const
    {
        return typeOf<PointLight>();
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace scene

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////

    template<>
    struct TypeOf<scene::DirectionalLight>
    {
        static TypePtr get()
        {
            static TypePtr ptr = nullptr;
            return (TypePtr)&ptr;
        }
    };

    template<>
    struct TypeOf<scene::PointLight>
    {
        static TypePtr get()
        {
            static TypePtr ptr = nullptr;
            return (TypePtr)&ptr;
        }
    };

////////////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace v3d