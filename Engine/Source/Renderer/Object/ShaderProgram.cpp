#include "ShaderProgram.h"
#include "Texture.h"
#include "Renderer/Formats.h"
#include "Renderer/Context.h"

#include "Utils/Logger.h"

namespace v3d
{
namespace renderer
{

const resource::Shader * ShaderProgram::getShader(resource::ShaderType type) const
{
    auto it = std::find_if(m_shaders.cbegin(), m_shaders.cend(), [type](const resource::Shader* shader) -> bool
    {
        return shader->getShaderHeader()._type == type;
    });

    if (it != m_shaders.cend())
    {
        return *it;
    }

    return nullptr;
}

const ShaderProgramDescription& ShaderProgram::getShaderDesc() const
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
        if (shader->getShaderHeader()._type == resource::ShaderType::ShaderType_Vertex)
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

        if (shader->getShaderHeader()._type == resource::ShaderType::ShaderType_Fragment)
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

bool ShaderProgram::setTexture(std::string& name, TextureTarget target, const Texture* texture)
{
    Image* image = nullptr;
    switch (target)
    {
    case TextureTarget::Texture2D:
        image = static_cast<const Texture2D*>(texture)->m_image;

    default:
        break;
    }
    if (!image)
    {
        ASSERT(false, "image nullptr");
        return false;
    }

    auto iter = m_programInfo._textures.find(name);
    if (iter == m_programInfo._textures.cend())
    {
        LOG_WARNING("ShaderProgram::setTexture: binding for texture [%s] not found ", name.c_str());
        return false;
    }

    if (m_cmdList.isImmediate())
    {
        m_cmdList.getContext()->bindTexture(image, (*iter).second);
    }
    else
    {
        //TODO
    }

    return false;
}

ShaderProgram::~ShaderProgram()
{
}

} //renderer
} //v3d
