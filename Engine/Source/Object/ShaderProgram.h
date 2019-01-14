#pragma once

#include "Common.h"
#include "Renderer/CommandList.h"
#include "Resource/Shader.h"

namespace v3d
{
namespace renderer
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * ShaderProgram class
    */
    class ShaderProgram : public Object //ref counter
    {
    public:

        ~ShaderProgram();
        ShaderProgram(const ShaderProgram &) = delete;

        const resource::Shader* getShader(resource::ShaderType type) const;


        //TODO: 
        //bool bindTexture(Texture* texture, std::string name);
        //bool bindUniform<Type>(data*);

    private:

        ShaderProgram(renderer::CommandList& cmdList, std::vector<resource::Shader*> shaders) noexcept;

        renderer::CommandList&          m_cmdList;
        std::vector<resource::Shader*>  m_shaders;

        friend renderer::CommandList;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace renderer
} //namespace v3d
