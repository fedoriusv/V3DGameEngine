#pragma once

#include "Common.h"
#include "Renderer/Shader.h"
#include "Resource/Resource.h"
#include "Resource/ResourceManager.h"
#include "Resource/Decoder/ShaderDecoder.h"
#include "ShaderSourceStreamLoader.h"

namespace v3d
{
namespace renderer
{
    class Device;
} //namespace renderer
namespace resource
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////
    
    /**
    * @brief ShaderCompiler
    */
    class ShaderCompiler
    {
    public:

        /**
        * @brief compileShader interface.
        * @return Shader resource, nullptr if failed
        */
        template<class TResource = renderer::Shader>
        [[nodiscard]] static const TResource* compileShader(renderer::Device* device, const std::string& name, const renderer::Shader::LoadPolicy& policy, const stream::Stream* stream, ShaderCompileFlags flags = 0);
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    template<class TResource>
    inline const TResource* ShaderCompiler::compileShader(renderer::Device* device, const std::string& name, const renderer::Shader::LoadPolicy& policy, const stream::Stream* stream, ShaderCompileFlags flags)
    {
        return ResourceManager::getLazyInstance()->composeShader<TResource, resource::ShaderSourceStreamLoader>(device, name, policy, stream, flags);
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace resource
} //namespace v3d
