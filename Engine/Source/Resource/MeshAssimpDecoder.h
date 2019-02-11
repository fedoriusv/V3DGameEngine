#pragma once

#include "ResourceDecoder.h"
#include "Scene/Model.h"

namespace v3d
{
namespace resource
{
    class Resource;

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////

    class MeshAssimpDecoder final : public ResourceDecoder
    {
    public:

        MeshAssimpDecoder(std::vector<std::string> supportedExtensions, const scene::ModleHeader& header) noexcept;
        ~MeshAssimpDecoder();

        Resource* decode(const stream::Stream* stream, const std::string& name = "") override;

    private:

        const scene::ModleHeader m_header;
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace decoders
} //namespace v3d
