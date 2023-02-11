#pragma once

#include "Common.h"
#include "Object.h"
#include "Utils/Observable.h"

#include "TextureProperties.h"
#include "SamplerProperties.h"
#include "ShaderProperties.h"
#include "CommandList.h"

namespace v3d
{
namespace renderer
{
    class Texture;
    class SamplerState;

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief ShaderProgram class. Game side
    */
    class ShaderProgram : public Object, public utils::Observer
    {
    public:

        ShaderProgram() = delete;
        ShaderProgram(const ShaderProgram &) = delete;
        ~ShaderProgram();

        const Shader* getShader(ShaderType type) const;
        const ShaderProgramDescription& getShaderDesc() const;

#if USE_STRING_ID_SHADER
        template<ShaderType shaderType, class TTexture>
        bool bindTexture(const ShaderParam& param, const TTexture* texture, s32 layer = k_generalLayer, s32 mip = k_allMipmapsLevels);

        template<ShaderType shaderType>
        bool bindSampler(const ShaderParam& param, const SamplerState* sampler);

        template<ShaderType shaderType, class TTexture>
        bool bindSampledTexture(const ShaderParam& param, const TTexture* texture, const SamplerState* sampler, s32 layer = k_generalLayer, s32 mip = k_allMipmapsLevels);

        template<ShaderType shaderType>
        bool bindUniformsBuffer(const ShaderParam& param, u32 offset, u32 size, const void* data);

        template<ShaderType shaderType, class TTexture>
        bool bindUAV(const ShaderParam& param, const TTexture* texture, s32 layer = k_generalLayer, s32 mip = k_allMipmapsLevels);
#else
        template<ShaderType shaderType, class TTexture>
        bool bindTexture(u32 index, const TTexture* texture);

        template<ShaderType shaderType, class TTexture>
        bool bindSampledTexture(u32 index, const TTexture* texture, const SamplerState* sampler);

        template<ShaderType shaderType>
        bool bindUniformsBuffer(u32 index, u32 offset, u32 size, const void* data);
#endif //USE_STRING_ID_SHADER
    private:

        ShaderProgram(renderer::CommandList& cmdList, const std::vector<const Shader*>& shaders) noexcept;
        ShaderProgram(renderer::CommandList& cmdList, const Shader* computeShader) noexcept;

        friend CommandList;
        CommandList& m_cmdList;

        ShaderProgramDescription m_programInfo;

        void composeProgramData(const std::vector<const Shader*>& shaders);

        bool bindTexture(ShaderType shaderType, u32 index, TextureTarget target, const Texture* texture, s32 layer, s32 mip);
        bool bindSampler(ShaderType shaderType, u32 index, const SamplerState* sampler);
        bool bindSampledTexture(ShaderType shaderType, u32 index, TextureTarget target, const Texture* texture, const SamplerState* sampler, s32 layer, s32 mip);
        bool bindUniformsBuffer(ShaderType shaderType, u32 index, u32 offset, u32 size, const void* data);
        bool bindUAV(ShaderType shaderType, u32 index, TextureTarget target, const Texture* texture, s32 layer, s32 mip);

        std::map<u32, u32> m_shaderParameters[toEnumType(ShaderType::Count)];

        void handleNotify(const utils::Observable* object, void* msg) override;
    };


#if USE_STRING_ID_SHADER
    template<ShaderType shaderType, class TTexture>
    inline bool ShaderProgram::bindTexture(const ShaderParam& param, const TTexture* texture, s32 layer, s32 mip)
    {
        static_assert(std::is_base_of<Texture, TTexture>(), "ShaderProgram::bindTexture wrong type");
        return ShaderProgram::bindTexture(shaderType, param._id, texture->m_target, texture, layer, mip);
    }

    template<ShaderType shaderType>
    bool ShaderProgram::bindSampler(const ShaderParam& param, const SamplerState* sampler)
    {
        return ShaderProgram::bindSampler(shaderType, param._id, sampler);
    }

    template<ShaderType shaderType, class TTexture>
    inline bool ShaderProgram::bindSampledTexture(const ShaderParam& param, const TTexture* texture, const SamplerState* sampler, s32 layer, s32 mip)
    {
        static_assert(std::is_base_of<Texture, TTexture>(), "ShaderProgram::bindSampledTexture wrong type");
        return ShaderProgram::bindSampledTexture(shaderType, param._id, texture->m_target, texture, sampler, layer, mip);
    }

    template<ShaderType shaderType>
    inline bool ShaderProgram::bindUniformsBuffer(const ShaderParam& param, u32 offset, u32 size, const void* data)
    {
        return ShaderProgram::bindUniformsBuffer(shaderType, param._id, offset, size, data);
    }

    template<ShaderType shaderType, class TTexture>
    inline bool ShaderProgram::bindUAV(const ShaderParam& param, const TTexture* texture, s32 layer, s32 mip)
    {
        static_assert(std::is_base_of<Texture, TTexture>(), "ShaderProgram::bindTexture wrong type");
        return ShaderProgram::bindUAV(shaderType, param._id, texture->m_target, texture, layer, mip);
    }
#else
    template<ShaderType shaderType, class TTexture>
    inline bool ShaderProgram::bindTexture(u32 index, const TTexture* texture)
    {
        static_assert(std::is_base_of<Texture, TTexture>(), "wrong type");
        return bindTexture(shaderType, index, texture->m_target, texture);
    }

    template<ShaderType shaderType, class TTexture>
    inline bool ShaderProgram::bindSampledTexture(u32 index, const TTexture* texture, const SamplerState* sampler)
    {
        static_assert(std::is_base_of<Texture, TTexture>(), "wrong type");
        return bindSampledTexture(shaderType, index, texture->m_target, texture, sampler);
    }

    template<ShaderType shaderType>
    inline bool ShaderProgram::bindUniformsBuffer(u32 index, u32 offset, u32 size, const void* data)
    {
        return ShaderProgram::bindUniformsBuffer(shaderType, index, offset, size, data);
    }
#endif //USE_STRING_ID_SHADER

    inline const Shader* ShaderProgram::getShader(ShaderType type) const
    {
        return m_programInfo._shaders[toEnumType(type)];
    }

    inline const ShaderProgramDescription& ShaderProgram::getShaderDesc() const
    {
        return m_programInfo;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace renderer
} //namespace v3d
