#pragma once

#include "Common.h"
#include "Resource.h"
#include "Renderer/Shader.h"
#include "ResourceManager.h"
#include "ShaderDecoder.h"
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
        [[nodiscard]] static const TResource* compileShader(renderer::Device* device, const std::string& name, const ShaderDecoder::ShaderPolicy& policy, const stream::Stream* stream, ShaderCompileFlags flags = 0);
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    template<class TResource>
    inline const TResource* ShaderCompiler::compileShader(renderer::Device* device, const std::string& name, const ShaderDecoder::ShaderPolicy& policy, const stream::Stream* stream, ShaderCompileFlags flags)
    {
        return ResourceManager::getLazyInstance()->composeShader<renderer::Shader, resource::ShaderSourceStreamLoader>(device, name, policy, stream, flags);
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace resource
} //namespace v3d
