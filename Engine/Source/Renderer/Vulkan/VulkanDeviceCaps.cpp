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
    VulkanWrapper::GetPhysicalDeviceProperties(info->_physicalDevice, &m_deviceProperties);
    VulkanWrapper::GetPhysicalDeviceFeatures(info->_physicalDevice, &m_deviceFeatures);
    VulkanWrapper::GetPhysicalDeviceMemoryProperties(info->_physicalDevice, &m_deviceMemoryProps);

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

    //TODO:
}

void VulkanDeviceCaps::initialize()
{
    maxDescriptorSetIndex = k_maxDescriptorSetIndex;
    maxDescriptorBindingIndex = maxDescriptorBindingIndex;

    individuallyResetForCommandBuffers = true; //For PC

    supportDeviceCoherentMemory = VulkanMemory::isSupportedMemoryType(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, true);
    supportHostCoherentMemory = VulkanMemory::isSupportedMemoryType(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, false);
    useDynamicUniforms = false;

    unifiedMemoryManager = false;
    useStagingBuffers = !VulkanMemory::isSupportedMemoryType(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, false);

    ASSERT(k_maxFramebufferAttachments <= m_deviceProperties.limits.maxFragmentOutputAttachments, "maxFragmentOutputAttachments less than k_maxFramebufferAttachments");
    ASSERT(k_maxVertexInputAttributes <= m_deviceProperties.limits.maxVertexInputAttributes, "maxVertexInputAttributes less than k_maxVertexInputAttributes");
    ASSERT(k_maxVertexInputBindings <= m_deviceProperties.limits.maxVertexInputBindings, "maxVertexInputBindings less than k_maxVertexInputBindings");

    useGlobalDescriptorPool = true;

    //extetions
    enableSamplerMirrorClampToEdge = std::find(s_enableExtensions.cbegin(), s_enableExtensions.cend(), VK_KHR_SAMPLER_MIRROR_CLAMP_TO_EDGE_EXTENSION_NAME) != s_enableExtensions.cend();
}

} //namespace vk
} //namespace renderer
} //namespace v3d
#endif //VULKAN_RENDER
