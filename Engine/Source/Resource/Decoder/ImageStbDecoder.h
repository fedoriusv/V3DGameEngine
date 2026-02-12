#pragma once

#include "ResourceDecoder.h"
#include "Resource/Bitmap.h"

#ifdef USE_STB
namespace v3d
{
namespace renderer
{
    class Device;
} // namespace renderer
namespace resource
{
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief BitmapStbDecoder decoder.
    * Support formats: "jpeg", "png", "bmp", "tga"
    * @see: https://github.com/nothings/stb.git
    */
    class BitmapStbDecoder final : public ResourceDecoder
    {
    public:

        explicit BitmapStbDecoder(const std::vector<std::string>& supportedExtensions) noexcept;
        explicit BitmapStbDecoder(std::vector<std::string>&& supportedExtensions) noexcept;
        ~BitmapStbDecoder();

        [[nodiscard]] Resource* decode(const stream::Stream* stream, const resource::Resource::LoadPolicy* policy, u32 flags = 0, const std::string& name = "") const override;
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief TextureStbDecoder decoder.
    * Support formats: "jpeg", "png", "bmp", "tga"
    * @see: https://github.com/nothings/stb.git
    */
    class TextureStbDecoder final : public ResourceDecoder
    {
    public:

        explicit TextureStbDecoder(renderer::Device* device, const std::vector<std::string>& supportedExtensions) noexcept;
        explicit TextureStbDecoder(renderer::Device* device, std::vector<std::string>&& supportedExtensions) noexcept;
        ~TextureStbDecoder();

        [[nodiscard]] Resource* decode(const stream::Stream* stream, const resource::Resource::LoadPolicy* policy, u32 flags = 0, const std::string& name = "") const override;

    private:

        renderer::Device* const m_device;
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace decoders
} //namespace v3d
#endif  //USE_STB