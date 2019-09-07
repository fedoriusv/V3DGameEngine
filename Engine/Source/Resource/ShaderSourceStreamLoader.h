#pragma once

#include "ResourceLoader.h"
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
    * ShaderSourceStreamLoader class. Loader from stream
    */
    class ShaderSourceStreamLoader : public ResourceLoader<renderer::Shader*>
    {
    public:

        ShaderSourceStreamLoader(const renderer::Context* context, const renderer::ShaderHeader* header, const stream::Stream* stream, bool enableReflection = true) noexcept;
        ~ShaderSourceStreamLoader();

        renderer::Shader* load(const std::string& name, const std::string& alias = "") override;

    private:

        const stream::Stream* m_stream;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace resource
} //namespace v3d
