#pragma once

#include "Common.h"
#include "Object.h"
#include "Utils/Observable.h"

#include "Formats.h"
#include "TextureProperties.h"

#include "CommandList.h"

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

        /**
        * @brief Texture constructor. parent class
        * @param CommandList& cmdList [required]
        * @param TextureTarget target [required]
        * @param Format format [required]
        * @param TextureSamples samples [required]
        * @param u32 layers [required]
        * @param cu32 mipmaps [required]
        * @param TextureUsageFlags usage [required]
        */
        explicit Texture(CommandList& cmdList, TextureTarget target, Format format, TextureSamples samples, u32 layers, u32 mipmaps, TextureUsageFlags usage) noexcept;

        /**
        * @brief Texture destructor
        */
        virtual ~Texture();

        Texture() = delete;
        Texture(const Texture&) = delete;

        Image* getImage() const;

        bool isBackbuffer() const;

        void handleNotify(const utils::Observable* object, void* msg) override;
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

        void createTexture(const math::Dimension3D& dimension, const void* data);
    };

    inline TextureTarget Texture::getTarget() const
    {
        return m_target;
    }

    inline Format Texture::getFormat() const
    {
        return m_format;
    }

    inline TextureSamples Texture::getSamples() const
    {
        return m_samples;
    }

    inline u32 Texture::getLayersCount() const
    {
        return m_layers;
    }

    inline u32 Texture::getMipmapsCount() const
    {
        return m_mipmaps;
    }

    inline bool Texture::isBackbuffer() const
    {
        return m_backbuffer;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief Texture2D class. Game side
    */
    class Texture2D final : public Texture
    {
    public:

        /**
        * @brief Texture2D destructor
        */
        ~Texture2D();

        /**
        * @brief getDimension method.
        * @return const math::Dimension2D& dimention of texture
        */
        const math::Dimension2D& getDimension() const;

        void update(const math::Dimension2D& offset, const math::Dimension2D& size, u32 mipLevel, const void* data);
        void read(const math::Dimension2D& offset, const math::Dimension2D& size, u32 mipLevel, void* const data);
        void clear(const math::Vector4D& color);
        void clear(f32 depth, u32 stencil);

    private:

        /**
        * @brief Texture2D constructor. Used for creating textures with mipmaps and layers.
        * Private method. Use createObject interface inside CommandList class to call.
        * 
        * @param TextureUsageFlags usage [required]
        * @param Format format [required]
        * @param const math::Dimension2D& dimension [required]
        * @param u32 mipmaps [optional]
        * @param const void* data [optional]
        * @param const std::string& name [optional]
        */
        explicit Texture2D(CommandList& cmdList, TextureUsageFlags usage, Format format, const math::Dimension2D& dimension, u32 mipmaps = 1, const void* data = nullptr, [[maybe_unused]] const std::string& name = "") noexcept;

        /**
        * @brief Texture2D constructor. Used for creating attachments
        * Private method. Use createObject interface inside CommandList class to call.
        *
        * @param TextureUsageFlags usage [required]
        * @param Format format [required]
        * @param const math::Dimension2D& dimension [required]
        * @param u32 mipmaps [optional]
        * @param const void* data [optional]
        * @param const std::string& name [optional]
        */
        explicit Texture2D(CommandList& cmdList, TextureUsageFlags usage, Format format, const math::Dimension2D& dimension, TextureSamples samples, [[maybe_unused]] const std::string& name = "") noexcept;

        Texture2D() = delete;
        Texture2D(const Texture2D&) = delete;

        const math::Dimension2D m_dimension;
        friend CommandList;
    };

    inline const math::Dimension2D& Texture2D::getDimension() const
    {
        return m_dimension;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief Texture2DArray class. Game side
    */
    class Texture2DArray : public Texture
    {
    public:

        /**
        * @brief Texture2DArray destructor
        */
        ~Texture2DArray();

        /**
        * @brief getDimension method.
        * @return const math::Dimension2D& dimention of texture
        */
        const math::Dimension2D& getDimension() const;

    private:

        /**
        * @brief Texture2DArray constructor. Used for creating 2D array of attachments
        * Private method. Use createObject interface inside CommandList class to call.
        *
        * @param TextureUsageFlags usage [required]
        * @param Format format [required]
        * @param const math::Dimension2D& dimension [required]
        * @param u32 layer [required]
        * @param TextureSamples samples [optional]
        * @param const std::string& name [optional]
        */
        explicit Texture2DArray(CommandList& cmdList, TextureUsageFlags usage, Format format, const math::Dimension2D& dimension, u32 layer, TextureSamples samples, [[maybe_unused]] const std::string& name = "") noexcept;

        /**
        * @brief Texture2DArray constructor. Used for creating array of 2D textures
        * Private method. Use createObject interface inside CommandList class to call.
        *
        * @param TextureUsageFlags usage [required]
        * @param Format format [required]
        * @param const math::Dimension2D& dimension [required]
        * @param u32 layer [required]
        * @param u32 mipmaps [optional]
        * @param const void* data [optional]
        * @param const std::string& name [optional]
        */
        explicit Texture2DArray(CommandList& cmdList, TextureUsageFlags usage, Format format, const math::Dimension2D& dimension, u32 layer, u32 mipmaps = 1, const void* data = nullptr, [[maybe_unused]] const std::string& name = "") noexcept;

        Texture2DArray() = delete;
        Texture2DArray(const Texture2DArray&) = delete;

        const math::Dimension2D m_dimension;
        friend CommandList;
    };

    inline const math::Dimension2D& Texture2DArray::getDimension() const
    {
        return m_dimension;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief TextureCube class. Game side
    */
    class TextureCube : public Texture
    {
    public:

        /**
        * @brief TextureCube destructor
        */
        ~TextureCube();

        /**
        * @brief getDimension method.
        * @return const math::Dimension2D& dimention of texture
        */
        const math::Dimension2D& getDimension() const;

    private:

        /**
        * @brief TextureCube constructor. Used for creating a cubemap attachments
        * Private method. Use createObject interface inside CommandList class to call.
        *
        * @param TextureUsageFlags usage [required]
        * @param Format format [required]
        * @param const math::Dimension2D& dimension [required]
        * @param TextureSamples samples [optional]
        * @param const std::string& name [optional]
        */
        explicit TextureCube(CommandList& cmdList, TextureUsageFlags usage, Format format, const math::Dimension2D& dimension, TextureSamples samples, [[maybe_unused]] const std::string& name = "") noexcept;

        /**
        * @brief TextureCube constructor. Used for creating a cubemap texture
        * Private method. Use createObject interface inside CommandList class to call.
        *
        * @param TextureUsageFlags usage [required]
        * @param Format format [required]
        * @param const math::Dimension2D& dimension [required]
        * @param u32 mipmaps [optional]
        * @param const void* data [optional]
        * @param const std::string& name [optional]
        */
        explicit TextureCube(CommandList& cmdList, TextureUsageFlags usage, Format format, const math::Dimension2D& dimension, u32 mipmaps = 1, const void* data = nullptr, [[maybe_unused]] const std::string& name = "") noexcept;

        TextureCube() = delete;
        TextureCube(const TextureCube&) = delete;

        const math::Dimension2D m_dimension;
        friend CommandList;
    };

    inline const math::Dimension2D& TextureCube::getDimension() const
    {
        return m_dimension;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief Texture3D class. Game side
    */
    class Texture3D : public Texture
    {
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief Backbuffer class. Wraps swapchain images
    */
    class Backbuffer : public Texture
    {
    public:

        /**
        * @brief Backbuffer destructor
        */
        ~Backbuffer();

        /**
        * @brief getDimension method.
        * @return const math::Dimension2D& dimention of backbuffer
        */
        const math::Dimension2D& getDimension() const;

        void read(const math::Dimension2D& offset, const math::Dimension2D& size, void* const data);
        
        /**
        * @brief clear method.
        * @param const math::Vector4D& color [required]
        */
        void clear(const math::Vector4D& color);

    private:

        Backbuffer() = delete;
        Backbuffer(const Backbuffer&) = delete;

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
