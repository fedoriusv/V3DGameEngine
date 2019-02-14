#include "ShaderProgram.h"
#include "Texture.h"
#include "Renderer/Formats.h"
#include "Renderer/Context.h"
#include "Renderer/Shader.h"

#include "Utils/Logger.h"
#include "crc32c/crc32c.h"

namespace v3d
{
namespace renderer
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /*CommandUpdateUniformsBuffer*/
class CommandUpdateUniformsBuffer : public Command
{
public:
    CommandUpdateUniformsBuffer(const Shader* shader, u32 bindIndex, u32 offset, u32 size, void* data, bool shared) noexcept
        : m_shader(shader)
        , m_bindIndex(bindIndex)
        , m_offset(offset)
        , m_size(size)
        , m_data(nullptr)
        , m_shared(shared)
    {
#if DEBUG_COMMAND_LIST
        LOG_DEBUG("CommandUpdateUniformsBuffer constructor");
#endif //DEBUG_COMMAND_LIST

        if (m_shared)
        {
            m_data = data;
        }
        else
        {
            ASSERT(m_size > 0, "invalid size");
            m_data = malloc(size);
            memcpy(m_data, data, size);
        }
    };
    CommandUpdateUniformsBuffer() = delete;
    CommandUpdateUniformsBuffer(CommandUpdateUniformsBuffer&) = delete;

    ~CommandUpdateUniformsBuffer()
    {
#if DEBUG_COMMAND_LIST
        LOG_DEBUG("CommandUpdateUniformsBuffer destructor");
#endif //DEBUG_COMMAND_LIST

        if (m_data && !m_shared)
        {
            free(m_data);
            m_data = nullptr;
        }
    };

    void execute(const CommandList& cmdList)
    {
#if DEBUG_COMMAND_LIST
        LOG_DEBUG("CommandUpdateUniformsBuffer execute");
#endif //DEBUG_COMMAND_LIST
        cmdList.getContext()->bindUniformsBuffer(m_shader, m_bindIndex, m_offset, m_size, m_data);
    }

private:
    const Shader* m_shader;
    u32 m_bindIndex;

    u32 m_offset;
    u32 m_size;
    void* m_data;

    bool m_shared;
};

    /*CommandBindTexture*/
class CommandBindTexture : public Command
{
public:
    CommandBindTexture(const Shader* shader, u32 bindIndex, Image* image) noexcept
        : m_shader(shader)
        , m_bindIndex(bindIndex)
        , m_image(image)
    {
#if DEBUG_COMMAND_LIST
        LOG_DEBUG("CommandBindTexture constructor");
#endif //DEBUG_COMMAND_LIST
    };
    CommandBindTexture() = delete;
    CommandBindTexture(CommandBindTexture&) = delete;

    ~CommandBindTexture()
    {
#if DEBUG_COMMAND_LIST
        LOG_DEBUG("CommandBindTexture destructor");
#endif //DEBUG_COMMAND_LIST
    };

    void execute(const CommandList& cmdList)
    {
#if DEBUG_COMMAND_LIST
        LOG_DEBUG("CommandBindTexture execute");
#endif //DEBUG_COMMAND_LIST
        cmdList.getContext()->bindTexture(m_shader, m_bindIndex, m_image);
    }

private:
    const Shader* m_shader;
    u32 m_bindIndex;

    Image* m_image;
};

    /////////////////////////////////////////////////////////////////////////////////////////////////////

const Shader * ShaderProgram::getShader(ShaderType type) const
{
    return m_programInfo._shaders[type];
}

const ShaderProgramDescription& ShaderProgram::getShaderDesc() const
{
    return m_programInfo;
}

ShaderProgram::ShaderProgram(renderer::CommandList & cmdList, std::vector<const Shader*> shaders) noexcept
    : m_cmdList(cmdList)
{
    for (auto shader : shaders)
    {
        ASSERT(shader, "nullptr");
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

void ShaderProgram::composeProgramData(const std::vector<const Shader*>& shaders)
{
    m_programInfo._hash = 0;
    for (auto shader : shaders)
    {
        m_programInfo._hash = crc32c::Extend(m_programInfo._hash, reinterpret_cast<const u8*>(&shader->m_hash), sizeof(u32));

        auto& prameters = m_shaderParameters[shader->getShaderHeader()._type];
        u32 uniformIndex = 0;
        for (auto& buffer : shader->m_reflectionInfo._uniformBuffers)
        {
            ASSERT(!buffer._name.empty(), "empty name");
            auto iter = prameters.emplace(buffer._name, uniformIndex);
            if (!iter.second)
            {
                ASSERT(false, "already present inside map");
            }
            ++uniformIndex;
        }

        u32 imageIndex = 0;
        for (auto& image : shader->m_reflectionInfo._sampledImages)
        {
            ASSERT(!image._name.empty(), "empty name");
            auto iter = prameters.emplace(image._name, imageIndex);
            if (!iter.second)
            {
                ASSERT(false, "already present inside map");
            }
            ++imageIndex;
        }
    }
}

bool ShaderProgram::bindUniformsBuffer(ShaderType shaderType, std::string& name, u32 offset, u32 size, const void* data)
{
    const Shader* shader = m_programInfo._shaders[shaderType];
    ASSERT(shader, "fail");
    ASSERT(!m_shaderParameters[shaderType].empty(), "fail");
    auto iter = m_shaderParameters[shaderType].find(name);
    if (iter == m_shaderParameters[shaderType].cend())
    {
        LOG_WARNING("ShaderProgram::bindUniformsBuffer: binding for buffer [%s] not found ", name.c_str());
        ASSERT(false, "not found");
        return false;
    }

    if (m_cmdList.isImmediate())
    {
        m_cmdList.getContext()->bindUniformsBuffer(shader, iter->second, offset, size, data);
    }
    else
    {
        m_cmdList.pushCommand(new CommandUpdateUniformsBuffer(shader, iter->second, offset, size, const_cast<void*>(data), false));
    }

    return true;
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

    const Shader* shader = m_programInfo._shaders[shaderType];
    ASSERT(shader, "fail");
    ASSERT(!m_shaderParameters[shaderType].empty(), "fail");
    auto iter = m_shaderParameters[shaderType].find(name);
    if (iter == m_shaderParameters[shaderType].cend())
    {
        LOG_WARNING("ShaderProgram::setTexture: binding for texture [%s] not found ", name.c_str());
        ASSERT(false, "not found");
        return false;
    }

    if (m_cmdList.isImmediate())
    {
        m_cmdList.getContext()->bindTexture(shader, iter->second, image);
    }
    else
    {
        m_cmdList.pushCommand(new CommandBindTexture(shader, iter->second, image));
    }

    return false;
}

ShaderProgram::~ShaderProgram()
{
}

} //renderer
} //v3d
