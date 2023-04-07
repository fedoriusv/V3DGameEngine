#pragma once

#include "ResourceLoader.h"
#include "ResourceDecoderRegistration.h"

namespace v3d
{
namespace resource
{
    class Image;
} //namespace scene

namespace resource
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief ImageFileLoader class. Loader from file
    * @see ImageStbDecoder
    * @see ImageGLiDecoder
    */
    class ImageFileLoader : public ResourceLoader<resource::Image*>, public ResourceDecoderRegistration
    {
    public:

        /**
        * @brief ImageLoaderFlag enum
        */
        enum ImageLoaderFlag : u32
        {
            ImageLoaderFlag_FlipY = 1 << 0,
            ImageLoaderFlag_GenerateMipmaps = 1 << 1,
        };
        typedef u32 ImageLoaderFlags;

        ImageFileLoader() = delete;
        ImageFileLoader(const ImageFileLoader&) = delete;
        ~ImageFileLoader() = default;

        /**
        * @brief ImageFileLoader constructor
        * @param ImageLoaderFlags flags [required]
        * @see ImageLoaderFlag
        */
        explicit ImageFileLoader(ImageLoaderFlags flags) noexcept;

        /**
        * @brief Load image resource by name from file
        * @param const std::string& name [required]
        * @param const std::string& alias [optional]
        * @return Image pointer
        */
        [[nodiscard]] resource::Image* load(const std::string& name, const std::string& alias = "") override;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace resource
} //namespace v3d
