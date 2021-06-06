#include "Shader.h"
#include "Utils/Logger.h"

#include "crc32c/crc32c.h"

namespace v3d
{
namespace renderer
{

ShaderHeader::ShaderHeader() noexcept
    : _type(renderer::ShaderType::Undefined)
    , _contentType(ShaderResource::Source)
    , _shaderModel(ShaderModel::Default)
    , _optLevel(0)
    , _entryPoint("main")
{
}

ShaderHeader::ShaderHeader(renderer::ShaderType type) noexcept
    : _type(type)
    , _contentType(ShaderResource::Source)
    , _shaderModel(ShaderModel::Default)
    , _optLevel(0)
    , _entryPoint("main")
{
}

Shader::Shader(const ShaderHeader* header) noexcept
    : Resource(header)
    , m_hash(0)
    , m_size(0)
    , m_source(nullptr)
{
    LOG_DEBUG("Shader::Shader constructor");
}

Shader::~Shader()
{
    LOG_DEBUG("Shader::~Shader destructor");

    if (m_source)
    {
        free(m_source);
        m_source = nullptr;
    }
    m_hash = 0;
    m_size = 0;
}

void Shader::init(stream::Stream* stream)
{
    ASSERT(stream, "nullptr");
    m_stream = stream;
}

bool Shader::load()
{
    if (m_loaded)
    {
        return true;
    }

    ASSERT(m_stream, "nullptr");
    
    m_stream->seekBeg(0);
    m_stream->read<u32>(m_size);
    m_source = malloc(m_size); //TODO memory manager
    m_stream->read(m_source, m_size);

    bool needParseReflect = false;
    m_stream->read<bool>(needParseReflect);
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
        m_stream->read<u32>(countInputAttachments);
        m_reflectionInfo._inputAttribute.resize(countInputAttachments);
        for (auto& attribute : m_reflectionInfo._inputAttribute)
        {
            attribute << m_stream;
        }
        std::sort(m_reflectionInfo._inputAttribute.begin(), m_reflectionInfo._inputAttribute.end(), sortByLocation);

        u32 countOutputAttachments;
        m_stream->read<u32>(countOutputAttachments);
        m_reflectionInfo._outputAttribute.resize(countOutputAttachments);
        for (auto& attribute : m_reflectionInfo._outputAttribute)
        {
            attribute << m_stream;
        }
        std::sort(m_reflectionInfo._outputAttribute.begin(), m_reflectionInfo._outputAttribute.end(), sortByLocation);

        u32 countUniformBuffers;
        m_stream->read<u32>(countUniformBuffers);
        m_reflectionInfo._uniformBuffers.resize(countUniformBuffers);
        for (auto& buffer : m_reflectionInfo._uniformBuffers)
        {
            buffer << m_stream;
        }
        std::sort(m_reflectionInfo._uniformBuffers.begin(), m_reflectionInfo._uniformBuffers.end(), sortByDescriptorSet);

        u32 countSampledImages;
        m_stream->read<u32>(countSampledImages);
        m_reflectionInfo._sampledImages.resize(countSampledImages);
        for (auto& image : m_reflectionInfo._sampledImages)
        {
            image << m_stream;
        }
        std::sort(m_reflectionInfo._sampledImages.begin(), m_reflectionInfo._sampledImages.end(), sortByDescriptorSet);

        u32 countImages;
        m_stream->read<u32>(countImages);
        m_reflectionInfo._images.resize(countImages);
        for (auto& image : m_reflectionInfo._images)
        {
            image << m_stream;
        }
        std::sort(m_reflectionInfo._images.begin(), m_reflectionInfo._images.end(), sortByDescriptorSet);

        u32 countSamplers;
        m_stream->read<u32>(countSamplers);
        m_reflectionInfo._samplers.resize(countSamplers);
        for (auto& sampler : m_reflectionInfo._samplers)
        {
            sampler << m_stream;
        }
        std::sort(m_reflectionInfo._samplers.begin(), m_reflectionInfo._samplers.end(), sortByDescriptorSet);

        u32 countStorageImage;
        m_stream->read<u32>(countStorageImage);
        m_reflectionInfo._storageImages.resize(countStorageImage);
        for (auto& storageImage : m_reflectionInfo._storageImages)
        {
            storageImage << m_stream;
        }
        std::sort(m_reflectionInfo._storageImages.begin(), m_reflectionInfo._storageImages.end(), sortByDescriptorSet);

        u32 countStorageBuffers;
        m_stream->read<u32>(countStorageBuffers);
        m_reflectionInfo._storageBuffers.resize(countStorageBuffers);
        for (auto& storageBuffer : m_reflectionInfo._storageBuffers)
        {
            storageBuffer << m_stream;
        }
        std::sort(m_reflectionInfo._storageBuffers.begin(), m_reflectionInfo._storageBuffers.end(), sortByDescriptorSet);

        u32 countPushConstant;
        m_stream->read<u32>(countPushConstant);
        m_reflectionInfo._pushConstant.resize(countPushConstant);
        for (auto& pushConstant : m_reflectionInfo._pushConstant)
        {
            pushConstant << m_stream;
        }
    }
    m_stream->close();

    m_hash = crc32c::Crc32c(reinterpret_cast<u8*>(m_source), m_size);

    m_loaded = true;
    return true;
}

const ShaderHeader& Shader::getShaderHeader() const
{
    return *static_cast<const ShaderHeader*>(m_header);
}

const Shader::ReflectionInfo& Shader::getReflectionInfo() const
{
    return m_reflectionInfo;
}

Shader::Attribute::Attribute() noexcept
    : _location(0)
    , _format(renderer::Format::Format_Undefined)
#if USE_STRING_ID_SHADER
    , _name("")
#endif
{
}

void Shader::Attribute::operator>>(stream::Stream* stream) const
{
    stream->write<u32>(_location);
    stream->write<renderer::Format>(_format);
#if USE_STRING_ID_SHADER
    stream->write(_name);
#endif
}

void Shader::Attribute::operator<<(const stream::Stream* stream)
{
    stream->read<u32>(_location);
    stream->read<renderer::Format>(_format);
#if USE_STRING_ID_SHADER
    stream->read(_name);
#endif
}


Shader::UniformBuffer::UniformBuffer() noexcept
    : _id(0)
    , _set(0)
    , _binding(0)
    , _array(1)
    , _size(0)
#if USE_STRING_ID_SHADER
    , _name("")
#endif
{
}

void Shader::UniformBuffer::operator>>(stream::Stream* stream) const
{
    stream->write<u32>(_id);
    stream->write<u32>(_set);
    stream->write<u32>(_binding);
    stream->write<u32>(_array);
    stream->write<u32>(_size);
#if USE_STRING_ID_SHADER
    stream->write(_name);
#endif
    stream->write<u32>(static_cast<u32>(_uniforms.size()));
    for (auto& uniform : _uniforms)
    {
        uniform >> stream;
    }
}

void Shader::UniformBuffer::operator<<(const stream::Stream* stream)
{
    stream->read<u32>(_id);
    stream->read<u32>(_set);
    stream->read<u32>(_binding);
    stream->read<u32>(_array);
    stream->read<u32>(_size);
#if USE_STRING_ID_SHADER
    stream->read(_name);
#endif
    u32 size;
    stream->read<u32>(size);
    _uniforms.resize(size);

    for (auto& uniform : _uniforms)
    {
        uniform << stream;
    }
}

Shader::UniformBuffer::Uniform::Uniform() noexcept
    : _bufferId(0)
    , _array(1)
    , _type(renderer::DataType::DataType_None)
    , _size(0)
    , _offset(0)
#if USE_STRING_ID_SHADER
    , _name("")
#endif
{
}

void Shader::UniformBuffer::Uniform::operator>>(stream::Stream* stream) const
{
    stream->write<u32>(_bufferId);
    stream->write<u32>(_array);
    stream->write<renderer::DataType>(_type);
    stream->write<u32>(_size);
    stream->write<u32>(_offset);
#if USE_STRING_ID_SHADER
    stream->write(_name);
#endif
}

void Shader::UniformBuffer::Uniform::operator<<(const stream::Stream* stream)
{
    stream->read<u32>(_bufferId);
    stream->read<u32>(_array);
    stream->read<renderer::DataType>(_type);
    stream->read<u32>(_size);
    stream->read<u32>(_offset);
#if USE_STRING_ID_SHADER
    stream->read(_name);
#endif
}


Shader::Image::Image() noexcept
    : _set(0)
    , _binding(0)
    , _target(renderer::TextureTarget::Texture2D)
    , _array(1)
    , _depth(false)
    , _ms(false)
#if USE_STRING_ID_SHADER
    , _name("")
#endif
{
}

void Shader::Image::operator>>(stream::Stream* stream) const
{
    stream->write<u32>(_set);
    stream->write<u32>(_binding);
    stream->write<renderer::TextureTarget>(_target);
    stream->write<u32>(_array);
    stream->write<bool>(_depth);
    stream->write<bool>(_ms);
#if USE_STRING_ID_SHADER
    stream->write(_name);
#endif
}

void Shader::Image::operator<<(const stream::Stream* stream)
{
    stream->read<u32>(_set);
    stream->read<u32>(_binding);
    stream->read<renderer::TextureTarget>(_target);
    stream->read<u32>(_array);
    stream->read<bool>(_depth);
    stream->read<bool>(_ms);
#if USE_STRING_ID_SHADER
    stream->read(_name);
#endif
}


Shader::Sampler::Sampler() noexcept
    : _set(0)
    , _binding(0)
#if USE_STRING_ID_SHADER
    , _name("")
#endif
{
}

void Shader::Sampler::operator>>(stream::Stream* stream) const
{
    stream->write<u32>(_set);
    stream->write<u32>(_binding);
#if USE_STRING_ID_SHADER
    stream->write(_name);
#endif
}

void Shader::Sampler::operator<<(const stream::Stream* stream)
{
    stream->read<u32>(_set);
    stream->read<u32>(_binding);
#if USE_STRING_ID_SHADER
    stream->read(_name);
#endif
}

Shader::StorageImage::StorageImage() noexcept
    : _set(0)
    , _binding(0)
    , _target(renderer::TextureTarget::Texture2D)
    , _format(renderer::Format::Format_Undefined)
    , _array(1)
    , _readonly(true)
#if USE_STRING_ID_SHADER
    , _name("")
#endif
{
}

void Shader::StorageImage::operator>>(stream::Stream* stream) const
{
    stream->write<u32>(_set);
    stream->write<u32>(_binding);
    stream->write<renderer::TextureTarget>(_target);
    stream->write<renderer::Format>(_format);
    stream->write<u32>(_array);
    stream->write<bool>(_readonly);
#if USE_STRING_ID_SHADER
    stream->write(_name);
#endif
}

void Shader::StorageImage::operator<<(const stream::Stream* stream)
{
    stream->read<u32>(_set);
    stream->read<u32>(_binding);
    stream->read<renderer::TextureTarget>(_target);
    stream->read<renderer::Format>(_format);
    stream->read<u32>(_array);
    stream->read<bool>(_readonly);
#if USE_STRING_ID_SHADER
    stream->read(_name);
#endif
}

Shader::StorageBuffer::StorageBuffer() noexcept
    : _set(0)
    , _binding(0)
#if USE_STRING_ID_SHADER
    , _name("")
#endif
{
}

void Shader::StorageBuffer::operator>>(stream::Stream* stream) const
{
    stream->write<u32>(_set);
    stream->write<u32>(_binding);
#if USE_STRING_ID_SHADER
    stream->write(_name);
#endif
}

void Shader::StorageBuffer::operator<<(const stream::Stream* stream)
{
    stream->read<u32>(_set);
    stream->read<u32>(_binding);
#if USE_STRING_ID_SHADER
    stream->read(_name);
#endif
}

Shader::PushConstant::PushConstant() noexcept
    : _offset(0)
    , _size(0)
#if USE_STRING_ID_SHADER
    , _name("")
#endif
{
}

void Shader::PushConstant::operator>>(stream::Stream* stream) const
{
    stream->write<u32>(_offset);
    stream->write<u32>(_size);
#if USE_STRING_ID_SHADER
    stream->write(_name);
#endif
}

void Shader::PushConstant::operator<<(const stream::Stream* stream)
{
    stream->read<u32>(_offset);
    stream->read<u32>(_size);
#if USE_STRING_ID_SHADER
    stream->read(_name);
#endif
}

} //namespace renderer
} //namespace v3d
