#pragma once

#include "ResourceLoader.h"
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
    * @brief ShaderBinaryFileLoader class. Loader from file
    * 
    * @see ShaderSpirVDecoder
    * @see ShaderHLSLDecoder
    */
    class ShaderBinaryFileLoader : public ResourceLoader<renderer::Shader>, public ResourceDecoderRegistration
    {
    public:

        using ResourceType = renderer::Shader;
        using PolicyType = renderer::Shader::LoadPolicy;

        /**
        * @brief ShaderBinaryFileLoader constructor
        * @param const renderer::Device* device [required]
        */
        explicit ShaderBinaryFileLoader(const renderer::Device* device, ShaderCompileFlags compileFlags = 0) noexcept;

        /**
        * @brief ShaderBinaryFileLoader destructor
        */
        ~ShaderBinaryFileLoader() = default;

        /**
        * @brief Load binary shader by name from file
        * @see Shader
        * @param const std::string& name [required]
        * @param const Resource::LoadPolicy& policy [required]
        * @param u32 flags [optional]
        * @return Shader pointer
        */
        [[nodiscard]] renderer::Shader* load(const std::string& name, const Resource::LoadPolicy& policy, u32 flags = 0) override;

    private:

        ShaderBinaryFileLoader() = delete;
        ShaderBinaryFileLoader(const ShaderBinaryFileLoader&) = delete;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace resource
} //namespace v3d
