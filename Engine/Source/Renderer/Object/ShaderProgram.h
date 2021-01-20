#pragma once

#include "Common.h"
#include "Object.h"
#include "Utils/Observable.h"
#include "Renderer/TextureProperties.h"
#include "Renderer/SamplerProperties.h"
#include "Renderer/ShaderProperties.h"
#include "Renderer/CommandList.h"

namespace v3d
{
namespace renderer
{
    class Texture;
    class SamplerState;

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * ShaderProgram class. Game side
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
        bool bindTexture(const ShaderParam& param, const TTexture* texture);

        template<ShaderType shaderType>
        bool bindSampler(const ShaderParam& param, const SamplerState* sampler);

        template<ShaderType shaderType, class TTexture>
        bool bindSampledTexture(const ShaderParam& param, const TTexture* texture, const SamplerState* sampler);

        template<ShaderType shaderType>
        bool bindUniformsBuffer(const ShaderParam& param, u32 offset, u32 size, const void* data);
#else
        template<ShaderType shaderType, class TTexture>
        bool bindTexture(u32 index, const TTexture* texture);

        template<ShaderType shaderType, class TTexture>
        bool bindSampledTexture(u32 index, const TTexture* texture, const SamplerState* sampler);

        template<ShaderType shaderType>
        bool bindUniformsBuffer(u32 index, u32 offset, u32 size, const void* data);
#endif
    private:

        ShaderProgram(renderer::CommandList& cmdList, const std::vector<const Shader*>& shaders) noexcept;

        friend CommandList;
        CommandList& m_cmdList;

        ShaderProgramDescription m_programInfo;

        void composeProgramData(const std::vector<const Shader*>& shaders);

        bool bindTexture(ShaderType shaderType, u32 index, TextureTarget target, const Texture* texture);
        bool bindSampler(ShaderType shaderType, u32 index, const SamplerState* sampler);
        bool bindSampledTexture(ShaderType shaderType, u32 index, TextureTarget target, const Texture* texture, const SamplerState* sampler);
        bool bindUniformsBuffer(ShaderType shaderType, u32 index, u32 offset, u32 size, const void* data);

        std::map<u32, u32> m_shaderParameters[ShaderType::ShaderType_Count];

        void handleNotify(const utils::Observable* obj) override;
    };


#if USE_STRING_ID_SHADER
    template<ShaderType shaderType, class TTexture>
    inline bool ShaderProgram::bindTexture(const ShaderParam& param, const TTexture* texture)
    {
        static_assert(std::is_base_of<Texture, TTexture>(), "ShaderProgram::bindTexture wrong type");
        return bindTexture(shaderType, param._id, texture->m_target, texture);
    }

    template<ShaderType shaderType>
    bool ShaderProgram::bindSampler(const ShaderParam& param, const SamplerState* sampler)
    {
        return bindSampler(shaderType, param._id, sampler);
    }

    template<ShaderType shaderType, class TTexture>
    inline bool ShaderProgram::bindSampledTexture(const ShaderParam& param, const TTexture* texture, const SamplerState* sampler)
    {
        static_assert(std::is_base_of<Texture, TTexture>(), "ShaderProgram::bindSampledTexture wrong type");
        return bindSampledTexture(shaderType, param._id, texture->m_target, texture, sampler);
    }

    template<ShaderType shaderType>
    inline bool ShaderProgram::bindUniformsBuffer(const ShaderParam& param, u32 offset, u32 size, const void* data)
    {
        return bindUniformsBuffer(shaderType, param._id, offset, size, data);
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

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace renderer
} //namespace v3d
