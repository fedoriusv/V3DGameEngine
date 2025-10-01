#pragma once

#include "ResourceLoader.h"
#include "Resource/Decoder/ImageDecoder.h"
#include "Resource/Decoder/ShaderDecoder.h"
#include "Resource/Decoder/ResourceDecoderRegistration.h"

namespace v3d
{
namespace renderer
{
    class Device;
    class Texture;
} //namespace scene
namespace resource
{
    class Bitmap;

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief BitmapFileLoader class. Loader from file
    * 
    * @see ImageStbDecoder
    * @see ImageGLiDecoder
    */
    class BitmapFileLoader : public ResourceLoader<resource::Bitmap*>, public ResourceDecoderRegistration
    {
    public:

        /**
        * @brief BitmapFileLoader constructor
        * @param ImageLoaderFlags flags [required]
        * @see ImageLoaderFlag
        */
        explicit BitmapFileLoader(ImageLoaderFlags flags = 0) noexcept;

        /**
        * @brief BitmapFileLoader destructor
        */
        ~BitmapFileLoader() = default;

        /**
        * @brief Load image resource by name from file
        * @param const std::string& name [required]
        * @param const std::string& alias [optional]
        * @return Image pointer
        */
        [[nodiscard]] resource::Bitmap* load(const std::string& name, const std::string& alias = "") override;

    private:

        BitmapFileLoader() = delete;
        BitmapFileLoader(const BitmapFileLoader&) = delete;
        BitmapFileLoader& operator=(const BitmapFileLoader&) = delete;

        ResourceDecoder::Policy m_policy;
        ImageLoaderFlags        m_flags;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief TextureFileLoader class. Loader from file
    *
    * @see ImageStbDecoder
    * @see ImageGLiDecoder
    */
    class TextureFileLoader : public ResourceLoader<renderer::Texture*>, public ResourceDecoderRegistration
    {
    public:

        using PolicyType = ImageDecoder::TexturePolicy;

        /**
        * @brief BitmapFileLoader constructor
        * @param ImageLoaderFlags flags [required]
        * @see ImageLoaderFlag
        */
        explicit TextureFileLoader(renderer::Device* device, const ImageDecoder::TexturePolicy& policy, ImageLoaderFlags flags = 0) noexcept;

        /**
        * @brief BitmapFileLoader destructor
        */
        ~TextureFileLoader() = default;

        /**
        * @brief Load image resource by name from file
        * @param const std::string& name [required]
        * @param const std::string& alias [optional]
        * @return Image pointer
        */
        [[nodiscard]] renderer::Texture* load(const std::string& name, const std::string& alias = "") override;

    private:

        TextureFileLoader() = delete;
        TextureFileLoader(const TextureFileLoader&) = delete;
        TextureFileLoader& operator=(const TextureFileLoader&) = delete;

        ImageDecoder::TexturePolicy m_policy;
        ImageLoaderFlags            m_flags;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace resource
} //namespace v3d
