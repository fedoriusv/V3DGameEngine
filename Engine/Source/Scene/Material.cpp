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
    //ASSERT(m_stream, "nullptr");

    //TODO

    m_loaded = true;
    return true;
}



} //namespace scene
} //namespace v3d
