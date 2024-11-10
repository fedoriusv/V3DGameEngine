#pragma once

#include "ResourceLoader.h"
#include "ResourceDecoderRegistration.h"
#include "Renderer/Shader.h"
#include "ShaderDecoder.h"

namespace v3d
{
namespace renderer
{
    class Device;
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

        /**
        * @brief ShaderSourceFileLoader constructor.
        * @param const renderer::Context* context [required]
        * @param const std::string& entrypoint [optional]
        * @param const renderer::Shader::DefineList& defines [optional]
        * @param const std::vector<std::string>& includes [optional]
        * @param ShaderCompileFlags flags [optional] @see ShaderCompileFlags
        */
        explicit ShaderSourceFileLoader(renderer::Device* device, const ShaderDecoder::ShaderPolicy& policy, ShaderCompileFlags flags = 0) noexcept;

        /**
        * @brief ShaderSourceFileLoader constructor.
        * Supports HLSL format only
        * @param const renderer::Context* context [required]
        * @param renderer::ShaderType type [required]
        * @param const std::string& entryPoint [optional]
        * @param const std::vector<std::pair<std::string, std::string>>& defines [optional]
        * @param ShaderCompileFlags flags [optional] @see ShaderCompileFlags
        */
        explicit ShaderSourceFileLoader(renderer::Device* device, renderer::ShaderType type, const std::string& entryPoint = "main",
             const std::vector<std::pair<std::string, std::string>>& defines = {}, const std::vector<std::string>& includes = {}, ShaderCompileFlags flags = 0) noexcept;

        /**
        * @brief ShaderSourceFileLoader destructor
        */
        ~ShaderSourceFileLoader() = default;

        /**
        * @brief Load a Shader source by name from file
        * @see Shader
        * @param const std::string& name [required]
        * @param const std::string& alias [optional]
        * @return a Shader pointer
        */
        [[nodiscard]] renderer::Shader* load(const std::string& name, const std::string& alias = "") override;

    private:

        ShaderSourceFileLoader() = delete;
        ShaderSourceFileLoader(const ShaderSourceFileLoader&) = delete;

        ShaderDecoder::ShaderPolicy m_policy;
        ShaderCompileFlags          m_flags;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace resource
} //namespace v3d
