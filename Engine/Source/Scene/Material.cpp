#include "Material.h"

#include "Utils/Logger.h"

namespace v3d
{
namespace scene
{

MaterialHeader::MaterialHeader()
{
}

MaterialHeader::PropertyInfo::PropertyInfo()
{
}

Material::Material(MaterialHeader* header) noexcept
    : Resource(header)
{
    LOG_DEBUG("Material constructor %xll", this);
}

Material::~Material()
{
    LOG_DEBUG("Material destructor %xll", this);
}

void Material::init(stream::Stream * stream)
{
    //ASSERT(stream, "nullptr");
    m_stream = stream;
}

bool Material::load()
{
    if (m_loaded)
    {
        return true;
    }
    
    const MaterialHeader& header = Material::getMaterialHeader();
    for (auto& prop : header._properties)
    {
        if (!prop.second._name.empty())
        {
            Material::setParameter<renderer::Texture*>(prop.first, nullptr);
        }

        u32 index = static_cast<u32>(prop.second._value.index());
        if (index == 0)
        {
            continue;
        }

        ASSERT(index != 0, "monostate");
        if (index == 1)
        {
            Material::setParameter<f32>(prop.first, std::get<1>(prop.second._value));
        }
        else if (index == 2)
        {
            Material::setParameter<core::Vector4D>(prop.first, std::get<2>(prop.second._value));
        }
    }

    m_loaded = true;
    return true;
}

/*void Material::setFloatParameter(MaterialHeader::Property property, f32 value)
{
    auto iter = m_properties.emplace(std::make_pair(property, std::make_pair(value, nullptr)));
    if (!iter.second)
    {
        ASSERT(iter.first->second.first.index() == 0 || iter.first->second.first.index() == 1, "invalid type");
        iter.first->second.first = value;
    }
}

void Material::setVectorParameter(MaterialHeader::Property property, const core::Vector4D & vector)
{
    auto iter = m_properties.emplace(std::make_pair(property, std::make_pair(vector, nullptr)));
    if (!iter.second)
    {
        ASSERT(iter.first->second.first.index() == 0 || iter.first->second.first.index() == 2, "invalid type");
        iter.first->second.first = vector;
    }
}

void Material::setTextureParameter(MaterialHeader::Property property, renderer::Texture * texture)
{
    auto iter = m_properties.emplace(std::make_pair(property, std::make_pair(std::monostate(), texture)));
    if (!iter.second)
    {
        ASSERT(texture, "nullptr");
        iter.first->second.second = texture;
    }
}

f32 Material::getFloatParameter(MaterialHeader::Property property) const
{
    auto iter = m_properties.find(property);
    if (iter != m_properties.cend() && iter->second.first.index() == 1)
    {
        ASSERT(iter->second.first.index() == 1, "invalid type");
        return std::get<1>(iter->second.first);
    }

    LOG_WARNING("Material::getFloatParameter property %d not found", property);
    return 0.0f;
}

core::Vector4D Material::getVectorParameter(MaterialHeader::Property property) const
{
    auto iter = m_properties.find(property);
    if (iter != m_properties.cend() && iter->second.first.index() == 2)
    {
        ASSERT(iter->second.first.index() == 2, "invalid type");
        return std::get<2>(iter->second.first);
    }

    LOG_WARNING("Material::getVectorParameter property %d not found", property);
    return core::Vector4D(0.0f);
}

renderer::Texture* Material::getTextureParameter(MaterialHeader::Property property) const
{
    auto iter = m_properties.find(property);
    if (iter != m_properties.cend())
    {
        return iter->second.second;
    }

    LOG_WARNING("Material::getTextureParameter property %d not found", property);
    return nullptr;
}*/

const MaterialHeader & Material::getMaterialHeader() const
{
    return *static_cast<const MaterialHeader*>(m_header);
}

} //namespace scene
} //namespace v3d
