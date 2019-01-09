#include "Texture.h"
#include "Context.h"
#include "Image.h"
#include "Utils/Logger.h"

namespace v3d
{
namespace renderer
{

    /*CreateTextureCommand*/
class CreateTextureCommand : public renderer::Command
{
public:
    CreateTextureCommand(renderer::Image* image, u32 dataSize, const void * data)
        : m_image(image) //TODO: need use handle
        , m_data(nullptr)
        , m_dataSize(0)
    {
        LOG_DEBUG("CreateTextureCommand constructor");

        if (data)
        {
            m_dataSize = dataSize;
            m_data = malloc(m_dataSize); //TODO: get from pool

            memcpy(m_data, data, dataSize);
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
        if (!m_image->create())
        {
            m_image->destroy();
            m_image->notifyObservers();
            delete m_image;

            return;
        }

        if (m_data)
        {
            //m_image->upload(m_data);
        }
    }

private:
    renderer::Image*            m_image;
    u32                         m_dataSize;
    void*                       m_data;
};

    /*UploadTextureCommand*/
class UploadTextureCommand : public renderer::Command, utils::Observer
{
public:
    UploadTextureCommand(renderer::Image* image)
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
    CommandClearColor(renderer::Image* image, const core::Vector4D& color)
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
    CommandClearDepthStencil(renderer::Image* image, f32 depth, u32 stencil)
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

    /*CommandClearBackbuffer*/
class CommandClearBackbuffer : public renderer::Command
{
public:
    CommandClearBackbuffer(const core::Vector4D& color)
        : m_clearColor(color)
    {
        LOG_DEBUG("CommandClearBackbuffer constructor");
    };

    ~CommandClearBackbuffer()
    {
        LOG_DEBUG("CommandClearBackbuffer destructor");
    };

    void execute(const renderer::CommandList& cmdList)
    {
        cmdList.getContext()->clearBackbuffer(m_clearColor);
    }

private:
    core::Vector4D m_clearColor;
};

/////////////////////////////////////////////////////////////////////////////////////////////////////

Texture2D::Texture2D(renderer::CommandList& cmdList, renderer::ImageFormat format, const core::Dimension2D& dimension, u32 mipmapCount, const void * data)
    : m_cmdList(cmdList)
    , m_target(renderer::TextureTarget::Texture2D)
    , m_format(format)
    , m_dimension(dimension)
    , m_mipmapLevel(mipmapCount)
    , m_samples(renderer::TextureSamples::TextureSamples_x1)

    , m_filter(renderer::TextureFilter_Nearest)
    , m_anisotropicLevel(renderer::TextureAnisotropic::TextureAnisotropic_None)
    , m_wrap(renderer::TextureWrap::TextureWrap_Repeat)

    , m_image(nullptr)

{
    core::Dimension3D dim = { m_dimension.width, m_dimension.height, 1 };
    m_image = m_cmdList.getContext()->createImage(m_target, m_format, dim, m_mipmapLevel, m_filter, m_anisotropicLevel, m_wrap);
    ASSERT(m_image, "m_image is nullptr");
    m_image->registerNotify(this);

    createTexture2D(data);
}

Texture2D::Texture2D(renderer::CommandList & cmdList, renderer::ImageFormat format, const core::Dimension2D & dimension, renderer::TextureSamples samples)
    : m_cmdList(cmdList)
    , m_target(renderer::TextureTarget::Texture2D)
    , m_format(format)
    , m_dimension(dimension)
    , m_mipmapLevel(0)
    , m_samples(samples)

    , m_filter(renderer::TextureFilter_Nearest)
    , m_anisotropicLevel(renderer::TextureAnisotropic::TextureAnisotropic_None)
    , m_wrap(renderer::TextureWrap::TextureWrap_Repeat)

    , m_image(nullptr)
{
    core::Dimension3D dim = { m_dimension.width, m_dimension.height, 1 };
    m_image = m_cmdList.getContext()->createAttachmentImage(m_format, dim, m_samples, m_filter, m_anisotropicLevel, m_wrap);
    ASSERT(m_image, "m_image is nullptr");
    m_image->registerNotify(this);

    createTexture2D(nullptr);
}

void Texture2D::handleNotify(utils::Observable* ob)
{
    m_image = nullptr;
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
            u32 calculatedSize = 0; //TODO:
            //m_image->upload(data);
        }
    }
    else
    {
        u32 calculatedSize = 0; //TODO:
        m_cmdList.pushCommand(new CreateTextureCommand(m_image, calculatedSize, data));
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

    m_image->unregisterNotify(this);
}

renderer::TextureTarget Texture2D::getTarget() const
{
    return m_target;
}

renderer::TextureFilter Texture2D::getMinFilter() const
{
    return renderer::TextureFilter_Nearest;
}

renderer::TextureFilter Texture2D::getMagFilter() const
{
    return renderer::TextureFilter_Nearest;
}

renderer::TextureWrap Texture2D::getWrap() const
{
    return m_wrap;
}

renderer::TextureAnisotropic Texture2D::getAnisotropic() const
{
    return m_anisotropicLevel;
}

renderer::TextureSamples Texture2D::getSampleCount() const
{
    return m_samples;
}

u32 Texture2D::getMipmaps() const
{
    return m_mipmapLevel;
}

const core::Dimension2D& Texture2D::getDimension() const
{
    return m_dimension;
}

renderer::ImageFormat Texture2D::getFormat() const
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

void SwapchainTexture::clear(const core::Vector4D & color)
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
