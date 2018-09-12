#include "Texture.h"
#include "Renderer/Image.h"
#include "Renderer/Context.h"
#include "CommandList.h"

namespace v3d
{

    class CreateTextureCommand : public renderer::Command
    {
    public:

        CreateTextureCommand(renderer::Image* image, u32 dataSize, const void * data)

            : m_image(image) //TODO: need use handle
        {
            if (data)
            {
                m_dataSize = dataSize;
                m_data = malloc(m_dataSize); //TODO: get from pool

                memcpy(m_data, data, dataSize);
            }
        }

        ~CreateTextureCommand()
        {
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

    class UploadTextureCommand : public renderer::Command
    {
    public:

        UploadTextureCommand(renderer::Image* image)
            : m_image(image)
        {
        }
        ~UploadTextureCommand()
        {
        }

        void execute(const renderer::CommandList& cmdList) override
        {
            //m_image->upload();
        }

    private:

        renderer::Image* m_image;
    };



Texture2D::Texture2D(renderer::CommandList& cmdList, renderer::ImageFormat format, const core::Dimension2D& dimension, u32 mipmapCount, const void * data)
    : m_cmdList(cmdList)
    , m_target(TextureTarget::Texture2D)
    , m_format(format)
    , m_dimension(dimension)
    , m_mipmapLevel(mipmapCount)
    , m_image(nullptr)

    , m_filter(TextureFilterNearest)
    , m_anisotropicLevel(TextureAnisotropic::TextureAnisotropicNone)
    , m_wrap(TextureWrap::TextureRepeat)
{
    core::Dimension3D dim = { m_dimension.width, m_dimension.height, 1 };
    m_image = m_cmdList.getContext()->createImage(m_target, m_format, dim, m_mipmapLevel, m_filter, m_anisotropicLevel, m_wrap);
    ASSERT(m_image, "m_image is nullptr");

    m_image->registerNotify(this);

    if (m_cmdList.isImmediate())
    {
        if (!m_image->create())
        {
            m_image->destroy();

            m_image->unregisterNotify(this);

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

void Texture2D::handleNotify()
{
    m_image->unregisterNotify(this);
    delete m_image;
    m_image = nullptr;
}

Texture2D::~Texture2D()
{
    //m_image delete
    //m_image = m_imageManager.destroyImage();

    m_image->unregisterNotify(this);
}

TextureTarget Texture2D::getTarget() const
{
    return m_target;
}

TextureFilter Texture2D::getMinFilter() const
{
    return TextureFilterNearest;
}

TextureFilter Texture2D::getMagFilter() const
{
    return TextureFilterNearest;
}

TextureWrap Texture2D::getWrap() const
{
    return m_wrap;
}

TextureAnisotropic Texture2D::getAnisotropic() const
{
    return m_anisotropicLevel;
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
    if (m_image)
    {
        //Assert
    }

    if (m_cmdList.isImmediate())
    {
        //m_image->upload();
    }
    else
    {
        //m_cmdList.pushCommand(new UploadTextureCommand());
    }
}

void Texture2D::read(const core::Dimension2D & offset, const core::Dimension2D & size, u32 mipLevel, void * const data)
{
    //m_imageManager.read();
}

} //namespace v3d
