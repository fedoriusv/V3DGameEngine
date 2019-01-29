#pragma once

#include "Renderer/DeviceCaps.h"
#include "Utils/Singleton.h"

#ifdef VULKAN_RENDER
#include "VulkanWrapper.h"

namespace v3d
{
namespace renderer
{
namespace vk
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    struct VulkanDeviceCaps : DeviceCaps, utils::Singleton<VulkanDeviceCaps>
    {
        VulkanDeviceCaps() = default;

        bool individuallyResetForCommandBuffers;
        bool supportCoherentMemory;

        bool useDynamicUniforms;
        bool unifiedMemoryManager;
        bool useStagingBuffers;
        bool useGlobalDescriptorPool;

        static bool checkInstanceExtension(const c8* extensionName);
        static bool checkDeviceExtension(VkPhysicalDevice physicalDevice, const c8* extensionName);

        static void listOfInstanceExtensions(std::vector<std::string>& extensionsList);
        static void listOfDeviceExtensions(VkPhysicalDevice physicalDevice, std::vector<std::string>& extensionsList);

        u32 getQueueFamiliyIndex(VkQueueFlagBits queueFlags);

        const VkPhysicalDeviceLimits& getPhysicalDeviceLimits() const;
        const VkPhysicalDeviceMemoryProperties&  getDeviceMemoryProperties() const;
        const VkPhysicalDeviceFeatures& getPhysicalDeviceFeatures() const;

    private:

        friend class VulkanGraphicContext;

        void fillCapabilitiesList(const struct DeviceInfo* info);
        void initialize();

        VkPhysicalDeviceFeatures            m_deviceFeatures;
        VkPhysicalDeviceProperties          m_deviceProperties;
        VkPhysicalDeviceMemoryProperties    m_deviceMemoryProps;
        std::vector<VkQueueFamilyProperties> m_queueFamilyProperties;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace vk
} //namespace renderer
} //namespace v3d
#endif //VULKAN_RENDER
