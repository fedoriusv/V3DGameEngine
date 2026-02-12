#pragma once

#include "ResourceLoader.h"
#include "Renderer/Shader.h"
#include "Resource/Decoder/ShaderDecoder.h"
#include "Resource/Decoder/ResourceDecoderRegistration.h"

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
    class ShaderSourceFileLoader : public ResourceLoader<renderer::Shader>, public ResourceDecoderRegistration
    {
    public:

        using ResourceType = renderer::Shader;
        using PolicyType = renderer::Shader::LoadPolicy;

        /**
        * @brief ShaderSourceFileLoader constructor.
        * @param renderer::Device* device [required]
        * @param ShaderCompileFlags compileFlags [optional]
        */
        explicit ShaderSourceFileLoader(renderer::Device* device, ShaderCompileFlags compileFlags = 0) noexcept;

        /**
        * @brief ShaderSourceFileLoader destructor
        */
        ~ShaderSourceFileLoader() = default;

        /**
        * @brief Load a Shader source by name from file
        * @see Shader
        * @param const std::string& name [required]
        * @param const Resource::LoadPolicy& policy [required]
        * @param u32 flags [optional]
        * @return a Shader pointer
        */
        [[nodiscard]] renderer::Shader* load(const std::string& name, const Resource::LoadPolicy& policy, u32 flags = 0) override;

    private:

        ShaderSourceFileLoader() = delete;
        ShaderSourceFileLoader(const ShaderSourceFileLoader&) = delete;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace resource
} //namespace v3d
