#include "ShaderProgram.h"
#include "Texture.h"
#include "Renderer/Formats.h"
#include "Renderer/Context.h"
#include "Resource/Shader.h"

#include "Utils/Logger.h"
#include "crc32c/crc32c.h"

namespace v3d
{
namespace renderer
{

const resource::Shader * ShaderProgram::getShader(ShaderType type) const
{
    return m_programInfo._shaders[type];
}

const ShaderProgramDescription& ShaderProgram::getShaderDesc() const
{
    return m_programInfo;
}

ShaderProgram::ShaderProgram(renderer::CommandList & cmdList, std::vector<resource::Shader*> shaders) noexcept
    : m_cmdList(cmdList)
{
    for (auto shader : shaders)
    {
        m_programInfo._shaders[shader->getShaderHeader()._type] = shader;
    }

    if (getShader(ShaderType::ShaderType_Vertex) && getShader(ShaderType::ShaderType_Fragment))
    {
        composeProgramData(shaders);
    }
    else
    {
        ASSERT(false, "unknown program");
    }
}

void ShaderProgram::composeProgramData(const std::vector<resource::Shader*>& shaders)
{
    m_programInfo._hash = 0;
    for (auto shader : shaders)
    {
        m_programInfo._hash = crc32c::Extend(m_programInfo._hash, reinterpret_cast<u8*>(&shader->m_hash), sizeof(u32));

       /* if (shader->getShaderHeader()._type == resource::ShaderType::ShaderType_Vertex)
        {
            u32 offest = 0;
            for (auto& attr : shader->m_reflectionInfo._inputAttribute)
            {
                ASSERT(!attr._name.empty(), "empty name");
                m_programInfo._inputAttachment[attr._name] = { attr._location, offest, attr._format };
                u32 dataSize = renderer::getFormatSize(attr._format);
                offest += dataSize;
            }

            for (auto& buffer : shader->m_reflectionInfo._uniformBuffers)
            {
                ASSERT(!buffer._name.empty(), "empty name");
                m_programInfo._uniformsBuffer[buffer._name] = { buffer._set, buffer._binding, buffer._size };
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

            for (auto& buffer : shader->m_reflectionInfo._uniformBuffers)
            {
                ASSERT(!buffer._name.empty(), "empty name");
                m_programInfo._uniformsBuffer[buffer._name] = { buffer._set, buffer._binding, buffer._size };
            }
        }*/

//        m_programInfo._shaders.push_back(shader);
    }
}

bool ShaderProgram::bindUniformsBuffer(ShaderType shaderType, std::string& name, u32 offset, u32 size, const u8* data)
{
   /* auto iter = m_programInfo._uniformsBuffer.find(name);
    if (iter == m_programInfo._uniformsBuffer.cend())
    {
        LOG_WARNING("ShaderProgram::bindUniformsBuffer: binding for uniform [%s] not found ", name.c_str());
        return false;
    }

    context->bindUniform(shader, const u8 * data, u32 size, u32 offset)*/

    return false;
}

bool ShaderProgram::bindTexture(ShaderType shaderType, std::string& name, TextureTarget target, const Texture* texture)
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

    //auto iter = m_programInfo._textures.find(name);
    //if (iter == m_programInfo._textures.cend())
    //{
    //    LOG_WARNING("ShaderProgram::setTexture: binding for texture [%s] not found ", name.c_str());
    //    return false;
    //}

    //if (m_cmdList.isImmediate())
    //{
    //    m_cmdList.getContext()->bindTexture(image, (*iter).second);
    //}
    //else
    //{
    //    //TODO
    //}

    return false;
}

ShaderProgram::~ShaderProgram()
{
}

} //renderer
} //v3d
