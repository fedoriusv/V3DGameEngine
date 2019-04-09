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

            std::variant<std::monostate, f32, core::Vector4D> _value;
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
        void setParmeter(MaterialHeader::Property property, TType value);

        void setTextureParameter(MaterialHeader::Property property, renderer::Texture* texture);
        void setFloatParameter(MaterialHeader::Property property, f32 value);
        void setVectorParameter(MaterialHeader::Property property, const core::Vector4D& vector);

        template<class TType>
        TType getParameter(MaterialHeader::Property property);

        renderer::Texture* getTextureParameter(MaterialHeader::Property property) const;
        f32 getFloatParameter(MaterialHeader::Property property) const;
        core::Vector4D getVectorParameter(MaterialHeader::Property property) const;

    private:

        struct Visitor
        {
            void operator()(MaterialHeader::Property property, const core::Vector4D& vector)
            {
                //TODO
            }

            void operator()(MaterialHeader::Property property, f32 value)
            {
                //TODO
            }
        };

        const MaterialHeader& getMaterialHeader() const;


        std::string m_name;
        std::map<MaterialHeader::Property, std::pair<std::variant<std::monostate, f32, core::Vector4D>, renderer::Texture*>> m_properties;

        friend MaterialHelper;

    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    template<class TType>
    inline TType Material::getParameter(MaterialHeader::Property property)
    {
        auto iter = m_properties.find(property);
        if (std::is_convertible<TType, renderer::Texture*>::value)
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

            return TType();
            //std::visit(Visitor, iter->second.first);
        }
        ASSERT(false, "not implemented");
    }

    template<class TType>
    inline void Material::setParmeter(MaterialHeader::Property property, TType value)
    {
        ASSERT(false, "not implemented");
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace scene
} //namespace v3d
