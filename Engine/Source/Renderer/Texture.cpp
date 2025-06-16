#include "Texture.h"
#include "Device.h"
#include "Utils/Logger.h"

namespace v3d
{
namespace renderer
{

Texture::Texture(TextureTarget target, Format format, TextureSamples samples, u32 layers, u32 mipmaps, TextureUsageFlags usage) noexcept
    : m_texture(nullptr)
    , m_target(target)
    , m_format(format)
    , m_samples(samples)
    , m_layers(layers)
    , m_mipmaps(mipmaps)
    , m_usage(usage)
{
}

Texture::~Texture()
{
}


Texture2D::Texture2D(Device* device, TextureUsageFlags usage, Format format, const math::Dimension2D& dimension, u32 array, u32 mipmaps, const std::string& name) noexcept
    : Texture(TextureTarget::Texture2D, format, TextureSamples::TextureSamples_x1, array, mipmaps, usage)
    , m_device(device)
    , m_dimension(dimension)
{
    LOG_DEBUG("Texture2D::Texture2D constructor %llx", this);

    m_texture = m_device->createTexture(TextureTarget::Texture2D, format, math::Dimension3D(dimension._width, dimension._height, 1), array, mipmaps, usage, name);
    ASSERT(m_texture.isValid(), "nullptr");
}

Texture2D::Texture2D(Device* device, TextureUsageFlags usage, Format format, const math::Dimension2D& dimension, TextureSamples samples, const std::string& name) noexcept
    : Texture(TextureTarget::Texture2D, format, samples, 1, 1, usage)
    , m_device(device)
    , m_dimension(dimension)
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


TextureCube::TextureCube(Device* device, TextureUsageFlags usage, Format format, const math::Dimension2D& dimension, u32 mipmaps, const std::string& name) noexcept
    : Texture(TextureTarget::TextureCubeMap, format, TextureSamples::TextureSamples_x1, 1, mipmaps, usage)
    , m_device(device)
    , m_dimension(dimension)
{
    LOG_DEBUG("TextureCube::TextureCube constructor %llx", this);

    m_texture = m_device->createTexture(TextureTarget::TextureCubeMap, format, math::Dimension3D(dimension._width, dimension._height, 1), 6, mipmaps, usage, name);
    ASSERT(m_texture.isValid(), "nullptr");
}

TextureCube::TextureCube(Device* device, TextureUsageFlags usage, Format format, const math::Dimension2D& dimension, TextureSamples samples, const std::string& name) noexcept
    : Texture(TextureTarget::TextureCubeMap, format, samples, 1, 1, usage)
    , m_device(device)
    , m_dimension(dimension)
{
    LOG_DEBUG("TextureCube::TextureCube constructor %llx", this);

    m_texture = m_device->createTexture(TextureTarget::TextureCubeMap, format, math::Dimension3D(dimension._width, dimension._height, 1), 6, samples, usage, name);
    ASSERT(m_texture.isValid(), "nullptr");
}

TextureCube::~TextureCube()
{
    LOG_DEBUG("TextureCube::TextureCube destructor %llx", this);

    ASSERT(m_texture.isValid(), "nullptr");
    m_device->destroyTexture(m_texture);
}


Texture3D::Texture3D(Device* device, TextureUsageFlags usage, Format format, const math::Dimension3D& dimension, u32 mipmaps, const std::string& name) noexcept
    : Texture(TextureTarget::Texture3D, format, TextureSamples::TextureSamples_x1, 1, mipmaps, usage)
    , m_device(device)
    , m_dimension(dimension)
{
    LOG_DEBUG("Texture3D::Texture3D constructor %llx", this);

    m_texture = m_device->createTexture(TextureTarget::Texture2DArray, format, dimension, 1, mipmaps, usage, name);
    ASSERT(m_texture.isValid(), "nullptr");
}

Texture3D::Texture3D(Device* device, TextureUsageFlags usage, Format format, const math::Dimension3D& dimension, TextureSamples samples, const std::string& name) noexcept
    : Texture(TextureTarget::Texture3D, format, samples, 1, 1, usage)
    , m_device(device)
    , m_dimension(dimension)
{
    LOG_DEBUG("Texture3D::Texture3D constructor %llx", this);

    m_texture = m_device->createTexture(TextureTarget::Texture2DArray, format, dimension, 1, samples, usage, name);
    ASSERT(m_texture.isValid(), "nullptr");
}

Texture3D::~Texture3D()
{
    LOG_DEBUG("Texture3D::Texture3D destructor %llx", this);

    ASSERT(m_texture.isValid(), "nullptr");
    m_device->destroyTexture(m_texture);
}

UnorderedAccessTexture2D::UnorderedAccessTexture2D(Device* device, TextureUsageFlags usage, Format format, const math::Dimension2D& dimension, u32 array, const std::string& name) noexcept
    : Texture(TextureTarget::Texture2D, format, TextureSamples::TextureSamples_x1, array, 1, usage)
    , m_device(device)
    , m_dimension(dimension)
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
    : Texture(TextureTarget::Texture2D, swapchain->getBackbufferFormat(), TextureSamples::TextureSamples_x1, 1, 1, swapchain->getUsageFlags())
    , m_swapchain(swapchain)
{
    LOG_DEBUG("SwapchainTexture::SwapchainTexture constructor %llx", this);
    m_texture = TextureHandle(swapchain);
}

SwapchainTexture::~SwapchainTexture()
{
    LOG_DEBUG("SwapchainTexture::SwapchainTexture destructor %llx", this);
}

} //namespace renderer
} //namespace v3d