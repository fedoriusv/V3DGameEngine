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
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    class Image;
    class RenderTargetState;
    class ShaderProgram;

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief Texture base class. Game side
    */
    class Texture : public Object, public utils::Observer
    {
    public:

        TextureTarget   getTarget() const;
        Format          getFormat() const;
        TextureSamples  getSamples() const;
        u32             getLayersCount() const;
        u32             getMipmapsCount() const;

    protected:

        explicit Texture(CommandList& cmdList, TextureTarget target, Format format, TextureSamples samples, u32 layers, u32 mipmaps, TextureUsageFlags usage) noexcept;
        virtual ~Texture();

        Image* getImage() const;

        void handleNotify(utils::Observable* ob) override;
        bool isTextureUsageFlagsContains(TextureUsageFlags usage) const;

        CommandList&        m_cmdList;

        const TextureTarget m_target;
        const Format        m_format;
        TextureSamples      m_samples;
        const u32           m_layers;
        const u32           m_mipmaps;

        TextureUsageFlags   m_usage;
        Image*              m_image;

        friend RenderTargetState;
        friend ShaderProgram;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief Texture2D class. Game side
    */
    class Texture2D final : public Texture
    {
    public:

        Texture2D() = delete;
        Texture2D(const Texture2D&) = delete;
        ~Texture2D();

        const core::Dimension2D& getDimension() const;

        void update(const core::Dimension2D& offset, const core::Dimension2D& size, u32 mipLevel, const void* data);
        void read(const core::Dimension2D& offset, const core::Dimension2D& size, u32 mipLevel, void* const data);
        void clear(const core::Vector4D& color);
        void clear(f32 depth, u32 stencil);

    private:

        /**
        * @brief Texture2D constructor. Used for creating textures with mipmaps and layers.
        * Private method. Use createObject interface inside CommandList class to call.
        * 
        * @param TextureUsageFlags usage [required]
        * @param Format format [required]
        * @param const Dimension2D& dimension [required]
        * @param u32 mipmaps [optional]
        * @param const void* data [optional]
        * @param const std::string& name [optional]
        */
        Texture2D(CommandList& cmdList, TextureUsageFlags usage, Format format, const core::Dimension2D& dimension, u32 layers = 1, u32 mipmaps = 1, const void* data = nullptr, [[maybe_unused]] const std::string& name = "") noexcept;

        /**
        * @brief Texture2D constructor. Used for creating attachments
        * Private method. Use createObject interface inside CommandList class to call.
        *
        * @param TextureUsageFlags usage [required]
        * @param Format format [required]
        * @param const Dimension2D& dimension [required]
        * @param u32 mipmaps [optional]
        * @param const void* data [optional]
        * @param const std::string& name [optional]
        */
        Texture2D(CommandList& cmdList, TextureUsageFlags usage, Format format, const core::Dimension2D& dimension, TextureSamples samples, [[maybe_unused]] const std::string& name = "") noexcept;

        void createTexture2D(const void* data = nullptr);

        const core::Dimension2D m_dimension;

        friend CommandList;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief TextureCube class. Game side
    */
    class TextureCube : public Texture
    {
    public:

        TextureCube() = delete;
        TextureCube(const TextureCube&) = delete;
        ~TextureCube();

        /**
        * @brief TextureCube constructor. Used for creating cubemap attachments
        * Private method. Use createObject interface inside CommandList class to call.
        *
        * @param TextureUsageFlags usage [required]
        * @param Format format [required]
        * @param const Dimension2D& dimension [required]
        * @param TextureSamples samples [optional]
        * @param const std::string& name [optional]
        */
        TextureCube(CommandList& cmdList, TextureUsageFlags usage, Format format, const core::Dimension2D& dimension, TextureSamples samples, [[maybe_unused]] const std::string& name = "") noexcept;

    private:

        void createTextureCube();

        const core::Dimension2D m_dimension;

        friend CommandList;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace renderer
} //namespace v3d
