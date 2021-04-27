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

        /**
        * @brief getTarget method
        * @return TextureTarget of texture
        */
        TextureTarget getTarget() const;

        /**
        * @brief getFormat method
        * @return Format of texture
        */
        Format getFormat() const;

        /**
        * @brief getSamples method
        * @return TextureSamples of texture
        */
        TextureSamples getSamples() const;

        /**
        * @brief getLayersCount method
        * @return count layers of texture
        */
        u32 getLayersCount() const;

        /**
        * @brief getMipmapsCount method
        * @return count mipmaps of texture
        */
        u32 getMipmapsCount() const;

    protected:

        explicit Texture(CommandList& cmdList, TextureTarget target, Format format, TextureSamples samples, u32 layers, u32 mipmaps, TextureUsageFlags usage) noexcept;
        virtual ~Texture();

        Image* getImage() const;

        bool isBackbuffer() const;

        void handleNotify(const utils::Observable* ob) override;
        bool isTextureUsageFlagsContains(TextureUsageFlags usage) const;

        CommandList&        m_cmdList;

        const TextureTarget m_target;
        const Format        m_format;
        TextureSamples      m_samples;
        const u32           m_layers;
        const u32           m_mipmaps;

        TextureUsageFlags   m_usage;
        Image*              m_image;

        bool                m_backbuffer;

        friend CommandList;
        friend RenderTargetState;
        friend ShaderProgram;

        void createTexture(const core::Dimension3D& dimension, const void* data);
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

        /**
        * @brief getDimension method.
        * @return const core::Dimension2D& dimention of texture
        */
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
        explicit Texture2D(CommandList& cmdList, TextureUsageFlags usage, Format format, const core::Dimension2D& dimension, u32 mipmaps = 1, const void* data = nullptr, [[maybe_unused]] const std::string& name = "") noexcept;

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
        explicit Texture2D(CommandList& cmdList, TextureUsageFlags usage, Format format, const core::Dimension2D& dimension, TextureSamples samples, [[maybe_unused]] const std::string& name = "") noexcept;

        const core::Dimension2D m_dimension;
        friend CommandList;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief Texture2DArray class. Game side
    */
    class Texture2DArray : public Texture
    {
    public:

        Texture2DArray() = delete;
        Texture2DArray(const Texture2DArray&) = delete;
        ~Texture2DArray();

        /**
        * @brief getDimension method.
        * @return const core::Dimension2D& dimention of texture
        */
        const core::Dimension2D& getDimension() const;

    private:

        /**
        * @brief Texture2DArray constructor. Used for creating 2D array of attachments
        * Private method. Use createObject interface inside CommandList class to call.
        *
        * @param TextureUsageFlags usage [required]
        * @param Format format [required]
        * @param const Dimension2D& dimension [required]
        * @param u32 layer [required]
        * @param TextureSamples samples [optional]
        * @param const std::string& name [optional]
        */
        explicit Texture2DArray(CommandList& cmdList, TextureUsageFlags usage, Format format, const core::Dimension2D& dimension, u32 layer, TextureSamples samples, [[maybe_unused]] const std::string& name = "") noexcept;

        /**
        * @brief Texture2DArray constructor. Used for creating array of 2D textures
        * Private method. Use createObject interface inside CommandList class to call.
        *
        * @param TextureUsageFlags usage [required]
        * @param Format format [required]
        * @param const Dimension2D& dimension [required]
        * @param u32 layer [required]
        * @param u32 mipmaps [optional]
        * @param const void* data [optional]
        * @param const std::string& name [optional]
        */
        explicit Texture2DArray(CommandList& cmdList, TextureUsageFlags usage, Format format, const core::Dimension2D& dimension, u32 layer, u32 mipmaps = 1, const void* data = nullptr, [[maybe_unused]] const std::string& name = "") noexcept;

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
        * @brief getDimension method.
        * @return const core::Dimension2D& dimention of texture
        */
        const core::Dimension2D& getDimension() const;

    private:

        /**
        * @brief TextureCube constructor. Used for creating a cubemap attachments
        * Private method. Use createObject interface inside CommandList class to call.
        *
        * @param TextureUsageFlags usage [required]
        * @param Format format [required]
        * @param const Dimension2D& dimension [required]
        * @param TextureSamples samples [optional]
        * @param const std::string& name [optional]
        */
        explicit TextureCube(CommandList& cmdList, TextureUsageFlags usage, Format format, const core::Dimension2D& dimension, TextureSamples samples, [[maybe_unused]] const std::string& name = "") noexcept;

        /**
        * @brief TextureCube constructor. Used for creating a cubemap texture
        * Private method. Use createObject interface inside CommandList class to call.
        *
        * @param TextureUsageFlags usage [required]
        * @param Format format [required]
        * @param const Dimension2D& dimension [required]
        * @param u32 mipmaps [optional]
        * @param const void* data [optional]
        * @param const std::string& name [optional]
        */
        explicit TextureCube(CommandList& cmdList, TextureUsageFlags usage, Format format, const core::Dimension2D& dimension, u32 mipmaps = 1, const void* data = nullptr, [[maybe_unused]] const std::string& name = "") noexcept;

        const core::Dimension2D m_dimension;
        friend CommandList;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief Backbuffer class. Wraps swapchain images
    */
    class Backbuffer : public Texture
    {
    public:

        Backbuffer() = delete;
        Backbuffer(const Backbuffer&) = delete;
        ~Backbuffer();

        /**
        * @brief getDimension method.
        * @return const core::Dimension2D& dimention of backbuffer
        */
        const core::Dimension2D& getDimension() const;

        void read(const core::Dimension2D& offset, const core::Dimension2D& size, void* const data);
        void clear(const core::Vector4D& color);

    private:

        /**
        * @brief Backbuffer constructor. Used for creating a backbuffer.
        * Private method. Creates inside CommandList object.
        */
        explicit Backbuffer(renderer::CommandList& cmdList) noexcept;

        friend CommandList;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace renderer
} //namespace v3d
