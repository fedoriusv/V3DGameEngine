#include "VulkanDeviceCaps.h"
#include "Utils/Logger.h"

#ifdef VULKAN_RENDER
#include "VulkanGraphicContext.h"
#include "VulkanMemory.h"
#include "VulkanImage.h"
#include "VulkanDescriptorSet.h"

namespace v3d
{
namespace renderer
{
namespace vk
{

std::vector<const c8*> VulkanDeviceCaps::s_enableExtensions = {};

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

const VkPhysicalDeviceLimits& VulkanDeviceCaps::getPhysicalDeviceLimits() const
{
    return m_deviceProperties.limits;
}

const VkPhysicalDeviceMemoryProperties & VulkanDeviceCaps::getDeviceMemoryProperties() const
{
    return m_deviceMemoryProps;
}

const VkPhysicalDeviceFeatures & VulkanDeviceCaps::getPhysicalDeviceFeatures() const
{
    return m_deviceFeatures;
}

void VulkanDeviceCaps::fillCapabilitiesList(const DeviceInfo* info)
{
    ASSERT(info->_physicalDevice != VK_NULL_HANDLE, "PhysicalDevice is nullptr");

    //extetions
    auto isEnableExtension = [](const c8* extension) -> bool
    {
        for (auto& iter : s_enableExtensions)
        {
            if (!strcmp(extension, iter))
            {
                return true;
            }
        }

        return false;
    };

    supportRenderpass2 = false;//isEnableExtension(VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME); //TODO temporary disabled because has validation layer errors bug
    enableSamplerMirrorClampToEdge = isEnableExtension(VK_KHR_SAMPLER_MIRROR_CLAMP_TO_EDGE_EXTENSION_NAME);
    supportDepthAutoResolve = isEnableExtension(VK_KHR_DEPTH_STENCIL_RESOLVE_EXTENSION_NAME);
#if PLATFORM_ANDROID
    supportDedicatedAllocation = false; //TODO temporary disabled because has crash on GetImageMemoryRequirements2
#else
    supportDedicatedAllocation = false;//isEnableExtension(VK_KHR_DEDICATED_ALLOCATION_EXTENSION_NAME);
#endif
    supportPipelineExecutableProperties = isEnableExtension(VK_KHR_PIPELINE_EXECUTABLE_PROPERTIES_EXTENSION_NAME);

    if (VulkanDeviceCaps::checkInstanceExtension(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME))
    {
        {
            void* vkExtensions = nullptr;

            if (isEnableExtension(VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME))
            {
                m_physicalDeviceDescriptorIndexingFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES_EXT;
                m_physicalDeviceDescriptorIndexingFeatures.pNext = nullptr;
                vkExtensions = &m_physicalDeviceDescriptorIndexingFeatures;

                supportDescriptorIndexing = true;
            }

            VkPhysicalDeviceFeatures2 physicalDeviceFeatures2 = {};
            physicalDeviceFeatures2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
            physicalDeviceFeatures2.pNext = vkExtensions;

            VulkanWrapper::GetPhysicalDeviceFeatures2(info->_physicalDevice, &physicalDeviceFeatures2);
            memcpy(&m_deviceFeatures, &physicalDeviceFeatures2.features, sizeof(VkPhysicalDeviceFeatures));


            supportDescriptorIndexing = m_physicalDeviceDescriptorIndexingFeatures.descriptorBindingUpdateUnusedWhilePending;
        }

        {
            void* vkExtensions = nullptr;

            VkPhysicalDeviceDescriptorIndexingPropertiesEXT physicalDeviceDescriptorIndexingProperties = {};
            if (isEnableExtension(VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME))
            {
                physicalDeviceDescriptorIndexingProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_PROPERTIES_EXT;
                physicalDeviceDescriptorIndexingProperties.pNext = nullptr;
                vkExtensions = &physicalDeviceDescriptorIndexingProperties;
            }

#ifdef VK_KHR_depth_stencil_resolve
            VkPhysicalDeviceDepthStencilResolvePropertiesKHR physicalDeviceDepthStencilResolveProperties = {};
            if (isEnableExtension(VK_KHR_DEPTH_STENCIL_RESOLVE_EXTENSION_NAME))
            {
                physicalDeviceDepthStencilResolveProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DEPTH_STENCIL_RESOLVE_PROPERTIES_KHR;
                physicalDeviceDepthStencilResolveProperties.pNext = vkExtensions;
                vkExtensions = &physicalDeviceDepthStencilResolveProperties;
            }
#endif // VK_KHR_depth_stencil_resolve

            VkPhysicalDeviceProperties2 physicalDeviceProperties = {};
            physicalDeviceProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
            physicalDeviceProperties.pNext = vkExtensions;

            VulkanWrapper::GetPhysicalDeviceProperties2(info->_physicalDevice, &physicalDeviceProperties);
            memcpy(&m_deviceProperties, &physicalDeviceProperties.properties, sizeof(VkPhysicalDeviceProperties));


#ifdef VK_KHR_depth_stencil_resolve
            supportDepthAutoResolve = physicalDeviceDepthStencilResolveProperties.supportedDepthResolveModes != VK_RESOLVE_MODE_NONE_KHR;
#endif // VK_KHR_depth_stencil_resolve
        }
    }
    else
    {
        VulkanWrapper::GetPhysicalDeviceProperties(info->_physicalDevice, &m_deviceProperties);
        VulkanWrapper::GetPhysicalDeviceFeatures(info->_physicalDevice, &m_deviceFeatures);
    }

    u32 queueFamilyCount = 0;
    VulkanWrapper::GetPhysicalDeviceQueueFamilyProperties(info->_physicalDevice, &queueFamilyCount, nullptr);
    ASSERT(queueFamilyCount > 0, "Must be greater than 0");
    m_queueFamilyProperties.resize(queueFamilyCount);
    VulkanWrapper::GetPhysicalDeviceQueueFamilyProperties(info->_physicalDevice, &queueFamilyCount, m_queueFamilyProperties.data());

    memset(m_imageFormatSupport, 0, sizeof(m_imageFormatSupport));
    for (u32 index = 0; index < Format::Format_Count; ++index)
    {
        {
            ImageFormatSupport& support = m_imageFormatSupport[index][TilingType::TilingType_Optimal];

            VkImageFormatProperties imageFormatProperties = {};
            VkFormat vkFormat = VulkanImage::convertImageFormatToVkFormat((Format)index);

            VkResult result = VulkanWrapper::GetPhysicalDeviceImageFormatProperties(info->_physicalDevice, vkFormat, VK_IMAGE_TYPE_2D, VK_IMAGE_TILING_OPTIMAL,
                VulkanImage::isColorFormat(vkFormat) ? VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT : VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, 0, &imageFormatProperties);
            support._supportAttachment = (result == VK_SUCCESS) ? true : false;

            result = VulkanWrapper::GetPhysicalDeviceImageFormatProperties(info->_physicalDevice, vkFormat, VK_IMAGE_TYPE_2D, VK_IMAGE_TILING_OPTIMAL,
                VK_IMAGE_USAGE_SAMPLED_BIT, 0, &imageFormatProperties);
            support._supportSampled = (result == VK_SUCCESS) ? true : false;
        }

        {
            ImageFormatSupport& support = m_imageFormatSupport[index][TilingType::TilingType_Linear];

            VkImageFormatProperties imageFormatProperties = {};
            VkFormat vkFormat = VulkanImage::convertImageFormatToVkFormat((Format)index);

            VkResult result = VulkanWrapper::GetPhysicalDeviceImageFormatProperties(info->_physicalDevice, vkFormat, VK_IMAGE_TYPE_2D, VK_IMAGE_TILING_LINEAR,
                VulkanImage::isColorFormat(vkFormat) ? VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT : VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, 0, &imageFormatProperties);
            support._supportAttachment = (result == VK_SUCCESS) ? true : false;

            result = VulkanWrapper::GetPhysicalDeviceImageFormatProperties(info->_physicalDevice, vkFormat, VK_IMAGE_TYPE_2D, VK_IMAGE_TILING_LINEAR,
                VK_IMAGE_USAGE_SAMPLED_BIT, 0, &imageFormatProperties);
            support._supportSampled = (result == VK_SUCCESS) ? true : false;
        }
    }

    //check !!!!
    immediateResourceSubmit = 2;

    //VK_EXT_memory_budget
    //VK_EXT_memory_priority

    LOG_INFO("VulkanDeviceCaps::initialize:  supportRenderpass2 is %s", supportRenderpass2 ? "supported" : "unsupported");
    LOG_INFO("VulkanDeviceCaps::initialize:  enableSamplerMirrorClampToEdge is %s", enableSamplerMirrorClampToEdge ? "supported" : "unsupported");
    LOG_INFO("VulkanDeviceCaps::initialize:  supportDepthAutoResolve is %s", supportDepthAutoResolve ? "supported" : "unsupported");
    LOG_INFO("VulkanDeviceCaps::initialize:  supportDedicatedAllocation is %s", supportDedicatedAllocation ? "supported" : "unsupported");

    LOG_INFO("VulkanDeviceCaps::initialize:  useDynamicUniforms is %s", useDynamicUniforms ? "enable" : "disable");

    VulkanWrapper::GetPhysicalDeviceMemoryProperties(info->_physicalDevice, &m_deviceMemoryProps);
    LOG_INFO("VulkanDeviceCaps::initialize:  memoryHeapCount is %d", m_deviceMemoryProps.memoryHeapCount);
    for (u32 i = 0; i < m_deviceMemoryProps.memoryHeapCount; ++i)
    {
        LOG_INFO("VulkanDeviceCaps::initialize:    memoryHeap [flags %d, size %llu]", m_deviceMemoryProps.memoryHeaps[i].flags, m_deviceMemoryProps.memoryHeaps[i].size);
    }
    LOG_INFO("VulkanDeviceCaps::initialize:  memoryTypeCount is %d", m_deviceMemoryProps.memoryTypeCount);
    for (u32 i = 0; i < m_deviceMemoryProps.memoryTypeCount; ++i)
    {
        LOG_INFO("VulkanDeviceCaps::initialize:    memoryType [heapIndex %u, propertyFlags %d]", m_deviceMemoryProps.memoryTypes[i].heapIndex, m_deviceMemoryProps.memoryTypes[i].propertyFlags);
    }
}

void VulkanDeviceCaps::initialize()
{
    maxDescriptorSetIndex = std::min(k_maxDescriptorSetIndex, m_deviceProperties.limits.maxBoundDescriptorSets);
    maxDescriptorBindingIndex = k_maxDescriptorBindingIndex; // std::min(k_maxDescriptorBindingIndex, m_deviceProperties.limits.maxPerSetDescriptors);

    individuallyResetForCommandBuffers = true; //For PC

    supportDeviceCoherentMemory = VulkanMemory::isSupportedMemoryType(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, true);
    supportHostCoherentMemory = VulkanMemory::isSupportedMemoryType(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, false);

    supportDeviceCacheMemory = VulkanMemory::isSupportedMemoryType(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_CACHED_BIT, true);
    supportHostCacheMemory = VulkanMemory::isSupportedMemoryType(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_CACHED_BIT, false);

    unifiedMemoryManager = false;

    ASSERT(k_maxFramebufferAttachments <= m_deviceProperties.limits.maxFragmentOutputAttachments, "maxFragmentOutputAttachments less than k_maxFramebufferAttachments");
    ASSERT(k_maxVertexInputAttributes <= m_deviceProperties.limits.maxVertexInputAttributes, "maxVertexInputAttributes less than k_maxVertexInputAttributes");
    ASSERT(k_maxVertexInputBindings <= m_deviceProperties.limits.maxVertexInputBindings, "maxVertexInputBindings less than k_maxVertexInputBindings");

    useGlobalDescriptorPool = false;
    useDynamicUniforms = true;
    useLateDescriptorSetUpdate = false;
}

} //namespace vk
} //namespace renderer
} //namespace v3d
#endif //VULKAN_RENDER
