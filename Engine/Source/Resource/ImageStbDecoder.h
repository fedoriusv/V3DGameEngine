#pragma once

#include "ResourceDecoder.h"
#include "Resource/Image.h"

namespace v3d
{
namespace resource
{
    class Resource;

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////

    class ImageStbDecoder final : public ResourceDecoder
    {
    public:

        ImageStbDecoder(std::vector<std::string> supportedExtensions, const resource::ImageHeader& header) noexcept;
        ~ImageStbDecoder();

        Resource* decode(const stream::Stream* stream, const std::string& name = "") override;

    private:

        const resource::ImageHeader m_header;
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace decoders
} //namespace v3d
