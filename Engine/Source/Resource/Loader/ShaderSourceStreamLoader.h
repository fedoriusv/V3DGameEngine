#pragma once

#include "Stream/Stream.h"
#include "Renderer/Shader.h"
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
    //////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief ShaderSourceStreamLoader class. Resource loader.
    * Loads a shader from source file
    * 
    * @see ShaderSpirVDecoder
    * @see ShaderHLSLDecoder
    * @see ShaderDXCDecoder
    */
    class ShaderSourceStreamLoader : public ResourceLoader<renderer::Shader>, public ResourceDecoderRegistration
    {
    public:

        using ResourceType = renderer::Shader;
        using PolicyType = renderer::Shader::LoadPolicy;

        /**
        * @brief ShaderSourceStreamLoader constructor.
        * @param const renderer::Device* device [required]
        * @param const stream::Stream* stream [required]
        * @param bool ShaderCompileFlags flags [optional]
        */
        explicit ShaderSourceStreamLoader(renderer::Device* device, const stream::Stream* stream, ShaderCompileFlags flags = 0) noexcept;

        /**
        * @brief Load a Shader source from by name from stream
        * @see Shader
        * @param const std::string& name [required]
        * @param const std::string& alias [optional]
        * @return a Shader pointer
        */
        [[nodiscard]] renderer::Shader* load(const std::string& name, const Resource::LoadPolicy& policy, ShaderCompileFlags flags = 0) override;

        /**
        * @brief ShaderSourceStreamLoader destructor.
        */
        ~ShaderSourceStreamLoader() = default;

    private:

        ShaderSourceStreamLoader() = delete;
        ShaderSourceStreamLoader(const ShaderSourceStreamLoader&) = delete;

        const stream::Stream* const m_stream;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace resource
} //namespace v3d
