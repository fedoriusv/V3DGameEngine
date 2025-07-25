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
        ShaderCompile_DontUseReflection = 1 << 0,

        ShaderCompile_OptimizationSize = 1 << 1,
        ShaderCompile_OptimizationPerformance = 1 << 2,
        ShaderCompile_OptimizationFull = 1 << 3,

        ShaderCompile_Patched = 1 << 4,
        ShaderCompile_UseLegacyCompilerForHLSL = 1 << 5,
        ShaderCompile_UseDXCompilerForSpirV = 1 << 6,
        ShaderCompile_ShaderModelFromExt = 1 << 7, // detect shader model by file extention
    };

    typedef u32 ShaderCompileFlags;

    /////////////////////////////////////////////////////////////////////////////////////////////////////
    
    /**
    * @brief ShaderCompiler. Interface class
    */
    class ShaderDecoder : public ResourceDecoder
    {

    public:

        struct ShaderPolicy : ResourceDecoder::Policy
        {
            renderer::ShaderType            _type = renderer::ShaderType::First;
            renderer::ShaderContent         _content = renderer::ShaderContent::Source;
            renderer::ShaderModel           _shaderModel = renderer::ShaderModel::Default;
            std::string                     _entryPoint = "main";
            renderer::Shader::DefineList    _defines = {};
            std::vector<std::string>        _includes = {};
            std::vector<std::string>        _paths = {};
        };

        explicit ShaderDecoder() noexcept
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