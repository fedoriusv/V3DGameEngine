#pragma once

#include "Common.h"
#include "Resource/Resource.h"
#include "Renderer/Shader.h"
#include "ResourceDecoder.h"

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
    * @brief ShaderCompileFlag enum.
    */
    enum ShaderCompileFlag
    {
        ShaderCompile_OptimizationSize = 1 << 0,
        ShaderCompile_OptimizationPerformance = 1 << 1,
        ShaderCompile_OptimizationFull = 1 << 2,
        ShaderCompile_UseLegacyCompilerForHLSL = 1 << 3,
        ShaderCompile_UseDXCompilerForSpirV = 1 << 4,
        ShaderCompile_ShaderModelFromExt = 1 << 5, // detect shader model by file extention
    };

    typedef u32 ShaderCompileFlags;

    /////////////////////////////////////////////////////////////////////////////////////////////////////
    
    /**
    * @brief ShaderCompiler. Interface class
    */
    class ShaderDecoder : public ResourceDecoder
    {

    public:

        ShaderDecoder() noexcept
            : ResourceDecoder()
        {
        }
        
        explicit ShaderDecoder(const std::vector<std::string>& supportedExtensions) noexcept
            : ResourceDecoder(supportedExtensions)
        {
        }

        explicit ShaderDecoder(std::vector<std::string>&& supportedExtensions) noexcept
            : ResourceDecoder(std::move(supportedExtensions))
        {
        }

        ~ShaderDecoder() = default;

    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace resource
} //namespace v3d