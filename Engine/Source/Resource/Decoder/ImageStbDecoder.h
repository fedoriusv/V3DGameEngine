#pragma once

#include "ResourceDecoder.h"
#include "Resource/Bitmap.h"
#include "ImageDecoder.h"

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
    * @see: https://github.com/nothings/stb.git
    */
    class ImageStbDecoder final : public ImageDecoder
    {
    public:

        explicit ImageStbDecoder(const std::vector<std::string>& supportedExtensions) noexcept;
        explicit ImageStbDecoder(std::vector<std::string>&& supportedExtensions) noexcept;
        ~ImageStbDecoder();

        [[nodiscard]] Resource* decode(const stream::Stream* stream, const Policy* policy, u32 flags = 0, const std::string& name = "") const override;

    private:

        stream::Stream* generateMipMaps(void* baseMipmap, u32 width, u32 height, u32 componentsCount, u32 componentSize, u32 componentType, u32& mipmapsCount) const;
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace decoders
} //namespace v3d
#endif  //USE_STB