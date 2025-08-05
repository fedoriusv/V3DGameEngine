#pragma once

#include "Common.h"
#include "Renderable.h"
#include "Resource/Resource.h"
#include "Transform.h"

namespace v3d
{
namespace renderer
{
    class Device;
    class CmdListRender;
} //namespace renderer
namespace scene
{
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////

    class Mesh;
    class LightHelper;

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief Light interface
    */
    class Light : public Object, public resource::Resource, public Renderable
    {
    public:

        void setColor(const color::ColorRGBF& color);
        void setIntensity(f32 intensity);
        void setTemperature(f32 temperature);

        f32 getIntensity() const;
        f32 getTemperature() const;
        const color::ColorRGBF& getColor() const;

        const Mesh* getVolumeMesh() const;

    protected:

        enum class Type
        {
            DirectionalLight,
            PointLight,
            SpotLight
        };

        explicit Light(Type type) noexcept;
        virtual ~Light();

        color::ColorRGBF  m_color;
        Type              m_type;
        f32               m_intensity;
        f32               m_temperature;
        Mesh*             m_volume;
        std::string       m_name;

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

    inline const Mesh* Light::getVolumeMesh() const
    {
        return m_volume;
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

        friend LightHelper;

        f32 m_attenuation;
    };

    inline TypePtr PointLight::getType() const
    {
        return typeOf<PointLight>();
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief SpotLight class
    */
    class SpotLight : public Light
    {
    public:

        SpotLight() noexcept;
        ~SpotLight();

        TypePtr getType() const final;

    private:
    };

    inline TypePtr SpotLight::getType() const
    {
        return typeOf<SpotLight>();
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////

    class LightHelper
    {
    public:

        [[nodisard]] static PointLight* createPointLight(renderer::Device* device, renderer::CmdListRender* cmdList, f32 radius, const std::string& name = "");
        [[nodisard]] static SpotLight* createSpotLight(renderer::Device* device, renderer::CmdListRender* cmdList, const std::string& name = "");
    };

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

    template<>
    struct TypeOf<scene::SpotLight>
    {
        static TypePtr get()
        {
            static TypePtr ptr = nullptr;
            return (TypePtr)&ptr;
        }
    };

////////////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace v3d