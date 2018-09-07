#include "Texture.h"
#include "Renderer/Image.h"
#include "CommandList.h"

namespace v3d
{

    class CreateTextureCommand : public renderer::Command //replace to image manager
    {
    public:

        CreateTextureCommand()
        {
        }
        ~CreateTextureCommand()
        {
        }

        void execute(const renderer::CommandList& cmdList) override
        {
            m_image->create();
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
            m_image->upload();
        }

    private:

        renderer::Image* m_image;
    };



Texture2D::Texture2D(const core::Dimension2D & size, u32 mipmapCount, const void * data)
    : m_target(TextureTarget::Texture2D)
    , m_mipmapLevel(mipmapCount)
    , m_size(size)
    , m_image(nullptr)

    , m_filter(TextureFilterNearest)
    , m_anisotropicLevel(TextureAnisotropic::TextureAnisotropicNone)
    , m_wrap(TextureWrap::TextureRepeat)
{
    //m_image create
    m_image = m_imageManager.createImage();
}

Texture2D::~Texture2D()
{
    //m_image delete
    m_image = m_imageManager.destroyImage();
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
    m_imageManager.upload();
}

void Texture2D::read(const core::Dimension2D & offset, const core::Dimension2D & size, u32 mipLevel, void * const data)
{
    m_imageManager.read();
}

} //namespace v3d
