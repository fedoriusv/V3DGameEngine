#pragma once

#include "Common.h"

#ifdef VULKAN_RENDER
#include "VulkanWrapper.h"

#include "Utils/Singleton.h"

namespace v3d
{
namespace renderer
{
namespace vk
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

#if USE_VULKAN_ALLOCATOR
#   define VULKAN_ALLOCATOR nullptr
#else
#   define VULKAN_ALLOCATOR nullptr
#endif //USE_VULKAN_ALLOCATOR

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /*
        Disable : 0,
        Error : 1,
        Warning : 2,
        Info : 3: 
        Debug : 4
    */
    constexpr u16 k_DebugLevel = 4U;

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    std::string ErrorString(VkResult errorCode);

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    class VulkanLayers
    {
    public:

        static bool checkInstanceLayerIsSupported(const c8* layerName);
        static bool checkDeviceLayerIsSupported(VkPhysicalDevice device, const c8* layerName);

        static const std::vector<const c8*> s_validationLayerNames;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    class VulkanDebugUtils
    {
    public:

        VulkanDebugUtils() = default;
        VulkanDebugUtils(const VulkanDebugUtils&) = delete;

        static const u16 k_severityDebugLevel = k_DebugLevel;

        static bool createDebugUtilsMessenger(VkInstance instance, VkDebugUtilsMessageSeverityFlagsEXT severityFlag, VkDebugUtilsMessageTypeFlagsEXT flags, PFN_vkDebugUtilsMessengerCallbackEXT callback, void* userData = nullptr);
        static void destroyDebugUtilsMessenger(VkInstance instance);

        static VkBool32 VKAPI_PTR defaultDebugUtilsMessengerCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData);

    private:

        static void debugCallbackData(const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData);

        static VkDebugUtilsMessengerEXT s_messenger;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    class VulkanDebugReport
    {
    public:

        static const u16 k_debugReportLevel = k_DebugLevel;

        VulkanDebugReport() = default;
        VulkanDebugReport(const VulkanDebugReport&) = delete;

        static bool createDebugReportCallback(VkInstance instance, VkDebugReportFlagsEXT flags, PFN_vkDebugReportCallbackEXT callback, void* userData = nullptr);
        static void destroyDebugReportCallback(VkInstance instance);

        static VkBool32 VKAPI_PTR defaultDebugReportCallback(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objectType, uint64_t object, size_t location, int32_t messageCode, const char* pLayerPrefix, const char* pMessage, void* pUserData);

    private:

        static VkDebugReportCallbackEXT s_callback;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

#if VULKAN_DUMP
    class VulkanDump : public utils::Singleton<VulkanDump>
    {
    public:

        enum DumpFlag : u32
        {
            DumpFlag_None = 0,
            DumpFlag_General = 1 << 0,
            DumpFlag_Memory = 1 << 1,
            DumpFlag_Image = 1 << 2,
            DumpFlag_Buffer = 1 << 3,

            DumpFlag_All = 0x7FFFFFFF
        };

        typedef u32 DumpFlags;

        VulkanDump();
        VulkanDump(const VulkanDump&) = delete;

        void init(DumpFlags flags = DumpFlag::DumpFlag_All);

        void dumpFrameNumber(u64 frame);

        void dumpPreGetBufferMemoryRequirements(VkDevice device, VkBuffer buffer);
        void dumpPostGetBufferMemoryRequirements(VkMemoryRequirements * pMemoryRequirements);

        void dumpPreGetImageMemoryRequirements(VkDevice device, VkImage image);
        void dumpPostGetImageMemoryRequirements(VkMemoryRequirements * pMemoryRequirements);

        void dumpPreCreateBuffer(VkDevice device, const VkBufferCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator);
        void dumpPostCreateBuffer(VkResult result, VkBuffer* pBuffer);
        void dumpDestroyBuffer(VkDevice device, VkBuffer buffer, const VkAllocationCallbacks * pAllocator);

        void dumpPreCreateImage(VkDevice device, const VkImageCreateInfo * pCreateInfo, const VkAllocationCallbacks * pAllocator);
        void dumpPostCreateImage(VkResult result, VkImage * pImage);
        void dumpDestroyImage(VkDevice device, VkImage image, const VkAllocationCallbacks * pAllocator);

        void dumpPreCreateImageView(VkDevice device,  const VkImageViewCreateInfo * pCreateInfo, const VkAllocationCallbacks * pAllocator);
        void dumpPostCreateImageView(VkResult result, VkImageView *pView);
        void dumpDestroyImageView(VkDevice device, VkImageView view, const VkAllocationCallbacks * pAllocator);

        void dumpPreAllocateMemory(VkDevice device, const VkMemoryAllocateInfo* pAllocateInfo, const VkAllocationCallbacks* pAllocator);
        void dumpPostAllocateMemory(VkResult result, VkDeviceMemory* pMemory);
        void dumpFreeMemory(VkDevice device, VkDeviceMemory memory, const VkAllocationCallbacks* pAllocator);

        void flush();

    private:

        void flushToConsole();

        void clearFile(const std::string& file);
        void flushToFile(const std::string& file);

        std::stringstream m_dump;
        static std::recursive_mutex s_mutex;

        DumpFlags   m_flags;

        const bool k_forceFlush = true;
    };
#endif //VULKAN_DUMP

#if VULKAN_STATISTICS
    class VulkanStatistics : public utils::Singleton<VulkanStatistics>
    {
    public:

        VulkanStatistics() = default;
        VulkanStatistics(const VulkanStatistics&) = delete;

        template<class VkObject>
        void addObject(VkObject object)
        {
            //TODO:
        }

        template<class VkObject>
        void removeObject(VkObject object)
        {
            //TODO:
        }
    };
#endif //VULKAN_STATICSIC

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace vk
} //namespace renderer
} //namespace v3d
#endif //VULKAN_RENDER
