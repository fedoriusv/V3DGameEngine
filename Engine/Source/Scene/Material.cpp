#include "Material.h"

#include "Utils/Logger.h"
#include "Stream/Stream.h"

namespace v3d
{
namespace scene
{

MaterialHeader::MaterialHeader() noexcept
    : ResourceHeader(resource::ResourceType::MaterialResource)
    , _numProperties(0)
    , _numTextures(0)
{
    static_assert(sizeof(MaterialHeader) == 8 + sizeof(ResourceHeader), "wrong size");
}

u32 MaterialHeader::operator>>(stream::Stream* stream) const
{
    u32 parentSize = ResourceHeader::operator>>(stream);
    u32 writePos = stream->tell();

    stream->write<u32>(_numProperties);
    stream->write<u32>(_numTextures);

    u32 writeSize = stream->tell() - writePos + parentSize;
    ASSERT(sizeof(MaterialHeader) == writeSize, "wrong size");
    return writeSize;
}

u32 MaterialHeader::operator<<(const stream::Stream* stream)
{
    u32 parentSize = ResourceHeader::operator<<(stream);
    u32 readPos = stream->tell();

    stream->read<u32>(_numProperties);
    stream->read<u32>(_numTextures);

    u32 readSize = stream->tell() - readPos + parentSize;
    ASSERT(sizeof(MaterialHeader) == readSize, "wrong size");
    return readSize;
}

Material::Property::Property() noexcept
    : _index(0)
    , _array(1)
    , _label(PropertyName::Property_Unknown)
    , _type(PropertyType::Empty)
    , _data()
    , _name("")
{
}

u32 Material::Property::operator>>(stream::Stream* stream) const
{
    u32 writePos = stream->tell();

    stream->write<u32>(_index);
    stream->write<u32>(_array);
    stream->write<PropertyName>(_label);
    stream->write<PropertyType>(_type);
    if (_type == PropertyType::Empty)
    {
        //don't writing anything
    }
    else if (_type == PropertyType::Value)
    {
        stream->write<f32>(std::get<PropertyType::Value>(_data)._value);
    }
    else if (_type == PropertyType::Vector)
    {
        stream->write<math::Vector4D>(std::get<PropertyType::Vector>(_data)._value);
    }
    else if (_type == PropertyType::Texture)
    {
        stream->write(std::get<PropertyType::Texture>(_data)._path);
    }
    stream->write(_name);

    u32 writeSize = stream->tell() - writePos;
    return writeSize;
}

u32 Material::Property::operator<<(const stream::Stream* stream)
{
    u32 readPos = stream->tell();

    stream->read<u32>(_index);
    stream->read<u32>(_array);
    stream->read<PropertyName>(_label);
    stream->read<PropertyType>(_type);
    if (_type == PropertyType::Empty)
    {
        //don't read anything
    }
    else if (_type == PropertyType::Value)
    {
        ValueProperty property;
        stream->read<f32>(property._value);

        _data = property;
    }
    else if (_type == PropertyType::Vector)
    {
        VectorProperty property;
        stream->read<math::Vector4D>(property._value);

        _data = property;
    }
    else if (_type == PropertyType::Texture)
    {
        TextureProperty property;
        stream->read(property._path);

        _data = property;
    }
    stream->read(_name);

    u32 readSize = stream->tell() - readPos;
    return readSize;
}


Material::Material() noexcept
    : m_header(nullptr)
{
    LOG_DEBUG("Material constructor %llx", this);
}

Material::Material(MaterialHeader* header) noexcept
    : m_header(header)
{
    LOG_DEBUG("Material constructor %llx", this);
}

Material::~Material()
{
    LOG_DEBUG("Material destructor %llx", this);

    if (m_header)
    {
        V3D_DELETE(m_header, memory::MemoryLabel::MemoryObject);
        m_header = nullptr;
    }
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

    if (!m_header)
    {
        m_header = V3D_NEW(MaterialHeader, memory::MemoryLabel::MemoryObject);
        ASSERT(m_header, "nullptr");
        m_header->operator<<(stream);
    }
    stream->seekBeg(m_header->_offset);

    for (u32 prop = 0; prop < m_header->_numProperties; ++prop)
    {
        Material::Property property;
        property << stream;

        ASSERT(property._type != PropertyType::Empty, "monostate");
        if (property._type == PropertyType::Value)
        {
            Material::setParameter<f32>(property._label, std::get<PropertyType::Value>(property._data)._value);
        }
        else if (property._type == PropertyType::Vector)
        {
            Material::setParameter<math::Vector4D>(property._label, std::get<PropertyType::Vector>(property._data)._value);
        }
        else if (property._type == PropertyType::Texture)
        {
            //TODO
            Material::setParameter<renderer::Texture*>(property._label, nullptr);
        }
    }

    m_loaded = true;
    return true;
}

bool Material::save(stream::Stream* stream, u32 offset) const
{
    ASSERT(false, "not impl");
    return false;
}

} //namespace scene
} //namespace v3d
