#include "Shader.h"

namespace v3d
{
namespace resource
{

Shader::Shader()
{
}

Shader::~Shader()
{
}

void Shader::init(const stream::Stream * stream, const ResourceHeader * header)
{
    m_stream = stream;
    m_header = header;
}

bool Shader::load()
{
    if (m_loaded)
    {
        return true;
    }

    //TODO:

    m_loaded = true;
    return false;
}

} //namespace resource
} //namespace v3d
