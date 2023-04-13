#include "Shader.h"
#include "Utils/Logger.h"
#include "Stream/Stream.h"

#include "crc32c/crc32c.h"

namespace v3d
{
namespace renderer
{

ShaderHeader::ShaderHeader() noexcept
    : ResourceHeader(resource::ResourceType::ShaderResource)
    , _shaderType(renderer::ShaderType::Undefined)
    , _contentType(ShaderContent::Source)
    , _shaderModel(ShaderModel::Default)
    , _optLevel(0)
{
    static_assert(sizeof(ShaderHeader) == sizeof(resource::ResourceHeader) + 8, "wrong size");
}

ShaderHeader::ShaderHeader(renderer::ShaderType type) noexcept
    : ResourceHeader(resource::ResourceType::ShaderResource)
    , _shaderType(type)
    , _contentType(ShaderContent::Source)
    , _shaderModel(ShaderModel::Default)
    , _optLevel(0)
{
}

ShaderHeader::ShaderHeader(const ShaderHeader& other) noexcept
    : ResourceHeader(other)
    , _shaderType(other._shaderType)
    , _contentType(other._contentType)
    , _shaderModel(other._shaderModel)
    , _optLevel(other._optLevel)
{
}

u32 ShaderHeader::operator>>(stream::Stream* stream) const
{
    u32 parentSize = ResourceHeader::operator>>(stream);
    u32 writePos = stream->tell();

    stream->write<renderer::ShaderType>(_shaderType);
    stream->write<ShaderContent>(_contentType);
    stream->write<ShaderModel>(_shaderModel);
    stream->write<u16>(_optLevel);

    u32 writeSize = stream->tell() - writePos + parentSize;
    ASSERT(sizeof(ShaderHeader) == writeSize, "wrong size");
    return writeSize;
}

u32 ShaderHeader::operator<<(const stream::Stream* stream)
{
    u32 parentSize = ResourceHeader::operator<<(stream);
    u32 readPos = stream->tell();

    stream->read<renderer::ShaderType>(_shaderType);
    stream->read<ShaderContent>(_contentType);
    stream->read<ShaderModel>(_shaderModel);
    stream->read<u16>(_optLevel);

    u32 readSize = stream->tell() - readPos + parentSize;
    ASSERT(sizeof(ShaderHeader) == readSize, "wrong size");
    return readSize;
}

Shader::Shader() noexcept
    : m_header(nullptr)
    , m_hash(0)
    , m_size(0)
    , m_source(nullptr)
    , m_entrypoint("")
{
    LOG_DEBUG("Shader::Shader constructor %llx", this);
}

Shader::Shader(ShaderHeader* header) noexcept
    : m_header(header)
    , m_hash(0)
    , m_size(0)
    , m_source(nullptr)
    , m_entrypoint("")
{
    LOG_DEBUG("Shader::Shader constructor %llx", this);
}

Shader::~Shader()
{
    LOG_DEBUG("Shader::~Shader destructor %llx", this);

    if (m_source)
    {
        V3D_FREE(m_source, memory::MemoryLabel::MemoryResource);
    }
    m_hash = 0;
    m_size = 0;

    if (m_header)
    {
        V3D_DELETE(m_header, memory::MemoryLabel::MemoryResource);
        m_header = nullptr;
    }
}

bool Shader::load(const stream::Stream* stream, u32 offset)
{
    if (m_loaded)
    {
        LOG_WARNING("Shader::load: the shader %llx is already loaded", this);
        return true;
    }

    ASSERT(stream, "nullptr");
    stream->seekBeg(offset);

    if (!m_header)
    {
        m_header = V3D_NEW(ShaderHeader, memory::MemoryLabel::MemoryResource);
        m_header->operator<<(stream);
    }
    stream->seekBeg(offset + m_header->_offset);

    stream->read<u32>(m_size);
    if (m_size > 0)
    {
        m_source = V3D_MALLOC(m_size, memory::MemoryLabel::MemoryResource);
        stream->read(m_source, m_size);
    }
    stream->read(m_entrypoint);
    ASSERT(!m_entrypoint.empty(), "must be filled");

    bool needParseReflect = false;
    stream->read<bool>(needParseReflect);
    if (needParseReflect)
    {
        auto sortByLocation = [](const Attribute& obj0, const Attribute& obj1) -> bool
        {
            return obj0._location < obj1._location;
        };

        auto sortByDescriptorSet = [](const auto& obj0, const auto& obj1) -> bool
        {
            if (obj0._set < obj1._set)
            {
                return true;
            }
            else if (obj0._set > obj1._set)
            {
                return false;
            }
            else
            {
                return obj0._binding < obj1._binding;
            }
        };

        u32 countInputAttachments;
        stream->read<u32>(countInputAttachments);
        m_reflectionInfo._inputAttribute.resize(countInputAttachments);
        for (auto& attribute : m_reflectionInfo._inputAttribute)
        {
            attribute << stream;
        }
        std::sort(m_reflectionInfo._inputAttribute.begin(), m_reflectionInfo._inputAttribute.end(), sortByLocation);

        u32 countOutputAttachments;
        stream->read<u32>(countOutputAttachments);
        m_reflectionInfo._outputAttribute.resize(countOutputAttachments);
        for (auto& attribute : m_reflectionInfo._outputAttribute)
        {
            attribute << stream;
        }
        std::sort(m_reflectionInfo._outputAttribute.begin(), m_reflectionInfo._outputAttribute.end(), sortByLocation);

        u32 countUniformBuffers;
        stream->read<u32>(countUniformBuffers);
        m_reflectionInfo._uniformBuffers.resize(countUniformBuffers);
        for (auto& buffer : m_reflectionInfo._uniformBuffers)
        {
            buffer << stream;
        }
        std::sort(m_reflectionInfo._uniformBuffers.begin(), m_reflectionInfo._uniformBuffers.end(), sortByDescriptorSet);

        u32 countSampledImages;
        stream->read<u32>(countSampledImages);
        m_reflectionInfo._sampledImages.resize(countSampledImages);
        for (auto& image : m_reflectionInfo._sampledImages)
        {
            image << stream;
        }
        std::sort(m_reflectionInfo._sampledImages.begin(), m_reflectionInfo._sampledImages.end(), sortByDescriptorSet);

        u32 countImages;
        stream->read<u32>(countImages);
        m_reflectionInfo._images.resize(countImages);
        for (auto& image : m_reflectionInfo._images)
        {
            image << stream;
        }
        std::sort(m_reflectionInfo._images.begin(), m_reflectionInfo._images.end(), sortByDescriptorSet);

        u32 countSamplers;
        stream->read<u32>(countSamplers);
        m_reflectionInfo._samplers.resize(countSamplers);
        for (auto& sampler : m_reflectionInfo._samplers)
        {
            sampler << stream;
        }
        std::sort(m_reflectionInfo._samplers.begin(), m_reflectionInfo._samplers.end(), sortByDescriptorSet);

        u32 countStorageImage;
        stream->read<u32>(countStorageImage);
        m_reflectionInfo._storageImages.resize(countStorageImage);
        for (auto& storageImage : m_reflectionInfo._storageImages)
        {
            storageImage << stream;
        }
        std::sort(m_reflectionInfo._storageImages.begin(), m_reflectionInfo._storageImages.end(), sortByDescriptorSet);

        u32 countStorageBuffers;
        stream->read<u32>(countStorageBuffers);
        m_reflectionInfo._storageBuffers.resize(countStorageBuffers);
        for (auto& storageBuffer : m_reflectionInfo._storageBuffers)
        {
            storageBuffer << stream;
        }
        std::sort(m_reflectionInfo._storageBuffers.begin(), m_reflectionInfo._storageBuffers.end(), sortByDescriptorSet);

        u32 countPushConstant;
        stream->read<u32>(countPushConstant);
        m_reflectionInfo._pushConstant.resize(countPushConstant);
        for (auto& pushConstant : m_reflectionInfo._pushConstant)
        {
            pushConstant << stream;
        }
    }

    m_hash = crc32c::Crc32c(reinterpret_cast<u8*>(m_source), m_size);
    LOG_DEBUG("Shader::load: The stream has been read %d from %d bytes", stream->tell() - m_header->_offset, m_header->_size);

    m_loaded = true;
    return true;
}

bool Shader::save(stream::Stream* stream, u32 offset) const
{
    ASSERT(false, "not impl");
    return false;
}


Shader::Attribute::Attribute() noexcept
    : _location(0)
    , _format(renderer::Format::Format_Undefined)
    , _name("")
{
}

u32 Shader::Attribute::operator>>(stream::Stream* stream) const
{
    u32 writePos = stream->tell();

    stream->write<u32>(_location);
    stream->write<renderer::Format>(_format);
    stream->write(_name);

    u32 writeSize = stream->tell() - writePos;
    return writeSize;
}

u32 Shader::Attribute::operator<<(const stream::Stream* stream)
{
    u32 readPos = stream->tell();

    stream->read<u32>(_location);
    stream->read<renderer::Format>(_format);
    stream->read(_name);

    u32 readSize = stream->tell() - readPos;
    return readSize;
}


Shader::UniformBuffer::UniformBuffer() noexcept
    : _id(0)
    , _set(0)
    , _binding(0)
    , _array(1)
    , _size(0)
    , _name("")
{
}

u32 Shader::UniformBuffer::operator>>(stream::Stream* stream) const
{
    u32 writePos = stream->tell();

    stream->write<u32>(_id);
    stream->write<u32>(_set);
    stream->write<u32>(_binding);
    stream->write<u32>(_array);
    stream->write<u32>(_size);
    stream->write(_name);

    stream->write<u32>(static_cast<u32>(_uniforms.size()));
    for (auto& uniform : _uniforms)
    {
        uniform >> stream;
    }

    u32 writeSize = stream->tell() - writePos;
    return writeSize;
}

u32 Shader::UniformBuffer::operator<<(const stream::Stream* stream)
{
    u32 readPos = stream->tell();

    stream->read<u32>(_id);
    stream->read<u32>(_set);
    stream->read<u32>(_binding);
    stream->read<u32>(_array);
    stream->read<u32>(_size);
    stream->read(_name);

    u32 count = 0;
    stream->read<u32>(count);
    _uniforms.resize(count);

    for (auto& uniform : _uniforms)
    {
        uniform << stream;
    }

    u32 readSize = stream->tell() - readPos;
    return readSize;
}

Shader::UniformBuffer::Uniform::Uniform() noexcept
    : _bufferID(0)
    , _array(1)
    , _type(renderer::DataType::DataType_None)
    , _size(0)
    , _offset(0)
    , _name("")
{
}

u32 Shader::UniformBuffer::Uniform::operator>>(stream::Stream* stream) const
{
    u32 writePos = stream->tell();

    stream->write<u32>(_bufferID);
    stream->write<u32>(_array);
    stream->write<renderer::DataType>(_type);
    stream->write<u32>(_size);
    stream->write<u32>(_offset);
    stream->write(_name);

    u32 writeSize = stream->tell() - writePos;
    return writeSize;
}

u32 Shader::UniformBuffer::Uniform::operator<<(const stream::Stream* stream)
{
    u32 readPos = stream->tell();

    stream->read<u32>(_bufferID);
    stream->read<u32>(_array);
    stream->read<renderer::DataType>(_type);
    stream->read<u32>(_size);
    stream->read<u32>(_offset);
    stream->read(_name);

    u32 readSize = stream->tell() - readPos;
    return readSize;
}


Shader::Image::Image() noexcept
    : _set(0)
    , _binding(0)
    , _target(renderer::TextureTarget::Texture2D)
    , _array(1)
    , _depth(false)
    , _ms(false)
    , _name("")
{
}

u32 Shader::Image::operator>>(stream::Stream* stream) const
{
    u32 writePos = stream->tell();

    stream->write<u32>(_set);
    stream->write<u32>(_binding);
    stream->write<renderer::TextureTarget>(_target);
    stream->write<u32>(_array);
    stream->write<bool>(_depth);
    stream->write<bool>(_ms);
    stream->write(_name);

    u32 writeSize = stream->tell() - writePos;
    return writeSize;
}

u32 Shader::Image::operator<<(const stream::Stream* stream)
{
    u32 readPos = stream->tell();

    stream->read<u32>(_set);
    stream->read<u32>(_binding);
    stream->read<renderer::TextureTarget>(_target);
    stream->read<u32>(_array);
    stream->read<bool>(_depth);
    stream->read<bool>(_ms);
    stream->read(_name);

    u32 readSize = stream->tell() - readPos;
    return readSize;
}


Shader::Sampler::Sampler() noexcept
    : _set(0)
    , _binding(0)
    , _name("")
{
}

u32 Shader::Sampler::operator>>(stream::Stream* stream) const
{
    u32 writePos = stream->tell();

    stream->write<u32>(_set);
    stream->write<u32>(_binding);
    stream->write(_name);

    u32 writeSize = stream->tell() - writePos;
    return writeSize;
}

u32 Shader::Sampler::operator<<(const stream::Stream* stream)
{
    u32 readPos = stream->tell();

    stream->read<u32>(_set);
    stream->read<u32>(_binding);
    stream->read(_name);

    u32 readSize = stream->tell() - readPos;
    return readSize;
}

Shader::StorageImage::StorageImage() noexcept
    : _set(0)
    , _binding(0)
    , _target(renderer::TextureTarget::Texture2D)
    , _format(renderer::Format::Format_Undefined)
    , _array(1)
    , _readonly(true)
    , _name("")
{
}

u32 Shader::StorageImage::operator>>(stream::Stream* stream) const
{
    u32 writePos = stream->tell();

    stream->write<u32>(_set);
    stream->write<u32>(_binding);
    stream->write<renderer::TextureTarget>(_target);
    stream->write<renderer::Format>(_format);
    stream->write<u32>(_array);
    stream->write<bool>(_readonly);
    stream->write(_name);

    u32 writeSize = stream->tell() - writePos;
    return writeSize;
}

u32 Shader::StorageImage::operator<<(const stream::Stream* stream)
{
    u32 readPos = stream->tell();

    stream->read<u32>(_set);
    stream->read<u32>(_binding);
    stream->read<renderer::TextureTarget>(_target);
    stream->read<renderer::Format>(_format);
    stream->read<u32>(_array);
    stream->read<bool>(_readonly);
    stream->read(_name);

    u32 readSize = stream->tell() - readPos;
    return readSize;
}

Shader::StorageBuffer::StorageBuffer() noexcept
    : _set(0)
    , _binding(0)
    , _format(renderer::Format::Format_Undefined)
    , _array(1)
    , _readonly(true)
    , _name("")
{
}

u32 Shader::StorageBuffer::operator>>(stream::Stream* stream) const
{
    u32 writePos = stream->tell();

    stream->write<u32>(_set);
    stream->write<u32>(_binding);
    stream->write<renderer::Format>(_format);
    stream->write<u32>(_array);
    stream->write<bool>(_readonly);
    stream->write(_name);

    u32 writeSize = stream->tell() - writePos;
    return writeSize;
}

u32 Shader::StorageBuffer::operator<<(const stream::Stream* stream)
{
    u32 readPos = stream->tell();

    stream->read<u32>(_set);
    stream->read<u32>(_binding);
    stream->read<renderer::Format>(_format);
    stream->read<u32>(_array);
    stream->read<bool>(_readonly);
    stream->read(_name);

    u32 readSize = stream->tell() - readPos;
    return readSize;
}

Shader::PushConstant::PushConstant() noexcept
    : _offset(0)
    , _size(0)
    , _name("")
{
}

u32 Shader::PushConstant::operator>>(stream::Stream* stream) const
{
    u32 writePos = stream->tell();

    stream->write<u32>(_offset);
    stream->write<u32>(_size);
    stream->write(_name);

    u32 writeSize = stream->tell() - writePos;
    return writeSize;
}

u32 Shader::PushConstant::operator<<(const stream::Stream* stream)
{
    u32 readPos = stream->tell();

    stream->read<u32>(_offset);
    stream->read<u32>(_size);
    stream->read(_name);

    u32 readSize = stream->tell() - readPos;
    return readSize;
}

} //namespace renderer
} //namespace v3d
