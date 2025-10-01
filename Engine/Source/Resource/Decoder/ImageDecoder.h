#pragma once

#include "Common.h"
#include "Resource/Resource.h"
#include "Renderer/Texture.h"
#include "ResourceDecoder.h"

namespace v3d
{
namespace resource
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief ImageLoaderFlag enum
    */
    enum ImageLoaderFlag : u32
    {
        ImageLoader_FlipY = 1 << 0,
        ImageLoader_GenerateMipmaps = 1 << 1,
    };
    typedef u32 ImageLoaderFlags;

    /////////////////////////////////////////////////////////////////////////////////////////////////////
    
    /**
    * @brief ImageDecoder. Interface class
    */
    class ImageDecoder : public ResourceDecoder
    {

    public:

        struct TexturePolicy : ResourceDecoder::Policy
        {
            renderer::TextureUsageFlags usage = 0;
        };
        typedef TexturePolicy PolicyType;

        explicit ImageDecoder() noexcept
            : ResourceDecoder()
        {
        }
        
        explicit ImageDecoder(const std::vector<std::string>& supportedExtensions) noexcept
            : ResourceDecoder(supportedExtensions)
        {
        }

        explicit ImageDecoder(std::vector<std::string>&& supportedExtensions) noexcept
            : ResourceDecoder(std::move(supportedExtensions))
        {
        }

        ~ImageDecoder() = default;

    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace resource
} //namespace v3d