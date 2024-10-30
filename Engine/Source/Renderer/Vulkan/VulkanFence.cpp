#include "VulkanFence.h"
#include "Utils/Logger.h"

#ifdef VULKAN_RENDER
#   include "VulkanDevice.h"
#   include "VulkanDeviceCaps.h"
#   include "VulkanDebug.h"

namespace v3d
{
namespace renderer
{
namespace vk
{

VulkanFence::VulkanFence(VulkanDevice* device) noexcept
    : m_device(*device)
    , m_fence(VK_NULL_HANDLE)
    , m_completed(0)
{
    VkFenceCreateInfo fenceCreateInfo = {};
    fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceCreateInfo.pNext = nullptr;
    fenceCreateInfo.flags = 0;

    VkResult result = VulkanWrapper::CreateFence(m_device.getDeviceInfo()._device, &fenceCreateInfo, VULKAN_ALLOCATOR, &m_fence);
    if (result != VK_SUCCESS)
    {
        LOG_ERROR("VulkanCommandBuffer::VulkanCommandBuffer vkCreateFence. Error %s", ErrorString(result).c_str());
    }
}

VulkanFence::~VulkanFence()
{
    if (m_fence != VK_NULL_HANDLE)
    {
        VulkanWrapper::DestroyFence(m_device.getDeviceInfo()._device, m_fence, VULKAN_ALLOCATOR);
        m_fence = VK_NULL_HANDLE;
    }
}

} //namespace vk
} //namespace renderer
} //namespace v3d
#endif  //VULKAN_RENDER