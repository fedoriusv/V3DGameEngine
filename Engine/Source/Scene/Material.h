#pragma once

#include "Common.h"
#include "Component.h"
#include "Resource/Resource.h"


namespace v3d
{
namespace renderer
{
    class Texture;
    class StreamBuffer;
} //namespace renderer

namespace scene
{
    class MaterialHelper;

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * MaterialHeader meta info about Material
    */
    struct MaterialHeader : resource::ResourceHeader
    {
        MaterialHeader() noexcept;
        ~MaterialHeader() noexcept = default;

        u32 operator>>(stream::Stream* stream) const;
        u32 operator<<(const stream::Stream* stream);

        u32 _numProperties;
        u32 _numTextures;
    };

    /**
    * Material class. Component, Resource
    */
    class Material : public Component, public resource::Resource
    {
    public:

        enum PropertyName : u32
        {
            Property_Unknown = 0,

            Property_Albedo,
            Property_Diffuse,
            Property_Ambient,
            Property_Specular,
            Property_Normal,
            Property_Emission,
            Property_Transparent,
            Property_Reflection,
            Property_Bump,
            Property_Height,
            Property_Opacity,
            Property_Shininess,
            Property_Refract,
            Property_Displacement,

            /*...*/

            Property_Count = 255
        };

        enum PropertyType
        {
            Empty   = 0,
            Value   = 1,
            Vector  = 2,
            Texture = 3,
        };

        struct Property
        {
            Property() noexcept;

            u32 operator>>(stream::Stream* stream) const;
            u32 operator<<(const stream::Stream* stream);

            struct ValueProperty
            {
                f32 _value;
            };

            struct VectorProperty
            {
                math::Vector4D _value;
            };

            struct TextureProperty
            {
                std::string _path;
            };

            u32          _index;
            u32          _array;
            PropertyName _label;
            PropertyType _type;
            std::variant<std::monostate, ValueProperty, VectorProperty, TextureProperty> _data;
            std::string _name;
        };

        Material() noexcept;
        explicit Material(MaterialHeader* header) noexcept;
        ~Material();

        template<class TType>
        void setParameter(PropertyName property, TType value);

        template<class TType>
        TType getParameter(PropertyName property) const;

    private:

        bool load(const stream::Stream* stream, u32 offset = 0) override;
        bool save(stream::Stream* stream, u32 offset = 0) const override;

        MaterialHeader* m_header;

        std::string m_name;
        std::unordered_map<PropertyName, std::pair<std::variant<std::monostate, f32, math::Vector4D>, renderer::Texture*>> m_properties;

        friend MaterialHelper;

    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    template<class TType>
    inline TType Material::getParameter(Material::PropertyName property) const
    {
        auto iter = m_properties.find(property);
        if constexpr (std::is_convertible<TType, renderer::Texture*>::value)
        {
            if (iter != m_properties.cend())
            {
                return static_cast<TType>(iter->second.second);
            }

            return nullptr;
        }
        else
        {
            if (iter == m_properties.cend())
            {
                return TType();
            }

            if constexpr (std::is_same<TType, f32>::value)
            {
                return std::get<1>(iter->second.first);
            }
            else if constexpr (std::is_same<TType, math::Vector4D>::value)
            {
                return std::get<2>(iter->second.first);
            }
            else
            {
                static_assert(std::is_same<TType, math::Vector4D>::value || std::is_same<TType, f32>::value, "invalid type");
            }
        }

        ASSERT(false, "fail");
        return TType();
    }

    template<class TType>
    inline void Material::setParameter(Material::PropertyName property, TType value)
    {
        if constexpr (std::is_convertible<TType, renderer::Texture*>::value)
        {
            auto iter = m_properties.emplace(std::make_pair(property, std::make_pair(std::monostate(), value)));
            if (!iter.second)
            {
                ASSERT(value, "nullptr");
                iter.first->second.second = value;
            }
        }
        else
        {
            auto iter = m_properties.emplace(std::make_pair(property, std::make_pair(value, nullptr)));
            if (!iter.second)
            {
                iter.first->second.first = value;
            }
        }
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace scene
} //namespace v3d
