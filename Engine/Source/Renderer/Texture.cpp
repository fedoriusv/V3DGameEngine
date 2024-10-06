#include "Texture.h"

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


Texture2D::Texture2D(Device* device, TextureUsageFlags usage, Format format, const math::Dimension2D& dimension, u32 mipmaps, const std::string& name) noexcept
    : Texture(TextureTarget::Texture2D, format, TextureSamples::TextureSamples_x1, 1, mipmaps, usage)
{
}

Texture2D::Texture2D(Device* device, TextureUsageFlags usage, Format format, const math::Dimension2D& dimension, TextureSamples samples, const std::string& name) noexcept
    : Texture(TextureTarget::Texture2D, format, samples, 1, 1, usage)
{
}

Texture2D::~Texture2D()
{
}


Texture2DArray::Texture2DArray(Device* device, TextureUsageFlags usage, Format format, const math::Dimension2D& dimension, u32 layer, u32 mipmaps, const std::string& name) noexcept
    : Texture(TextureTarget::Texture2DArray, format, TextureSamples::TextureSamples_x1, 1, mipmaps, usage)
{
}

Texture2DArray::Texture2DArray(Device* device, TextureUsageFlags usage, Format format, const math::Dimension2D& dimension, u32 layer, TextureSamples samples, const std::string& name) noexcept
    : Texture(TextureTarget::Texture2DArray, format, samples, 1, 1, usage)
{
}

Texture2DArray::~Texture2DArray()
{
}


TextureCube::TextureCube(Device* device, TextureUsageFlags usage, Format format, const math::Dimension2D& dimension, u32 mipmaps, const std::string& name) noexcept
    : Texture(TextureTarget::TextureCubeMap, format, TextureSamples::TextureSamples_x1, 1, mipmaps, usage)
{
}

TextureCube::TextureCube(Device* device, TextureUsageFlags usage, Format format, const math::Dimension2D& dimension, TextureSamples samples, const std::string& name) noexcept
    : Texture(TextureTarget::TextureCubeMap, format, samples, 1, 1, usage)
{
}

TextureCube::~TextureCube()
{
}


Texture3D::Texture3D(Device* device, TextureUsageFlags usage, Format format, const math::Dimension3D& dimension, u32 mipmaps, const std::string& name) noexcept
    : Texture(TextureTarget::Texture3D, format, TextureSamples::TextureSamples_x1, 1, mipmaps, usage)
{
}

Texture3D::Texture3D(Device* device, TextureUsageFlags usage, Format format, const math::Dimension3D& dimension, TextureSamples samples, const std::string& name) noexcept
    : Texture(TextureTarget::Texture3D, format, samples, 1, 1, usage)
{
}

Texture3D::~Texture3D()
{
}

SwapchainTexture::SwapchainTexture(Device* device, Swapchain* swapchain) noexcept
    : Texture(TextureTarget::Texture2D, swapchain->getBackbufferFormat(), TextureSamples::TextureSamples_x1, 1, 1, swapchain->getUsageFlags())
    , m_swapchain(swapchain)
{
    m_texture = TextureHandle(swapchain);
}

SwapchainTexture::~SwapchainTexture()
{
}

} //namespace renderer
} //namespace v3d