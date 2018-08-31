#include "VulkanSwapchain.h"
#include "Utils/Logger.h"

namespace v3d
{
namespace renderer
{
namespace vk
{

VulkanSwapchain::VulkanSwapchain()
{
    LOG_DEBUG("VulkanSwapchain constructor %llx", this);
}

VulkanSwapchain::~VulkanSwapchain()
{
    LOG_DEBUG("VulkanSwapchain destructor %llx", this);
}

bool VulkanSwapchain::create()
{
#ifdef PLATFORM_WINDOWS

#else 
    LOG_FATAL("VulkanSwapchain::create not implement for this platform");
    return false;
#endif
}

void VulkanSwapchain::destroy()
{
}

void VulkanSwapchain::present()
{
}

void VulkanSwapchain::acquireImage()
{
}

} //namespace vk
} //namespace renderer
} //namespace v3d
