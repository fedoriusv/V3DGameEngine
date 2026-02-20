#pragma once

#include "Common.h"
#include "Component.h"
#include "Resource/Resource.h"
#include "Renderer/Color.h"

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
    class Model;
    class LightHelper;

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief Light interface
    */
    class Light : /*public Object,*/ public resource::Resource, public ComponentBase<Light, Component>
    {
    public:

        enum class Type
        {
            DirectionalLight,
            PointLight,
            SpotLight
        };

        /**
        * @brief LightHeader meta info
        */
        struct LightHeader : resource::ResourceHeader
        {
            LightHeader() noexcept
                : resource::ResourceHeader(resource::ResourceType::Light)
            {
            }

            LightHeader(Type subtype) noexcept
                : resource::ResourceHeader(resource::ResourceType::Light, toEnumType(subtype))
            {
            }
        };

        void setIntensity(f32 intensity);
        void setColor(const color::ColorRGBAF& color);
        void setTemperature(f32 temperature);
        void setAttenuation(f32 constant, f32 linear, f32 quadratic);

        f32 getIntensity() const;
        f32 getTemperature() const;
        math::float4 getAttenuation() const;
        const color::ColorRGBAF& getColor() const;
        const std::string_view getName() const;

    protected:

        explicit Light(renderer::Device* device, Type type) noexcept;
        explicit Light(renderer::Device* device, const LightHeader& header) noexcept;
        virtual ~Light();

        LightHeader             m_header;
        renderer::Device* const m_device;
        color::ColorRGBAF       m_color;
        math::float4            m_attenuation;
        f32                     m_intensity;
        f32                     m_temperature;
        bool                    m_shadowCaster;

        template<class T>
        friend void memory::internal_delete(T* ptr, v3d::memory::MemoryLabel label, const v3d::c8* file, v3d::u32 line);
    };

    inline void Light::setColor(const color::ColorRGBAF& color)
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

    inline void Light::setAttenuation(f32 constant, f32 linear, f32 quadratic)
    {
        m_attenuation._x = constant;
        m_attenuation._y = linear;
        m_attenuation._z = quadratic;
    }

    inline f32 Light::getIntensity() const
    {
        return m_intensity;
    }

    inline f32 Light::getTemperature() const
    {
        return m_temperature;
    }

    inline math::float4 Light::getAttenuation() const
    {
        return m_attenuation;
    }

    inline const color::ColorRGBAF& Light::getColor() const
    {
        return m_color;
    }

    inline const std::string_view Light::getName() const
    {
        return m_header.getName();
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief DirectionalLight class
    */
    class DirectionalLight : public ComponentBase<DirectionalLight, Light>
    {
    public:

        explicit DirectionalLight(renderer::Device* device) noexcept;
        explicit DirectionalLight(renderer::Device* device, const LightHeader& header) noexcept;

    private:

        ~DirectionalLight();

        bool load(const stream::Stream* stream, u32 offset = 0) override;
        bool save(stream::Stream* stream, u32 offset = 0) const override;

        friend LightHelper;
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief PointLight class
    */
    class PointLight : public ComponentBase<PointLight, Light>
    {
    public:

        explicit PointLight(renderer::Device* device) noexcept;
        explicit PointLight(renderer::Device* device, const LightHeader& header) noexcept;

        void setRadius(f32 radius);
        f32 getRadius() const;

    private:

        ~PointLight();

        bool load(const stream::Stream* stream, u32 offset = 0) override;
        bool save(stream::Stream* stream, u32 offset = 0) const override;

        friend LightHelper;
    };

    inline void PointLight::setRadius(f32 radius)
    {
        m_attenuation._w = radius;
    }

    inline f32 PointLight::getRadius() const
    {
        return m_attenuation._w;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief SpotLight class
    */
    class SpotLight : public ComponentBase<SpotLight, Light>
    {
    public:

        explicit SpotLight(renderer::Device* device) noexcept;
        explicit SpotLight(renderer::Device* device, const LightHeader& header) noexcept;

        void setRange(f32 range);
        void setInnerAngle(f32 degree);
        void setOuterAngle(f32 degree);

        f32 getRange() const;
        f32 getInnerAngle() const;
        f32 getOuterAngle() const;

    private:

        ~SpotLight();

        bool load(const stream::Stream* stream, u32 offset = 0) override;
        bool save(stream::Stream* stream, u32 offset = 0) const override;

        f32 m_outerAngle;
        f32 m_innerAngle;

        friend LightHelper;
    };

    inline void SpotLight::setRange(f32 range)
    {
        m_attenuation._w = range;
    }

    inline void SpotLight::setInnerAngle(f32 degree)
    {
        m_innerAngle = degree;
    }

    inline void SpotLight::setOuterAngle(f32 degree)
    {
        m_outerAngle = degree;
    }

    inline f32 SpotLight::getRange() const
    {
        return m_attenuation._w;
    }

    inline f32 SpotLight::getInnerAngle() const
    {
        return m_innerAngle;
    }

    inline f32 SpotLight::getOuterAngle() const
    {
        return m_outerAngle;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////

    class LightHelper
    {
    public:

        [[nodisard]] static DirectionalLight* createDirectionLight(renderer::Device* device, const std::string& name = "");
        [[nodisard]] static PointLight* createPointLight(renderer::Device* device, f32 radius, const std::string& name = "");
        [[nodisard]] static SpotLight* createSpotLight(renderer::Device* device, f32 range, f32 apexAngle, const std::string& name = "");
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace scene

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////

    template<>
    struct TypeOf<scene::Light>
    {
        static TypePtr get()
        {
            static TypePtr ptr = nullptr;
            return (TypePtr)&ptr;
        }
    };

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