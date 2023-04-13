#pragma once

#include "ResourceLoader.h"
#include "ResourceDecoderRegistration.h"
#include "Renderer/ShaderProperties.h"
#include "Renderer/Shader.h"
#include "Stream/Stream.h"

namespace v3d
{
namespace renderer
{
    class Context;
    class Shader;
    struct ShaderHeader;
} //namespace renderer
namespace resource
{
    //////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief ShaderSourceStreamLoader class. Resource loader.
    * Loads a shader from source file
    * 
    * @see ShaderSpirVDecoder
    * @see ShaderHLSLDecoder
    * @see ShaderDXCDecoder
    */
    class ShaderSourceStreamLoader : public ResourceLoader<renderer::Shader*>, public ResourceDecoderRegistration
    {
    public:

        typedef renderer::ShaderCompileFlag ShaderSourceFlag;
        typedef renderer::ShaderCompileFlags ShaderSourceFlags;

        ShaderSourceStreamLoader() = delete;
        ShaderSourceStreamLoader(const ShaderSourceStreamLoader&) = delete;

        /**
        * @brief ShaderSourceStreamLoader destructor.
        */
        ~ShaderSourceStreamLoader() = default;

        /**
        * @brief ShaderSourceStreamLoader constructor.
        * @param const renderer::Context* context [required]
        * @param const renderer::ShaderHeader* header [required]
        * @param const stream::Stream* stream [required]
        * @param bool enableReflection [optional]
        */
        explicit ShaderSourceStreamLoader(const renderer::Context* context, const renderer::ShaderHeader* header, const stream::Stream* stream, 
            const std::string& entrypoint = "main", const renderer::Shader::DefineList& defines = {}, const std::vector<std::string>& includes = {}, ShaderSourceFlags flags = 0) noexcept;

        /**
        * @brief Load a Shader source from by name from stream
        * @see Shader
        * @param const std::string& name [required]
        * @param const std::string& alias [optional]
        * @return a Shader pointer
        */
        [[nodiscard]] renderer::Shader* load(const std::string& name, const std::string& alias = "") override;

    private:

        const stream::Stream* const m_stream;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace resource
} //namespace v3d
