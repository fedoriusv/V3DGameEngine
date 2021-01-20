#include "Texture.h"
#include "Renderer/Context.h"
#include "Renderer/Image.h"
#include "Utils/Logger.h"

namespace v3d
{
namespace renderer
{

/////////////////////////////////////////////////////////////////////////////////////////////////////

    /*CreateTextureCommand*/
class CreateTextureCommand : public renderer::Command
{
public:
    CreateTextureCommand(renderer::Image* image, const core::Dimension3D & offsets, const core::Dimension3D & dim, u32 mips, u32 layers, u32 size, void * data, bool shared) noexcept
        : m_image(image)
        , m_offsets(offsets)
        , m_dimension(dim)
        , m_mipmaps(mips)
        , m_layers(layers)
        , m_data(nullptr)

        , m_shared(shared)
    {
#if DEBUG_COMMAND_LIST
        LOG_DEBUG("CreateTextureCommand constructor");
#endif
        if (data)
        {
            if (m_shared)
            {
                m_data = data;
            }
            else
            {
                m_data = malloc(size); //TODO: get from pool
                memcpy(m_data, data, size);
            }
        }
    }

    ~CreateTextureCommand()
    {
#if DEBUG_COMMAND_LIST
        LOG_DEBUG("CreateTextureCommand destructor");
#endif
        if (m_data && !m_shared)
        {
            free(m_data); //TODO: return to pool
            m_data = nullptr;
        }
    }

    void execute(const renderer::CommandList& cmdList) override
    {
#if DEBUG_COMMAND_LIST
        LOG_DEBUG("CreateTextureCommand execute");
#endif
        if (!m_image->create())
        {
            m_image->notifyObservers();

            m_image->destroy();
            return;
        }

        if (m_data)
        {
            m_image->upload(cmdList.getContext(), m_dimension, m_layers, m_mipmaps, m_data);
        }
    }

private:
    renderer::Image*            m_image;
    core::Dimension3D           m_offsets;
    core::Dimension3D           m_dimension;
    u32                         m_mipmaps;
    u32                         m_layers;

    void*                       m_data;

    bool                        m_shared;
};

    /*CommandDestroyImage*/
class CommandDestroyImage : public Command
{
public:
    CommandDestroyImage(Image* image) noexcept
        : m_image(image)
    {
#if DEBUG_COMMAND_LIST
        LOG_DEBUG("CommandDestroyImage constructor");
#endif //DEBUG_COMMAND_LIST
    }

    ~CommandDestroyImage()
    {
#if DEBUG_COMMAND_LIST
        LOG_DEBUG("CommandDestroyImage destructor");
#endif //DEBUG_COMMAND_LIST
    }

    void execute(const renderer::CommandList& cmdList) override
    {
#if DEBUG_COMMAND_LIST
        LOG_DEBUG("CommandDestroyImage execute");
#endif //DEBUG_COMMAND_LIST
        cmdList.getContext()->removeImage(m_image);
    }

private:
    Image* m_image;
};

    /*UploadTextureCommand*/
class UploadTextureCommand : public renderer::Command, utils::Observer
{
public:
    UploadTextureCommand(renderer::Image* image) noexcept
        : m_image(image)
    {
#if DEBUG_COMMAND_LIST
        LOG_DEBUG("UploadTextureCommand constructor");
#endif
        m_image->registerNotify(this);
    }

    ~UploadTextureCommand()
    {
#if DEBUG_COMMAND_LIST
        LOG_DEBUG("UploadTextureCommand destructor");
#endif
        m_image->unregisterNotify(this);
    }

    void execute(const renderer::CommandList& cmdList) override
    {
#if DEBUG_COMMAND_LIST
        LOG_DEBUG("UploadTextureCommand execute");
#endif
        if (m_image)
        {
            //m_image->upload();
        }
    }

    void handleNotify(const utils::Observable* ob) override
    {
        LOG_ERROR("UploadTextureCommand image %llx was deleted", m_image);
        m_image = nullptr;
    }

private:
    renderer::Image* m_image;
};

    /*CommandClearColor*/
class CommandClearColor : public renderer::Command, utils::Observer
{
public:
    CommandClearColor(renderer::Image* image, const core::Vector4D& color) noexcept
        : m_image(image)
        , m_clearColor(color)
    {
#if DEBUG_COMMAND_LIST
        LOG_DEBUG("CommandClearColor constructor");
#endif
        m_image->registerNotify(this);
    };

    ~CommandClearColor()
    {
#if DEBUG_COMMAND_LIST
        LOG_DEBUG("CommandClearColor destructor");
#endif
        m_image->unregisterNotify(this);
    };

    void execute(const renderer::CommandList& cmdList) override
    {
#if DEBUG_COMMAND_LIST
        LOG_DEBUG("CommandClearColor execute");
#endif
        if (m_image)
        {
            m_image->clear(cmdList.getContext(), m_clearColor);
        }
    }

    void handleNotify(const utils::Observable* ob) override
    {
        LOG_ERROR("UploadTextureCommand image %llx was deleted", m_image);
        m_image = nullptr;
    }

private:
    renderer::Image*    m_image;
    core::Vector4D      m_clearColor;
};

    /*CommandClearDepthStencil*/
class CommandClearDepthStencil : public renderer::Command, utils::Observer
{
public:
    CommandClearDepthStencil(renderer::Image* image, f32 depth, u32 stencil) noexcept
        : m_image(image)
        , m_depth(depth)
        , m_stencil(stencil)
    {
#if DEBUG_COMMAND_LIST
        LOG_DEBUG("CommandClearDepthStencil constructor");
#endif
        m_image->registerNotify(this);
    };

    ~CommandClearDepthStencil()
    {
#if DEBUG_COMMAND_LIST
        LOG_DEBUG("CommandClearDepthStencil destructor");
#endif
        m_image->unregisterNotify(this);
    };

    void execute(const renderer::CommandList& cmdList) override
    {
#if DEBUG_COMMAND_LIST
        LOG_DEBUG("CommandClearDepthStencil execute");
#endif
        if (m_image)
        {
            m_image->clear(cmdList.getContext(), m_depth, m_stencil);
        }
    }

    void handleNotify(const utils::Observable* ob) override
    {
        LOG_ERROR("UploadTextureCommand image %llx was deleted", m_image);
        m_image = nullptr;
    }

private:
    renderer::Image*    m_image;
    f32                 m_depth;
    u32                 m_stencil;
};

/*CommandClearBackbuffer*/
class CommandClearBackbuffer : public renderer::Command
{
public:
    CommandClearBackbuffer(const core::Vector4D& color) noexcept
        : m_clearColor(color)
    {
#if DEBUG_COMMAND_LIST
        LOG_DEBUG("CommandClearBackbuffer constructor");
#endif //DEBUG_COMMAND_LIST
    };

    ~CommandClearBackbuffer()
    {
#if DEBUG_COMMAND_LIST
        LOG_DEBUG("CommandClearBackbuffer destructor");
#endif //DEBUG_COMMAND_LIST
    };

    void execute(const renderer::CommandList& cmdList)
    {
#if DEBUG_COMMAND_LIST
        LOG_DEBUG("CommandClearBackbuffer execute");
#endif //DEBUG_COMMAND_LIST
        cmdList.getContext()->clearBackbuffer(m_clearColor);
    }

private:
    core::Vector4D m_clearColor;
};

/////////////////////////////////////////////////////////////////////////////////////////////////////

Texture::Texture(CommandList& cmdList, TextureTarget target, Format format, TextureSamples samples, u32 layers, u32 mipmaps, TextureUsageFlags usage) noexcept
    : m_cmdList(cmdList)
    , m_target(target)
    , m_format(format)
    , m_samples(samples)
    , m_layers(layers)
    , m_mipmaps(mipmaps)

    , m_usage(usage)
    , m_image(nullptr)

    , m_backbuffer(false)
{
}

Texture::~Texture()
{
    //m_image may exists, because render still holds the resource
    //ASSERT(!m_image, "image isn't nullptr");
}

TextureTarget Texture::getTarget() const
{
    return m_target;
}

Format Texture::getFormat() const
{
    return m_format;
}

TextureSamples Texture::getSamples() const
{
    return m_samples;
}

u32 Texture::getLayersCount() const
{
    return m_layers;
}

u32 Texture::getMipmapsCount() const
{
    return m_mipmaps;
}

Image* Texture::getImage() const
{
    ASSERT(m_image, "nullptr");
    return m_image;
}

bool Texture::isBackbuffer() const
{
    return m_backbuffer;
}

bool Texture::isTextureUsageFlagsContains(TextureUsageFlags usage) const
{
    return m_usage & usage;
}

void Texture::handleNotify(const utils::Observable* ob)
{
    LOG_DEBUG("Texture::handleNotify to delete image %xll", this);
    ASSERT(m_image == ob, "not same");

    m_image = nullptr;
}

void Texture::createTexture(const core::Dimension3D& dimension, const void* data)
{
    if (m_cmdList.isImmediate())
    {
        if (!m_image->create())
        {
            m_image->destroy();

            delete m_image;
            m_image = nullptr;
        }

        if (data)
        {
            m_image->upload(m_cmdList.getContext(), dimension, m_layers, m_mipmaps, data);
        }
    }
    else
    {
        u32 calculatedSize = 0;
        if (data && (m_usage & TextureUsage_Shared) == 0)
        {
            //TODO
            ASSERT(m_mipmaps == 1, "impl");

            calculatedSize = ImageFormat::getFormatBlockSize(m_format) * dimension.getArea() * m_layers;
            if (ImageFormat::isFormatCompressed(m_format))
            {
                calculatedSize /= 16;
            }
            ASSERT(calculatedSize > 0, "size is 0");
        }

        m_cmdList.pushCommand(
            new CreateTextureCommand(m_image, core::Dimension3D(0, 0, 0), dimension, m_mipmaps, m_layers, calculatedSize, const_cast<void*>(data), (m_usage & TextureUsage_Shared)));
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////

Texture2D::Texture2D(renderer::CommandList& cmdList, TextureUsageFlags usage, Format format, const core::Dimension2D& dimension, u32 mipmaps, const void* data, const std::string& name) noexcept
    : Texture(cmdList, TextureTarget::Texture2D, format, TextureSamples::TextureSamples_x1, 1U, mipmaps, usage)
    , m_dimension(dimension)
{
    LOG_DEBUG("Texture2D::Texture2D constructor %llx", this);

    core::Dimension3D dim = { m_dimension.width, m_dimension.height, 1 };

    m_image = m_cmdList.getContext()->createImage(m_target, m_format, dim, m_layers, m_mipmaps, m_usage, name);
    ASSERT(m_image, "m_image is nullptr");
    m_image->registerNotify(this);

    createTexture(dim, data);
}

Texture2D::Texture2D(renderer::CommandList & cmdList, TextureUsageFlags usage, renderer::Format format, const core::Dimension2D& dimension, TextureSamples samples, const std::string& name) noexcept
    : Texture(cmdList, TextureTarget::Texture2D, format, samples, 1U, 1U, usage)
    , m_dimension(dimension)
{
    LOG_DEBUG("Texture2D::Texture2D constructor %llx", this);

    core::Dimension3D dim = { m_dimension.width, m_dimension.height, 1 };

    m_image = m_cmdList.getContext()->createImage(TextureTarget::Texture2D, m_format, dim, 1U, m_samples, m_usage, name);
    ASSERT(m_image, "m_image is nullptr");
    m_image->registerNotify(this);

    createTexture(dim, nullptr);
}

Texture2D::~Texture2D()
{
    LOG_DEBUG("Texture2D::Texture2D destructor %llx", this);

    ASSERT(m_image, "image nullptr");
    m_image->unregisterNotify(this);
    if (m_cmdList.isImmediate())
    {
        m_cmdList.getContext()->removeImage(m_image);
    }
    else
    {
        m_cmdList.pushCommand(new CommandDestroyImage(m_image));
    }
}

const core::Dimension2D& Texture2D::getDimension() const
{
    return m_dimension;
}

void Texture2D::update(const core::Dimension2D& offset, const core::Dimension2D& size, u32 mipLevel, const void* data)
{
    ASSERT(m_image, "m_image is nullptr");
    if (m_image)
    {
        NOT_IMPL;
    }
}

void Texture2D::read(const core::Dimension2D& offset, const core::Dimension2D& size, u32 mipLevel, void* const data)
{
    ASSERT(m_image, "m_image is nullptr");
    if (m_image)
    {
        NOT_IMPL;
    }
}

void Texture2D::clear(const core::Vector4D& color)
{
    ASSERT(m_image, "m_image is nullptr");
    if (m_image)
    {
        if (m_cmdList.isImmediate())
        {
            m_image->clear(m_cmdList.getContext(), color);
        }
        else
        {
            m_cmdList.pushCommand(new CommandClearColor(m_image, color));
        }
    }
}

void Texture2D::clear(f32 depth, u32 stencil)
{
    ASSERT(m_image, "m_image is nullptr");
    if (m_image)
    {
        if (m_cmdList.isImmediate())
        {
            m_image->clear(m_cmdList.getContext(), depth, stencil);
        }
        else
        {
            m_cmdList.pushCommand(new CommandClearDepthStencil(m_image, depth, stencil));
        }
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////

Texture2DArray::Texture2DArray(CommandList& cmdList, TextureUsageFlags usage, Format format, const core::Dimension2D& dimension, u32 layer, TextureSamples samples, const std::string& name) noexcept
    : Texture(cmdList, TextureTarget::Texture2DArray, format, samples, layer, 1U, usage)
    , m_dimension(dimension)
{
    LOG_DEBUG("Texture2DArray::Texture2DArray constructor %llx", this);

    core::Dimension3D dim = { m_dimension.width, m_dimension.height, 1 };

    m_image = m_cmdList.getContext()->createImage(TextureTarget::Texture2DArray, m_format, dim, m_layers, m_samples, m_usage, name);
    ASSERT(m_image, "m_image is nullptr");
    m_image->registerNotify(this);

    createTexture(dim, nullptr);
}

Texture2DArray::Texture2DArray(CommandList& cmdList, TextureUsageFlags usage, Format format, const core::Dimension2D& dimension, u32 layer, u32 mipmaps, const void* data, const std::string& name) noexcept
    : Texture(cmdList, TextureTarget::Texture2DArray, format, TextureSamples::TextureSamples_x1, layer, mipmaps, usage)
    , m_dimension(dimension)
{
    LOG_DEBUG("Texture2DArray::Texture2DArray constructor %llx", this);

    core::Dimension3D dim = { m_dimension.width, m_dimension.height, 1 };

    m_image = m_cmdList.getContext()->createImage(TextureTarget::Texture2DArray, m_format, dim, m_layers, m_mipmaps, m_usage, name);
    ASSERT(m_image, "m_image is nullptr");
    m_image->registerNotify(this);

    createTexture(dim, data);
}

Texture2DArray::~Texture2DArray()
{
    LOG_DEBUG("Texture2DArray::Texture2DArray destructor %llx", this);

    ASSERT(m_image, "image nullptr");
    m_image->unregisterNotify(this);
    if (m_cmdList.isImmediate())
    {
        m_cmdList.getContext()->removeImage(m_image);
    }
    else
    {
        m_cmdList.pushCommand(new CommandDestroyImage(m_image));
    }
}

const core::Dimension2D& Texture2DArray::getDimension() const
{
    return m_dimension;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////

TextureCube::TextureCube(CommandList& cmdList, TextureUsageFlags usage, Format format, const core::Dimension2D& dimension, TextureSamples samples, const std::string& name) noexcept
    : Texture(cmdList, TextureTarget::TextureCubeMap, format, samples, 6U, 1U, usage)
    , m_dimension(dimension)
{
    LOG_DEBUG("TextureCube::TextureCube constructor %llx", this);

    m_image = m_cmdList.getContext()->createImage(TextureTarget::TextureCubeMap, m_format, { m_dimension.width, m_dimension.height, 1 }, 6U, m_samples, m_usage, name);
    ASSERT(m_image, "m_image is nullptr");
    m_image->registerNotify(this);

    createTexture({ m_dimension.width, m_dimension.height, 1 }, nullptr);
}

TextureCube::TextureCube(CommandList& cmdList, TextureUsageFlags usage, Format format, const core::Dimension2D& dimension, u32 mipmaps, const void* data, const std::string& name) noexcept
    : Texture(cmdList, TextureTarget::TextureCubeMap, format, TextureSamples::TextureSamples_x1, 6U, mipmaps, usage)
{
    LOG_DEBUG("TextureCube::TextureCube constructor %llx", this);

    m_image = m_cmdList.getContext()->createImage(TextureTarget::TextureCubeMap, m_format, { m_dimension.width, m_dimension.height, 1 }, 6U, m_mipmaps, m_usage, name);
    ASSERT(m_image, "m_image is nullptr");
    m_image->registerNotify(this);

    createTexture({ m_dimension.width, m_dimension.height, 1 }, data);
}

TextureCube::~TextureCube()
{
    LOG_DEBUG("TextureCube::TextureCube destructor %llx", this);

    ASSERT(m_image, "image nullptr");
    m_image->unregisterNotify(this);
    if (m_cmdList.isImmediate())
    {
        m_cmdList.getContext()->removeImage(m_image);
    }
    else
    {
        m_cmdList.pushCommand(new CommandDestroyImage(m_image));
    }
}

const core::Dimension2D& TextureCube::getDimension() const
{
    return m_dimension;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////

Backbuffer::Backbuffer(renderer::CommandList& cmdList) noexcept
    : Texture(cmdList, TextureTarget::Texture2D, cmdList.getContext()->m_backufferDescription._format, TextureSamples::TextureSamples_x1, 1U, 1U, 
        TextureUsage::TextureUsage_Sampled | TextureUsage::TextureUsage_Attachment | TextureUsage::TextureUsage_Read)
{
    LOG_DEBUG("Backbuffer::Backbuffer constructor %llx", this);
    m_backbuffer = true;
}

Backbuffer::~Backbuffer()
{
    LOG_DEBUG("Backbuffer::Backbuffer constructor %llx", this);
}

const core::Dimension2D& Backbuffer::getDimension() const
{
    return m_cmdList.getContext()->m_backufferDescription._size;
}

void Backbuffer::read(const core::Dimension2D& offset, const core::Dimension2D& size, void* const data)
{
    NOT_IMPL;
}

void Backbuffer::clear(const core::Vector4D& color)
{
    if (m_cmdList.isImmediate())
    {
        m_cmdList.getContext()->clearBackbuffer(color);
    }
    else
    {
        m_cmdList.pushCommand(new CommandClearBackbuffer(color));
    }
}

} //namespace renderer
} //namespace v3d
