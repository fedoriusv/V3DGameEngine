#pragma once

#include "ResourceLoader.h"
#include "ResourceDecoderRegistration.h"

namespace v3d
{
namespace resource
{
    class Bitmap;
} //namespace scene

namespace resource
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief ImageFileLoader class. Loader from file
    * 
    * @see ImageStbDecoder
    * @see ImageGLiDecoder
    */
    class ImageFileLoader : public ResourceLoader<resource::Bitmap*>, public ResourceDecoderRegistration
    {
    public:

        /**
        * @brief ImageLoaderFlag enum
        */
        enum ImageLoaderFlag : u32
        {
            ReadHeader = 1 << 0,
            FlipY = 1 << 1,
            GenerateMipmaps = 1 << 2,
        };
        typedef u32 ImageLoaderFlags;

        ImageFileLoader() = delete;
        ImageFileLoader(const ImageFileLoader&) = delete;
        ImageFileLoader& operator=(const ImageFileLoader&) = delete;

        /**
        * @brief ImageFileLoader constructor
        * @param ImageLoaderFlags flags [required]
        * @see ImageLoaderFlag
        */
        explicit ImageFileLoader(ImageLoaderFlags flags) noexcept;
        ~ImageFileLoader() = default;

        /**
        * @brief Load image resource by name from file
        * @param const std::string& name [required]
        * @param const std::string& alias [optional]
        * @return Image pointer
        */
        [[nodiscard]] resource::Bitmap* load(const std::string& name, const std::string& alias = "") override;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace resource
} //namespace v3d
