#include "Shader.h"

namespace v3d
{
namespace resource
{

Shader::Shader(const ShaderHeader* header) noexcept
    : Resource(header)
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
}

void Shader::init(const stream::Stream * stream)
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
        u32 countInputAttachments;
        m_stream->read<u32>(countInputAttachments);
        m_reflectionInfo._inputAttribute.resize(countInputAttachments);
        for (auto& attribute : m_reflectionInfo._inputAttribute)
        {
            attribute << m_stream;
        }

        u32 countOutputAttachments;
        m_stream->read<u32>(countOutputAttachments);
        m_reflectionInfo._outputAttribute.resize(countOutputAttachments);
        for (auto& attribute : m_reflectionInfo._outputAttribute)
        {
            attribute << m_stream;
        }

        //TODO:
    }

    m_loaded = true;
    return true;
}

const ShaderHeader* Shader::getShaderHeader() const
{
    return static_cast<const ShaderHeader*>(m_header);
}

Shader::Attribute::Attribute()
    : _location(0)
    //, _binding(0)
    //, _offset(0)
    , _format(renderer::Format::Format_Undefined)
    , _name("")
{
}

void Shader::Attribute::operator>>(stream::Stream * stream)
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


ShaderHeader::ShaderHeader() noexcept
    : _type(ShaderType::ShaderType_Undefined)
    , _contentType(ShaderResource::ShaderResource_Source)
    , _shaderLang(ShaderLang::ShaderLang_GLSL)
    , _apiVersion(0)
    , _optLevel(0)
    , _entyPoint("main")
{
}

} //namespace resource
} //namespace v3d
