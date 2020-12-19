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

    /**
    * @brief VulkanDeviceCaps struct. Vulkan Render side
    */
    struct VulkanDeviceCaps : DeviceCaps, utils::Singleton<VulkanDeviceCaps>
    {
        VulkanDeviceCaps() = default;

        VulkanDeviceCaps(const VulkanDeviceCaps&) = delete;
        VulkanDeviceCaps& operator=(const VulkanDeviceCaps&) = delete;


        bool individuallyResetForCommandBuffers = false;

        //coherent memmory
        bool supportDeviceCoherentMemory = false;
        bool supportHostCoherentMemory = false;

        //cache memory
        bool supportDeviceCacheMemory = false;
        bool supportHostCacheMemory = false;

        bool supportDepthAutoResolve = false;

        bool useDynamicUniforms = false;
        bool unifiedMemoryManager = false;
        bool useGlobalDescriptorPool = false;
        bool useLateDescriptorSetUpdate = false;

        bool supportRenderpass2 = false;
        bool supportDedicatedAllocation = false;

        bool supportPipelineExecutableProperties = false;

        bool supportDescriptorIndexing = false;

        //immediateResourceSubmit: 0 - not submit, 1 - submit, 2 - submit and wait complete
        u32 immediateResourceSubmit = 0; 

        u32 maxDescriptorSetIndex = 0;
        u32 maxDescriptorBindingIndex = 0;

        bool enableSamplerMirrorClampToEdge = false;
        bool supportSamplerBorderColor = false;

        bool renderpassTransformQCOM = false;
        bool fixRenderPassTransformQCOMDriverIssue = false;

        static bool checkInstanceExtension(const c8* extensionName);
        static bool checkDeviceExtension(VkPhysicalDevice physicalDevice, const c8* extensionName);

        static void listOfInstanceExtensions(std::vector<std::string>& extensionsList);
        static void listOfDeviceExtensions(VkPhysicalDevice physicalDevice, std::vector<std::string>& extensionsList);

        u32 getQueueFamiliyIndex(VkQueueFlagBits queueFlags);

        const VkPhysicalDeviceLimits& getPhysicalDeviceLimits() const;
        const VkPhysicalDeviceMemoryProperties&  getDeviceMemoryProperties() const;
        const VkPhysicalDeviceFeatures& getPhysicalDeviceFeatures() const;

        bool debugUtilsObjectNameEnabled = false;
        bool pipelineExecutablePropertiesEnabled = false;

    private:

        friend class VulkanGraphicContext;

        void fillCapabilitiesList(const struct DeviceInfo* info);
        void initialize();

        VkPhysicalDeviceFeatures            m_deviceFeatures = {};
        VkPhysicalDeviceProperties          m_deviceProperties = {};
        VkPhysicalDeviceMemoryProperties    m_deviceMemoryProps = {};
        std::vector<VkQueueFamilyProperties> m_queueFamilyProperties = {};

        VkPhysicalDeviceDescriptorIndexingFeaturesEXT m_physicalDeviceDescriptorIndexingFeatures = {};
        VkPhysicalDeviceCustomBorderColorFeaturesEXT m_physicalDeviceCustomBorderColorFeatures = {};

        static std::vector<const c8*>        s_enableExtensions;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace vk
} //namespace renderer
} //namespace v3d
#endif //VULKAN_RENDER
