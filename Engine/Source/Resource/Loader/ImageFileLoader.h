#pragma once

#include "ResourceLoader.h"
#include "Resource/Decoder/ImageDecoder.h"
#include "Resource/Decoder/ShaderDecoder.h"
#include "Resource/Decoder/ResourceDecoderRegistration.h"

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
        * @brief ImageFileLoader constructor
        * @param ImageLoaderFlags flags [required]
        * @see ImageLoaderFlag
        */
        explicit ImageFileLoader(ImageLoaderFlags flags = 0) noexcept;

        /**
        * @brief ImageFileLoader destructor
        */
        ~ImageFileLoader() = default;

        /**
        * @brief Load image resource by name from file
        * @param const std::string& name [required]
        * @param const std::string& alias [optional]
        * @return Image pointer
        */
        [[nodiscard]] resource::Bitmap* load(const std::string& name, const std::string& alias = "") override;

    private:

        ImageFileLoader() = delete;
        ImageFileLoader(const ImageFileLoader&) = delete;
        ImageFileLoader& operator=(const ImageFileLoader&) = delete;

        ImageDecoder::ImagePolicy   m_policy;
        ImageLoaderFlags            m_flags;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace resource
} //namespace v3d
