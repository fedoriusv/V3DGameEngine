#include "VulkanDeviceCaps.h"
#include "Utils/Logger.h"

#ifdef VULKAN_RENDER
#include "VulkanContext.h"
#include "VulkanMemory.h"
#include "VulkanImage.h"
#include "VulkanDescriptorSet.h"

namespace v3d
{
namespace renderer
{
namespace vk
{

u32 makeDriverVersion(VendorID vendor, u32 major, u32 minor, u32 patch = 0)
{
    if (vendor == VendorID::VendorID_NVIDIA)
    {
        return (((major) << 22) | ((minor) << 14) | (patch));
    }

    return (((major) << 22) | ((minor) << 12) | (patch));
}


template <u32 index>
u32 getDriverVersion(VendorID vendor, u32 version)
{
    if (vendor == VendorID::VendorID_NVIDIA)
    {
        return std::get<index>(std::make_tuple((u32)(version) >> 22, (((u32)(version) >> 14) & 0x3ff), 0));
    }

    return std::get<index>(std::make_tuple((u32)(version) >> 22, (((u32)(version) >> 12) & 0x3ff), ((u32)(version) & 0xfff)));
}

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

u32 VulkanDeviceCaps::getQueueFamiliyIndex(VkQueueFlags queueFlags)
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

const VkPhysicalDeviceMemoryProperties& VulkanDeviceCaps::getDeviceMemoryProperties() const
{
    return m_deviceMemoryProps;
}

const VkPhysicalDeviceFeatures& VulkanDeviceCaps::getPhysicalDeviceFeatures() const
{
    return m_deviceFeatures;
}

void VulkanDeviceCaps::fillCapabilitiesList(const DeviceInfo* info)
{
    ASSERT(info->_physicalDevice != VK_NULL_HANDLE, "PhysicalDevice is nullptr");

    //extetions
    auto isEnabledExtension = [](const c8* extension) -> bool
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

    debugUtilsObjectNameEnabled = VulkanDeviceCaps::checkInstanceExtension(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

    ASSERT(isEnabledExtension(VK_KHR_MAINTENANCE1_EXTENSION_NAME), "required VK_KHR_maintenance1 extension");
    ASSERT(isEnabledExtension(VK_KHR_MAINTENANCE2_EXTENSION_NAME), "required VK_KHR_maintenance2 extension");
#if FORCE_DISABLE_FUNCTIONALITY_VULKAN_1_2
    supportRenderpass2 = false;
    supportMultiview = false;
#else
    supportRenderpass2 = isEnabledExtension(VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME);
    supportMultiview = isEnabledExtension(VK_KHR_MULTIVIEW_EXTENSION_NAME);
#endif //FORCE_DISABLE_FUNCTIONALITY_VULKAN_1_2
    enableSamplerMirrorClampToEdge = isEnabledExtension(VK_KHR_SAMPLER_MIRROR_CLAMP_TO_EDGE_EXTENSION_NAME);
    supportDepthAutoResolve = isEnabledExtension(VK_KHR_DEPTH_STENCIL_RESOLVE_EXTENSION_NAME);
    supportDedicatedAllocation = isEnabledExtension(VK_KHR_DEDICATED_ALLOCATION_EXTENSION_NAME);
    supportPipelineExecutableProperties = isEnabledExtension(VK_KHR_PIPELINE_EXECUTABLE_PROPERTIES_EXTENSION_NAME);

    supportBlitImage = true;

    if (VulkanDeviceCaps::checkInstanceExtension(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME))
    {
        //PhysicalDeviceFeatures2
        {
            void* vkExtensions = nullptr;
            if (isEnabledExtension(VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME))
            {
                m_physicalDeviceDescriptorIndexingFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES_EXT;
                m_physicalDeviceDescriptorIndexingFeatures.pNext = nullptr;
                vkExtensions = &m_physicalDeviceDescriptorIndexingFeatures;

                supportDescriptorIndexing = true;
            }

#ifdef VK_EXT_custom_border_color
            if (isEnabledExtension(VK_EXT_CUSTOM_BORDER_COLOR_EXTENSION_NAME))
            {
                m_physicalDeviceCustomBorderColorFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_CUSTOM_BORDER_COLOR_FEATURES_EXT;
                m_physicalDeviceCustomBorderColorFeatures.pNext = vkExtensions;
                vkExtensions = &m_physicalDeviceCustomBorderColorFeatures;
            }
#endif

            VkPhysicalDevicePipelineExecutablePropertiesFeaturesKHR physicalDevicePipelineExecutablePropertiesFeatures = {};
            if (isEnabledExtension(VK_KHR_PIPELINE_EXECUTABLE_PROPERTIES_EXTENSION_NAME))
            {
                physicalDevicePipelineExecutablePropertiesFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PIPELINE_EXECUTABLE_PROPERTIES_FEATURES_KHR;
                physicalDevicePipelineExecutablePropertiesFeatures.pNext = nullptr;
                vkExtensions = &physicalDevicePipelineExecutablePropertiesFeatures;
            }

            VkPhysicalDeviceFeatures2 physicalDeviceFeatures2 = {};
            physicalDeviceFeatures2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
            physicalDeviceFeatures2.pNext = vkExtensions;

            VulkanWrapper::GetPhysicalDeviceFeatures2(info->_physicalDevice, &physicalDeviceFeatures2);
            memcpy(&m_deviceFeatures, &physicalDeviceFeatures2.features, sizeof(VkPhysicalDeviceFeatures));

            supportDescriptorIndexing = m_physicalDeviceDescriptorIndexingFeatures.descriptorBindingUpdateUnusedWhilePending;
#ifdef VK_EXT_custom_border_color
            supportSamplerBorderColor = m_physicalDeviceCustomBorderColorFeatures.customBorderColors && m_physicalDeviceCustomBorderColorFeatures.customBorderColorWithoutFormat;
#endif
            pipelineExecutablePropertiesEnabled = physicalDevicePipelineExecutablePropertiesFeatures.pipelineExecutableInfo;
        }

        //PhysicalDeviceProperties2
        {
            void* vkExtensions = nullptr;

            VkPhysicalDeviceDescriptorIndexingPropertiesEXT physicalDeviceDescriptorIndexingProperties = {};
            if (isEnabledExtension(VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME))
            {
                physicalDeviceDescriptorIndexingProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_PROPERTIES_EXT;
                physicalDeviceDescriptorIndexingProperties.pNext = nullptr;
                vkExtensions = &physicalDeviceDescriptorIndexingProperties;
            }

#if VULKAN_DEBUG
            VkPhysicalDeviceDriverPropertiesKHR physicalDeviceDriverProperties = {};
            if (isEnabledExtension(VK_KHR_DRIVER_PROPERTIES_EXTENSION_NAME))
            {
                physicalDeviceDriverProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ID_PROPERTIES;
                physicalDeviceDriverProperties.pNext = vkExtensions;
                vkExtensions = &physicalDeviceDriverProperties;
            }
#endif //VULKAN_DEBUG

            VkPhysicalDeviceDepthStencilResolvePropertiesKHR physicalDeviceDepthStencilResolveProperties = {};
            if (isEnabledExtension(VK_KHR_DEPTH_STENCIL_RESOLVE_EXTENSION_NAME))
            {
                physicalDeviceDepthStencilResolveProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DEPTH_STENCIL_RESOLVE_PROPERTIES_KHR;
                physicalDeviceDepthStencilResolveProperties.pNext = vkExtensions;
                vkExtensions = &physicalDeviceDepthStencilResolveProperties;
            }

            VkPhysicalDeviceProperties2 physicalDeviceProperties = {};
            physicalDeviceProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
            physicalDeviceProperties.pNext = vkExtensions;

            VulkanWrapper::GetPhysicalDeviceProperties2(info->_physicalDevice, &physicalDeviceProperties);
            memcpy(&m_deviceProperties, &physicalDeviceProperties.properties, sizeof(VkPhysicalDeviceProperties));

#if VULKAN_DEBUG
            if (isEnabledExtension(VK_KHR_DRIVER_PROPERTIES_EXTENSION_NAME) && 0)
            {
                LOG_INFO("VulkanDeviceCaps::initialize: Device Driver Properties:");
                LOG_INFO("VulkanDeviceCaps::initialize: DriverName : %s", physicalDeviceDriverProperties.driverName);
                LOG_INFO("VulkanDeviceCaps::initialize: DriverInfo: %s", physicalDeviceDriverProperties.driverInfo);
                LOG_INFO("VulkanDeviceCaps::initialize: DriverID: %d", physicalDeviceDriverProperties.driverID);
            }
#endif //VULKAN_DEBUG

            supportDepthAutoResolve = supportRenderpass2 && physicalDeviceDepthStencilResolveProperties.supportedDepthResolveModes != VK_RESOLVE_MODE_NONE_KHR;
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
            VkImageFormatProperties imageFormatProperties = {};
            VkFormat vkFormat = VulkanImage::convertImageFormatToVkFormat((Format)index);

            VkFormatProperties foramtProperties = {};
            VulkanWrapper::GetPhysicalDeviceFormatProperties(info->_physicalDevice, vkFormat, &foramtProperties);

            ImageFormatSupport& support = m_imageFormatSupport[index][TilingType::TilingType_Optimal];
            support._supportMip = true;
            support._supportResolve = true;

            {
                VkImageUsageFlags attachmentUsage =  VulkanImage::isColorFormat(vkFormat) ? VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT : VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
                VkFormatFeatureFlags attachmentFeature = VulkanImage::isColorFormat(vkFormat) ? VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT : VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT;
                VkResult result = VulkanWrapper::GetPhysicalDeviceImageFormatProperties(info->_physicalDevice, vkFormat, VK_IMAGE_TYPE_2D, VK_IMAGE_TILING_OPTIMAL, attachmentUsage, 0, &imageFormatProperties);
                support._supportAttachment = (result == VK_SUCCESS && foramtProperties.optimalTilingFeatures & attachmentFeature) ? true : false;
            }

            {
                VkResult result = VulkanWrapper::GetPhysicalDeviceImageFormatProperties(info->_physicalDevice, vkFormat, VK_IMAGE_TYPE_2D, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_SAMPLED_BIT, 0, &imageFormatProperties);
                support._supportSampled = (result == VK_SUCCESS && foramtProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT) ? true : false;
            }

            {
                VkResult result = VulkanWrapper::GetPhysicalDeviceImageFormatProperties(info->_physicalDevice, vkFormat, VK_IMAGE_TYPE_2D, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_STORAGE_BIT, 0, &imageFormatProperties);
                support._supportStorage = (result == VK_SUCCESS && foramtProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_STORAGE_IMAGE_BIT) ? true : false;
            }
        }

        {
            ImageFormatSupport& support = m_imageFormatSupport[index][TilingType::TilingType_Linear];
            support._supportMip = true;

            VkImageFormatProperties imageFormatProperties = {};
            VkFormat vkFormat = VulkanImage::convertImageFormatToVkFormat((Format)index);

            {
                VkResult result = VulkanWrapper::GetPhysicalDeviceImageFormatProperties(info->_physicalDevice, vkFormat, VK_IMAGE_TYPE_2D, VK_IMAGE_TILING_LINEAR, 
                    VulkanImage::isColorFormat(vkFormat) ? VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT : VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, 0, &imageFormatProperties);
                support._supportAttachment = (result == VK_SUCCESS) ? true : false;
            }

            {
                VkResult result = VulkanWrapper::GetPhysicalDeviceImageFormatProperties(info->_physicalDevice, vkFormat, VK_IMAGE_TYPE_2D, VK_IMAGE_TILING_LINEAR, VK_IMAGE_USAGE_SAMPLED_BIT, 0, &imageFormatProperties);
                support._supportSampled = (result == VK_SUCCESS) ? true : false;
            }

            {
                VkResult result = VulkanWrapper::GetPhysicalDeviceImageFormatProperties(info->_physicalDevice, vkFormat, VK_IMAGE_TYPE_2D, VK_IMAGE_TILING_LINEAR, VK_IMAGE_USAGE_STORAGE_BIT, 0, &imageFormatProperties);
                support._supportStorage = (result == VK_SUCCESS) ? true : false;
            }
        }
    }

    VulkanWrapper::GetPhysicalDeviceMemoryProperties(info->_physicalDevice, &m_deviceMemoryProps);
    LOG_INFO("VulkanDeviceCaps Memory:");
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

    supportDeviceCoherentMemory = VulkanMemory::isSupportedMemoryType(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, true);
    supportHostCoherentMemory = VulkanMemory::isSupportedMemoryType(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, false);

    supportDeviceCacheMemory = VulkanMemory::isSupportedMemoryType(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_CACHED_BIT, true);
    supportHostCacheMemory = VulkanMemory::isSupportedMemoryType(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_CACHED_BIT, false);

    vendorID = VendorID(m_deviceProperties.vendorID);
    LOG_INFO("VulkanDeviceCaps::initialize: API version: %u (%u.%u.%u)", m_deviceProperties.apiVersion, VK_VERSION_MAJOR(m_deviceProperties.apiVersion), VK_VERSION_MINOR(m_deviceProperties.apiVersion), VK_VERSION_PATCH(m_deviceProperties.apiVersion));
    LOG_INFO("VulkanDeviceCaps::initialize: Driver version: %u (%u.%u.%u)", m_deviceProperties.driverVersion, getDriverVersion<0>(vendorID, m_deviceProperties.driverVersion), getDriverVersion<1>(vendorID, m_deviceProperties.driverVersion), getDriverVersion<2>(vendorID, m_deviceProperties.driverVersion));
    LOG_INFO("VulkanDeviceCaps::initialize: Vendor ID: %u", m_deviceProperties.vendorID);
    LOG_INFO("VulkanDeviceCaps::initialize: Device ID: %u", m_deviceProperties.deviceID);
    LOG_INFO("VulkanDeviceCaps::initialize: Device Name: %s", m_deviceProperties.deviceName);

    LOG_INFO("VulkanDeviceCaps::initialize:  supportRenderpass2 is %s", supportRenderpass2 ? "supported" : "unsupported");
    LOG_INFO("VulkanDeviceCaps::initialize:  enableSamplerMirrorClampToEdge is %s", enableSamplerMirrorClampToEdge ? "supported" : "unsupported");
    LOG_INFO("VulkanDeviceCaps::initialize:  supportDepthAutoResolve is %s", supportDepthAutoResolve ? "supported" : "unsupported");
    LOG_INFO("VulkanDeviceCaps::initialize:  supportDedicatedAllocation is %s", supportDedicatedAllocation ? "supported" : "unsupported");
    LOG_INFO("VulkanDeviceCaps::initialize:  supportMultiview is %s", supportMultiview ? "supported" : "unsupported");
    LOG_INFO("VulkanDeviceCaps::initialize:  supportBlitImage is %s", supportBlitImage ? "supported" : "unsupported");

#ifdef PLATFORM_ANDROID
#   ifdef VK_QCOM_render_pass_transform
    renderpassTransformQCOM = isEnabledExtension(VK_QCOM_RENDER_PASS_TRANSFORM_EXTENSION_NAME);
    renderpassTransformQCOM = false; //TODO has conflict with preTransform, need check
    LOG_INFO("VulkanDeviceCaps::initialize renderpassTransformQCOM is %u", renderpassTransformQCOM);

    preTransform = true && !renderpassTransformQCOM;
    LOG_INFO("VulkanDeviceCaps::initialize preTransform is %u", preTransform);
#   endif //VK_QCOM_render_pass_transform

#   ifdef VK_EXT_astc_decode_mode
    ASTC_TexturesDecompressed = isEnabledExtension(VK_EXT_ASTC_DECODE_MODE_EXTENSION_NAME);
    LOG_INFO("VulkanDeviceCaps::initialize ASTC_TexturesDecompressed is %u", ASTC_TexturesDecompressed);
#   endif //VK_EXT_astc_decode_mode
#endif //PLATFORM_ANDROID
}

void VulkanDeviceCaps::initialize()
{
    maxDescriptorSetIndex = std::min(k_maxDescriptorSetIndex, m_deviceProperties.limits.maxBoundDescriptorSets);
    maxDescriptorBindingIndex = k_maxDescriptorBindingIndex; // std::min(k_maxDescriptorBindingIndex, m_deviceProperties.limits.maxPerSetDescriptors);
    maxColorAttachments = std::min(k_maxColorAttachments, m_deviceProperties.limits.maxColorAttachments);

    ASSERT(maxColorAttachments <= m_deviceProperties.limits.maxFragmentOutputAttachments, "maxFragmentOutputAttachments less than maxColorAttachments");
    ASSERT(k_maxVertexInputAttributes <= m_deviceProperties.limits.maxVertexInputAttributes, "maxVertexInputAttributes less than k_maxVertexInputAttributes");
    ASSERT(k_maxVertexInputBindings <= m_deviceProperties.limits.maxVertexInputBindings, "maxVertexInputBindings less than k_maxVertexInputBindings");

    individuallyResetForCommandBuffers = true; //For PC

    unifiedMemoryManager = false;

    useGlobalDescriptorPool = true;
    useDynamicUniforms = true;
    useLateDescriptorSetUpdate = false;

    LOG_INFO("VulkanDeviceCaps::initialize:  useDynamicUniforms is %s", useDynamicUniforms ? "enable" : "disable");
    LOG_INFO("VulkanDeviceCaps::initialize:  useGlobalDescriptorPool is %s", useGlobalDescriptorPool ? "enable" : "disable");

    //check !!!!
    immediateResourceSubmit = 2;

#if defined(PLATFORM_ANDROID)
#   ifdef VK_QCOM_render_pass_transform
    fixRenderPassTransformQCOMDriverIssue = (m_deviceProperties.driverVersion < makeDriverVersion(vendorID, 512, 469, 0)) ? true : false;
#   endif //VK_QCOM_render_pass_transform
#endif
}

} //namespace vk
} //namespace renderer
} //namespace v3d
#endif //VULKAN_RENDER
