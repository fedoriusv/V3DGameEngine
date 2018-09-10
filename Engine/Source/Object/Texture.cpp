#include "Texture.h"
#include "Renderer/Image.h"
#include "Renderer/Context.h"
#include "CommandList.h"

namespace v3d
{

    class CreateTextureCommand : public renderer::Command
    {
    public:

        CreateTextureCommand(renderer::Image* image)
            : m_image(image)
        {
        }

        ~CreateTextureCommand()
        {
        }

        void execute(const renderer::CommandList& cmdList) override
        {
            if (!m_image->create())
            {
                //m_image->destroy();
                //m_image->deleteNotify();
            }

        }

    private:

        renderer::Image* m_image;

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



Texture2D::Texture2D(renderer::CommandList& cmdList, const core::Dimension2D & size, u32 mipmapCount, const void * data)
    : m_cmdList(cmdList)
    , m_target(TextureTarget::Texture2D)
    , m_mipmapLevel(mipmapCount)
    , m_size(size)
    , m_image(nullptr)

    , m_filter(TextureFilterNearest)
    , m_anisotropicLevel(TextureAnisotropic::TextureAnisotropicNone)
    , m_wrap(TextureWrap::TextureRepeat)
{
    //m_image create

    m_image = m_cmdList.getContext()->createImage();
    //m_image->registerNotify(this);

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
        //m_cmdList.pushCommand(new CreateTextureCommand());
    }
}

Texture2D::~Texture2D()
{
    //m_image delete
    //m_image = m_imageManager.destroyImage();
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

const core::Dimension2D & Texture2D::getSize() const
{
    return m_size;
}

void Texture2D::update(const core::Dimension2D & offset, const core::Dimension2D & size, u32 mips, const void * data)
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
