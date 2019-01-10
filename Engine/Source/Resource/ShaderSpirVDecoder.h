#pragma once

#include "ResourceDecoder.h"

namespace v3d
{
namespace resource
{
    class Resource;
    struct ShaderHeader;

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////

    class ShaderSpirVDecoder final : public ResourceDecoder
    {
    public:

        ShaderSpirVDecoder(const ShaderHeader* header, bool reflections = false);
        ShaderSpirVDecoder(std::vector<std::string> supportedExtensions, const ShaderHeader* header, bool reflections);

        ~ShaderSpirVDecoder();

        Resource* decode(const stream::Stream* stream) override;

    private:

        void                    parseReflections(const std::vector<u32>& spirv, const stream::Stream* stream);

        const ShaderHeader*         m_header;
        bool                        m_reflections;

    };

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace decoders
} //namespace v3d
