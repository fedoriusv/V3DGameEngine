#include "ShaderProgram.h"
#include "Renderer/Formats.h"

namespace v3d
{
namespace renderer
{

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

const ShaderProgram::ShaderProgramInfo& ShaderProgram::getShaderMetaInfo() const
{
    return m_programInfo;
}

ShaderProgram::ShaderProgram(renderer::CommandList & cmdList, std::vector<resource::Shader*> shaders) noexcept
    : m_cmdList(cmdList)
    , m_shaders(shaders)
{
    if (getShader(resource::ShaderType::ShaderType_Vertex) &&
        getShader(resource::ShaderType::ShaderType_Fragment))
    {
        composeProgramData();
    }
    else
    {
        ASSERT(false, "unknown program");
    }
}

void ShaderProgram::composeProgramData()
{
    for (auto shader : m_shaders)
    {
        if (shader->getShaderHeader()->_type == resource::ShaderType::ShaderType_Vertex)
        {
            u32 offest = 0;
            for (auto& attr : shader->m_reflectionInfo._inputAttribute)
            {
                ASSERT(!attr._name.empty(), "empty name");
                m_programInfo._inputAttachment[attr._name] = { attr._location, offest, attr._format };
                u32 dataSize = renderer::getFormatSize(attr._format);
                offest += dataSize;
            }
        }

        if (shader->getShaderHeader()->_type == resource::ShaderType::ShaderType_Fragment)
        {
            u32 offest = 0;
            for (auto& attr : shader->m_reflectionInfo._outputAttribute)
            {
                ASSERT(!attr._name.empty(), "empty name");
                m_programInfo._outputAttachment[attr._name] = { attr._location, offest, attr._format };
                u32 dataSize = renderer::getFormatSize(attr._format);
                offest += dataSize;
            }
        }
    }
}

ShaderProgram::~ShaderProgram()
{
}

} //renderer
} //v3d
