#include "Shader.h"

#include "crc32c/crc32c.h"

namespace v3d
{
namespace renderer
{

ShaderHeader::ShaderHeader() noexcept
    : _type(renderer::ShaderType::ShaderType_Undefined)
    , _contentType(ShaderResource::ShaderResource_Source)
    , _shaderLang(ShaderLang::ShaderLang_GLSL)
    , _apiVersion(0)
    , _optLevel(0)
    , _entyPoint("main")
{
}

Shader::Shader(const ShaderHeader* header) noexcept
    : Resource(header)
    , m_hash(0)
    , m_size(0)
    , m_source(nullptr)
{
}

Shader::~Shader()
{
    if (m_source)
    {
        free(m_source);
    }
    m_size = 0;

    if (m_stream)
    {
        delete m_stream;
    }
}

void Shader::init(stream::Stream * stream)
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
        auto sortAttributtes = [](const Attribute& obj0, const Attribute& obj1) -> bool
        {
            return obj0._location < obj1._location;
        };

        u32 countInputAttachments;
        m_stream->read<u32>(countInputAttachments);
        m_reflectionInfo._inputAttribute.resize(countInputAttachments);
        for (auto& attribute : m_reflectionInfo._inputAttribute)
        {
            attribute << m_stream;
        }
        std::sort(m_reflectionInfo._inputAttribute.begin(), m_reflectionInfo._inputAttribute.end(), sortAttributtes);

        u32 countOutputAttachments;
        m_stream->read<u32>(countOutputAttachments);
        m_reflectionInfo._outputAttribute.resize(countOutputAttachments);
        for (auto& attribute : m_reflectionInfo._outputAttribute)
        {
            attribute << m_stream;
        }
        std::sort(m_reflectionInfo._outputAttribute.begin(), m_reflectionInfo._outputAttribute.end(), sortAttributtes);

        u32 countUniformBuffers;
        m_stream->read<u32>(countUniformBuffers);
        m_reflectionInfo._uniformBuffers.resize(countUniformBuffers);
        for (auto& buffer : m_reflectionInfo._uniformBuffers)
        {
            buffer << m_stream;
        }

        u32 countImages;
        m_stream->read<u32>(countImages);
        m_reflectionInfo._sampledImages.resize(countImages);
        for (auto& image : m_reflectionInfo._sampledImages)
        {
            image << m_stream;
        }

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

Shader::Attribute::Attribute()
    : _location(0)
    , _format(renderer::Format::Format_Undefined)
    , _name("")
{
}

void Shader::Attribute::operator>>(stream::Stream * stream) const
{
    stream->write<u32>(_location);
    stream->write<renderer::Format>(_format);
    stream->write(_name);
}

void Shader::Attribute::operator<<(const stream::Stream * stream)
{
    stream->read<u32>(_location);
    stream->read<renderer::Format>(_format);
    stream->read(_name);
}


Shader::UniformBuffer::UniformBuffer()
    : _id(0)
    , _set(0)
    , _binding(0)
    , _array(1)
    , _size(0)
    , _name("")
{
}

void Shader::UniformBuffer::operator>>(stream::Stream * stream) const
{
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
}

void Shader::UniformBuffer::operator<<(const stream::Stream * stream)
{
    stream->read<u32>(_id);
    stream->read<u32>(_set);
    stream->read<u32>(_binding);
    stream->read<u32>(_array);
    stream->read<u32>(_size);
    stream->read(_name);

    u32 size;
    stream->read<u32>(size);
    _uniforms.resize(size);

    for (auto& uniform : _uniforms)
    {
        uniform << stream;
    }
}

Shader::UniformBuffer::Uniform::Uniform()
    : _bufferId(0)
    , _array(1)
    , _type(renderer::DataType::DataType_None)
    , _name("")
{
}

void Shader::UniformBuffer::Uniform::operator>>(stream::Stream * stream) const
{
    stream->write<u32>(_bufferId);
    stream->write<u32>(_array);
    stream->write<renderer::DataType>(_type);
    stream->write(_name);
}

void Shader::UniformBuffer::Uniform::operator<<(const stream::Stream * stream)
{
    stream->read<u32>(_bufferId);
    stream->read<u32>(_array);
    stream->read<renderer::DataType>(_type);
    stream->read(_name);
}


Shader::SampledImage::SampledImage()
    : _set(0)
    , _binding(0)
    , _target(renderer::TextureTarget::Texture2D)
    , _array(1)
    , _depth(false)
    , _ms(false)
    , _name("")
{
}

void Shader::SampledImage::operator>>(stream::Stream * stream) const
{
    stream->write<u32>(_set);
    stream->write<u32>(_binding);
    stream->write<renderer::TextureTarget>(_target);
    stream->write<u32>(_array);
    stream->write<bool>(_depth);
    stream->write<bool>(_ms);
    stream->write(_name);
}

void Shader::SampledImage::operator<<(const stream::Stream * stream)
{
    stream->read<u32>(_set);
    stream->read<u32>(_binding);
    stream->read<renderer::TextureTarget>(_target);
    stream->read<u32>(_array);
    stream->read<bool>(_depth);
    stream->read<bool>(_ms);
    stream->read(_name);
}

Shader::PushConstant::PushConstant()
    : _offset(0)
    , _size(0)
    , _name("")
{
}

void Shader::PushConstant::operator>>(stream::Stream * stream) const
{
    stream->write<u32>(_offset);
    stream->write<u32>(_size);
    stream->write(_name);
}

void Shader::PushConstant::operator<<(const stream::Stream * stream)
{
    stream->read<u32>(_offset);
    stream->read<u32>(_size);
    stream->read(_name);
}

} //namespace renderer
} //namespace v3d