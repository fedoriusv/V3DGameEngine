#pragma once

#include "ResourceDecoder.h"
#include "Shader.h"

namespace v3d
{
namespace resource
{
    class Resource;

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////

    class ShaderSpirVDecoder final : public ResourceDecoder
    {
    public:

        ShaderSpirVDecoder(const ShaderHeader& header, bool reflections = false);
        ShaderSpirVDecoder(std::vector<std::string> supportedExtensions, const ShaderHeader& header, bool reflections);

        ~ShaderSpirVDecoder();

        Resource* decode(const stream::Stream* stream, const std::string& name = "") override;

    private:

        bool parseReflections(const std::vector<u32>& spirv);

        const ShaderHeader   m_header;
        bool                 m_reflections;
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace decoders
} //namespace v3d
