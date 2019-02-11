#pragma once

#include "ResourceDecoder.h"
#include "Renderer/Shader.h"

namespace v3d
{
namespace resource
{
    class Resource;

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////

    class ShaderSpirVDecoder final : public ResourceDecoder
    {
    public:

        ShaderSpirVDecoder(const renderer::ShaderHeader& header, bool reflections = false) noexcept;
        ShaderSpirVDecoder(std::vector<std::string> supportedExtensions, const renderer::ShaderHeader& header, bool reflections) noexcept;

        ~ShaderSpirVDecoder();

        Resource* decode(const stream::Stream* stream, const std::string& name = "") override;

    private:

        bool parseReflections(const std::vector<u32>& spirv, stream::Stream* stream);

        u32 m_sourceVersion;

        const renderer::ShaderHeader m_header;
        bool m_reflections;
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace decoders
} //namespace v3d
