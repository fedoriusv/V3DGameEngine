#pragma once

#include "ResourceLoader.h"
#include "Resource/Decoder/ShaderDecoder.h"
#include "Resource/Decoder/ResourceDecoderRegistration.h"
#include "Resource/Bitmap.h"
#include "Renderer/Texture.h"

namespace v3d
{
namespace resource
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief BitmapFileLoader class. Loader from file
    * 
    * @see ImageStbDecoder
    * @see ImageGLiDecoder
    */
    class BitmapFileLoader : public ResourceLoader<resource::Bitmap>, public ResourceDecoderRegistration
    {
    public:

        using ResourceType = resource::Bitmap;
        using PolicyType = Bitmap::LoadPolicy;

        /**
        * @brief BitmapFileLoader constructor
        */
        explicit BitmapFileLoader() noexcept;

        /**
        * @brief BitmapFileLoader destructor
        */
        ~BitmapFileLoader() = default;

        /**
        * @brief Load bitmap resource to CPU by name from file
        * @param const std::string& name [required]
        * @param const PolicyType& policy [required]
        * @param u32 flags [optional]
        * @return Bitmap pointer
        */
        [[nodiscard]] resource::Bitmap* load(const std::string& name, const Resource::LoadPolicy& policy, u32 flags = 0) override;

    private:

        BitmapFileLoader(const BitmapFileLoader&) = delete;
        BitmapFileLoader& operator=(const BitmapFileLoader&) = delete;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief TextureFileLoader class. Loader from file
    *
    * @see ImageStbDecoder
    * @see ImageGLiDecoder
    */
    class TextureFileLoader : public ResourceLoader<renderer::Texture>, public ResourceDecoderRegistration
    {
    public:

        using ResourceType = renderer::Texture;
        using PolicyType = renderer::Texture::LoadPolicy;

        /**
        * @brief TextureFileLoader constructor
        * @param renderer::Device* device [required]
        */
        explicit TextureFileLoader(renderer::Device* device) noexcept;

        /**
        * @brief TextureFileLoader destructor
        */
        ~TextureFileLoader() = default;

        /**
        * @brief Load texture resource to GPU  by name from file
        * @param const std::string& name [required]
        * @param const PolicyType& policy [required]
        * @param  u32 flags [optional]
        * @return Texture pointer
        */
        [[nodiscard]] renderer::Texture* load(const std::string& name, const Resource::LoadPolicy& policy, u32 flags = 0) override;

    private:

        TextureFileLoader(const TextureFileLoader&) = delete;
        TextureFileLoader& operator=(const TextureFileLoader&) = delete;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace resource
} //namespace v3d
