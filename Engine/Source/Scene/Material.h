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
        MaterialHeader();

        enum Property : u32
        {
            Property_BaseColor,
            Property_Diffuse = Property_BaseColor,
            Property_Ambient,
            Property_Specular,
            Property_Emission,
            Property_Normals,
            Property_Heightmap,
            Property_Opacity,
            Property_Shininess
        };

        struct PropertyInfo
        {
            PropertyInfo();

            std::variant<std::monostate, f32, math::Vector4D> _value;
            std::string _name;
        };

        std::map<Property, PropertyInfo> _properties;
    };

    /**
    * Material class. Component
    */
    class Material : public Component, public resource::Resource
    {
    public:

        Material(MaterialHeader* header) noexcept;
        ~Material();

        void init(stream::Stream* stream) override;
        bool load() override;

        template<class TType>
        void setParameter(MaterialHeader::Property property, TType value);

        template<class TType>
        TType getParameter(MaterialHeader::Property property) const;

    private:

        const MaterialHeader& getMaterialHeader() const;


        std::string m_name;
        std::map<MaterialHeader::Property, std::pair<std::variant<std::monostate, f32, math::Vector4D>, renderer::Texture*>> m_properties;

        friend MaterialHelper;

    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    template<class TType>
    inline TType Material::getParameter(MaterialHeader::Property property) const
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
    inline void Material::setParameter(MaterialHeader::Property property, TType value)
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
