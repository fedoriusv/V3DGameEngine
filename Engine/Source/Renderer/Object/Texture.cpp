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
    CreateTextureCommand(renderer::Image* image, const core::Dimension3D & offsets, const core::Dimension3D & size, u32 mips, void * data, bool shared) noexcept
        : m_image(image)
        , m_offsets(offsets)
        , m_size(size)
        , m_mipmaps(mips)
        , m_data(nullptr)

        , m_shared(shared)
    {
        LOG_DEBUG("CreateTextureCommand constructor");

        if (data)
        {
            if (m_shared)
            {
                m_data = data;
            }
            else
            {
                ASSERT(false, "impl");
                u32 dataSize = 0;
                m_data = malloc(dataSize); //TODO: get from pool

                memcpy(m_data, data, dataSize);
            }
        }
    }

    ~CreateTextureCommand()
    {
        LOG_DEBUG("CreateTextureCommand destructor");

        if (m_data)
        {
            free(m_data); //TODO: return to pool
            m_data = nullptr;
        }
    }

    void execute(const renderer::CommandList& cmdList) override
    {
        LOG_DEBUG("CreateTextureCommand execute");
        if (!m_image->create())
        {
            m_image->notifyObservers();

            m_image->destroy();
            return;
        }

        if (m_data)
        {
            m_image->upload(cmdList.getContext(), m_offsets, m_size, m_mipmaps, m_data);
        }
    }

private:
    renderer::Image*            m_image;
    core::Dimension3D           m_offsets;
    core::Dimension3D           m_size;
    u32                         m_mipmaps;

    void*                       m_data;

    bool                        m_shared;
};

    /*UploadTextureCommand*/
class UploadTextureCommand : public renderer::Command, utils::Observer
{
public:
    UploadTextureCommand(renderer::Image* image) noexcept
        : m_image(image)
    {
        LOG_DEBUG("UploadTextureCommand constructor");
        m_image->registerNotify(this);
    }

    ~UploadTextureCommand()
    {
        LOG_DEBUG("UploadTextureCommand destructor");
        m_image->unregisterNotify(this);
    }

    void execute(const renderer::CommandList& cmdList) override
    {
        LOG_DEBUG("UploadTextureCommand execute");
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
        LOG_DEBUG("CommandClearColor constructor");
        m_image->registerNotify(this);
    };

    ~CommandClearColor()
    {
        LOG_DEBUG("CommandClearColor destructor");
        m_image->unregisterNotify(this);
    };

    void execute(const renderer::CommandList& cmdList)
    {
        LOG_DEBUG("CommandClearColor execute");
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
        LOG_DEBUG("CommandClearDepthStencil constructor");
        m_image->registerNotify(this);
    };

    ~CommandClearDepthStencil()
    {
        LOG_DEBUG("CommandClearDepthStencil destructor");
        m_image->unregisterNotify(this);
    };

    void execute(const renderer::CommandList& cmdList)
    {
        LOG_DEBUG("CommandClearDepthStencil execute");
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

Texture2D::Texture2D(renderer::CommandList& cmdList, TextureUsageFlags usage, renderer::Format format, const core::Dimension2D& dimension, u32 mipmapCount, const void * data) noexcept
    : m_cmdList(cmdList)
    , m_target(renderer::TextureTarget::Texture2D)
    , m_format(format)
    , m_dimension(dimension)
    , m_mipmaps(mipmapCount)
    , m_samples(renderer::TextureSamples::TextureSamples_x1)

    , m_usage(usage)
    , m_image(nullptr)

{
    core::Dimension3D dim = { m_dimension.width, m_dimension.height, 1 };
    m_image = m_cmdList.getContext()->createImage(m_target, m_format, dim, m_mipmaps, m_usage);
    ASSERT(m_image, "m_image is nullptr");
    m_image->registerNotify(this);

    createTexture2D(data);
}

Texture2D::Texture2D(renderer::CommandList & cmdList, TextureUsageFlags usage, renderer::Format format, const core::Dimension2D & dimension, renderer::TextureSamples samples) noexcept
    : m_cmdList(cmdList)
    , m_target(renderer::TextureTarget::Texture2D)
    , m_format(format)
    , m_dimension(dimension)
    , m_mipmaps(1)
    , m_samples(samples)

    , m_usage(usage)
    , m_image(nullptr)
{
    core::Dimension3D dim = { m_dimension.width, m_dimension.height, 1 };

    m_image = m_cmdList.getContext()->createImage(m_target, m_format, dim, m_mipmaps, m_usage);
    ASSERT(m_image, "m_image is nullptr");
    m_image->registerNotify(this);

    createTexture2D(nullptr);
}

void Texture2D::handleNotify(utils::Observable* ob)
{
    if (m_image)
    {
        delete m_image;
        m_image = nullptr;
    }
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
            m_image->upload(m_cmdList.getContext(), core::Dimension3D(0, 0, 0), core::Dimension3D(m_dimension.width, m_dimension.height, 1), m_mipmaps, data);
        }
    }
    else
    {
        m_cmdList.pushCommand(new CreateTextureCommand(m_image, core::Dimension3D(0, 0, 0), core::Dimension3D(m_dimension.width, m_dimension.height, 1), m_mipmaps, const_cast<void*>(data), true));
    }
}

renderer::Image * Texture2D::getImage() const
{
    ASSERT(m_image, "nullptr");
    return m_image;
}

Texture2D::~Texture2D()
{
    //m_image delete
    //m_image = m_imageManager.destroyImage();
    //create command
    m_image->unregisterNotify(this);
}

renderer::TextureTarget Texture2D::getTarget() const
{
    return m_target;
}

renderer::TextureSamples Texture2D::getSampleCount() const
{
    return m_samples;
}

u32 Texture2D::getMipmaps() const
{
    return m_mipmaps;
}

const core::Dimension2D& Texture2D::getDimension() const
{
    return m_dimension;
}

renderer::Format Texture2D::getFormat() const
{
    return m_format;
}

void Texture2D::update(const core::Dimension2D & offset, const core::Dimension2D & size, u32 mipLevel, const void * data)
{
    ASSERT(m_image, "m_image is nullptr");
    if (m_image)
    {
        ASSERT(false, "not implemented");
    }
}

void Texture2D::read(const core::Dimension2D & offset, const core::Dimension2D & size, u32 mipLevel, void * const data)
{
    ASSERT(m_image, "m_image is nullptr");
    if (m_image)
    {
        ASSERT(false, "not implemented");
    }
}

void Texture2D::clear(const core::Vector4D & color)
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

} //namespace renderer
} //namespace v3d
