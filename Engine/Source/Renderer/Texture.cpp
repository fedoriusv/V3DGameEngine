#include "Texture.h"
#include "Device.h"
#include "Utils/Logger.h"
#include "Stream/Stream.h"

namespace v3d
{
namespace renderer
{

Texture::Texture(Device* const device, TextureTarget target, Format format, const math::Dimension3D& size, TextureSamples samples, u32 layers, u32 mipmaps, TextureUsageFlags usage) noexcept
    : m_header(target)
    , m_device(device)
    , m_texture(nullptr)
    , m_format(format)
    , m_dimension(size)
    , m_target(target)
    , m_samples(samples)
    , m_layers(layers)
    , m_mipmaps(mipmaps)
    , m_usage(usage)
{
}

Texture::~Texture()
{
}

bool Texture::load(const stream::Stream* stream, u32 offset)
{
    if (m_loaded)
    {
        LOG_WARNING("Texture::load: the texture %llx is already loaded", this);
        return true;
    }

    ASSERT(stream, "stream is nullptr");
    stream->seekBeg(offset);
    ASSERT(offset == m_header._offset, "wrong offset");

    stream->read<TextureTarget>(m_target);
    stream->read<math::Dimension3D>(m_dimension);
    stream->read<Format>(m_format);
    stream->read<TextureSamples>(m_samples); //always TextureSamples_x1
    stream->read<u32>(m_layers);
    stream->read<u32>(m_mipmaps);
    stream->read<u32>(m_usage);

    m_texture = m_device->createTexture(m_target, m_format, m_dimension, m_layers, m_mipmaps, m_usage, std::string(m_header.getName()));
    ASSERT(m_texture.isValid(), "nullptr");

    u32 size = 0;
    stream->read<u32>(size);
    if (size > 0)
    {
        void* ptr = stream->map(size);
        renderer::CmdListRender* cmdList = m_device->createCommandList<renderer::CmdListRender>(renderer::Device::GraphicMask);

        cmdList->upload(this, size, ptr);
        m_device->submit(cmdList, true);

        m_device->destroyCommandList(cmdList);
        stream->unmap();
    }
    LOG_DEBUG("Texture::load: The stream has been read %d from %d bytes", stream->tell() - m_header._offset, m_header._size);

    m_loaded = true;
    return true;
}

bool Texture::save(stream::Stream* stream, u32 offset) const
{
    //TODO

    return false;
}

Texture1D::Texture1D(Device* device, const TextureHeader& header) noexcept
    : Texture(device, TextureTarget::Texture1D, Format::Format_Undefined, {}, TextureSamples::TextureSamples_x1, 0, 0, 0)
{
    LOG_DEBUG("Texture1D::Texture1D constructor %llx", this);
}

Texture1D::Texture1D(Device* device, TextureUsageFlags usage, Format format, u32 width, u32 array, u32 mipmaps, const std::string& name) noexcept
    : Texture(device, TextureTarget::Texture2D, format, { width, 1, 1 }, TextureSamples::TextureSamples_x1, array, mipmaps, usage)
{
    LOG_DEBUG("Texture1D::Texture1D constructor %llx", this);

    m_texture = m_device->createTexture(TextureTarget::Texture1D, format, math::Dimension3D(width, 1, 1), array, mipmaps, usage, name);
    ASSERT(m_texture.isValid(), "nullptr");
}

Texture1D::~Texture1D()
{
    LOG_DEBUG("Texture1D::Texture1D destructor %llx", this);

    ASSERT(m_texture.isValid(), "nullptr");
    m_device->destroyTexture(m_texture);
}


Texture2D::Texture2D(Device* device, const TextureHeader& header) noexcept
    : Texture(device, TextureTarget::Texture2D, Format::Format_Undefined, {}, TextureSamples::TextureSamples_x1, 0, 0, 0)
{
    LOG_DEBUG("Texture2D::Texture2D constructor %llx", this);
}

Texture2D::Texture2D(Device* device, TextureUsageFlags usage, Format format, const math::Dimension2D& dimension, u32 array, u32 mipmaps, const std::string& name) noexcept
    : Texture(device, TextureTarget::Texture2D, format, dimension, TextureSamples::TextureSamples_x1, array, mipmaps, usage)
{
    LOG_DEBUG("Texture2D::Texture2D constructor %llx", this);

    m_texture = m_device->createTexture(TextureTarget::Texture2D, format, math::Dimension3D(dimension._width, dimension._height, 1), array, mipmaps, usage, name);
    ASSERT(m_texture.isValid(), "nullptr");
}

Texture2D::Texture2D(Device* device, TextureUsageFlags usage, Format format, const math::Dimension2D& dimension, TextureSamples samples, const std::string& name) noexcept
    : Texture(device, TextureTarget::Texture2D, format, dimension, samples, 1, 1, usage)
{
    LOG_DEBUG("Texture2D::Texture2D constructor %llx", this);

    m_texture = m_device->createTexture(TextureTarget::Texture2D, format, math::Dimension3D(dimension._width, dimension._height, 1), 1, samples, usage, name);
    ASSERT(m_texture.isValid(), "nullptr");
}

Texture2D::~Texture2D()
{
    LOG_DEBUG("Texture2D::Texture2D destructor %llx", this);

    ASSERT(m_texture.isValid(), "nullptr");
    m_device->destroyTexture(m_texture);
}


TextureCube::TextureCube(Device* device, const TextureHeader& header) noexcept
    : Texture(device, TextureTarget::TextureCubeMap, Format::Format_Undefined, {}, TextureSamples::TextureSamples_x1, 0, 0, 0)
{
    LOG_DEBUG("TextureCube::TextureCube constructor %llx", this);
}

TextureCube::TextureCube(Device* device, TextureUsageFlags usage, Format format, const math::Dimension2D& dimension, u32 mipmaps, const std::string& name) noexcept
    : Texture(device, TextureTarget::TextureCubeMap, format, dimension, TextureSamples::TextureSamples_x1, 6, mipmaps, usage)
{
    LOG_DEBUG("TextureCube::TextureCube constructor %llx", this);

    m_texture = m_device->createTexture(TextureTarget::TextureCubeMap, format, math::Dimension3D(dimension._width, dimension._height, 1), 6, mipmaps, usage, name);
    ASSERT(m_texture.isValid(), "nullptr");
}

TextureCube::~TextureCube()
{
    LOG_DEBUG("TextureCube::TextureCube destructor %llx", this);

    ASSERT(m_texture.isValid(), "nullptr");
    m_device->destroyTexture(m_texture);
}


Texture3D::Texture3D(Device* device, const TextureHeader& header) noexcept
    : Texture(device, TextureTarget::Texture3D, Format::Format_Undefined, {}, TextureSamples::TextureSamples_x1, 0, 0, 0)
{
    LOG_DEBUG("Texture3D::Texture3D constructor %llx", this);
}

Texture3D::Texture3D(Device* device, TextureUsageFlags usage, Format format, const math::Dimension3D& dimension, u32 mipmaps, const std::string& name) noexcept
    : Texture(device, TextureTarget::Texture3D, format, dimension, TextureSamples::TextureSamples_x1, 1, mipmaps, usage)
{
    LOG_DEBUG("Texture3D::Texture3D constructor %llx", this);

    m_texture = m_device->createTexture(TextureTarget::Texture3D, format, dimension, 1, mipmaps, usage, name);
    ASSERT(m_texture.isValid(), "nullptr");
}

Texture3D::~Texture3D()
{
    LOG_DEBUG("Texture3D::Texture3D destructor %llx", this);

    ASSERT(m_texture.isValid(), "nullptr");
    m_device->destroyTexture(m_texture);
}


UnorderedAccessTexture2D::UnorderedAccessTexture2D(Device* device, TextureUsageFlags usage, Format format, const math::Dimension2D& dimension, u32 array, const std::string& name) noexcept
    : Texture(device, TextureTarget::Texture2D, format, dimension, TextureSamples::TextureSamples_x1, array, 1, usage)
{
    LOG_DEBUG("UnorderedAccessTexture2D::UnorderedAccessTexture2D constructor %llx", this);

    m_texture = m_device->createTexture(TextureTarget::Texture2D, format, math::Dimension3D(dimension._width, dimension._height, 1), array, 1, usage, name);
    ASSERT(m_texture.isValid(), "nullptr");
}

UnorderedAccessTexture2D::~UnorderedAccessTexture2D()
{
    LOG_DEBUG("UnorderedAccessTexture2D::UnorderedAccessTexture2D destructor %llx", this);

    ASSERT(m_texture.isValid(), "nullptr");
    m_device->destroyTexture(m_texture);
}

SwapchainTexture::SwapchainTexture(Device* device, Swapchain* swapchain) noexcept
    : Texture(device, TextureTarget::Texture2D, swapchain->getBackbufferFormat(), swapchain->getBackbufferSize(), TextureSamples::TextureSamples_x1, 1, 1, swapchain->getUsageFlags())
    , m_swapchain(swapchain)
{
    LOG_DEBUG("SwapchainTexture::SwapchainTexture constructor %llx", this);
    m_texture = TextureHandle(swapchain);
}

SwapchainTexture::~SwapchainTexture()
{
    LOG_DEBUG("SwapchainTexture::SwapchainTexture destructor %llx", this);
}

inline u32 SwapchainTexture::getWidth() const
{
    return m_swapchain->getBackbufferSize()._width;
}

inline u32 SwapchainTexture::getHeight() const
{
    return m_swapchain->getBackbufferSize()._height;
}

} //namespace renderer
} //namespace v3d