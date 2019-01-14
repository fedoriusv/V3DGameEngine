#include "ShaderProgram.h"

namespace v3d
{
namespace renderer
{

ShaderProgram::~ShaderProgram()
{
}

const resource::Shader * ShaderProgram::getShader(resource::ShaderType type) const
{
    auto it = std::find_if(m_shaders.cbegin(), m_shaders.cend(), [type](const resource::Shader* shader) -> bool
    {
        return shader->getShaderHeader()->_type == type;
    });

    if (it != m_shaders.cend())
    {
        return *it;
    }

    return nullptr;
}

ShaderProgram::ShaderProgram(renderer::CommandList & cmdList, std::vector<resource::Shader*> shaders) noexcept
    : m_cmdList(cmdList)
    , m_shaders(shaders)
{
}

} //renderer
} //v3d
