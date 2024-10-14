#pragma once

#include "Stream/Stream.h"
#include "Renderer/Shader.h"
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

        /////////////////////////////////////////////////////////////////////////////////////////////////////

        /**
        * @brief ShaderSourceStreamLoader constructor.
        * @param const renderer::Device* device [required]
        * @param const stream::Stream* stream [required]
        * @param const renderer::ShaderPolicy* policy [required]
        * @param bool ShaderCompileFlags flags [optional]
        */
        explicit ShaderSourceStreamLoader(const renderer::Device* device, const stream::Stream* stream, const ShaderDecoder::ShaderPolicy& policy, ShaderCompileFlags flags = 0) noexcept;

        /**
        * @brief Load a Shader source from by name from stream
        * @see Shader
        * @param const std::string& name [required]
        * @param const std::string& alias [optional]
        * @return a Shader pointer
        */
        [[nodiscard]] renderer::Shader* load(const std::string& name, const std::string& alias = "") override;

        /**
        * @brief ShaderSourceStreamLoader destructor.
        */
        ~ShaderSourceStreamLoader() = default;

    private:

        ShaderSourceStreamLoader() = delete;
        ShaderSourceStreamLoader(const ShaderSourceStreamLoader&) = delete;

        const stream::Stream* const         m_stream;
        const ShaderDecoder::ShaderPolicy   m_policy;
        ShaderCompileFlags                  m_flags;

    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace resource
} //namespace v3d
