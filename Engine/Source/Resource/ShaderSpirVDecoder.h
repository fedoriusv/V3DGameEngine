#pragma once

#include "ResourceDecoder.h"
#include "Renderer/Shader.h"

#ifdef USE_SPIRV
namespace v3d
{
namespace resource
{
    class Resource;

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief ShaderSpirVDecoder decoder.
    * Support source formats: "vert", "frag", "ps", "vs", "hlsl"
    * Support binary formats: "vspv", "fspv"
    */
    class ShaderSpirVDecoder final : public ResourceDecoder
    {
    public:

        ShaderSpirVDecoder(const renderer::ShaderHeader& header, bool reflections = false) noexcept;
        ShaderSpirVDecoder(std::vector<std::string> supportedExtensions, const renderer::ShaderHeader& header, bool reflections) noexcept;

        ~ShaderSpirVDecoder();

        Resource* decode(const stream::Stream* stream, const std::string& name = "") const override;

    private:

        bool parseReflections(const std::vector<u32>& spirv, stream::Stream* stream) const;

        const renderer::ShaderHeader m_header;
        mutable u32 m_sourceVersion;

        bool m_reflections;
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace resource
} //namespace v3d
#endif //USE_SPIRV