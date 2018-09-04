#include "VulkanDeviceCaps.h"
#include "VulkanGraphicContext.h"
#include "Utils/Logger.h"

#ifdef VULKAN_RENDER
namespace v3d
{
namespace renderer
{
namespace vk
{

bool VulkanDeviceCaps::checkInstanceExtension(const c8 * extensionName)
{
    u32 extensionCount = 0;
    std::vector<VkExtensionProperties> extensions;
    VulkanWrapper::EnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
    extensions.resize(extensionCount);
    VulkanWrapper::EnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());
    for (auto& ext : extensions)
    {
        if (!strcmp(extensionName, ext.extensionName))
        {
            return true;
        }
    }
    return false;
}

bool VulkanDeviceCaps::checkDeviceExtension(VkPhysicalDevice physicalDevice, const c8 * extensionName)
{
    u32 extensionCount = 0;
    std::vector<VkExtensionProperties> extensions;
    VulkanWrapper::EnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount, nullptr);
    extensions.resize(extensionCount);
    VulkanWrapper::EnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount, extensions.data());
    for (auto& ext : extensions)
    {
        if (!strcmp(extensionName, ext.extensionName))
        {
            return true;
        }
    }
    return false;
}

void VulkanDeviceCaps::listOfInstanceExtensions(std::vector<std::string>& extensionsList)
{
    u32 extensionCount = 0;
    std::vector<VkExtensionProperties> extensions;
    VulkanWrapper::EnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
    extensions.resize(extensionCount);
    VulkanWrapper::EnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());

    extensionsList.clear();
    extensionsList.reserve(extensionCount);
    for (auto& ext : extensions)
    {
        extensionsList.push_back(std::string(ext.extensionName));
    }
}

void VulkanDeviceCaps::listOfDeviceExtensions(VkPhysicalDevice physicalDevice, std::vector<std::string>& extensionsList)
{
    u32 extensionCount = 0;
    std::vector<VkExtensionProperties> extensions;
    VulkanWrapper::EnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount, nullptr);
    extensions.resize(extensionCount);
    VulkanWrapper::EnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount, extensions.data());

    extensionsList.clear();
    extensionsList.reserve(extensionCount);
    for (auto& ext : extensions)
    {
        extensionsList.push_back(std::string(ext.extensionName));
    }
}

u32 VulkanDeviceCaps::getQueueFamiliyIndex(VkQueueFlagBits queueFlags)
{
    // Dedicated queue for compute
    // Try to find a queue family index that supports compute but not graphics
    if (queueFlags & VK_QUEUE_COMPUTE_BIT)
    {
        for (uint32_t i = 0; i < static_cast<uint32_t>(m_queueFamilyProperties.size()); i++)
        {
            if ((m_queueFamilyProperties[i].queueFlags & queueFlags) && ((m_queueFamilyProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) == 0))
            {
                return i;
                break;
            }
        }
    }

    // Dedicated queue for transfer
    // Try to find a queue family index that supports transfer but not graphics and compute
    if (queueFlags & VK_QUEUE_TRANSFER_BIT)
    {
        for (uint32_t i = 0; i < static_cast<uint32_t>(m_queueFamilyProperties.size()); i++)
        {
            if ((m_queueFamilyProperties[i].queueFlags & queueFlags) &&
                ((m_queueFamilyProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) == 0) &&
                ((m_queueFamilyProperties[i].queueFlags & VK_QUEUE_COMPUTE_BIT) == 0))
            {
                return i;
                break;
            }
        }
    }

    // For other queue types or if no separate compute queue is present, return the first one to support the requested flags
    for (uint32_t i = 0; i < static_cast<uint32_t>(m_queueFamilyProperties.size()); i++)
    {
        if (m_queueFamilyProperties[i].queueFlags & queueFlags)
        {
            return i;
            break;
        }
    }

    LOG_WARNING("VulkanDeviceCaps::getQueueFamiliyIndex: Could not find a matching queue family index %d", queueFlags);
    return 0;
}

void VulkanDeviceCaps::fillCapabilitiesList(const DeviceInfo* info)
{
    ASSERT(info->_physicalDevice != VK_NULL_HANDLE, "PhysicalDevice is nullptr");
    VulkanWrapper::GetPhysicalDeviceProperties(info->_physicalDevice, &m_deviceProperties);
    VulkanWrapper::GetPhysicalDeviceFeatures(info->_physicalDevice, &m_deviceFeatures);
    VulkanWrapper::GetPhysicalDeviceMemoryProperties(info->_physicalDevice, &m_deviceMemoryProps);

    u32 queueFamilyCount = 0;
    VulkanWrapper::GetPhysicalDeviceQueueFamilyProperties(info->_physicalDevice, &queueFamilyCount, nullptr);
    ASSERT(queueFamilyCount > 0, "Must be greater than 0");
    m_queueFamilyProperties.resize(queueFamilyCount);
    VulkanWrapper::GetPhysicalDeviceQueueFamilyProperties(info->_physicalDevice, &queueFamilyCount, m_queueFamilyProperties.data());

    //TODO:
}

} //namespace vk
} //namespace renderer
} //namespace v3d
#endif //VULKAN_RENDER
