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

VulkanFence::VulkanFence(VulkanDevice* device, const std::string& name) noexcept
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
        LOG_ERROR("VulkanFence::VulkanFence vkCreateFence. Error %s", ErrorString(result).c_str());
    }

#if VULKAN_DEBUG_MARKERS
    m_debugName = "Fence";
    m_debugName.append(VulkanDebugUtils::k_addressPreffix);
    m_debugName.append(std::to_string(reinterpret_cast<const u64>(this)));

    if (m_device.getVulkanDeviceCaps()._debugUtilsObjectNameEnabled)
    {
        VkDebugUtilsObjectNameInfoEXT debugUtilsObjectNameInfo = {};
        debugUtilsObjectNameInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
        debugUtilsObjectNameInfo.pNext = nullptr;
        debugUtilsObjectNameInfo.objectType = VK_OBJECT_TYPE_FENCE;
        debugUtilsObjectNameInfo.objectHandle = reinterpret_cast<u64>(m_fence);
        debugUtilsObjectNameInfo.pObjectName = m_debugName.c_str();

        VulkanWrapper::SetDebugUtilsObjectName(m_device.getDeviceInfo()._device, &debugUtilsObjectNameInfo);
    }
#endif //VULKAN_DEBUG_MARKERS
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