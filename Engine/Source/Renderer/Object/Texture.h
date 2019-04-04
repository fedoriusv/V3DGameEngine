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
    class RenderTargetState;
    class ShaderProgram;

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * Texture base class. Game side
    */
    class Texture : public Object
    {
    public:

        Texture() = default;
        virtual ~Texture() {};
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * Texture2D class. Game side
    *\n
    * Constructor param:
    TextureUsageFlags usage
    Format format
    const Dimension2D& dimension
    u32 mipmapCount = 1
    const void* data = nullptr
    *\n
    * Constructor param:
    TextureUsageFlags usage
    Format format
    const Dimension2D& dimension
    TextureSamples samples
    */
    class Texture2D : public Texture, public utils::Observer
    {
    public:

        Texture2D() = delete;
        Texture2D(const Texture2D &) = delete;
        ~Texture2D();

        renderer::TextureTarget  getTarget() const;
        renderer::TextureSamples getSampleCount() const;
        u32                      getMipmaps() const;
        const core::Dimension2D& getDimension() const;
        renderer::Format         getFormat() const;

        void update(const core::Dimension2D& offset, const core::Dimension2D& size, u32 mipLevel, const void* data);
        void read(const core::Dimension2D& offset, const core::Dimension2D& size, u32 mipLevel, void* const data);
        void clear(const core::Vector4D& color);
        void clear(f32 depth, u32 stencil);


    private:

        Texture2D(renderer::CommandList& cmdList, TextureUsageFlags usage, renderer::Format format, const core::Dimension2D& dimension, u32 mipmapCount = 1, const void* data = nullptr) noexcept;
        Texture2D(renderer::CommandList& cmdList, TextureUsageFlags usage, renderer::Format format, const core::Dimension2D& dimension, renderer::TextureSamples samples) noexcept;

        void handleNotify(utils::Observable* ob) override;

        void createTexture2D(const void* data = nullptr);

        renderer::Image* getImage() const;

        friend CommandList;
        CommandList& m_cmdList;

        const  renderer::TextureTarget      m_target;
        const renderer::Format              m_format;
        const core::Dimension2D             m_dimension;
        const u32                           m_mipmaps;
        const u32                           m_layers;
        renderer::TextureSamples            m_samples;

        TextureUsageFlags                   m_usage;
        renderer::Image*                    m_image;

        friend ShaderProgram;
        friend RenderTargetState;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace renderer
} //namespace v3d
