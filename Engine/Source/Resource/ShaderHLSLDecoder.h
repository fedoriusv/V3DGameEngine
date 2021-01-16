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

    /**
    * @brief ShaderHLSLDecoder decoder.
    * Support source formats: "vs", "ps", "hlsl"
    */
    class ShaderHLSLDecoder final : public ResourceDecoder
    {
    public:

        ShaderHLSLDecoder(const renderer::ShaderHeader& header, bool reflections = true) noexcept;
        ShaderHLSLDecoder(std::vector<std::string> supportedExtensions, const renderer::ShaderHeader& header, bool reflections = true) noexcept;

        ~ShaderHLSLDecoder();

        Resource* decode(const stream::Stream* stream, const std::string& name = "") const override;

    private:

        const renderer::ShaderHeader m_header;
        const bool m_reflections;

        mutable u32 m_version;
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace decoders
} //namespace v3d
#endif //D3D_RENDER