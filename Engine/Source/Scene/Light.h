#pragma once

#include "Common.h"
#include "Component.h"
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
    class Model;
    class LightHelper;

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief Light interface
    */
    class Light : public Object, public resource::Resource, public ComponentBase<Light, Component>
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

        virtual ~Light();

        void setIntensity(f32 intensity);
        void setColor(const color::ColorRGBAF& color);
        void setTemperature(f32 temperature);

        f32 getIntensity() const;
        f32 getTemperature() const;
        f32 getAttenuation() const;
        const color::ColorRGBAF& getColor() const;

    protected:

        explicit Light(renderer::Device* device, Type type) noexcept;
        explicit Light(renderer::Device* device, const LightHeader& header) noexcept;

        LightHeader             m_header;
        renderer::Device* const m_device;
        color::ColorRGBAF       m_color;
        f32                     m_intensity;
        f32                     m_temperature;
        f32                     m_attenuation;

    private:

        bool load(const stream::Stream* stream, u32 offset = 0) override;
        bool save(stream::Stream* stream, u32 offset = 0) const override;
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

    inline f32 Light::getIntensity() const
    {
        return m_intensity;
    }

    inline f32 Light::getTemperature() const
    {
        return m_temperature;
    }

    inline f32 Light::getAttenuation() const
    {
        return m_attenuation;
    }

    inline const color::ColorRGBAF& Light::getColor() const
    {
        return m_color;
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
        ~DirectionalLight();
    private:

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
        ~PointLight();

    private:

        friend LightHelper;
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief SpotLight class
    */
    class SpotLight : public ComponentBase<SpotLight, Light>
    {
    public:

        explicit SpotLight(renderer::Device* device) noexcept;
        explicit SpotLight(renderer::Device* device, const LightHeader& header) noexcept;
        ~SpotLight();

    private:
    };

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