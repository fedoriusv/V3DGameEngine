#include "Swapchain.h"
#include "Texture.h"
#include "Utils/Logger.h"

namespace v3d
{
namespace renderer
{

Swapchain::Swapchain() noexcept
    : m_frameCounter(0)
{
    memset(&m_backufferDescription, 0, sizeof(m_backufferDescription));
}

Swapchain::~Swapchain() noexcept
{
}

bool Swapchain::create(Device* device, Format format, const math::Dimension2D& size, TextureUsageFlags flags)
{
    m_backufferDescription._format = format;
    m_backufferDescription._size = size;
    m_backufferDescription._usageFlags = flags;
    m_backufferDescription._swapchain = V3D_NEW(SwapchainTexture, memory::MemoryLabel::MemoryRenderCore)(device, this);

    return true;
}

void Swapchain::destroy()
{
    V3D_DELETE(m_backufferDescription._swapchain, memory::MemoryLabel::MemoryRenderCore);
}

} //namespace renderer
} //namespace v3d