#pragma once

#include "ResourceLoader.h"
#include "ResourceDecoderRegistration.h"
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
    * Loads shaders for source file
    * @see ShaderSpirVDecoder
    * @see ShaderHLSLDecoder
    */
    class ShaderSourceStreamLoader : public ResourceLoader<renderer::Shader*>, public ResourceDecoderRegistration
    {
    public:

        ShaderSourceStreamLoader() = delete;
        ShaderSourceStreamLoader(const ShaderSourceStreamLoader&) = delete;

        /**
        * @brief ShaderSourceStreamLoader constructor.
        * @param const renderer::Context* context [required]
        * @param const renderer::ShaderHeader* header [required]
        * @param const stream::Stream* stream [required]
        * @param bool enableReflection [optional]
        */
        ShaderSourceStreamLoader(const renderer::Context* context, const renderer::ShaderHeader* header, const stream::Stream* stream, bool enableReflection = true) noexcept;
        ~ShaderSourceStreamLoader();

        /**
        * @brief Load a Shader source from by name from stream
        * @see Shader
        * @param const std::string& name [required]
        * @param const std::string& alias [optional]
        * @return a Shader pointer
        */
        renderer::Shader* load(const std::string& name, const std::string& alias = "") override;

    private:

        const stream::Stream* m_stream;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace resource
} //namespace v3d
