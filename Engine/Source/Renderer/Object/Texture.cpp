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

    void handleNotify(utils::Observable* ob) override
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

    void handleNotify(utils::Observable* ob) override
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

    void handleNotify(utils::Observable* ob) override
    {
        LOG_ERROR("UploadTextureCommand image %llx was deleted", m_image);
        m_image = nullptr;
    }

private:
    renderer::Image*    m_image;
    f32                 m_depth;
    u32                 m_stencil;
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

bool Texture::isTextureUsageFlagsContains(TextureUsageFlags usage) const
{
    return m_usage & usage;
}

void Texture::handleNotify(utils::Observable* ob)
{
    LOG_DEBUG("Texture::handleNotify to delete image %xll", this);
    ASSERT(m_image == ob, "not same");

    m_image = nullptr;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////

Texture2D::Texture2D(renderer::CommandList& cmdList, TextureUsageFlags usage, Format format, const core::Dimension2D& dimension, u32 layers, u32 mipmaps, const void* data, const std::string& name) noexcept
    : Texture(cmdList, TextureTarget::Texture2D, format, TextureSamples::TextureSamples_x1, layers, mipmaps, usage)
    , m_dimension(dimension)
{
    core::Dimension3D dim = { m_dimension.width, m_dimension.height, 1 };
    m_image = m_cmdList.getContext()->createImage(m_target, m_format, dim, m_layers, m_mipmaps, m_usage, name);
    ASSERT(m_image, "m_image is nullptr");
    m_image->registerNotify(this);

    createTexture2D(data);
}

Texture2D::Texture2D(renderer::CommandList & cmdList, TextureUsageFlags usage, renderer::Format format, const core::Dimension2D& dimension, TextureSamples samples, const std::string& name) noexcept
    : Texture(cmdList, TextureTarget::Texture2D, format, samples, 1, 1, usage)
    , m_dimension(dimension)
{
    core::Dimension3D dim = { m_dimension.width, m_dimension.height, 1 };

    m_image = m_cmdList.getContext()->createImage(TextureTarget::Texture2D, m_format, dim, m_samples, m_usage, name);
    ASSERT(m_image, "m_image is nullptr");
    m_image->registerNotify(this);

    createTexture2D(nullptr);
}

void Texture2D::createTexture2D(const void * data)
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
            m_image->upload(m_cmdList.getContext(), core::Dimension3D(m_dimension.width, m_dimension.height, 1), m_layers, m_mipmaps, data);
        }
    }
    else
    {
        u32 calculatedSize = 0;
        if (data && (m_usage & TextureUsage_Shared) == 0)
        {
            //TODO
            ASSERT(m_mipmaps == 1, "impl");

            calculatedSize = ImageFormat::getFormatBlockSize(m_format) * m_dimension.getArea() * m_layers;
            if (ImageFormat::isFormatCompressed(m_format))
            {
                calculatedSize /= 16;
            }
            ASSERT(calculatedSize > 0, "size is 0");
        }

        m_cmdList.pushCommand(
            new CreateTextureCommand(m_image, core::Dimension3D(0, 0, 0), core::Dimension3D(m_dimension.width, m_dimension.height, 1), m_mipmaps, m_layers, calculatedSize, const_cast<void*>(data), (m_usage & TextureUsage_Shared)));
    }
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
        ASSERT(false, "not implemented");
    }
}

void Texture2D::read(const core::Dimension2D& offset, const core::Dimension2D& size, u32 mipLevel, void* const data)
{
    ASSERT(m_image, "m_image is nullptr");
    if (m_image)
    {
        ASSERT(false, "not implemented");
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

TextureCube::TextureCube(CommandList& cmdList, TextureUsageFlags usage, Format format, const core::Dimension2D& dimension, TextureSamples samples, const std::string& name) noexcept
    : Texture(cmdList, TextureTarget::TextureCubeMap, format, samples, 6, 1, usage)
    , m_dimension(dimension)
{
    m_image = m_cmdList.getContext()->createImage(TextureTarget::TextureCubeMap, m_format, { m_dimension.width, m_dimension.height, 1 }, m_samples, m_usage, name);
    ASSERT(m_image, "m_image is nullptr");
    m_image->registerNotify(this);

    createTextureCube();
}

void TextureCube::createTextureCube()
{
    if (m_cmdList.isImmediate())
    {
        if (!m_image->create())
        {
            m_image->destroy();

            delete m_image;
            m_image = nullptr;
        }
    }
    else
    {
        m_cmdList.pushCommand(
            new CreateTextureCommand(m_image, core::Dimension3D(0, 0, 0), core::Dimension3D(m_dimension.width, m_dimension.height, 1), m_mipmaps, m_layers, 0, nullptr, (m_usage & TextureUsage_Shared)));
    }
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


} //namespace renderer
} //namespace v3d
