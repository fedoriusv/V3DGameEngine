#pragma once

#include "ResourceDecoder.h"
#include "Resource/Image.h"

#ifdef USE_GLI
namespace v3d
{
namespace resource
{
    class Resource;

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * ImageGLiDecoder decoder. Support formats: "ktx", "kmg", "dds"
    */
    class ImageGLiDecoder final : public ResourceDecoder
    {
    public:

        ImageGLiDecoder(std::vector<std::string> supportedExtensions, const resource::ImageHeader& header, bool readHeader) noexcept;
        ~ImageGLiDecoder();

        Resource* decode(const stream::Stream* stream, const std::string& name = "") override;

    private:

        const resource::ImageHeader m_header;
        bool m_readHeader;
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace decoders
} //namespace v3d
#endif //USE_GLI