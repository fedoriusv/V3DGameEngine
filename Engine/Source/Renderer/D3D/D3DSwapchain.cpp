#include "D3DSwapchain.h"

#include "Utils/Logger.h"

#ifdef D3D_RENDER
namespace v3d
{
namespace renderer
{
namespace d3d
{

D3DSwapchain::D3DSwapchain(const DeviceInfo* info)
{
}

D3DSwapchain::~D3DSwapchain()
{
}

bool D3DSwapchain::create(const SwapchainConfig& config)
{
    return false;
}

void D3DSwapchain::destroy()
{
}

void D3DSwapchain::present()
{
}

u32 D3DSwapchain::acquireImage()
{
    return u32();
}

bool D3DSwapchain::recteate(const SwapchainConfig& config)
{
    return false;
}

} //namespace d3d
} //namespace renderer
} //namespace v3d
#endif //D3D_RENDER
