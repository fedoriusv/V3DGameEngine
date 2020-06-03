#include "ShaderProgram.h"
#include "Texture.h"
#include "SamplerState.h"
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
    const Shader* const m_shader;
    u32 m_bindIndex;

    u32 m_offset;
    u32 m_size;
    void* m_data;

    bool m_shared;
};

    /*CommandBindImage*/
class CommandBindImage : public Command
{
public:
    CommandBindImage(const Shader* shader, u32 bindIndex, Image* image) noexcept
        : m_shader(shader)
        , m_bindIndex(bindIndex)
        , m_image(image)
    {
#if DEBUG_COMMAND_LIST
        LOG_DEBUG("CommandBindImage constructor");
#endif //DEBUG_COMMAND_LIST
    };
    CommandBindImage() = delete;
    CommandBindImage(CommandBindImage&) = delete;

    ~CommandBindImage()
    {
#if DEBUG_COMMAND_LIST
        LOG_DEBUG("CommandBindImage destructor");
#endif //DEBUG_COMMAND_LIST
    };

    void execute(const CommandList& cmdList)
    {
#if DEBUG_COMMAND_LIST
        LOG_DEBUG("CommandBindImage execute");
#endif //DEBUG_COMMAND_LIST
        cmdList.getContext()->bindImage(m_shader, m_bindIndex, m_image);
    }

private:
    const Shader* const m_shader;
    u32 m_bindIndex;

    Image* m_image;
};

    /*CommandBindSampler*/
class CommandBindSampler : public Command
{
    public:
    CommandBindSampler(const Shader* shader, u32 bindIndex, const Sampler::SamplerInfo& samplerInfo) noexcept
        : m_shader(shader)
        , m_bindIndex(bindIndex)
        , m_sampler(samplerInfo)
    {
#if DEBUG_COMMAND_LIST
        LOG_DEBUG("CommandBindSampler constructor");
#endif //DEBUG_COMMAND_LIST
    };
    CommandBindSampler() = delete;
    CommandBindSampler(CommandBindSampler&) = delete;

    ~CommandBindSampler()
    {
#if DEBUG_COMMAND_LIST
        LOG_DEBUG("CommandBindSampler destructor");
#endif //DEBUG_COMMAND_LIST
    };

    void execute(const CommandList& cmdList)
    {
#if DEBUG_COMMAND_LIST
        LOG_DEBUG("CommandBindSampler execute");
#endif //DEBUG_COMMAND_LIST
        cmdList.getContext()->bindSampler(m_shader, m_bindIndex, &m_sampler);
    }

private:
    const Shader* const m_shader;
    u32 m_bindIndex;

    const Sampler::SamplerInfo m_sampler;
};

    /*CommandBindSampledImage*/
class CommandBindSampledImage : public Command
{
public:
    CommandBindSampledImage(const Shader* shader, u32 bindIndex, Image* image, const Sampler::SamplerInfo& samplerInfo) noexcept
        : m_shader(shader)
        , m_bindIndex(bindIndex)
        , m_image(image)
        , m_samplerInfo(samplerInfo)
    {
#if DEBUG_COMMAND_LIST
        LOG_DEBUG("CommandBindSampledImage constructor");
#endif //DEBUG_COMMAND_LIST
    };
    CommandBindSampledImage() = delete;
    CommandBindSampledImage(CommandBindSampledImage&) = delete;

    ~CommandBindSampledImage()
    {
#if DEBUG_COMMAND_LIST
        LOG_DEBUG("CommandBindSampledImage destructor");
#endif //DEBUG_COMMAND_LIST
    };

    void execute(const CommandList& cmdList)
    {
#if DEBUG_COMMAND_LIST
        LOG_DEBUG("CommandBindSampledImage execute");
#endif //DEBUG_COMMAND_LIST
        cmdList.getContext()->bindSampledImage(m_shader, m_bindIndex, m_image, &m_samplerInfo);
    }

private:
    const Shader* const m_shader;
    u32 m_bindIndex;

    Image* m_image;
    Sampler::SamplerInfo m_samplerInfo;
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

ShaderProgram::ShaderProgram(renderer::CommandList& cmdList, const std::vector<const Shader*>& shaders) noexcept
    : m_cmdList(cmdList)
{
    for (auto shader : shaders)
    {
        ASSERT(shader, "nullptr");
        m_programInfo._shaders[shader->getShaderHeader()._type] = shader;
    }

    if (getShader(ShaderType::ShaderType_Vertex) ||
    (getShader(ShaderType::ShaderType_Fragment) && getShader(ShaderType::ShaderType_Vertex)))
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
#if USE_STRING_ID_SHADER
        auto& prameters = m_shaderParameters[shader->getShaderHeader()._type];
        u32 uniformIndex = 0;
        for (auto& buffer : shader->m_reflectionInfo._uniformBuffers)
        {
            ASSERT(!buffer._name.empty(), "empty name");
            ShaderParam param(buffer._name);
            auto iter = prameters.emplace(param._id, uniformIndex);
            if (!iter.second)
            {
                ASSERT(false, "already present inside map");
            }
            ++uniformIndex;
        }

        u32 sampledImageIndex = 0;
        for (auto& image : shader->m_reflectionInfo._sampledImages)
        {

            ASSERT(!image._name.empty(), "empty name");
            ShaderParam param(image._name);
            auto iter = prameters.emplace(param._id, sampledImageIndex);
            if (!iter.second)
            {
                ASSERT(false, "already present inside map");
            }
            ++sampledImageIndex;
        }

        u32 imageIndex = 0;
        for (auto& image : shader->m_reflectionInfo._images)
        {

            ASSERT(!image._name.empty(), "empty name");
            ShaderParam param(image._name);
            auto iter = prameters.emplace(param._id, imageIndex);
            if (!iter.second)
            {
                ASSERT(false, "already present inside map");
            }
            ++imageIndex;
        }

        u32 samplerIndex = 0;
        for (auto& sampler : shader->m_reflectionInfo._samplers)
        {

            ASSERT(!sampler._name.empty(), "empty name");
            ShaderParam param(sampler._name);
            auto iter = prameters.emplace(param._id, samplerIndex);
            if (!iter.second)
            {
                ASSERT(false, "already present inside map");
            }
            ++samplerIndex;
        }
#endif //USE_STRING_ID_SHADER
    }
}

bool ShaderProgram::bindTexture(ShaderType shaderType, u32 index, TextureTarget target, const Texture * texture)
{
#if USE_STRING_ID_SHADER
    auto param = m_shaderParameters[shaderType].find(index);
    ASSERT(param != m_shaderParameters[shaderType].cend(), "not found");
    index = param->second;
#endif //USE_STRING_ID_SHADER
    ASSERT(texture, "nullptr");
    Image* image = nullptr;
    switch (target)
    {
    case TextureTarget::Texture2D:
        image = static_cast<const Texture2D*>(texture)->m_image;
        break;

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
    ASSERT(index < shader->getReflectionInfo()._images.size(), "range out");

    if (m_cmdList.isImmediate())
    {
        m_cmdList.getContext()->bindImage(shader, index, image);
    }
    else
    {
        m_cmdList.pushCommand(new CommandBindImage(shader, index, image));
    }

    return true;
}

bool ShaderProgram::bindSampler(ShaderType shaderType, u32 index, const SamplerState* sampler)
{
#if USE_STRING_ID_SHADER
    auto param = m_shaderParameters[shaderType].find(index);
    ASSERT(param != m_shaderParameters[shaderType].cend(), "not found");
    index = param->second;
#endif //USE_STRING_ID_SHADER
    ASSERT(sampler, "nullptr");

    const Shader* shader = m_programInfo._shaders[shaderType];
    ASSERT(shader, "fail");
    ASSERT(index < shader->getReflectionInfo()._samplers.size(), "range out");

    Sampler::SamplerInfo samplerInfo;
    samplerInfo._tracker = const_cast<ObjectTracker<Sampler>*>(&sampler->m_trackerSampler);
    samplerInfo._value._desc = sampler->m_samplerDesc;

    if (m_cmdList.isImmediate())
    {
        m_cmdList.getContext()->bindSampler(shader, index, &samplerInfo);
    }
    else
    {
        m_cmdList.pushCommand(new CommandBindSampler(shader, index, samplerInfo));
    }

    return true;
}

bool ShaderProgram::bindSampledTexture(ShaderType shaderType, u32 index, TextureTarget target, const Texture * texture, const SamplerState* sampler)
{
#if USE_STRING_ID_SHADER
    auto param = m_shaderParameters[shaderType].find(index);
    ASSERT(param != m_shaderParameters[shaderType].cend(), "not found");
    index = param->second;
#endif //USE_STRING_ID_SHADER
    ASSERT(texture, "nullptr");
    Image* image = nullptr;
    switch (target)
    {
    case TextureTarget::Texture2D:
        image = static_cast<const Texture2D*>(texture)->m_image;
        break;

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
    ASSERT(index < shader->getReflectionInfo()._sampledImages.size(), "range out");

    Sampler::SamplerInfo samplerInfo;
    samplerInfo._tracker = const_cast<ObjectTracker<Sampler>*>(&sampler->m_trackerSampler);
    samplerInfo._value._desc = sampler->m_samplerDesc;

    if (m_cmdList.isImmediate())
    {
        m_cmdList.getContext()->bindSampledImage(shader, index, image, &samplerInfo);
    }
    else
    {
        m_cmdList.pushCommand(new CommandBindSampledImage(shader, index, image, samplerInfo));
    }

    return true;
}

bool ShaderProgram::bindUniformsBuffer(ShaderType shaderType, u32 index, u32 offset, u32 size, const void * data)
{
#if USE_STRING_ID_SHADER
    auto param = m_shaderParameters[shaderType].find(index);
    ASSERT(param != m_shaderParameters[shaderType].cend(), "not found");
    index = param->second;
#endif //USE_STRING_ID_SHADER

    const Shader* shader = m_programInfo._shaders[shaderType];
    ASSERT(shader, "fail");
    ASSERT(index < shader->getReflectionInfo()._uniformBuffers.size(), "range out");
   
    if (m_cmdList.isImmediate())
    {
        m_cmdList.getContext()->bindUniformsBuffer(shader, index, offset, size, data);
    }
    else
    {
        m_cmdList.pushCommand(new CommandUpdateUniformsBuffer(shader, index, offset, size, const_cast<void*>(data), false));
    }

    return true;
}

ShaderProgram::~ShaderProgram()
{
}

} //renderer
} //v3d
