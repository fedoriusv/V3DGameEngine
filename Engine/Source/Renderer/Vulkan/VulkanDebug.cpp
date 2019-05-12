#include "VulkanDebug.h"
#include "Utils/Logger.h"

#ifdef VULKAN_RENDER
#include "VulkanWrapper.h"
namespace v3d
{
namespace renderer
{
namespace vk
{

std::string ErrorString(VkResult errorCode)
{
    switch (errorCode)
    {
#define STR(r) case VK_ ##r: return #r
        STR(SUCCESS);
        STR(NOT_READY);
        STR(TIMEOUT);
        STR(EVENT_SET);
        STR(EVENT_RESET);
        STR(INCOMPLETE);
        STR(ERROR_OUT_OF_HOST_MEMORY);
        STR(ERROR_OUT_OF_DEVICE_MEMORY);
        STR(ERROR_INITIALIZATION_FAILED);
        STR(ERROR_DEVICE_LOST);
        STR(ERROR_MEMORY_MAP_FAILED);
        STR(ERROR_LAYER_NOT_PRESENT);
        STR(ERROR_EXTENSION_NOT_PRESENT);
        STR(ERROR_FEATURE_NOT_PRESENT);
        STR(ERROR_INCOMPATIBLE_DRIVER);
        STR(ERROR_TOO_MANY_OBJECTS);
        STR(ERROR_FORMAT_NOT_SUPPORTED);
        STR(ERROR_OUT_OF_POOL_MEMORY);
        STR(ERROR_SURFACE_LOST_KHR);
        STR(ERROR_NATIVE_WINDOW_IN_USE_KHR);
        STR(SUBOPTIMAL_KHR);
        STR(ERROR_OUT_OF_DATE_KHR);
        STR(ERROR_INCOMPATIBLE_DISPLAY_KHR);
        STR(ERROR_VALIDATION_FAILED_EXT);
        STR(ERROR_INVALID_SHADER_NV);
#undef STR
    default:
        return "UNKNOWN_ERROR";
    }
}

const std::vector<const c8*> VulkanDebug::s_validationLayerNames =
{
#ifdef PLATFORM_WINDOWS
    // This is a meta layer that enables all of the standard
    // validation layers in the correct order :
    // threading, parameter_validation, device_limits, object_tracker, image, core_validation, swapchain, and unique_objects
    "VK_LAYER_LUNARG_standard_validation"
#else
    // Parse individual layers in specific order and as they're available, include them
    "VK_LAYER_GOOGLE_threading",
    "VK_LAYER_LUNARG_parameter_validation", // should be early in the list
    "VK_LAYER_LUNARG_object_tracker",
    "VK_LAYER_LUNARG_core_validation",
    "VK_LAYER_LUNARG_device_limits",
    "VK_LAYER_LUNARG_image",
    "VK_LAYER_LUNARG_swapchain",
    "VK_LAYER_LUNARG_swapchain",
    "VK_LAYER_GOOGLE_unique_objects", // should be late in the list, gets data early from the driver
#endif
};

VkDebugUtilsMessengerEXT VulkanDebug::s_messeger = VK_NULL_HANDLE;

bool VulkanDebug::createDebugUtilsMesseger(VkInstance instance, VkDebugUtilsMessageSeverityFlagsEXT severityFlag, VkDebugUtilsMessageTypeFlagsEXT flags, PFN_vkDebugUtilsMessengerCallbackEXT callback, void * userData)
{
    if (s_messeger)
    {
        LOG_WARNING("VulkanDebug::createDebugUtilsMessagerCallback: already exist");
        return true;
    }

    VkDebugUtilsMessengerCreateInfoEXT debugUtilsMessengerCreateInfo = {};
    debugUtilsMessengerCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    debugUtilsMessengerCreateInfo.pNext = nullptr;
    debugUtilsMessengerCreateInfo.flags = 0;
    debugUtilsMessengerCreateInfo.messageSeverity = severityFlag;
    debugUtilsMessengerCreateInfo.messageType = flags;
    debugUtilsMessengerCreateInfo.pfnUserCallback = (callback) ? callback : VulkanDebug::defaultDebugUtilsMessegerCallback;
    debugUtilsMessengerCreateInfo.pUserData = userData;

    VkResult result = VulkanWrapper::CreateDebugUtilsMessenger(instance, &debugUtilsMessengerCreateInfo, VULKAN_ALLOCATOR, &s_messeger);
    if (result != VK_SUCCESS)
    {
        LOG_ERROR("VulkanDebug::createDebugUtilsMessagerCallback: vkCreateDebugUtilsMessengerEXT error %s", ErrorString(result).c_str());
        return false;
    }
    return true;
}

void VulkanDebug::destroyDebugUtilsMesseger(VkInstance instance)
{
    if (s_messeger)
    {
        VulkanWrapper::DestroyDebugUtilsMessenger(instance, s_messeger, VULKAN_ALLOCATOR);
        s_messeger = VK_NULL_HANDLE;
    }
}

VkBool32 VulkanDebug::defaultDebugUtilsMessegerCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT * pCallbackData, void * pUserData)
{
    switch (messageType)
    {
    case VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT:
    {
        if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
        {
            LOG_ERROR("General[%s]Code %d: %s", pCallbackData->pMessageIdName, pCallbackData->messageIdNumber, pCallbackData->pMessage);
        };

        if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
        {
            LOG_WARNING("General[%s]Code %d: %s", pCallbackData->pMessageIdName, pCallbackData->messageIdNumber, pCallbackData->pMessage);
        };

        if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT)
        {
            LOG_INFO("General[%s]Code %d: %s", pCallbackData->pMessageIdName, pCallbackData->messageIdNumber, pCallbackData->pMessage);
        }

        if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT)
        {
            LOG_DEBUG("General[%s]Code %d: %s", pCallbackData->pMessageIdName, pCallbackData->messageIdNumber, pCallbackData->pMessage);
        }
        return VK_FALSE;
    }

    case VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT:
    {
        if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
        {
            LOG_ERROR("Validation[%s]Code %d: %s", pCallbackData->pMessageIdName, pCallbackData->messageIdNumber, pCallbackData->pMessage);
        };

        if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
        {
            LOG_WARNING("Validation[%s]Code %d: %s", pCallbackData->pMessageIdName, pCallbackData->messageIdNumber, pCallbackData->pMessage);
        };

        if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT)
        {
            LOG_INFO("Validation[%s]Code %d: %s", pCallbackData->pMessageIdName, pCallbackData->messageIdNumber, pCallbackData->pMessage);
        }

        if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT)
        {
            LOG_DEBUG("Validation[%s]Code %d: %s", pCallbackData->pMessageIdName, pCallbackData->messageIdNumber, pCallbackData->pMessage);
        }
        return VK_TRUE;
    }

    case VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT:
    {
        LOG_WARNING("Performance[Servity level %u][%s]Code %d: %s", messageSeverity, pCallbackData->pMessageIdName, pCallbackData->messageIdNumber, pCallbackData->pMessage);
        return VK_FALSE;
    }

    }
    return VK_FALSE;
}

bool VulkanDebug::checkInstanceLayerIsSupported(const c8* layerName)
{
    u32 instanceLayerPropertyCount = 0;
    VkResult result = VulkanWrapper::EnumerateInstanceLayerProperties(&instanceLayerPropertyCount, nullptr);
    if (result != VK_SUCCESS)
    {
        LOG_ERROR("VulkanDebug::checkInstanceLayerIsSupported: vkEnumerateInstanceLayerProperties count error %s", vk::ErrorString(result).c_str());
        return false;
    }

    std::vector<VkLayerProperties> instanceLayerProperties(instanceLayerPropertyCount);
    if (!instanceLayerProperties.empty())
    {
        result = VulkanWrapper::EnumerateInstanceLayerProperties(&instanceLayerPropertyCount, instanceLayerProperties.data());
        if (result != VK_SUCCESS)
        {
            LOG_ERROR("VulkanDebug::checkInstanceLayerIsSupported: vkEnumerateInstanceLayerProperties list error %s", vk::ErrorString(result).c_str());
            return false;
        }

        for (auto iter = instanceLayerProperties.cbegin(); iter < instanceLayerProperties.cend(); ++iter)
        {
            if (!strcmp((*iter).layerName, layerName))
            {
                return true;
            }
        }

        return false;
    }

    return false;
}

bool VulkanDebug::checkDeviceLayerIsSupported(VkPhysicalDevice device, const c8* layerName)
{
    u32 deviceLayerPropertyCount = 0;
    VkResult result = VulkanWrapper::EnumerateDeviceLayerProperties(device, &deviceLayerPropertyCount, nullptr);
    if (result != VK_SUCCESS)
    {
        LOG_ERROR("VulkanDebug::checkDeviceLayerIsSupported: vkEnumerateDeviceLayerProperties count error %s", vk::ErrorString(result).c_str());
        return false;
    }

    std::vector<VkLayerProperties> deviceLayerProperties(deviceLayerPropertyCount);
    if (!deviceLayerProperties.empty())
    {
        result = VulkanWrapper::EnumerateDeviceLayerProperties(device, &deviceLayerPropertyCount, deviceLayerProperties.data());
        if (result != VK_SUCCESS)
        {
            LOG_ERROR("VulkanDebug::checkDeviceLayerIsSupported: vkEnumerateDeviceLayerProperties list error %s", vk::ErrorString(result).c_str());
            return false;
        }

        for (auto iter = deviceLayerProperties.cbegin(); iter < deviceLayerProperties.cend(); ++iter)
        {
            if (!strcmp((*iter).layerName, layerName))
            {
                return true;
            }
        }

        return false;
    }

    return false;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////

#if VULKAN_DUMP
std::recursive_mutex VulkanDump::s_mutex;

VulkanDump::VulkanDump()
    : m_flags(DumpFlag::DumpFlag_None)
{
}

void VulkanDump::init(DumpFlags flags)
{
    m_flags = flags;
    clearFile(VULKAN_DUMP_FILE);
}

void VulkanDump::dumpFrameNumber(u64 frame)
{
    if (m_flags & DumpFlag::DumpFlag_General)
    {
        m_dump << "------------FrameNamber #" << std::dec << frame << "------------" << std::endl;

        if (k_forceFlush)
        {
            VulkanDump::flushToFile(VULKAN_DUMP_FILE);
        }
    }
}

void VulkanDump::dumpPreGetBufferMemoryRequirements(VkDevice device, VkBuffer buffer)
{
    if (m_flags & DumpFlag::DumpFlag_Buffer || m_flags & DumpFlag::DumpFlag_Memory)
    {
        std::lock_guard<std::recursive_mutex> lock(s_mutex);

        m_dump << "--GetBufferMemoryRequirements--" << std::endl;
        m_dump << "PreGetBufferMemoryRequirements(" << std::endl;
        m_dump << "VkDevice device: " << std::hex << device << std::endl;
        m_dump << "VkBuffer: " << std::hex << buffer << ")" << std::endl;

        if (k_forceFlush)
        {
            VulkanDump::flushToFile(VULKAN_DUMP_FILE);
        }
    }
}

void VulkanDump::dumpPostGetBufferMemoryRequirements(VkMemoryRequirements * pMemoryRequirements)
{
    if (m_flags & DumpFlag::DumpFlag_Buffer || m_flags & DumpFlag::DumpFlag_Memory)
    {
        std::lock_guard<std::recursive_mutex> lock(s_mutex);

        m_dump << "PostGetBufferMemoryRequirements(" << std::endl;
        m_dump << "VkMemoryRequirements: pMemoryRequirements[" << std::endl;
        m_dump << "     VkDeviceSize    size: " << pMemoryRequirements->size << std::endl;
        m_dump << "     VkDeviceSize    alignment: " << pMemoryRequirements->alignment << std::endl;
        m_dump << "     uint32_t        memoryTypeBits: " << pMemoryRequirements->memoryTypeBits << std::endl;
        m_dump << "])" << std::endl;
        m_dump << "----------------" << std::endl;

        if (k_forceFlush)
        {
            VulkanDump::flushToFile(VULKAN_DUMP_FILE);
        }
    }
}

void VulkanDump::dumpPreGetImageMemoryRequirements(VkDevice device, VkImage image)
{
    if (m_flags & DumpFlag::DumpFlag_Image || m_flags & DumpFlag::DumpFlag_Memory)
    {
        std::lock_guard<std::recursive_mutex> lock(s_mutex);

        m_dump << "--GetImageMemoryRequirements--" << std::endl;
        m_dump << "PreGetImageMemoryRequirements(" << std::endl;
        m_dump << "VkDevice device: " << std::hex << device << std::endl;
        m_dump << "VkBuffer: " << std::hex << image << ")" << std::endl;

        if (k_forceFlush)
        {
            VulkanDump::flushToFile(VULKAN_DUMP_FILE);
        }
    }
}

void VulkanDump::dumpPostGetImageMemoryRequirements(VkMemoryRequirements * pMemoryRequirements)
{
    if (m_flags & DumpFlag::DumpFlag_Image || m_flags & DumpFlag::DumpFlag_Memory)
    {
        std::lock_guard<std::recursive_mutex> lock(s_mutex);

        m_dump << "PostGetImageMemoryRequirements(" << std::endl;
        m_dump << "VkMemoryRequirements: pMemoryRequirements[" << std::endl;
        m_dump << "     VkDeviceSize    size: " << pMemoryRequirements->size << std::endl;
        m_dump << "     VkDeviceSize    alignment: " << pMemoryRequirements->alignment << std::endl;
        m_dump << "     uint32_t        memoryTypeBits: " << pMemoryRequirements->memoryTypeBits << std::endl;
        m_dump << "])" << std::endl;
        m_dump << "----------------" << std::endl;

        if (k_forceFlush)
        {
            VulkanDump::flushToFile(VULKAN_DUMP_FILE);
        }
    }
}

void VulkanDump::dumpPreCreateBuffer(VkDevice device, const VkBufferCreateInfo * pCreateInfo, const VkAllocationCallbacks * pAllocator)
{
    if (m_flags & DumpFlag::DumpFlag_Buffer)
    {
        std::lock_guard<std::recursive_mutex> lock(s_mutex);

        m_dump << "--CreateBuffer--" << std::endl;
        m_dump << "PreCreateBuffer(" << std::endl;
        m_dump << "VkDevice device: " << std::hex << device << std::endl;
        m_dump << "VkBufferCreateInfo: pCreateInfo[" << std::endl;
        m_dump << "     VkStructureType     sType: " << std::dec << pCreateInfo->sType << std::endl;
        m_dump << "     const void*         pNext: " << std::hex << pCreateInfo->pNext << std::endl;
        m_dump << "     VkBufferCreateFlags flags: " << std::dec << pCreateInfo->flags << std::endl;
        m_dump << "     VkDeviceSize        size: " << std::dec << pCreateInfo->size << std::endl;
        m_dump << "     VkBufferUsageFlags  usage: " << std::dec << pCreateInfo->usage << std::endl;
        m_dump << "     VkSharingMode       sharingMode: " << std::dec << pCreateInfo->sharingMode << std::endl;
        m_dump << "     uint32_t            queueFamilyIndexCount: " << std::dec << pCreateInfo->queueFamilyIndexCount << std::endl;
        m_dump << "     const uint32_t*     pQueueFamilyIndices: " << std::hex << pCreateInfo->pQueueFamilyIndices << std::endl;
        m_dump << "]," << std::endl;
        m_dump << "VkAllocationCallbacks pAllocator: " << std::hex << pAllocator << ")" << std::endl;

        if (k_forceFlush)
        {
            VulkanDump::flushToFile(VULKAN_DUMP_FILE);
        }
    }
}

void VulkanDump::dumpPostCreateBuffer(VkResult result, VkBuffer * pBuffer)
{
    if (m_flags & DumpFlag::DumpFlag_Buffer)
    {
        std::lock_guard<std::recursive_mutex> lock(s_mutex);

        m_dump << "PostCreateBuffer( VkResult: " << ErrorString(result) << ", VkBuffer: " << std::hex << *pBuffer << " )" << std::endl;
        m_dump << "----------------" << std::endl;

        if (k_forceFlush)
        {
            VulkanDump::flushToFile(VULKAN_DUMP_FILE);
        }
    }
}

void VulkanDump::dumpPreCreateImage(VkDevice device, const VkImageCreateInfo * pCreateInfo, const VkAllocationCallbacks * pAllocator)
{
    if (m_flags & DumpFlag::DumpFlag_Image)
    {
        std::lock_guard<std::recursive_mutex> lock(s_mutex);

        m_dump << "--CreateImage--" << std::endl;
        m_dump << "PreCreateImage(" << std::endl;
        m_dump << "VkDevice device: " << std::hex << device << std::endl;
        m_dump << "VkImageCreateInfo: pCreateInfo[" << std::endl;
        m_dump << "     VkStructureType         sType: " << std::dec << pCreateInfo->sType << std::endl;
        m_dump << "     const void*             pNext: " << std::hex << pCreateInfo->pNext << std::endl;
        m_dump << "     VkImageCreateFlagss     flags: " << std::dec << pCreateInfo->flags << std::endl;
        m_dump << "     VkImageType             imageType: " << std::dec << pCreateInfo->imageType << std::endl;
        m_dump << "     VkFormat                format: " << std::dec << pCreateInfo->format << std::endl;
        m_dump << "     VkExtent3D              extent:[ " << std::dec << pCreateInfo->extent.width << ", " << pCreateInfo->extent.height << ", " << pCreateInfo->extent.depth << "]" << std::endl;
        m_dump << "     uint32_t                mipLevels: " << std::dec << pCreateInfo->mipLevels << std::endl;
        m_dump << "     uint32_t                arrayLayers: " << std::dec << pCreateInfo->arrayLayers << std::endl;
        m_dump << "     VkSampleCountFlagBits   samples: " << std::dec << pCreateInfo->samples << std::endl;
        m_dump << "     VkImageTiling           tiling: " << std::dec << pCreateInfo->tiling << std::endl;
        m_dump << "     VkImageUsageFlags       usage: " << std::dec << pCreateInfo->usage << std::endl;
        m_dump << "     VkSharingMode           sharingMode: " << std::dec << pCreateInfo->sharingMode << std::endl;
        m_dump << "     uint32_t                queueFamilyIndexCount: " << std::dec << pCreateInfo->queueFamilyIndexCount << std::endl;
        m_dump << "     const uint32_t*         pQueueFamilyIndices: " << std::hex << pCreateInfo->pQueueFamilyIndices << std::endl;
        m_dump << "     VkImageLayout           initialLayout: " << pCreateInfo->initialLayout << std::endl;
        m_dump << "]," << std::endl;
        m_dump << "VkAllocationCallbacks pAllocator: " << std::hex << pAllocator << ")" << std::endl;

        if (k_forceFlush)
        {
            VulkanDump::flushToFile(VULKAN_DUMP_FILE);
        }
    }
}

void VulkanDump::dumpDestroyBuffer(VkDevice device, VkBuffer buffer, const VkAllocationCallbacks * pAllocator)
{
    if (m_flags & DumpFlag::DumpFlag_Buffer)
    {
        std::lock_guard<std::recursive_mutex> lock(s_mutex);

        m_dump << "--DestroyBuffer--" << std::endl;
        m_dump << "DestroyBuffer(" << std::endl;
        m_dump << "VkDevice device: " << std::hex << device << std::endl;
        m_dump << "VkBuffer buffer: " << std::hex << buffer << std::endl;
        m_dump << "VkAllocationCallbacks pAllocator: " << std::hex << pAllocator << " )" <<std::endl;
        m_dump << "----------------" << std::endl;

        if (k_forceFlush)
        {
            VulkanDump::flushToFile(VULKAN_DUMP_FILE);
        }
    }
}


void VulkanDump::dumpPostCreateImage(VkResult result, VkImage * pImage)
{
    if (m_flags & DumpFlag::DumpFlag_Image)
    {
        std::lock_guard<std::recursive_mutex> lock(s_mutex);

        m_dump << "PostCreateImage( VkResult: " << ErrorString(result) << ", VkImage: " << std::hex << *pImage << " )" << std::endl;
        m_dump << "----------------" << std::endl;

        if (k_forceFlush)
        {
            VulkanDump::flushToFile(VULKAN_DUMP_FILE);
        }
    }
}

void VulkanDump::dumpDestroyImage(VkDevice device, VkImage image, const VkAllocationCallbacks * pAllocator)
{
    if (m_flags & DumpFlag::DumpFlag_Image)
    {
        std::lock_guard<std::recursive_mutex> lock(s_mutex);

        m_dump << "--DestroyImage--" << std::endl;
        m_dump << "DestroyImage(" << std::endl;
        m_dump << "VkDevice device: " << std::hex << device << std::endl;
        m_dump << "VkImage image: " << std::hex << image << std::endl;
        m_dump << "VkAllocationCallbacks pAllocator: " << std::hex << pAllocator << " )" << std::endl;
        m_dump << "----------------" << std::endl;

        if (k_forceFlush)
        {
            VulkanDump::flushToFile(VULKAN_DUMP_FILE);
        }
    }
}

void VulkanDump::dumpPreAllocateMemory(VkDevice device, const VkMemoryAllocateInfo* pAllocateInfo, const VkAllocationCallbacks* pAllocator)
{
    if (m_flags & DumpFlag::DumpFlag_Memory)
    {
        std::lock_guard<std::recursive_mutex> lock(s_mutex);

        m_dump << "--AllocateMemory--" << std::endl;
        m_dump << "PreAllocateMemory(" << std::endl;
        m_dump << "VkDevice device: " << std::hex << device << std::endl;
        m_dump << "VkMemoryAllocateInfo: pAllocateInfo[" << std::endl;
        m_dump << "     VkStructureType         sType: " << std::dec << pAllocateInfo->sType << std::endl;
        m_dump << "     const void*             pNext: " << std::hex << pAllocateInfo->pNext << std::endl;
        m_dump << "     VkDeviceSize            allocationSize: " << std::dec << pAllocateInfo->allocationSize << std::endl;
        m_dump << "     uint32_t                memoryTypeIndex: " << std::dec << pAllocateInfo->memoryTypeIndex << std::endl;
        m_dump << "]," << std::endl;
        m_dump << "VkAllocationCallbacks pAllocator: " << std::hex << pAllocator << ")" << std::endl;

        if (k_forceFlush)
        {
            VulkanDump::flushToFile(VULKAN_DUMP_FILE);
        }
    }
}

void VulkanDump::dumpPostAllocateMemory(VkResult result, VkDeviceMemory* pMemory)
{
    if (m_flags & DumpFlag::DumpFlag_Memory)
    {
        std::lock_guard<std::recursive_mutex> lock(s_mutex);

        m_dump << "PostAllocateMemory( VkResult: " << ErrorString(result) << ", VkImage: " << std::hex << *pMemory << " )" << std::endl;
        m_dump << "----------------" << std::endl;

        if (k_forceFlush)
        {
            VulkanDump::flushToFile(VULKAN_DUMP_FILE);
        }
    }
}

void VulkanDump::dumpFreeMemory(VkDevice device, VkDeviceMemory memory, const VkAllocationCallbacks* pAllocator)
{
    if (m_flags & DumpFlag::DumpFlag_Memory)
    {
        std::lock_guard<std::recursive_mutex> lock(s_mutex);

        m_dump << "--FreeMemory--" << std::endl;
        m_dump << "FreeMemory(" << std::endl;
        m_dump << "VkDevice device: " << std::hex << device << std::endl;
        m_dump << "VkDeviceMemory memory: " << std::hex << memory << std::endl;
        m_dump << "VkAllocationCallbacks pAllocator: " << std::hex << pAllocator << ")" << std::endl;

        if (k_forceFlush)
        {
            VulkanDump::flushToFile(VULKAN_DUMP_FILE);
        }
    }
}

void VulkanDump::flushToConsole()
{
    std::lock_guard<std::recursive_mutex> lock(s_mutex);

    if (!m_dump.str().empty())
    {
        LOG("", m_dump.str().c_str());
        m_dump.clear();
    }
}

void VulkanDump::clearFile(const std::string & file)
{
    std::lock_guard<std::recursive_mutex> lock(s_mutex);

    std::ofstream sfile;
    sfile.open(file, std::ios::out);
    sfile << "";
    sfile.close();
}

void VulkanDump::flushToFile(const std::string & file)
{
    std::lock_guard<std::recursive_mutex> lock(s_mutex);

    if (!m_dump.str().empty())
    {
        std::ofstream sfile;
        sfile.open(file, std::ios::out | std::ios::ate);
        sfile << m_dump.str();
        sfile.close();

        m_dump.clear();
    }
}
#endif //VULKAN_DUMP

} //namespace vk
} //namespace renderer
} //namespace v3d
#endif //VULKAN_RENDER
