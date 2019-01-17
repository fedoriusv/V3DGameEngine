#pragma once

#include "Common.h"
#include "Renderer/CommandList.h"
#include "Renderer/ShaderProperties.h"
#include "Resource/Shader.h"

namespace v3d
{
namespace renderer
{
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


        //bool bindTexture(Texture* texture, std::string name);
        //bool bindUniform<Type>(data*);

    private:

        ShaderProgram(renderer::CommandList& cmdList, std::vector<resource::Shader*> shaders) noexcept;

        friend renderer::CommandList;
        renderer::CommandList&              m_cmdList;

        std::vector<resource::Shader*>      m_shaders;
        ShaderProgramDescription            m_programInfo;

        void composeProgramData();
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace renderer
} //namespace v3d
