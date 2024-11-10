#pragma once

#include "ResourceDecoder.h"
#include "Resource/Bitmap.h"
#include "Resource/ImageDecoder.h"

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
    class ImageGLiDecoder final : public ImageDecoder
    {
    public:

        explicit ImageGLiDecoder(const std::vector<std::string>& supportedExtensions) noexcept;
        explicit ImageGLiDecoder(std::vector<std::string>&& supportedExtensions) noexcept;
        ~ImageGLiDecoder();

        [[nodiscard]] Resource* decode(const stream::Stream* stream, const Policy* policy, u32 flags = 0, const std::string& name = "") const override;
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace decoders
} //namespace v3d
#endif //USE_GLI