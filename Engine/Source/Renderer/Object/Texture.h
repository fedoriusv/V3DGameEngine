#pragma once

#include "Common.h"
#include "Object.h"
#include "Utils/Observable.h"
#include "Renderer/Formats.h"
#include "Renderer/TextureProperties.h"
#include "Renderer/CommandList.h"


namespace v3d
{
namespace renderer
{
    class Image;
    class RenderTarget;

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * Texture2D class. Game side
    */
    class Texture2D : public Object, public utils::Observer //ref counter, 
    {
    public:

        Texture2D() = delete;
        Texture2D(const Texture2D &) = delete;
        ~Texture2D();

        renderer::TextureTarget         getTarget() const;
        renderer::TextureFilter         getMinFilter() const;
        renderer::TextureFilter         getMagFilter() const;
        renderer::TextureWrap           getWrap() const;
        renderer::TextureAnisotropic    getAnisotropic() const;
        renderer::TextureSamples        getSampleCount() const;
        u32                             getMipmaps() const;
        const core::Dimension2D&        getDimension() const;
        renderer::Format                getFormat() const;

        void update(const core::Dimension2D& offset, const core::Dimension2D& size, u32 mipLevel, const void* data);
        void read(const core::Dimension2D& offset, const core::Dimension2D& size, u32 mipLevel, void* const data);
        void clear(const core::Vector4D& color);
        void clear(f32 depth, u32 stencil);


    private:

        Texture2D(renderer::CommandList& cmdList, renderer::Format format, const core::Dimension2D& dimension, u32 mipmapCount = 0, const void* data = nullptr) noexcept;
        Texture2D(renderer::CommandList& cmdList, renderer::Format format, const core::Dimension2D& dimension, renderer::TextureSamples samples) noexcept;

        void handleNotify(utils::Observable* ob) override;

        void createTexture2D(const void* data = nullptr);

        renderer::Image* getImage() const;

        friend renderer::CommandList;
        renderer::CommandList&              m_cmdList;

        const  renderer::TextureTarget      m_target;
        const renderer::Format              m_format;
        const core::Dimension2D             m_dimension;
        const u32                           m_mipmapLevel;
        renderer::TextureSamples            m_samples;

        s16                                 m_filter;
        renderer::TextureAnisotropic        m_anisotropicLevel;
        renderer::TextureWrap               m_wrap;

        renderer::Image*                    m_image;

        friend RenderTarget;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * SwapchainTexture class
    */
    class SwapchainTexture : public Object
    {
    public:

        ~SwapchainTexture() {};
        SwapchainTexture(const SwapchainTexture &) = delete;

        void clear(const core::Vector4D& color);

    private:

        SwapchainTexture(renderer::CommandList& cmdList) noexcept
            : m_cmdList(cmdList)
        {
        }

        renderer::CommandList& m_cmdList;

        friend renderer::CommandList;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace renderer
} //namespace v3d