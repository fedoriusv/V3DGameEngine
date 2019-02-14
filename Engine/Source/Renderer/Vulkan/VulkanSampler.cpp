#include "VulkanSampler.h"

#include "Utils/Logger.h"

#ifdef VULKAN_RENDER
#include "VulkanDebug.h"
#include "VulkanDeviceCaps.h"

namespace v3d
{
namespace renderer
{
namespace vk
{

VulkanSampler::VulkanSampler()
{
}

VulkanSampler::~VulkanSampler()
{
}

bool VulkanSampler::create()
{
    return false;
}

void VulkanSampler::destroy()
{
}

VkSampler VulkanSampler::getHandle() const
{
    return VK_NULL_HANDLE/*m_sampler*/;
}

} //namespace vk
} //namespace renderer
} //namespace v3d
#endif //VULKAN_RENDER
