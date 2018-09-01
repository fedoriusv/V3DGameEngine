#pragma once

#include "DeviceCaps.h"
#ifdef VULKAN_RENDER
#include "VulkanWrapper.h"

namespace v3d
{
namespace renderer
{
namespace vk
{
    struct VulkanDeviceCaps : DeviceCaps
    {
        //TODO

        static bool checkInstanceExtension(const c8* extensionName);
        static bool checkDeviceExtension(VkPhysicalDevice physicalDevice, const c8* extensionName);

        static void listOfInstanceExtensions(std::vector<std::string>& extensionsList);
        static void listOfDeviceExtensions(VkPhysicalDevice physicalDevice, std::vector<std::string>& extensionsList);

    private:

        void fillCapabilitiesList();

    };

} //namespace vk
} //namespace renderer
} //namespace v3d
#endif //VULKAN_RENDER
