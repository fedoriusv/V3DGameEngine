#include "VulkanDeviceCaps.h"
#include "Utils/Logger.h"

#ifdef VULKAN_RENDER
#   include "VulkanDevice.h"
#   include "VulkanMemory.h"
#   include "VulkanImage.h"
//#   include "VulkanDescriptorSet.h"

#   ifdef PLATFORM_WINDOWS
extern "C"
{
    __declspec(dllexport) DWORD NvOptimusEnablement = 0x00000001;
    __declspec(dllexport) DWORD AmdPowerXpressRequestHighPerformance = 0x00000001;
}
#   endif //PLATFORM_WINDOWS

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

std::tuple<u32, u32> VulkanDeviceCaps::getQueueFamiliyIndex(VkQueueFlags queueFlags)
{
    // Dedicated queue for compute
    // Try to find a queue family index that supports compute but not graphics
    if (queueFlags & VK_QUEUE_COMPUTE_BIT)
    {
        for (uint32_t i = 0; i < static_cast<uint32_t>(_queueFamilyProperties.size()); i++)
        {
            if ((_queueFamilyProperties[i].queueFlags & queueFlags) && ((_queueFamilyProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) == 0))
            {
                return { i, _queueFamilyProperties[i].queueFlags };
            }
        }
    }

    // Dedicated queue for transfer
    // Try to find a queue family index that supports transfer but not graphics and compute
    if (queueFlags & VK_QUEUE_TRANSFER_BIT)
    {
        for (uint32_t i = 0; i < static_cast<uint32_t>(_queueFamilyProperties.size()); i++)
        {
            if ((_queueFamilyProperties[i].queueFlags & queueFlags) && ((_queueFamilyProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) == 0) && ((_queueFamilyProperties[i].queueFlags & VK_QUEUE_COMPUTE_BIT) == 0))
            {
                return { i, _queueFamilyProperties[i].queueFlags };
            }
        }
    }

    // For other queue types or if no separate compute queue is present, return the first one to support the requested flags
    for (uint32_t i = 0; i < static_cast<uint32_t>(_queueFamilyProperties.size()); i++)
    {
        if (_queueFamilyProperties[i].queueFlags & queueFlags)
        {
            return { i, _queueFamilyProperties[i].queueFlags };
        }
    }

    LOG_WARNING("VulkanDeviceCaps::getQueueFamiliyIndex: Could not find a matching queue family index %d", queueFlags);
    return { -1U, -1U };
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

    _debugUtilsObjectNameEnabled = VulkanDeviceCaps::checkInstanceExtension(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

    ASSERT(isEnabledExtension(VK_KHR_MAINTENANCE_1_EXTENSION_NAME), "required VK_KHR_maintenance1 extension");
    ASSERT(isEnabledExtension(VK_KHR_MAINTENANCE_2_EXTENSION_NAME), "required VK_KHR_maintenance2 extension");
    ASSERT(isEnabledExtension(VK_KHR_MAINTENANCE_3_EXTENSION_NAME), "required VK_KHR_maintenance2 extension");
    _supportRenderpass2 = isEnabledExtension(VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME);
    _supportMultiview = isEnabledExtension(VK_KHR_MULTIVIEW_EXTENSION_NAME);
    _enableSamplerMirrorClampToEdge = isEnabledExtension(VK_KHR_SAMPLER_MIRROR_CLAMP_TO_EDGE_EXTENSION_NAME);
    _supportDepthAutoResolve = isEnabledExtension(VK_KHR_DEPTH_STENCIL_RESOLVE_EXTENSION_NAME);
    _supportDedicatedAllocation = isEnabledExtension(VK_KHR_DEDICATED_ALLOCATION_EXTENSION_NAME);
    _supportPipelineExecutableProperties = isEnabledExtension(VK_KHR_PIPELINE_EXECUTABLE_PROPERTIES_EXTENSION_NAME);

    _supportBlitImage = true;

    if (VulkanDeviceCaps::checkInstanceExtension(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME))
    {
        //PhysicalDeviceFeatures2
        {
            void* vkExtensions = nullptr;

#ifdef VK_EXT_descriptor_indexing
            if (isEnabledExtension(VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME))
            {
                _physicalDeviceDescriptorIndexingFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES_EXT;
                _physicalDeviceDescriptorIndexingFeatures.pNext = vkExtensions;
                vkExtensions = &_physicalDeviceDescriptorIndexingFeatures;

                _supportDescriptorIndexing = true;
            }
#endif

#ifdef VK_EXT_custom_border_color
            if (isEnabledExtension(VK_EXT_CUSTOM_BORDER_COLOR_EXTENSION_NAME))
            {
                _physicalDeviceCustomBorderColorFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_CUSTOM_BORDER_COLOR_FEATURES_EXT;
                _physicalDeviceCustomBorderColorFeatures.pNext = vkExtensions;
                vkExtensions = &_physicalDeviceCustomBorderColorFeatures;
            }
#endif

#ifdef VK_EXT_host_query_reset
            if (isEnabledExtension(VK_EXT_HOST_QUERY_RESET_EXTENSION_NAME))
            {
                _physicalDeviceHostQueryResetFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_HOST_QUERY_RESET_FEATURES;
                _physicalDeviceHostQueryResetFeatures.pNext = vkExtensions;
                vkExtensions = &_physicalDeviceHostQueryResetFeatures;
            }
#endif

            VkPhysicalDevicePipelineExecutablePropertiesFeaturesKHR physicalDevicePipelineExecutablePropertiesFeatures = {};
            if (isEnabledExtension(VK_KHR_PIPELINE_EXECUTABLE_PROPERTIES_EXTENSION_NAME))
            {
                physicalDevicePipelineExecutablePropertiesFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PIPELINE_EXECUTABLE_PROPERTIES_FEATURES_KHR;
                physicalDevicePipelineExecutablePropertiesFeatures.pNext = vkExtensions;
                vkExtensions = &physicalDevicePipelineExecutablePropertiesFeatures;
            }

            VkPhysicalDeviceFeatures2 physicalDeviceFeatures2 = {};
            physicalDeviceFeatures2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
            physicalDeviceFeatures2.pNext = vkExtensions;

            VulkanWrapper::GetPhysicalDeviceFeatures2(info->_physicalDevice, &physicalDeviceFeatures2);
            memcpy(&_deviceFeatures, &physicalDeviceFeatures2.features, sizeof(VkPhysicalDeviceFeatures));

#ifdef VK_EXT_descriptor_indexing
            _supportDescriptorIndexing = _physicalDeviceDescriptorIndexingFeatures.descriptorBindingUpdateUnusedWhilePending;
#endif
#ifdef VK_EXT_custom_border_color
            _supportSamplerBorderColor = _physicalDeviceCustomBorderColorFeatures.customBorderColors && _physicalDeviceCustomBorderColorFeatures.customBorderColorWithoutFormat;
#endif
#ifdef VK_EXT_host_query_reset
            _hostQueryReset = _physicalDeviceHostQueryResetFeatures.hostQueryReset;
#endif
            _pipelineExecutablePropertiesEnabled = physicalDevicePipelineExecutablePropertiesFeatures.pipelineExecutableInfo;
        }

        //PhysicalDeviceProperties2
        {
            void* vkExtensions = nullptr;

            //VK_KHR_maintenance3
            _physicalDeviceMaintenance3Properties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MAINTENANCE_3_PROPERTIES;
            _physicalDeviceMaintenance3Properties.pNext = vkExtensions;
            vkExtensions = &_physicalDeviceMaintenance3Properties;

#ifdef VK_EXT_descriptor_indexing
            VkPhysicalDeviceDescriptorIndexingProperties physicalDeviceDescriptorIndexingProperties = {};
            if (isEnabledExtension(VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME))
            {
                physicalDeviceDescriptorIndexingProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_PROPERTIES_EXT;
                physicalDeviceDescriptorIndexingProperties.pNext = vkExtensions;
                vkExtensions = &physicalDeviceDescriptorIndexingProperties;
            }
#endif

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
            memcpy(&_deviceProperties, &physicalDeviceProperties.properties, sizeof(VkPhysicalDeviceProperties));

#if VULKAN_DEBUG
            if (isEnabledExtension(VK_KHR_DRIVER_PROPERTIES_EXTENSION_NAME) && 0)
            {
                LOG_INFO("VulkanDeviceCaps::initialize: Device Driver Properties:");
                LOG_INFO("VulkanDeviceCaps::initialize: DriverName : %s", physicalDeviceDriverProperties.driverName);
                LOG_INFO("VulkanDeviceCaps::initialize: DriverInfo: %s", physicalDeviceDriverProperties.driverInfo);
                LOG_INFO("VulkanDeviceCaps::initialize: DriverID: %d", physicalDeviceDriverProperties.driverID);
            }
#endif //VULKAN_DEBUG

            _supportDepthAutoResolve = _supportRenderpass2 && physicalDeviceDepthStencilResolveProperties.supportedDepthResolveModes != VK_RESOLVE_MODE_NONE_KHR;
        }
    }
    else
    {
        VulkanWrapper::GetPhysicalDeviceProperties(info->_physicalDevice, &_deviceProperties);
        VulkanWrapper::GetPhysicalDeviceFeatures(info->_physicalDevice, &_deviceFeatures);
    }

    u32 queueFamilyCount = 0;
    VulkanWrapper::GetPhysicalDeviceQueueFamilyProperties(info->_physicalDevice, &queueFamilyCount, nullptr);
    ASSERT(queueFamilyCount > 0, "Must be greater than 0");
    _queueFamilyProperties.resize(queueFamilyCount);
    VulkanWrapper::GetPhysicalDeviceQueueFamilyProperties(info->_physicalDevice, &queueFamilyCount, _queueFamilyProperties.data());

    memset(_imageFormatSupport, 0, sizeof(_imageFormatSupport));
    for (u32 index = 0; index < Format::Format_Count; ++index)
    {
        {
            VkImageFormatProperties imageFormatProperties = {};
            VkFormat vkFormat = VulkanImage::convertImageFormatToVkFormat((Format)index);

            VkFormatProperties foramtProperties = {};
            VulkanWrapper::GetPhysicalDeviceFormatProperties(info->_physicalDevice, vkFormat, &foramtProperties);

            ImageFormatSupport& support = _imageFormatSupport[index][TilingType::TilingType_Optimal];
            support._supportMip = true;
            support._supportResolve = true;

            {
                VkImageUsageFlags attachmentUsage = VulkanImage::isColorFormat(vkFormat) ? VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT : VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
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
            ImageFormatSupport& support = _imageFormatSupport[index][TilingType::TilingType_Linear];
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

    auto memoryProperyFlagsString = [](VkMemoryPropertyFlags flags) -> std::string
    {
        std::string string;
        VkMemoryPropertyFlagBits flagsList[] =
        {
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
            VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            VK_MEMORY_PROPERTY_HOST_CACHED_BIT,
            VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT,
            VK_MEMORY_PROPERTY_PROTECTED_BIT,
            VK_MEMORY_PROPERTY_DEVICE_COHERENT_BIT_AMD,
            VK_MEMORY_PROPERTY_DEVICE_UNCACHED_BIT_AMD,
            VK_MEMORY_PROPERTY_RDMA_CAPABLE_BIT_NV,
        };

        for (auto& flag : flagsList)
        {
            if (flag & flags)
            {
                if (!string.empty())
                {
                    string.append("|");
                }
                string.append(VulkanMemory::memoryPropertyFlagToStringVK(flag));
            }
        }

        return string.empty() ? "EMPTY" : string;
    };

    auto memoryHeapFlags = [](VkMemoryHeapFlags flags) -> std::string
    {
        std::string string;
        if (VK_MEMORY_HEAP_DEVICE_LOCAL_BIT & flags)
        {
            string.append("MEMORY_HEAP_DEVICE_LOCAL_BIT");
        }

        if (VK_MEMORY_HEAP_MULTI_INSTANCE_BIT & flags)
        {
            if (!string.empty())
            {
                string.append("|");
            }
            string.append("MEMORY_HEAP_DEVICE_LOCAL_BIT");
        }

        return string.empty() ? "GENERAL" : string;
    };

    VulkanWrapper::GetPhysicalDeviceMemoryProperties(info->_physicalDevice, &_deviceMemoryProps);
    LOG_INFO("VulkanDeviceCaps::initialize Memory:");
    LOG_INFO("VulkanDeviceCaps::initialize:  memoryHeapCount is %d", _deviceMemoryProps.memoryHeapCount);
    for (u32 i = 0; i < _deviceMemoryProps.memoryHeapCount; ++i)
    {
        LOG_INFO("VulkanDeviceCaps::initialize:    memoryHeap [index %u, flags %s, size %llu]", i, memoryHeapFlags(_deviceMemoryProps.memoryHeaps[i].flags).c_str(), _deviceMemoryProps.memoryHeaps[i].size);
    }
    LOG_INFO("VulkanDeviceCaps::initialize:  memoryTypeCount is %d", _deviceMemoryProps.memoryTypeCount);
    for (u32 i = 0; i < _deviceMemoryProps.memoryTypeCount; ++i)
    {
        LOG_INFO("VulkanDeviceCaps::initialize:    memoryType [heapIndex %u, propertyFlags %s]", _deviceMemoryProps.memoryTypes[i].heapIndex, memoryProperyFlagsString(_deviceMemoryProps.memoryTypes[i].propertyFlags).c_str());
    }

    _supportDeviceCoherentMemory = VulkanMemory::isSupportedMemoryType(_deviceMemoryProps, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, true);
    _supportHostCoherentMemory = VulkanMemory::isSupportedMemoryType(_deviceMemoryProps, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, false);

    _supportDeviceCacheMemory = VulkanMemory::isSupportedMemoryType(_deviceMemoryProps, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_CACHED_BIT, true);
    _supportHostCacheMemory = VulkanMemory::isSupportedMemoryType(_deviceMemoryProps, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_CACHED_BIT, false);

    _vendorID = VendorID(_deviceProperties.vendorID);
    LOG_INFO("VulkanDeviceCaps::initialize: API version: %u (%u.%u.%u)", _deviceProperties.apiVersion, VK_VERSION_MAJOR(_deviceProperties.apiVersion), VK_VERSION_MINOR(_deviceProperties.apiVersion), VK_VERSION_PATCH(_deviceProperties.apiVersion));
    LOG_INFO("VulkanDeviceCaps::initialize: Driver version: %u (%u.%u.%u)", _deviceProperties.driverVersion, getDriverVersion<0>(_vendorID, _deviceProperties.driverVersion), getDriverVersion<1>(_vendorID, _deviceProperties.driverVersion), getDriverVersion<2>(_vendorID, _deviceProperties.driverVersion));
    LOG_INFO("VulkanDeviceCaps::initialize: Vendor ID: %u", _deviceProperties.vendorID);
    LOG_INFO("VulkanDeviceCaps::initialize: Device ID: %u", _deviceProperties.deviceID);
    LOG_INFO("VulkanDeviceCaps::initialize: Device Name: %s", _deviceProperties.deviceName);

    LOG_INFO("VulkanDeviceCaps::initialize:  supportRenderpass2 is %s", _supportRenderpass2 ? "supported" : "unsupported");
    LOG_INFO("VulkanDeviceCaps::initialize:  enableSamplerMirrorClampToEdge is %s", _enableSamplerMirrorClampToEdge ? "supported" : "unsupported");
    LOG_INFO("VulkanDeviceCaps::initialize:  supportDepthAutoResolve is %s", _supportDepthAutoResolve ? "supported" : "unsupported");
    LOG_INFO("VulkanDeviceCaps::initialize:  supportDedicatedAllocation is %s", _supportDedicatedAllocation ? "supported" : "unsupported");
    LOG_INFO("VulkanDeviceCaps::initialize:  supportMultiview is %s", _supportMultiview ? "supported" : "unsupported");
    LOG_INFO("VulkanDeviceCaps::initialize:  supportBlitImage is %s", _supportBlitImage ? "supported" : "unsupported");

#ifdef PLATFORM_ANDROID
#   ifdef VK_QCOM_render_pass_transform
    _renderpassTransformQCOM = isEnabledExtension(VK_QCOM_RENDER_PASS_TRANSFORM_EXTENSION_NAME);
    _renderpassTransformQCOM = false; //TODO has conflict with preTransform, need check
    LOG_INFO("VulkanDeviceCaps::initialize renderpassTransformQCOM is %u", _renderpassTransformQCOM);

    _preTransform = true && !_renderpassTransformQCOM;
    LOG_INFO("VulkanDeviceCaps::initialize preTransform is %u", preTransform);
#   endif //VK_QCOM_render_pass_transform

#   ifdef VK_EXT_astc_decode_mode
    _ASTC_TexturesDecompressed = isEnabledExtension(VK_EXT_ASTC_DECODE_MODE_EXTENSION_NAME);
    LOG_INFO("VulkanDeviceCaps::initialize ASTC_TexturesDecompressed is %u", _ASTC_TexturesDecompressed);
#   endif //VK_EXT_astc_decode_mode
#endif //PLATFORM_ANDROID
}

void VulkanDeviceCaps::initialize()
{
    _maxDescriptorSets = std::min(k_maxDescriptorSetCount, _deviceProperties.limits.maxBoundDescriptorSets);
    _maxDescriptorBindings = std::min(k_maxDescriptorBindingCount, _physicalDeviceMaintenance3Properties.maxPerSetDescriptors);
    _maxColorAttachments = std::min(k_maxColorAttachments, _deviceProperties.limits.maxColorAttachments);

    ASSERT(_maxColorAttachments <= _deviceProperties.limits.maxFragmentOutputAttachments, "maxFragmentOutputAttachments less than maxColorAttachments");
    ASSERT(k_maxVertexInputAttributes <= _deviceProperties.limits.maxVertexInputAttributes, "maxVertexInputAttributes less than k_maxVertexInputAttributes");
    ASSERT(k_maxVertexInputBindings <= _deviceProperties.limits.maxVertexInputBindings, "maxVertexInputBindings less than k_maxVertexInputBindings");

    _individuallyResetForCommandBuffers = true; //For PC

    _unifiedMemoryManager = false;

    _useGlobalDescriptorPool = true;
    _useDynamicUniforms = false; //TODO
    _useLateDescriptorSetUpdate = false;

    LOG_INFO("VulkanDeviceCaps::initialize:  useDynamicUniforms is %s", _useDynamicUniforms ? "enable" : "disable");
    LOG_INFO("VulkanDeviceCaps::initialize:  useGlobalDescriptorPool is %s", _useGlobalDescriptorPool ? "enable" : "disable");

    //check !!!!
    _immediateResourceSubmit = 2;

#if defined(PLATFORM_ANDROID)
#   ifdef VK_QCOM_render_pass_transform
    _fixRenderPassTransformQCOMDriverIssue = (_deviceProperties.driverVersion < makeDriverVersion(vendorID, 512, 469, 0)) ? true : false;
#   endif //VK_QCOM_render_pass_transform
#endif
}

} //namespace vk
} //namespace renderer
} //namespace v3d
#endif //VULKAN_RENDER
