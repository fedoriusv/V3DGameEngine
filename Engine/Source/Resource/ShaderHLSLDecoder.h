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
    * Support source formats: "vs", "ps", "hlsl".
    * Support shader model 5.0, 5.1
    */
    class ShaderHLSLDecoder final : public ResourceDecoder
    {
    public:

        explicit ShaderHLSLDecoder(const renderer::ShaderHeader& header, bool reflections = true) noexcept;
        explicit ShaderHLSLDecoder(std::vector<std::string> supportedExtensions, const renderer::ShaderHeader& header, bool reflections = true) noexcept;
        ~ShaderHLSLDecoder() = default;

        [[nodiscard]] Resource* decode(const stream::Stream* stream, const std::string& name = "") const override;

    private:

        const renderer::ShaderHeader m_header;
        const bool m_reflections;

        mutable u32 m_version;
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace resource
} //namespace v3d
#endif //D3D_RENDER