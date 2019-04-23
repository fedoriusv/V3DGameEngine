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

        VulkanDump() = default;
        VulkanDump(const VulkanDump&) = delete;

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

        void flushToConsole();

        void clearFile(const std::string& file);
        void flushToFile(const std::string& file);

    private:

        std::stringstream m_dump;
        static std::recursive_mutex s_mutex;

        const bool k_forceFlush = true;
    };
#endif //VULKAN_DUMP

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace vk
} //namespace renderer
} //namespace v3d
#endif //VULKAN_RENDER
