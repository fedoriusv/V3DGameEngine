#pragma once

#include "ResourceDecoder.h"
#include "Resource/Image.h"

#ifdef USE_STB
namespace v3d
{
namespace resource
{
    class Resource;

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////

    class ImageStbDecoder final : public ResourceDecoder
    {
    public:

        ImageStbDecoder(std::vector<std::string> supportedExtensions, const resource::ImageHeader& header, bool readHeader) noexcept;
        ~ImageStbDecoder();

        Resource* decode(const stream::Stream* stream, const std::string& name = "") override;

    private:

        const resource::ImageHeader m_header;
        bool m_readHeader;
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace decoders
} //namespace v3d
#endif  //USE_STB
