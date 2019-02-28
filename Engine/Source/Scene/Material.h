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
        std::string _name;
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

    private:

        struct ColorProperty
        {
            std::string    _name;

            core::Vector4D _color;
        };

        struct TextureProperty
        {
            std::string         _name;
            renderer::Texture*  _texture;
        };

        struct BufferProperty
        {
            std::string             _name;
            renderer::StreamBuffer* _buffer;
        };

        std::string m_name;

    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace scene
} //namespace v3d
