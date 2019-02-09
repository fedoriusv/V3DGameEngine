#pragma once

#include "Common.h"
#include "Object.h"
#include "Renderer/TextureProperties.h"
#include "Renderer/ShaderProperties.h"
#include "Renderer/CommandList.h"

namespace v3d
{
namespace renderer
{
    class Texture;

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * ShaderProgram class. Game side
    */
    class ShaderProgram : public Object
    {
    public:

        ShaderProgram() = delete;
        ShaderProgram(const ShaderProgram &) = delete;
        ~ShaderProgram();

        const resource::Shader* getShader(ShaderType type) const;

        const ShaderProgramDescription& getShaderDesc() const;

        template<class TTexture, ShaderType shaderType>
        bool bindTexture(std::string name, const TTexture* texture);

        template<class TDataType, ShaderType shaderType>
        bool bindUniform(std::string name, const TDataType& data);

        template<ShaderType shaderType>
        bool bindUniformsBuffer(std::string name, u32 offset, u32 size, const void* data);

    private:

        ShaderProgram(renderer::CommandList& cmdList, std::vector<resource::Shader*> shaders) noexcept;

        friend CommandList;
        CommandList& m_cmdList;

        ShaderProgramDescription m_programInfo;

        void composeProgramData(const std::vector<resource::Shader*>& shaders);

        bool bindTexture(ShaderType shaderType, std::string& name, TextureTarget target, const Texture* texture);
        bool bindUniformsBuffer(ShaderType shaderType, std::string& name, u32 offset, u32 size, const void* data);

        std::map<std::string, u32> m_shaderParameters[ShaderType::ShaderType_Count];
    };


    template<class TTexture, ShaderType shaderType>
    inline bool ShaderProgram::bindTexture(std::string name, const TTexture* texture)
    {
        static_assert(std::is_base_of<Texture, TTexture>());
        return bindTexture(shaderType, name, texture->m_target, texture);
    }

    template<class TDataType, ShaderType shaderType>
    inline bool ShaderProgram::bindUniform(std::string name, const TDataType & data)
    {
        return false;
    }

    template<ShaderType shaderType>
    inline bool ShaderProgram::bindUniformsBuffer(std::string name, u32 offset, u32 size, const void* data)
    {
        //static_assert(std::is_base_of<Texture, TTexture>());
        return ShaderProgram::bindUniformsBuffer(shaderType, name, offset, size, data);
    }


    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace renderer
} //namespace v3d
