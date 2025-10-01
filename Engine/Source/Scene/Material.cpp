#include "Material.h"

#include "Utils/Logger.h"
#include "Stream/Stream.h"
#include "Resource/ResourceManager.h"
#include "Resource/Loader/ImageFileLoader.h"

namespace v3d
{
namespace scene
{

Material::Material(renderer::Device* device, MaterialShadingModel shadingModel) noexcept
    : m_header()
    , m_device(device)
    , m_shadingModel(shadingModel)
{
}

Material::Material(renderer::Device* device, const MaterialHeader& header) noexcept
    : m_header(header)
    , m_device(device)
    , m_shadingModel(MaterialShadingModel::Custom)
{
}

bool Material::load(const stream::Stream* stream, u32 offset)
{
    if (m_loaded)
    {
        LOG_WARNING("Material::load: the material %llx is already loaded", this);
        return true;
    }

    ASSERT(stream, "nullptr");
    stream->seekBeg(offset);
    ASSERT(offset == m_header._offset, "wrong offset");

    enum PropertyType : u8
    {
        Scalar = 0,
        Vector = 1,
        TexturePath = 2
    };

    static auto propertyValue = [this](const stream::Stream* stream, PropertyType type) -> Material::Property
        {
            switch (type)
            {
            default:
            case Scalar:
            {
                f32 value = 0.f;
                stream->read<f32>(value);
                return value;
            }
            case Vector:
            {
                math::float4 value;
                stream->read<math::float4>(value);
                return value;
            }
            case TexturePath:
            {
                std::string name;
                stream->read(name);
                
                resource::ImageDecoder::TexturePolicy policy;
                policy.usage = renderer::TextureUsage::TextureUsage_Sampled | renderer::TextureUsage_Shared | renderer::TextureUsage_Write;

                renderer::Texture2D* texture = resource::ResourceManager::getInstance()->load<renderer::Texture2D, resource::TextureFileLoader>(m_device, name, policy);
                ASSERT(texture, "is not loaded");
                return texture;
            }
            }
        };

    stream->read<MaterialShadingModel>(m_shadingModel);

    u32 properties = 0;
    stream->read<u32>(properties);

    for (u32 i = 0; i < properties; ++i)
    {
        std::string propertyName;
        PropertyType type;
        Material::Property property;

        stream->read(propertyName);
        stream->read<PropertyType>(type);
        property = propertyValue(stream, type);

        setProperty(propertyName, property);
    }

    m_loaded = true;
    return true;
}

bool Material::save(stream::Stream* stream, u32 offset) const
{
    if (!m_loaded)
    {
        LOG_WARNING("Material::save: the material %llx is not loaded", this);
        return false;
    }

    //TODO

    return true;
}


} //namespace scene
} //namespace v3d
