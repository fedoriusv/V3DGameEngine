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
    LOG_DEBUG("Mesh constructor %xll", this);
}

Material::~Material()
{
    LOG_DEBUG("Mesh destructor %xll", this);
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
        /*if (prop.second._name.empty())
        {
            Material::setTextureParameter(prop.first, nullptr);
        }
        else*/
        {
            u32 index = static_cast<u32>(prop.second._value.index());
            ASSERT(index != 0, "monostate");
            if (index == 1)
            {
                Material::setFloatParameter(prop.first, std::get<1>(prop.second._value));
            }
            else if (index == 2)
            {
                Material::setVectorParameter(prop.first, std::get<2>(prop.second._value));
            }
        }
    }

    m_loaded = true;
    return true;
}

void Material::setFloatParameter(MaterialHeader::Property property, f32 value)
{
    auto iter = m_properties.emplace(std::make_pair(property, value));
    if (!iter.second)
    {
        ASSERT(iter.first->second.index() == 1, "invalid type");
        iter.first->second = value;
    }
}

void Material::setVectorParameter(MaterialHeader::Property property, const core::Vector4D & vector)
{
    auto iter = m_properties.emplace(std::make_pair(property, vector));
    if (!iter.second)
    {
        ASSERT(iter.first->second.index() == 2, "invalid type");
        iter.first->second = vector;
    }
}

void Material::setTextureParameter(MaterialHeader::Property property, renderer::Texture * texture)
{
    auto iter = m_properties.emplace(std::make_pair(property, texture));
    if (!iter.second)
    {
        ASSERT(iter.first->second.index() == 3, "invalid type");
        iter.first->second = texture;
    }
}

f32 Material::getFloatParameter(MaterialHeader::Property property) const
{
    auto iter = m_properties.find(property);
    if (iter != m_properties.cend())
    {
        ASSERT(iter->second.index() == 1, "invalid type");
        return std::get<1>(iter->second);
    }

    ASSERT(false, "not found");
    return 0.0f;
}

core::Vector4D Material::getVectorParameter(MaterialHeader::Property property) const
{
    auto iter = m_properties.find(property);
    if (iter != m_properties.cend())
    {
        ASSERT(iter->second.index() == 2, "invalid type");
        return std::get<2>(iter->second);
    }

    ASSERT(false, "not found");
    return core::Vector4D(0.0f);
}

renderer::Texture * Material::getTextureParameter(MaterialHeader::Property property) const
{
    auto iter = m_properties.find(property);
    if (iter != m_properties.cend())
    {
        ASSERT(iter->second.index() == 3, "invalid type");
        return std::get<3>(iter->second);
    }

    ASSERT(false, "not found");
    return nullptr;
}

const MaterialHeader & Material::getMaterialHeader() const
{
    return *static_cast<const MaterialHeader*>(m_header);
}

} //namespace scene
} //namespace v3d
