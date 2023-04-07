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
    * @brief ImageGLiDecoder decoder.
    * Support formats: "ktx", "kmg", "dds"
    * @see: https://github.com/g-truc/gli.git
    */
    class ImageGLiDecoder final : public ResourceDecoder
    {
    public:

        explicit ImageGLiDecoder(std::vector<std::string> supportedExtensions, const resource::ImageHeader& header, bool readHeader, u32 flags = 0) noexcept;
        ~ImageGLiDecoder();

        [[nodiscard]] Resource* decode(const stream::Stream* stream, const std::string& name = "") const override;

    private:

        const resource::ImageHeader m_header;
        bool m_readHeader;

        bool m_generateMipmaps;
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace decoders
} //namespace v3d
#endif //USE_GLI