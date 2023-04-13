#pragma once

#include "ResourceLoader.h"
#include "ResourceDecoderRegistration.h"
#include "Renderer/ShaderProperties.h"
#include "Renderer/Shader.h"

namespace v3d
{
namespace renderer
{
    class Context;
    class Shader;
} //namespace renderer
namespace resource
{

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief ShaderSourceFileLoader class. Resource loader.
    * Loads shaders for source file
    * 
    * @see ShaderSpirVDecoder
    * @see ShaderHLSLDecoder
    * @see ShaderDXCDecoder
    */
    class ShaderSourceFileLoader : public ResourceLoader<renderer::Shader*>, public ResourceDecoderRegistration
    {
    public:

        typedef renderer::ShaderCompileFlag ShaderSourceFlag;
        typedef renderer::ShaderCompileFlags ShaderSourceFlags;

        ShaderSourceFileLoader() = delete;
        ShaderSourceFileLoader(const ShaderSourceFileLoader&) = delete;

        ~ShaderSourceFileLoader() = default;

        /**
        * @brief ShaderSourceFileLoader constructor.
        * @param const renderer::Context* context [required]
        * @param const std::string& entrypoint [optional]
        * @param const renderer::Shader::DefineList& defines [optional]
        * @param const std::vector<std::string>& includes [optional]
        * @param ShaderCompileFlags flags [optional] @see ShaderCompileFlags
        */
        explicit ShaderSourceFileLoader(const renderer::Context* context, const std::string& entrypoint = "main", const renderer::Shader::DefineList& defines = {},
            const std::vector<std::string>& includes = {}, ShaderSourceFlags flags = 0) noexcept;

        /**
        * @brief ShaderSourceFileLoader constructor.
        * Supports HLSL format only
        * @param const renderer::Context* context [required]
        * @param renderer::ShaderType type [required]
        * @param const std::string& entryPoint [optional]
        * @param const std::vector<std::pair<std::string, std::string>>& defines [optional]
        * @param ShaderCompileFlags flags [optional] @see ShaderCompileFlags
        */
        explicit ShaderSourceFileLoader(const renderer::Context* context, renderer::ShaderType type, const std::string& entryPoint = "main",
             const std::vector<std::pair<std::string, std::string>>& defines = {}, const std::vector<std::string>& includes = {}, ShaderSourceFlags flags = 0) noexcept;

        /**
        * @brief Load a Shader source by name from file
        * @see Shader
        * @param const std::string& name [required]
        * @param const std::string& alias [optional]
        * @return a Shader pointer
        */
        [[nodiscard]] renderer::Shader* load(const std::string& name, const std::string& alias = "") override;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace resource
} //namespace v3d
