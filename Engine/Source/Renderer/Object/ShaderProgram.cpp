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

ShaderProgram::ShaderProgram(renderer::CommandList& cmdList, const std::vector<const Shader*>& shaders) noexcept
    : m_cmdList(cmdList)
{
    for (auto& shader : shaders)
    {
        ASSERT(shader, "nullptr");
        m_programInfo._shaders[toEnumType(shader->getShaderHeader()._type)] = shader;
    }

    if (ShaderProgram::getShader(ShaderType::Vertex) ||
    (ShaderProgram::getShader(ShaderType::Fragment) && ShaderProgram::getShader(ShaderType::Vertex)))
    {
        composeProgramData(shaders);
    }
    else
    {
        ASSERT(false, "unknown program");
    }
}

ShaderProgram::ShaderProgram(renderer::CommandList& cmdList, const Shader* computeShader) noexcept
    : m_cmdList(cmdList)
{
    ASSERT(computeShader, "nullptr");
    m_programInfo._shaders[toEnumType(computeShader->getShaderHeader()._type)] = computeShader;
    if (ShaderProgram::getShader(ShaderType::Compute))
    {
        composeProgramData({ computeShader });
    }
    else
    {
        ASSERT(false, "wrong program");
    }
}

ShaderProgram::~ShaderProgram()
{
    LOG_DEBUG("ShaderProgram::ShaderProgram destructor %llx", this);
}

const Shader* ShaderProgram::getShader(ShaderType type) const
{
    return m_programInfo._shaders[toEnumType(type)];
}

const ShaderProgramDescription& ShaderProgram::getShaderDesc() const
{
    return m_programInfo;
}

void ShaderProgram::composeProgramData(const std::vector<const Shader*>& shaders)
{
    m_programInfo._hash = 0;
    for (auto shader : shaders)
    {
        const_cast<Shader*>(shader)->registerNotify(this);

        m_programInfo._hash = crc32c::Extend(m_programInfo._hash, reinterpret_cast<const u8*>(&shader->m_hash), sizeof(u32));
#if USE_STRING_ID_SHADER
        auto& prameters = m_shaderParameters[toEnumType(shader->getShaderHeader()._type)];
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

        u32 storageImageIndex = 0;
        for (auto& storageImage : shader->m_reflectionInfo._storageImages)
        {

            ASSERT(!storageImage._name.empty(), "empty name");
            ShaderParam param(storageImage._name);
            auto iter = prameters.emplace(param._id, storageImageIndex);
            if (!iter.second)
            {
                ASSERT(false, "already present inside map");
            }
            ++storageImageIndex;
        }
#endif //USE_STRING_ID_SHADER
    }
}

bool ShaderProgram::bindTexture(ShaderType shaderType, u32 index, TextureTarget target, const Texture* texture, s32 layer, s32 mip)
{
#if USE_STRING_ID_SHADER
    auto param = m_shaderParameters[toEnumType(shaderType)].find(index);
    ASSERT(param != m_shaderParameters[toEnumType(shaderType)].cend(), "bindTexture: bind index not found");
    index = param->second;
#endif //USE_STRING_ID_SHADER
    ASSERT(texture, "nullptr");
    Image* image = texture->getImage();
    if (!image)
    {
        ASSERT(false, "image nullptr");
        return false;
    }

    const Shader* shader = m_programInfo._shaders[toEnumType(shaderType)];
    ASSERT(shader, "fail");
    ASSERT(index < shader->getReflectionInfo()._images.size(), "range out");

    if (m_cmdList.isImmediate())
    {
        m_cmdList.getContext()->bindImage(shader, index, image, layer, mip);
    }
    else
    {
        /*CommandBindImage*/
        class CommandBindImage : public Command
        {
        public:

            explicit CommandBindImage(const Shader* shader, u32 bindIndex, Image* image, s32 layer, s32 mip) noexcept
                : m_shader(shader)
                , m_bindIndex(bindIndex)
                , m_image(image)
                , m_layer(layer)
                , m_mip(mip)
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
                cmdList.getContext()->bindImage(m_shader, m_bindIndex, m_image, m_layer, m_mip);
            }

        private:

            const Shader* const m_shader;
            u32 m_bindIndex;

            Image* m_image;
            s32 m_layer;
            s32 m_mip;
        };

        m_cmdList.pushCommand(new CommandBindImage(shader, index, image, layer, mip));
    }

    return true;
}

bool ShaderProgram::bindSampler(ShaderType shaderType, u32 index, const SamplerState* sampler)
{
#if USE_STRING_ID_SHADER
    auto param = m_shaderParameters[toEnumType(shaderType)].find(index);
    ASSERT(param != m_shaderParameters[toEnumType(shaderType)].cend(), "bindSampler: bind index not found");
    index = param->second;
#endif //USE_STRING_ID_SHADER
    ASSERT(sampler, "nullptr");

    const Shader* shader = m_programInfo._shaders[toEnumType(shaderType)];
    ASSERT(shader, "fail");
    ASSERT(index < shader->getReflectionInfo()._samplers.size(), "range out");

    Sampler::SamplerInfo samplerInfo;
    samplerInfo._tracker = const_cast<ObjectTracker<Sampler>*>(&sampler->m_trackerSampler);
    samplerInfo._desc = sampler->m_samplerDesc;

    if (m_cmdList.isImmediate())
    {
        m_cmdList.getContext()->bindSampler(shader, index, &samplerInfo);
    }
    else
    {
        /*CommandBindSampler*/
        class CommandBindSampler : public Command
        {
        public:

            explicit CommandBindSampler(const Shader* shader, u32 bindIndex, const Sampler::SamplerInfo& samplerInfo) noexcept
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

        m_cmdList.pushCommand(new CommandBindSampler(shader, index, samplerInfo));
    }

    return true;
}

bool ShaderProgram::bindSampledTexture(ShaderType shaderType, u32 index, TextureTarget target, const Texture* texture, const SamplerState* sampler, s32 layer, s32 mip)
{
#if USE_STRING_ID_SHADER
    auto param = m_shaderParameters[toEnumType(shaderType)].find(index);
    ASSERT(param != m_shaderParameters[toEnumType(shaderType)].cend(), "bindSampledTexture: bind index not found");
    index = param->second;
#endif //USE_STRING_ID_SHADER
    ASSERT(texture, "nullptr");
    Image* image = texture->getImage();
    if (!image)
    {
        ASSERT(false, "image nullptr");
        return false;
    }
    const Shader* shader = m_programInfo._shaders[toEnumType(shaderType)];
    ASSERT(shader, "fail");
    ASSERT(index < shader->getReflectionInfo()._sampledImages.size(), "range out");

    Sampler::SamplerInfo samplerInfo;
    samplerInfo._tracker = const_cast<ObjectTracker<Sampler>*>(&sampler->m_trackerSampler);
    samplerInfo._desc = sampler->m_samplerDesc;

    if (m_cmdList.isImmediate())
    {
        m_cmdList.getContext()->bindSampledImage(shader, index, image, &samplerInfo, layer, mip);
    }
    else
    {
        /*CommandBindSampledImage*/
        class CommandBindSampledImage : public Command
        {
        public:

            explicit CommandBindSampledImage(const Shader* shader, u32 bindIndex, Image* image, const Sampler::SamplerInfo& samplerInfo, s32 layer, s32 mip) noexcept
                : m_shader(shader)
                , m_bindIndex(bindIndex)
                , m_image(image)
                , m_layer(layer)
                , m_mip(mip)
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
                cmdList.getContext()->bindSampledImage(m_shader, m_bindIndex, m_image, &m_samplerInfo, m_layer, m_mip);
            }

        private:

            const Shader* const m_shader;
            u32 m_bindIndex;

            Image* m_image;
            s32 m_layer;
            s32 m_mip;
            Sampler::SamplerInfo m_samplerInfo;
        };

        m_cmdList.pushCommand(new CommandBindSampledImage(shader, index, image, samplerInfo, layer, mip));
    }

    return true;
}

bool ShaderProgram::bindUniformsBuffer(ShaderType shaderType, u32 index, u32 offset, u32 size, const void* data)
{
#if USE_STRING_ID_SHADER
    auto param = m_shaderParameters[toEnumType(shaderType)].find(index);
    ASSERT(param != m_shaderParameters[toEnumType(shaderType)].cend(), "not found");
    index = param->second;
#endif //USE_STRING_ID_SHADER

    const Shader* shader = m_programInfo._shaders[toEnumType(shaderType)];
    ASSERT(shader, "fail");
    ASSERT(index < shader->getReflectionInfo()._uniformBuffers.size(), "range out");
   
    if (m_cmdList.isImmediate())
    {
        m_cmdList.getContext()->bindUniformsBuffer(shader, index, offset, size, data);
    }
    else
    {
        /*CommandUpdateUniformsBuffer*/
        class CommandUpdateUniformsBuffer : public Command
        {
        public:

            explicit CommandUpdateUniformsBuffer(const Shader* shader, u32 bindIndex, u32 offset, u32 size, void* data, bool shared) noexcept
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

        m_cmdList.pushCommand(new CommandUpdateUniformsBuffer(shader, index, offset, size, const_cast<void*>(data), false));
    }

    return true;
}

bool ShaderProgram::bindUAV(ShaderType shaderType, u32 index, TextureTarget target, const Texture* texture, s32 layer, s32 mip)
{
#if USE_STRING_ID_SHADER
    auto param = m_shaderParameters[toEnumType(shaderType)].find(index);
    ASSERT(param != m_shaderParameters[toEnumType(shaderType)].cend(), "bindTexture: bind index not found");
    index = param->second;
#endif //USE_STRING_ID_SHADER
    ASSERT(texture, "nullptr");
    Image* image = texture->getImage();
    if (!image)
    {
        ASSERT(false, "image nullptr");
        return false;
    }

    const Shader* shader = m_programInfo._shaders[toEnumType(shaderType)];
    ASSERT(shader, "fail");
    ASSERT(index < shader->getReflectionInfo()._storageImages.size(), "range out");

    if (m_cmdList.isImmediate())
    {
        m_cmdList.getContext()->bindStorageImage(shader, index, image, layer, mip);
    }
    else
    {
        /*CommandBindStorageImage*/
        class CommandBindStorageImage : public Command
        {
        public:

            explicit CommandBindStorageImage(const Shader* shader, u32 bindIndex, Image* image, s32 layer, s32 mip) noexcept
                : m_shader(shader)
                , m_bindIndex(bindIndex)
                , m_image(image)
                , m_layer(layer)
                , m_mip(mip)
            {
#if DEBUG_COMMAND_LIST
                LOG_DEBUG("CommandBindStorageImage constructor");
#endif //DEBUG_COMMAND_LIST
            };

            CommandBindStorageImage() = delete;
            CommandBindStorageImage(CommandBindStorageImage&) = delete;

            ~CommandBindStorageImage()
            {
#if DEBUG_COMMAND_LIST
                LOG_DEBUG("CommandBindStorageImage destructor");
#endif //DEBUG_COMMAND_LIST
            };

            void execute(const CommandList& cmdList)
            {
#if DEBUG_COMMAND_LIST
                LOG_DEBUG("CommandBindStorageImage execute");
#endif //DEBUG_COMMAND_LIST
                cmdList.getContext()->bindStorageImage(m_shader, m_bindIndex, m_image, m_layer, m_mip);
            }

        private:

            const Shader* const m_shader;
            u32 m_bindIndex;

            Image* m_image;
            s32 m_layer;
            s32 m_mip;
        };

        m_cmdList.pushCommand(new CommandBindStorageImage(shader, index, image, layer, mip));
    }

    return true;
}

void ShaderProgram::handleNotify(const utils::Observable* obj)
{
    //remove shader from list
    auto found = std::find(m_programInfo._shaders.begin(), m_programInfo._shaders.end(), static_cast<const renderer::Shader*>(obj));
    ASSERT(found != m_programInfo._shaders.end(), "nuot found");
    if (found != m_programInfo._shaders.end())
    {
        *found = nullptr;
    }
}

} //renderer
} //v3d
