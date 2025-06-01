#pragma once

#include "Renderer/DeviceCaps.h"

#ifdef VULKAN_RENDER
#   include "VulkanWrapper.h"

namespace v3d
{
namespace renderer
{
namespace vk
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    class VulkanDevice;

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief VulkanDeviceCaps struct. Vulkan Render side
    */
    struct VulkanDeviceCaps : DeviceCaps
    {
        bool _individuallyResetForCommandBuffers = false;

        //coherent memmory
        bool _supportDeviceCoherentMemory = false;
        bool _supportHostCoherentMemory = false;

        //cache memory
        bool _supportDeviceCacheMemory = false;
        bool _supportHostCacheMemory = false;

        bool _supportDepthAutoResolve = false;

        bool _useDynamicUniforms = false;
        bool _unifiedMemoryManager = false;
        bool _useGlobalDescriptorPool = false;
        bool _useLateDescriptorSetUpdate = false;

        bool _supportRenderpass2 = false;
        bool _supportDedicatedAllocation = false;

        bool _supportPipelineExecutableProperties = false;

        bool _supportDescriptorIndexing = false;

        //immediateResourceSubmit: 0 - not submit, 1 - submit, 2 - submit and wait complete
        u32 _immediateResourceSubmit = 0; 

        bool _enableSamplerMirrorClampToEdge = false;
        bool _supportSamplerBorderColor = false;
        bool _hostQueryReset = false;
        bool _timelineSemaphore = false;
        bool _supportDiscardAsDemote = false;

        bool _preTransform = false;
        bool _renderpassTransformQCOM = false;
        bool _fixRenderPassTransformQCOMDriverIssue = false;

        bool _ASTC_TexturesDecompressed = false;

        static bool checkInstanceExtension(const c8* extensionName);
        static bool checkDeviceExtension(VkPhysicalDevice physicalDevice, const c8* extensionName);

        static void listOfInstanceExtensions(std::vector<std::string>& extensionsList);
        static void listOfDeviceExtensions(VkPhysicalDevice physicalDevice, std::vector<std::string>& extensionsList);

        std::tuple<u32, u32> getQueueFamiliyIndex(VkQueueFlags queueFlags);

        const VkPhysicalDeviceLimits& getPhysicalDeviceLimits() const;
        const VkPhysicalDeviceMemoryProperties&  getDeviceMemoryProperties() const;
        const VkPhysicalDeviceFeatures& getPhysicalDeviceFeatures() const;
        const VkPhysicalDeviceMaintenance3Properties& getPhysicalDeviceMaintenance3Properties() const;

        bool _debugUtilsObjectNameEnabled = false;
        bool _pipelineExecutablePropertiesEnabled = false;

        u32 _memoryImagePoolSize = 64 * 1024 * 1024; //default: 64MB
        u32 _memoryBufferPoolSize = 4 * 1024 * 1024; //default: 4MB
        u32 _memoryMinQueryPoolCount = 1024;//default: 1024

        u32 _globalDescriptorPoolSize = 2048; //Count of sets in the single pool
        u32 _layoutDescriptorPoolSize = 2048; //Count of sets in the single pool

    private:

        VulkanDeviceCaps() = default;

        VulkanDeviceCaps(const VulkanDeviceCaps&) = delete;
        VulkanDeviceCaps& operator=(const VulkanDeviceCaps&) = delete;

        friend VulkanDevice;

        void fillCapabilitiesList(const struct DeviceInfo* info);
        void initialize();

        VkPhysicalDeviceFeatures _deviceFeatures = {};
        VkPhysicalDeviceProperties _deviceProperties = {};
        VkPhysicalDeviceMemoryProperties _deviceMemoryProps = {};
        std::vector<VkQueueFamilyProperties> _queueFamilyProperties = {};
        VkPhysicalDeviceMaintenance3Properties _physicalDeviceMaintenance3Properties = {};
#ifdef VK_EXT_descriptor_indexing
        VkPhysicalDeviceDescriptorIndexingFeaturesEXT _physicalDeviceDescriptorIndexingFeatures = {};
#endif
#ifdef VK_EXT_custom_border_color
        VkPhysicalDeviceCustomBorderColorFeaturesEXT _physicalDeviceCustomBorderColorFeatures = {};
#endif
#ifdef VK_EXT_host_query_reset
        VkPhysicalDeviceHostQueryResetFeatures _physicalDeviceHostQueryResetFeatures = {};
#endif
#ifdef VK_EXT_shader_demote_to_helper_invocation
        VkPhysicalDeviceShaderDemoteToHelperInvocationFeaturesEXT _physicalDeviceShaderDemoteToHelperInvocationFeatures = {};
#endif
        static std::vector<const c8*> s_enableExtensions;
    };


    inline const VkPhysicalDeviceLimits& VulkanDeviceCaps::getPhysicalDeviceLimits() const
    {
        return _deviceProperties.limits;
    }

    inline const VkPhysicalDeviceMemoryProperties& VulkanDeviceCaps::getDeviceMemoryProperties() const
    {
        return _deviceMemoryProps;
    }

    inline const VkPhysicalDeviceFeatures& VulkanDeviceCaps::getPhysicalDeviceFeatures() const
    {
        return _deviceFeatures;
    }

    inline const VkPhysicalDeviceMaintenance3Properties& VulkanDeviceCaps::getPhysicalDeviceMaintenance3Properties() const
    {
        return _physicalDeviceMaintenance3Properties;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace vk
} //namespace renderer
} //namespace v3d
#endif //VULKAN_RENDER
