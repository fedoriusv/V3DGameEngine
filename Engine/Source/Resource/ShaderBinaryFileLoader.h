#pragma once

#include "ResourceLoader.h"
#include "ShaderDecoder.h"
#include "ResourceDecoderRegistration.h"

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
    class ShaderBinaryFileLoader : public ResourceLoader<renderer::Shader*>, public ResourceDecoderRegistration
    {
    public:

        /**
        * @brief ShaderBinaryFileLoader constructor
        * @param const renderer::Device* device [required]
        * @param ShaderBinaryBuildFlags flags [optional]
        * @see ShaderBinaryBuildFlags
        */
        explicit ShaderBinaryFileLoader(const renderer::Device* device, ShaderCompileFlags flags = 0) noexcept;

        /**
        * @brief ShaderBinaryFileLoader constructor
        * @param const renderer::Device* device [required]
        * @param ShaderDecoder::ShaderPolicy& policy [required]
        * @param ShaderBinaryBuildFlags flags [optional]
        * @see ShaderBinaryBuildFlags
        */
        explicit ShaderBinaryFileLoader(const renderer::Device* device, const ShaderDecoder::ShaderPolicy& policy, ShaderCompileFlags flags = 0) noexcept;

        /**
        * @brief ShaderBinaryFileLoader destructor
        */
        ~ShaderBinaryFileLoader() = default;

        /**
        * @brief Load binary shader by name from file
        * @see Shader
        * @param const std::string& name [required]
        * @param const std::string& alias [optional]
        * @return Shader pointer
        */
        [[nodiscard]] renderer::Shader* load(const std::string& name, const std::string& alias = "") override;

    private:

        ShaderBinaryFileLoader() = delete;
        ShaderBinaryFileLoader(const ShaderBinaryFileLoader&) = delete;

        ShaderDecoder::ShaderPolicy m_policy;
        ShaderCompileFlags          m_flags;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace resource
} //namespace v3d
