#pragma once

#include "Common.h"
#include "Renderer/CommandList.h"
#include "Renderer/TextureProperties.h"
#include "Renderer/ShaderProperties.h"
#include "Resource/Shader.h"


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

        const resource::Shader* getShader(resource::ShaderType type) const;

        const ShaderProgramDescription& getShaderDesc() const;

        template<class TTexture>
        bool setTexture(std::string name, const TTexture* texture);

        template<class TDataType>
        bool bindUniform(std::string name, const TDataType& data);

        bool bindUniformsBuffer(std::string name, const u8* data, u32 size);

    private:

        ShaderProgram(renderer::CommandList& cmdList, std::vector<resource::Shader*> shaders) noexcept;

        friend CommandList;
        CommandList& m_cmdList;

        std::vector<resource::Shader*>      m_shaders;
        ShaderProgramDescription            m_programInfo;

        void composeProgramData();

        bool setTexture(std::string& name, TextureTarget target, const Texture* texture);
    };


    template<class TTexture>
    inline bool ShaderProgram::setTexture(std::string name, const TTexture* texture)
    {
        static_assert(std::is_base_of<Texture, TTexture>());
        return setTexture(name, texture->m_target, texture);
    }

    template<class TDataType>
    inline bool ShaderProgram::bindUniform(std::string name, const TDataType & data)
    {
        return false;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace renderer
} //namespace v3d
