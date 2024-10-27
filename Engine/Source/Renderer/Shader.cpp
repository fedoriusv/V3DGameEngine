#include "Shader.h"
#include "Stream/Stream.h"
#include "Utils/Logger.h"

namespace v3d
{
namespace renderer
{

std::string ShaderTypeString(ShaderType type)
{
    switch (type)
    {
    case ShaderType::Vertex:
        return "vertex";

    case ShaderType::Fragment:
        return "fragment";

    case ShaderType::Compute:
        return "compute";

    default:
        ASSERT(false, "not found");
    }

    return "unknown";
}

Shader::Shader(ShaderType type) noexcept
    : m_header()
    , m_type(type)
    , m_shaderModel(ShaderModel::Default)
    , m_data({})
{
    LOG_DEBUG("Shader::Shader constructor %llx", this);
}

Shader::Shader(const ShaderHeader& header) noexcept
    : m_header(header)
    , m_type(ShaderType::First)
    , m_shaderModel(ShaderModel::Default)
    , m_data({})
{
    LOG_DEBUG("Shader::Shader constructor %llx", this);
}

Shader::~Shader()
{
    LOG_DEBUG("Shader::~Shader destructor %llx", this);

    if (m_data._data)
    {
        V3D_FREE(m_data._data, memory::MemoryLabel::MemoryObject);
        m_data._data = nullptr;
    }
    m_data._size = 0;
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

    stream->read<ShaderType>(m_type);
    stream->read<ShaderModel>(m_shaderModel);
    stream->read(m_entryPoint);
    ASSERT(!m_entryPoint.empty(), "must be filled");

    u32 size = 0;
    void* data = nullptr;
    stream->read<u32>(size);
    if (size > 0)
    {
        data = V3D_MALLOC(size, memory::MemoryLabel::MemoryObject);
        stream->read(data, size);
    }
    m_data = { size, data };

    bool parseReflect = false;
    stream->read<bool>(parseReflect);
    if (parseReflect)
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
        m_mappedResources._inputAttribute.resize(countInputAttachments);
        for (auto& attribute : m_mappedResources._inputAttribute)
        {
            attribute << stream;
        }
        std::sort(m_mappedResources._inputAttribute.begin(), m_mappedResources._inputAttribute.end(), sortByLocation);

        u32 countOutputAttachments;
        stream->read<u32>(countOutputAttachments);
        m_mappedResources._outputAttribute.resize(countOutputAttachments);
        for (auto& attribute : m_mappedResources._outputAttribute)
        {
            attribute << stream;
        }
        std::sort(m_mappedResources._outputAttribute.begin(), m_mappedResources._outputAttribute.end(), sortByLocation);

        u32 countUniformBuffers;
        stream->read<u32>(countUniformBuffers);
        m_mappedResources._uniformBuffers.resize(countUniformBuffers);
        for (auto& buffer : m_mappedResources._uniformBuffers)
        {
            buffer << stream;
        }
        std::sort(m_mappedResources._uniformBuffers.begin(), m_mappedResources._uniformBuffers.end(), sortByDescriptorSet);

        u32 countSampledImages;
        stream->read<u32>(countSampledImages);
        m_mappedResources._sampledImages.resize(countSampledImages);
        for (auto& image : m_mappedResources._sampledImages)
        {
            image << stream;
        }
        std::sort(m_mappedResources._sampledImages.begin(), m_mappedResources._sampledImages.end(), sortByDescriptorSet);

        u32 countImages;
        stream->read<u32>(countImages);
        m_mappedResources._images.resize(countImages);
        for (auto& image : m_mappedResources._images)
        {
            image << stream;
        }
        std::sort(m_mappedResources._images.begin(), m_mappedResources._images.end(), sortByDescriptorSet);

        u32 countSamplers;
        stream->read<u32>(countSamplers);
        m_mappedResources._samplers.resize(countSamplers);
        for (auto& sampler : m_mappedResources._samplers)
        {
            sampler << stream;
        }
        std::sort(m_mappedResources._samplers.begin(), m_mappedResources._samplers.end(), sortByDescriptorSet);

        u32 countStorageImage;
        stream->read<u32>(countStorageImage);
        m_mappedResources._storageImages.resize(countStorageImage);
        for (auto& storageImage : m_mappedResources._storageImages)
        {
            storageImage << stream;
        }
        std::sort(m_mappedResources._storageImages.begin(), m_mappedResources._storageImages.end(), sortByDescriptorSet);

        u32 countStorageBuffers;
        stream->read<u32>(countStorageBuffers);
        m_mappedResources._storageBuffers.resize(countStorageBuffers);
        for (auto& storageBuffer : m_mappedResources._storageBuffers)
        {
            storageBuffer << stream;
        }
        std::sort(m_mappedResources._storageBuffers.begin(), m_mappedResources._storageBuffers.end(), sortByDescriptorSet);

        u32 countPushConstant;
        stream->read<u32>(countPushConstant);
        m_mappedResources._pushConstant.resize(countPushConstant);
        for (auto& pushConstant : m_mappedResources._pushConstant)
        {
            pushConstant << stream;
        }
    }

    //m_hash = crc32c::Crc32c(reinterpret_cast<u8*>(m_source), m_size);
    LOG_DEBUG("Shader::load: The stream has been read %d from %d bytes", stream->tell() - m_header._offset, m_header._size);

    m_loaded = true;
    return true;
}

bool Shader::save(stream::Stream* stream, u32 offset) const
{
    if (!m_loaded)
    {
        LOG_WARNING("Shader::save: the shader %llx ist loaded", this);
        return false;
    }

    stream->write<renderer::ShaderType>(m_type);
    stream->write<renderer::ShaderModel>(m_shaderModel);
    stream->write(m_entryPoint);
    stream->write<u32>(m_data._size);
    stream->write(m_data._data, m_data._size);

    //stream->write<bool>(reflections);
    //TODO

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
    , _type(renderer::DataType::None)
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

VertexShader::VertexShader() noexcept
    : Shader(ShaderType::Vertex)
{
}

FragmentShader::FragmentShader() noexcept
    : Shader(ShaderType::Fragment)
{
}

ComputeShader::ComputeShader() noexcept
    : Shader(ShaderType::Compute)
{
}

} //namespace renderer
} //namespace v3d
