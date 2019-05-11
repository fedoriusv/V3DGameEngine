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

    std::string ErrorString(VkResult errorCode);

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    class VulkanDebug
    {
    public:

        VulkanDebug(const VulkanDebug&) = delete;

        static const u16    k_severityDebugLevel = 2;

        static bool         createDebugUtilsMesseger(VkInstance instance, VkDebugUtilsMessageSeverityFlagsEXT severityFlag, VkDebugUtilsMessageTypeFlagsEXT flags, PFN_vkDebugUtilsMessengerCallbackEXT callback, void* userData);
        static void         destroyDebugUtilsMesseger(VkInstance instance);

        static VkBool32 VKAPI_PTR defaultDebugUtilsMessegerCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData);


        static bool         checkInstanceLayerIsSupported(const c8* layerName);
        static bool         checkDeviceLayerIsSupported(VkPhysicalDevice device, const c8* layerName);

        static const std::vector<const c8*> s_validationLayerNames;

    private:

        static VkDebugUtilsMessengerEXT s_messeger;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

#if VULKAN_DUMP
    class VulkanDump : public utils::Singleton<VulkanDump>
    {
    public:

        enum DumpFlag : u32
        {
            DumpFlag_None = 0,
            DumpFlag_Memory = 1 << 0,
            DumpFlag_Image = 1 << 1,
            DumpFlag_Buffer = 1 << 2,

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

        void dumpPreAllocateMemory(VkDevice device, const VkMemoryAllocateInfo* pAllocateInfo, const VkAllocationCallbacks* pAllocator);
        void dumpPostAllocateMemory(VkResult result, VkDeviceMemory* pMemory);
        void dumpFreeMemory(VkDevice device, VkDeviceMemory memory, const VkAllocationCallbacks* pAllocator);

        void flushToConsole();

        void clearFile(const std::string& file);
        void flushToFile(const std::string& file);

    private:

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
