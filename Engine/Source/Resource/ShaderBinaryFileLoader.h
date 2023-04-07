#pragma once

#include "ResourceLoader.h"
#include "ResourceDecoderRegistration.h"

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
    * @brief ShaderBinaryFileLoader class. Loader from file
    * @see ShaderSpirVDecoder
    */
    class ShaderBinaryFileLoader : public ResourceLoader<renderer::Shader*>, public ResourceDecoderRegistration
    {
    public:

        enum ShaderBinaryBuildFlag
        {
            ShaderBinary_DontUseReflaction = 1 << 0,
        };
        typedef u32 ShaderBinaryBuildFlags;

        ShaderBinaryFileLoader() = delete;
        ShaderBinaryFileLoader(const ShaderBinaryFileLoader&) = delete;
        ~ShaderBinaryFileLoader() = default;

        /**
        * @brief ShaderBinaryFileLoader constructor
        * @param const renderer::Context* context [required]
        * @param const std::vector<std::pair<std::string, std::string>>& defines [optional]
        * @param ShaderBinaryBuildFlags flags [optional]
        * @see ShaderBinaryBuildFlags
        */
        explicit ShaderBinaryFileLoader(const renderer::Context* context, const std::vector<std::pair<std::string, std::string>>& defines = {}, ShaderBinaryBuildFlags flags = 0) noexcept;

        /**
        * @brief Load binary shader by name from file
        * @see Shader
        * @param const std::string& name [required]
        * @param const std::string& alias [optional]
        * @return Shader pointer
        */
        [[nodiscard]] renderer::Shader* load(const std::string& name, const std::string& alias = "") override;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace resource
} //namespace v3d
