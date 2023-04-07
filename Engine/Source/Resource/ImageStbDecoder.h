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

    /**
    * @brief ImageStbDecoder decoder.
    * Support formats: "jpeg", "png", "bmp", "tga"
    */
    class ImageStbDecoder final : public ResourceDecoder
    {
    public:

        explicit ImageStbDecoder(std::vector<std::string> supportedExtensions, const resource::ImageHeader& header, bool readHeader, bool generateMipmaps = false) noexcept;
        ~ImageStbDecoder();

        [[nodiscard]] Resource* decode(const stream::Stream* stream, const std::string& name = "") const override;

    private:

        stream::Stream* generateMipMaps(void* baseMipmap, u32 width, u32 height, u32 componentsCount, u32 componentSize, u32 componentType, u32& mipmapsCount) const;

        const resource::ImageHeader m_header;
        bool m_readHeader;

        bool m_generateMipmaps;
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace decoders
} //namespace v3d
#endif  //USE_STB