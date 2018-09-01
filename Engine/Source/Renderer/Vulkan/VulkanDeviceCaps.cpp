#include "VulkanDeviceCaps.h"

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

void VulkanDeviceCaps::fillCapabilitiesList()
{
}

} //namespace vk
} //namespace renderer
} //namespace v3d
#endif //VULKAN_RENDER
