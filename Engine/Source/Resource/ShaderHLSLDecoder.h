#pragma once

#include "ResourceDecoder.h"
#include "Renderer/Shader.h"

#ifdef D3D_RENDER
namespace v3d
{
namespace resource
{
    class Resource;

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////

    class ShaderHLSLDecoder final : public ResourceDecoder
    {
    public:

        ShaderHLSLDecoder(const renderer::ShaderHeader& header, bool reflections = false) noexcept;
        ShaderHLSLDecoder(std::vector<std::string> supportedExtensions, const renderer::ShaderHeader& header, bool reflections = false) noexcept;

        ~ShaderHLSLDecoder();

        Resource* decode(const stream::Stream* stream, const std::string& name = "") override;

    private:

        const renderer::ShaderHeader m_header;
        const bool m_reflections;
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace decoders
} //namespace v3d
#endif //D3D_RENDER