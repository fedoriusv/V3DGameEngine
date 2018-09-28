#pragma once

#include "Common.h"
#include "CommandList.h"
#include "ImageFormats.h"
#include "Utils/Observable.h"
#include "TextureEnums.h"
#include "Object.h"

namespace v3d
{
namespace renderer
{
    class Image;
} //namespace renderer

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    class Texture2D : public Object, public utils::Observer //ref couter, 
    {
    public:

        ~Texture2D();
        Texture2D(const Texture2D &) = delete;

        TextureTarget               getTarget() const;
        TextureFilter               getMinFilter() const;
        TextureFilter               getMagFilter() const;
        TextureWrap                 getWrap() const;
        TextureAnisotropic          getAnisotropic() const;
        u32                         getMipmaps() const;
        const core::Dimension2D&    getDimension() const;
        renderer::ImageFormat       getFormat() const;

        void update(const core::Dimension2D& offset, const core::Dimension2D& size, u32 mipLevel, const void* data);
        void read(const core::Dimension2D& offset, const core::Dimension2D& size, u32 mipLevel, void* const data);
        void clear(const core::Vector4D& color);
        void clear(f32 depth, u32 stencil);


    private:

        Texture2D(renderer::CommandList& cmdList, renderer::ImageFormat format, const core::Dimension2D& dimension, u32 mipmapCount = 0, const void* data = nullptr);
        Texture2D(renderer::CommandList& cmdList, renderer::ImageFormat format, const core::Dimension2D& dimension, RenderTargetSamples samples);

        void handleNotify() override;

        void createTexture2D(const void* data = nullptr);

        renderer::CommandList&      m_cmdList;

        const TextureTarget         m_target;
        const renderer::ImageFormat m_format;
        const core::Dimension2D     m_dimension;
        const u32                   m_mipmapLevel;
        RenderTargetSamples         m_samples;

        s16                         m_filter;
        TextureAnisotropic          m_anisotropicLevel;
        TextureWrap                 m_wrap;

        renderer::Image*            m_image;

        friend renderer::CommandList;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    class SwapchainTexture : public Object
    {
    public:

        ~SwapchainTexture() {};
        SwapchainTexture(const SwapchainTexture &) = delete;

        void clear(const core::Vector4D& color);

    private:

        SwapchainTexture(renderer::CommandList& cmdList)
            : m_cmdList(cmdList)
        {
        }

        renderer::CommandList& m_cmdList;

        friend renderer::CommandList;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace v3d
