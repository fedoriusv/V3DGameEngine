#pragma once

#include "ResourceDecoder.h"

#ifdef USE_GLI
namespace v3d
{
namespace renderer
{
    class Device;
    class Texture;
} // namespace renderer
namespace resource
{
    class Resource;

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief BitmapGLiDecoder decoder.
    * Support formats: "ktx", "kmg", "dds"
    * @see: https://github.com/g-truc/gli.git
    */
    class BitmapGLiDecoder final : public ResourceDecoder
    {
    public:

        explicit BitmapGLiDecoder(const std::vector<std::string>& supportedExtensions) noexcept;
        explicit BitmapGLiDecoder(std::vector<std::string>&& supportedExtensions) noexcept;
        ~BitmapGLiDecoder();

        [[nodiscard]] Resource* decode(const stream::Stream* stream, const resource::Resource::LoadPolicy* policy, u32 flags = 0, const std::string& name = "") const override;

    private:
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief TextureGLiDecoder decoder.
    * Support formats: "ktx", "kmg", "dds"
    * @see: https://github.com/g-truc/gli.git
    */
    class TextureGLiDecoder final : public ResourceDecoder
    {
    public:

        explicit TextureGLiDecoder(renderer::Device* device, const std::vector<std::string>& supportedExtensions) noexcept;
        explicit TextureGLiDecoder(renderer::Device* device, std::vector<std::string>&& supportedExtensions) noexcept;
        ~TextureGLiDecoder();

        [[nodiscard]] Resource* decode(const stream::Stream* stream, const resource::Resource::LoadPolicy* policy, u32 flags = 0, const std::string& name = "") const override;

    private:

        renderer::Device* const m_device;
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////

} // namespace decoders
} // namespace v3d
#endif //USE_GLI