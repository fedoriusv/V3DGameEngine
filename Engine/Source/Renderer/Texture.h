#pragma once

#include "Object.h"
#include "Render.h"
#include "Swapchain.h"

namespace v3d
{
namespace renderer
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    class Device;
    class RenderTargetState;
    class ShaderProgram;
    class RenderTexture;

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief Texture base class. Wrapper on RenderTexture object
    */
    class Texture : public Object
    {
    public:

        struct SharedData
        {
        };

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

        /**
        * @brief getTextureHandle method
        * @return TextureHandle of texture
        */
        TextureHandle getTextureHandle() const;

        /**
        * @brief hasUsageFlag method
        * @return true if flag contains
        */
        bool hasUsageFlag(TextureUsage usage) const;

    protected:

        /**
        * @brief Texture constructor. parent class
        * @param CommandList& cmdList [required]
        * @param TextureTarget target [required]
        * @param Format format [required]
        * @param TextureSamples samples [required]
        * @param u32 layers [required]
        * @param cu32 mipmaps [required]
        */
        explicit Texture(TextureTarget target, Format format, TextureSamples samples, u32 layers, u32 mipmaps, TextureUsageFlags usage) noexcept;

        /**
        * @brief Texture destructor
        */
        virtual ~Texture();

    protected:

        Texture(const Texture&) = delete;
        Texture& operator=(const Texture&) = delete;

        TextureHandle           m_texture;
        const TextureTarget     m_target;
        const Format            m_format;
        const TextureSamples    m_samples;
        const u32               m_layers;
        const u32               m_mipmaps;
        TextureUsageFlags       m_usage;
        SharedData              m_sharedData;

        friend RenderTargetState;
        friend ShaderProgram;
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

    inline TextureHandle Texture::getTextureHandle() const
    {
        return m_texture;
    }

    inline bool Texture::hasUsageFlag(TextureUsage usage) const
    {
        return m_usage & usage;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief Texture2D class. Game side
    */
    class Texture2D final : public Texture
    {
    public:

        /**
        * @brief getDimension method.
        * @return const math::Dimension2D& dimention of texture
        */
        const math::Dimension2D& getDimension() const;

    public:

        /**
        * @brief Texture2D constructor. Used for creating textures with mipmaps and layers.
        *
        * @param TextureUsageFlags usage [required]
        * @param Format format [required]
        * @param const math::Dimension2D& dimension [required]
        * @param u32 array [optional]
        * @param u32 mipmaps [optional]
        * @param const std::string& name [optional]
        */
        explicit Texture2D(Device* device, TextureUsageFlags usage, Format format, const math::Dimension2D& dimension, u32 array = 1, u32 mipmaps = 1, const std::string& name = "") noexcept;

        /**
        * @brief Texture2D constructor. Used for creating attachments
        *
        * @param TextureUsageFlags usage [required]
        * @param Format format [required]
        * @param const math::Dimension2D& dimension [required]
        * @param u32 mipmaps [optional]
        * @param const std::string& name [optional]
        */
        explicit Texture2D(Device* device, TextureUsageFlags usage, Format format, const math::Dimension2D& dimension, TextureSamples samples, const std::string& name = "") noexcept;

        /**
        * @brief Texture2D destructor
        */
        ~Texture2D();

    private:

        Texture2D(const Texture2D&) = delete;
        Texture2D& operator=(const Texture2D&) = delete;

        Device* const           m_device;
        const math::Dimension2D m_dimension;
    };

    inline const math::Dimension2D& Texture2D::getDimension() const
    {
        return m_dimension;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief TextureCube class. Game side
    */
    class TextureCube final : public Texture
    {
    public:

        /**
        * @brief getDimension method.
        * @return const math::Dimension2D& dimention of texture
        */
        const math::Dimension2D& getDimension() const;

    public:

        /**
        * @brief TextureCube constructor. Used for creating a cubemap texture
        * Private method. Use createObject interface inside CommandList class to call.
        *
        * @param TextureUsageFlags usage [required]
        * @param Format format [required]
        * @param const math::Dimension2D& dimension [required]
        * @param u32 mipmaps [optional]
        * @param const std::string& name [optional]
        */
        explicit TextureCube(Device* device, TextureUsageFlags usage, Format format, const math::Dimension2D& dimension, u32 mipmaps = 1, const std::string& name = "") noexcept;

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
        explicit TextureCube(Device* device, TextureUsageFlags usage, Format format, const math::Dimension2D& dimension, TextureSamples samples, const std::string& name = "") noexcept;

        /**
        * @brief TextureCube destructor
        */
        ~TextureCube();

    private:

        TextureCube(const TextureCube&) = delete;
        TextureCube& operator=(const TextureCube&) = delete;

        Device* const           m_device;
        const math::Dimension2D m_dimension;
    };

    inline const math::Dimension2D& TextureCube::getDimension() const
    {
        return m_dimension;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief Texture3D class. Game side
    */
    class Texture3D final : public Texture
    {
    public:

        /**
        * @brief getDimension method.
        * @return const math::Dimension3D& dimention of texture
        */
        const math::Dimension3D& getDimension() const;

    public:

        /**
        * @brief Texture3D constructor. Used for creating a cubemap texture
        * Private method. Use createObject interface inside CommandList class to call.
        *
        * @param TextureUsageFlags usage [required]
        * @param Format format [required]
        * @param const math::Dimension2D& dimension [required]
        * @param u32 mipmaps [optional]
        * @param const std::string& name [optional]
        */
        explicit Texture3D(Device* device, TextureUsageFlags usage, Format format, const math::Dimension3D& dimension, u32 mipmaps = 1, const std::string& name = "") noexcept;

        /**
        * @brief Texture3D constructor. Used for creating a cubemap attachments
        * Private method. Use createObject interface inside CommandList class to call.
        *
        * @param TextureUsageFlags usage [required]
        * @param Format format [required]
        * @param const math::Dimension2D& dimension [required]
        * @param TextureSamples samples [optional]
        * @param const std::string& name [optional]
        */
        explicit Texture3D(Device* device, TextureUsageFlags usage, Format format, const math::Dimension3D& dimension, TextureSamples samples, const std::string& name = "") noexcept;

        /**
        * @brief Texture3D destructor
        */
        ~Texture3D();

    private:

        Texture3D(const Texture3D&) = delete;
        Texture3D& operator=(const Texture3D&) = delete;

        Device* const           m_device;
        const math::Dimension3D m_dimension;
    };

    inline const math::Dimension3D& Texture3D::getDimension() const
    {
        return m_dimension;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief UnorderedAccessTexture class. Game side
    */
    class UnorderedAccessTexture2D : public Texture
    {
    public:

        /**
        * @brief getDimension method.
        * @return const math::Dimension2D& dimention of texture
        */
        const math::Dimension2D& getDimension() const;

    public:

        /**
        * @brief UnorderedAccessTexture2D constructor. Used for creating UAV
        *
        * @param TextureUsageFlags usage [required]
        * @param Format format [required]
        * @param u32 array [required]
        * @param const math::Dimension2D& dimension [required]
        * @param const std::string& name [optional]
        */
        explicit UnorderedAccessTexture2D(Device* device, TextureUsageFlags usage, Format format, const math::Dimension2D& dimension, u32 array = 1, const std::string& name = "") noexcept;

        /**
        * @brief UnorderedAccessTexture destructor
        */
        ~UnorderedAccessTexture2D();

    private:

        UnorderedAccessTexture2D(const UnorderedAccessTexture2D&) = delete;
        UnorderedAccessTexture2D& operator=(const UnorderedAccessTexture2D&) = delete;

        Device* const           m_device;
        const math::Dimension2D m_dimension;
    };

    inline const math::Dimension2D& UnorderedAccessTexture2D::getDimension() const
    {
        return m_dimension;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief SwapchainTexture class. Wraps swapchain images
    */
    class SwapchainTexture final : public Texture
    {
    public:

        /**
        * @brief getDimension method.
        * @return const math::Dimension2D& dimention of backbuffer
        */
        const math::Dimension2D& getDimension() const;

    private:

        SwapchainTexture(const SwapchainTexture&) = delete;
        SwapchainTexture& operator=(const SwapchainTexture&) = delete;

        /**
        * @brief Backbuffer constructor. Used for creating a backbuffer.
        * Private method. Creates inside CommandList object.
        */
        explicit SwapchainTexture(Device* device, Swapchain* swapchain) noexcept;

        /**
        * @brief Backbuffer destructor
        */
        ~SwapchainTexture();

    private:

        Swapchain* const m_swapchain;

        friend Swapchain;
        friend RenderTargetState;

        template<class T>
        friend void memory::internal_delete(T* ptr, v3d::memory::MemoryLabel label, const v3d::c8* file, v3d::u32 line);
    };

    inline const math::Dimension2D& SwapchainTexture::getDimension() const
    {
        return m_swapchain->getBackbufferSize();
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief TextureView struct
    */
    struct TextureView
    {
        TextureView(const Texture* texture, u32 layer = k_generalLayer, u32 mip = k_allMipmapsLevels) noexcept
            : _texture(texture)
            , _subresource({ layer, 1, mip, 1 })
        {
            ASSERT(texture, "nullptr");
            if (layer == k_generalLayer)
            {
                _subresource._baseLayer = 0;
                _subresource._layers = _texture->getLayersCount();
            }

            if (mip == k_allMipmapsLevels)
            {
                _subresource._baseMip = 0;
                _subresource._mips = texture->getMipmapsCount();
            }
        }

        TextureView(const Texture* texture, u32 baseLayer, u32 layers, u32 baseMip, u32 mips) noexcept
            : _texture(texture)
            , _subresource({ baseLayer, layers, baseMip, mips })
        {
            ASSERT(texture, "nullptr");
        }

        const Texture* const       _texture;
        RenderTexture::Subresource _subresource;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace renderer

    ///////////////////////////////////////////////////////////////////////////////////////////////////////

    template<>
    struct TypeOf<renderer::Texture2D>
    {
        static TypePtr get()
        {
            static TypePtr ptr = nullptr;
            return (TypePtr)&ptr;
        }
    };

    template<>
    struct TypeOf<renderer::Texture3D>
    {
        static TypePtr get()
        {
            static TypePtr ptr = nullptr;
            return (TypePtr)&ptr;
        }
    };

    template<>
    struct TypeOf<renderer::TextureCube>
    {
        static TypePtr get()
        {
            static TypePtr ptr = nullptr;
            return (TypePtr)&ptr;
        }
    };

    template<>
    struct TypeOf<renderer::UnorderedAccessTexture2D>
    {
        static TypePtr get()
        {
            static TypePtr ptr = nullptr;
            return (TypePtr)&ptr;
        }
    };

    template<>
    struct TypeOf<renderer::SwapchainTexture>
    {
        static TypePtr get()
        {
            static TypePtr ptr = nullptr;
            return (TypePtr)&ptr;
        }
    };

    template<>
    struct TypeOf<renderer::TextureView>
    {
        static TypePtr get()
        {
            static TypePtr ptr = nullptr;
            return (TypePtr)&ptr;
        }
    };

    ///////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace v3d
