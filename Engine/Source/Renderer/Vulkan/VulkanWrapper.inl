#include "VulkanWrapper.h"
#include "FrameProfiler.h"
#ifdef VULKAN_RENDER

namespace v3d
{
namespace renderer
{
namespace vk
{

//VULKAN_VERSION_1_0
/////////////////////////////////////////////////////////////////////////////////////////////////////

inline VkResult VulkanWrapper::CreateInstance(const VkInstanceCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkInstance* pInstance) noexcept
{
    return vkCreateInstance(pCreateInfo, pAllocator, pInstance);
}

inline void VulkanWrapper::DestroyInstance(VkInstance instance, const VkAllocationCallbacks* pAllocator) noexcept
{
    vkDestroyInstance(instance, pAllocator);
}

inline VkResult VulkanWrapper::EnumeratePhysicalDevices(VkInstance instance, uint32_t* pPhysicalDeviceCount, VkPhysicalDevice* pPhysicalDevices) noexcept
{
    return vkEnumeratePhysicalDevices(instance, pPhysicalDeviceCount, pPhysicalDevices);
}

inline void VulkanWrapper::GetPhysicalDeviceFeatures(VkPhysicalDevice physicalDevice, VkPhysicalDeviceFeatures* pFeatures) noexcept
{
    vkGetPhysicalDeviceFeatures(physicalDevice, pFeatures);
}

inline void VulkanWrapper::GetPhysicalDeviceFormatProperties(VkPhysicalDevice physicalDevice, VkFormat format, VkFormatProperties* pFormatProperties) noexcept
{
    vkGetPhysicalDeviceFormatProperties(physicalDevice, format, pFormatProperties);
}

inline VkResult VulkanWrapper::GetPhysicalDeviceImageFormatProperties(VkPhysicalDevice physicalDevice, VkFormat format, VkImageType type, VkImageTiling tiling, VkImageUsageFlags usage, VkImageCreateFlags flags, VkImageFormatProperties* pImageFormatProperties) noexcept
{
    return vkGetPhysicalDeviceImageFormatProperties(physicalDevice, format, type, tiling, usage, flags, pImageFormatProperties);
}

inline void VulkanWrapper::GetPhysicalDeviceProperties(VkPhysicalDevice physicalDevice, VkPhysicalDeviceProperties* pProperties) noexcept
{
    vkGetPhysicalDeviceProperties(physicalDevice, pProperties);
}

inline void VulkanWrapper::GetPhysicalDeviceQueueFamilyProperties(VkPhysicalDevice physicalDevice, uint32_t* pQueueFamilyPropertyCount, VkQueueFamilyProperties* pQueueFamilyProperties) noexcept
{
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, pQueueFamilyPropertyCount, pQueueFamilyProperties);
}

inline void VulkanWrapper::GetPhysicalDeviceMemoryProperties(VkPhysicalDevice physicalDevice, VkPhysicalDeviceMemoryProperties* pMemoryProperties) noexcept
{
    vkGetPhysicalDeviceMemoryProperties(physicalDevice, pMemoryProperties);
}

inline PFN_vkVoidFunction VulkanWrapper::GetInstanceProcAddr(VkInstance instance, const char* pName) noexcept
{
    return vkGetInstanceProcAddr(instance, pName);
}

inline PFN_vkVoidFunction VulkanWrapper::GetDeviceProcAddr(VkDevice device, const char* pName) noexcept
{
    return vkGetDeviceProcAddr(device, pName);
}

inline VkResult VulkanWrapper::CreateDevice(VkPhysicalDevice physicalDevice, const VkDeviceCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDevice* pDevice) noexcept
{
    return vkCreateDevice(physicalDevice, pCreateInfo, pAllocator, pDevice);
}

inline void VulkanWrapper::DestroyDevice(VkDevice device, const VkAllocationCallbacks* pAllocator) noexcept
{
    vkDestroyDevice(device, pAllocator);
}

inline VkResult VulkanWrapper::EnumerateInstanceExtensionProperties(const char* pLayerName, uint32_t* pPropertyCount, VkExtensionProperties* pProperties) noexcept
{
    return vkEnumerateInstanceExtensionProperties(pLayerName, pPropertyCount, pProperties);
}

inline VkResult VulkanWrapper::EnumerateDeviceExtensionProperties(VkPhysicalDevice physicalDevice, const char* pLayerName, uint32_t* pPropertyCount, VkExtensionProperties* pProperties) noexcept
{
    return vkEnumerateDeviceExtensionProperties(physicalDevice, pLayerName, pPropertyCount, pProperties);
}

inline VkResult VulkanWrapper::EnumerateInstanceLayerProperties(uint32_t* pPropertyCount, VkLayerProperties* pProperties) noexcept
{
    return vkEnumerateInstanceLayerProperties(pPropertyCount, pProperties);
}

inline VkResult VulkanWrapper::EnumerateDeviceLayerProperties(VkPhysicalDevice physicalDevice, uint32_t* pPropertyCount, VkLayerProperties* pProperties) noexcept
{
    return vkEnumerateDeviceLayerProperties(physicalDevice, pPropertyCount, pProperties);
}

inline void VulkanWrapper::GetDeviceQueue(VkDevice device, uint32_t queueFamilyIndex, uint32_t queueIndex, VkQueue* pQueue) noexcept
{
    vkGetDeviceQueue(device, queueFamilyIndex, queueIndex, pQueue);
}

inline VkResult VulkanWrapper::QueueSubmit(VkQueue queue, uint32_t submitCount, const VkSubmitInfo* pSubmits, VkFence fence) noexcept
{
    {
        TRACE_PROFILER_VULKAN_SCOPE("vkQueueSubmit", color::rgba8::BLACK);
        return vkQueueSubmit(queue, submitCount, pSubmits, fence);
    }
}

inline VkResult VulkanWrapper::QueueWaitIdle(VkQueue queue) noexcept
{
    {
        TRACE_PROFILER_VULKAN_SCOPE("vkQueueWaitIdle", color::rgba8::BLACK);
        return vkQueueWaitIdle(queue);
    }
}

inline VkResult VulkanWrapper::DeviceWaitIdle(VkDevice device) noexcept
{
    {
        TRACE_PROFILER_VULKAN_SCOPE("vkDeviceWaitIdle", color::rgba8::BLACK);
        return vkDeviceWaitIdle(device);
    }
}

inline VkResult VulkanWrapper::AllocateMemory(VkDevice device, const VkMemoryAllocateInfo* pAllocateInfo, const VkAllocationCallbacks* pAllocator, VkDeviceMemory* pMemory) noexcept
{
#if VULKAN_DUMP
    VulkanDump::getInstance()->dumpPreAllocateMemory(device, pAllocateInfo, pAllocator);
    VkResult result = vkAllocateMemory(device, pAllocateInfo, pAllocator, pMemory);
    VulkanDump::getInstance()->dumpPostAllocateMemory(result, pMemory);
    return result;
#else
    return vkAllocateMemory(device, pAllocateInfo, pAllocator, pMemory);
#endif //VULKAN_DUMP
}

inline void VulkanWrapper::FreeMemory(VkDevice device, VkDeviceMemory memory, const VkAllocationCallbacks* pAllocator) noexcept
{
#if VULKAN_DUMP
    VulkanDump::getInstance()->dumpFreeMemory(device, memory, pAllocator);
#endif //VULKAN_DUMP
    vkFreeMemory(device, memory, pAllocator);
}

inline VkResult VulkanWrapper::MapMemory(VkDevice device, VkDeviceMemory memory, VkDeviceSize offset, VkDeviceSize size, VkMemoryMapFlags flags, void** ppData) noexcept
{
    return vkMapMemory(device, memory, offset, size, flags, ppData);
}

inline void VulkanWrapper::UnmapMemory(VkDevice device, VkDeviceMemory memory) noexcept
{
    vkUnmapMemory(device, memory);
}

inline VkResult VulkanWrapper::FlushMappedMemoryRanges(VkDevice device, uint32_t memoryRangeCount, const VkMappedMemoryRange* pMemoryRanges) noexcept
{
    return vkFlushMappedMemoryRanges(device, memoryRangeCount, pMemoryRanges);
}

inline VkResult VulkanWrapper::InvalidateMappedMemoryRanges(VkDevice device, uint32_t memoryRangeCount, const VkMappedMemoryRange* pMemoryRanges) noexcept
{
    return vkInvalidateMappedMemoryRanges(device, memoryRangeCount, pMemoryRanges);
}

inline void VulkanWrapper::GetDeviceMemoryCommitment(VkDevice device, VkDeviceMemory memory, VkDeviceSize* pCommittedMemoryInBytes) noexcept
{
    vkGetDeviceMemoryCommitment(device, memory, pCommittedMemoryInBytes);
}

inline VkResult VulkanWrapper::BindBufferMemory(VkDevice device, VkBuffer buffer, VkDeviceMemory memory, VkDeviceSize memoryOffset) noexcept
{
    return vkBindBufferMemory(device, buffer, memory, memoryOffset);
}

inline VkResult VulkanWrapper::BindImageMemory(VkDevice device, VkImage image, VkDeviceMemory memory, VkDeviceSize memoryOffset) noexcept
{
    return vkBindImageMemory(device, image, memory, memoryOffset);
}

inline void VulkanWrapper::GetBufferMemoryRequirements(VkDevice device, VkBuffer buffer, VkMemoryRequirements* pMemoryRequirements) noexcept
{
#if VULKAN_DUMP
    VulkanDump::getInstance()->dumpPreGetBufferMemoryRequirements(device, buffer);
    vkGetBufferMemoryRequirements(device, buffer, pMemoryRequirements);
    VulkanDump::getInstance()->dumpPostGetBufferMemoryRequirements(pMemoryRequirements);
#else
    vkGetBufferMemoryRequirements(device, buffer, pMemoryRequirements);
#endif //VULKAN_DUMP
}

inline void VulkanWrapper::GetImageMemoryRequirements(VkDevice device, VkImage image, VkMemoryRequirements* pMemoryRequirements) noexcept
{
#if VULKAN_DUMP
    VulkanDump::getInstance()->dumpPreGetImageMemoryRequirements(device, image);
    vkGetImageMemoryRequirements(device, image, pMemoryRequirements);
    VulkanDump::getInstance()->dumpPostGetImageMemoryRequirements(pMemoryRequirements);
#else
    vkGetImageMemoryRequirements(device, image, pMemoryRequirements);
#endif //VULKAN_DUMP
}

inline void VulkanWrapper::GetImageSparseMemoryRequirements(VkDevice device, VkImage image, uint32_t* pSparseMemoryRequirementCount, VkSparseImageMemoryRequirements* pSparseMemoryRequirements) noexcept
{
    vkGetImageSparseMemoryRequirements(device, image, pSparseMemoryRequirementCount, pSparseMemoryRequirements);
}

inline void VulkanWrapper::GetPhysicalDeviceSparseImageFormatProperties(VkPhysicalDevice physicalDevice, VkFormat format, VkImageType type, VkSampleCountFlagBits samples, VkImageUsageFlags usage, VkImageTiling tiling, uint32_t* pPropertyCount, VkSparseImageFormatProperties* pProperties) noexcept
{
    vkGetPhysicalDeviceSparseImageFormatProperties(physicalDevice, format, type, samples, usage, tiling, pPropertyCount, pProperties);
}

inline VkResult VulkanWrapper::QueueBindSparse(VkQueue queue, uint32_t bindInfoCount, const VkBindSparseInfo* pBindInfo, VkFence fence) noexcept
{
    return vkQueueBindSparse(queue, bindInfoCount, pBindInfo, fence);
}

inline VkResult VulkanWrapper::CreateFence(VkDevice device, const VkFenceCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkFence* pFence) noexcept
{
    return vkCreateFence(device, pCreateInfo, pAllocator, pFence);
}

inline void VulkanWrapper::DestroyFence(VkDevice device, VkFence fence, const VkAllocationCallbacks* pAllocator) noexcept
{
    vkDestroyFence(device, fence, pAllocator);
}

inline VkResult VulkanWrapper::ResetFences(VkDevice device, uint32_t fenceCount, const VkFence* pFences) noexcept
{
    return vkResetFences(device, fenceCount, pFences);
}

inline VkResult VulkanWrapper::GetFenceStatus(VkDevice device, VkFence fence) noexcept
{
    return vkGetFenceStatus(device, fence);
}

inline VkResult VulkanWrapper::WaitForFences(VkDevice device, uint32_t fenceCount, const VkFence* pFences, VkBool32 waitAll, uint64_t timeout) noexcept
{
    {
        TRACE_PROFILER_VULKAN_SCOPE("vkWaitForFences", color::rgba8::BLACK);
        return vkWaitForFences(device, fenceCount, pFences, waitAll, timeout);
    }
}

inline VkResult VulkanWrapper::CreateSemaphore(VkDevice device, const VkSemaphoreCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSemaphore* pSemaphore) noexcept
{
    return vkCreateSemaphore(device, pCreateInfo, pAllocator, pSemaphore);
}

inline void VulkanWrapper::DestroySemaphore(VkDevice device, VkSemaphore semaphore, const VkAllocationCallbacks* pAllocator) noexcept
{
    vkDestroySemaphore(device, semaphore, pAllocator);
}

inline VkResult VulkanWrapper::CreateEvent(VkDevice device, const VkEventCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkEvent* pEvent) noexcept
{
    return vkCreateEvent(device, pCreateInfo, pAllocator, pEvent);
}

inline void VulkanWrapper::DestroyEvent(VkDevice device, VkEvent event, const VkAllocationCallbacks* pAllocator) noexcept
{
    vkDestroyEvent(device, event, pAllocator);
}

inline VkResult VulkanWrapper::GetEventStatus(VkDevice device, VkEvent event) noexcept
{
    return vkGetEventStatus(device, event);
}

inline VkResult VulkanWrapper::SetEvent(VkDevice device, VkEvent event) noexcept
{
    return vkSetEvent(device, event);
}

inline VkResult VulkanWrapper::ResetEvent(VkDevice device, VkEvent event) noexcept
{
    return vkResetEvent(device, event);
}

inline VkResult VulkanWrapper::CreateQueryPool(VkDevice device, const VkQueryPoolCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkQueryPool* pQueryPool) noexcept
{
    return vkCreateQueryPool(device, pCreateInfo, pAllocator, pQueryPool);
}

inline void VulkanWrapper::DestroyQueryPool(VkDevice device, VkQueryPool queryPool, const VkAllocationCallbacks* pAllocator) noexcept
{
    vkDestroyQueryPool(device, queryPool, pAllocator);
}

inline VkResult VulkanWrapper::GetQueryPoolResults(VkDevice device, VkQueryPool queryPool, uint32_t firstQuery, uint32_t queryCount, size_t dataSize, void* pData, VkDeviceSize stride, VkQueryResultFlags flags) noexcept
{
    {
        TRACE_PROFILER_VULKAN_SCOPE("vkGetQueryPoolResults", color::rgba8::BLACK);
        return vkGetQueryPoolResults(device, queryPool, firstQuery, queryCount, dataSize, pData, stride, flags);
    }
}

inline VkResult VulkanWrapper::CreateBuffer(VkDevice device, const VkBufferCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkBuffer* pBuffer) noexcept
{
#if VULKAN_DUMP
    VulkanDump::getInstance()->dumpPreCreateBuffer(device, pCreateInfo, pAllocator);
    VkResult result = vkCreateBuffer(device, pCreateInfo, pAllocator, pBuffer);
    VulkanDump::getInstance()->dumpPostCreateBuffer(result, pBuffer);
    return result;
#else
    return vkCreateBuffer(device, pCreateInfo, pAllocator, pBuffer);
#endif //VULKAN_DUMP
}

inline void VulkanWrapper::DestroyBuffer(VkDevice device, VkBuffer buffer, const VkAllocationCallbacks* pAllocator) noexcept
{
#if VULKAN_DUMP
    VulkanDump::getInstance()->dumpDestroyBuffer(device, buffer, pAllocator);
#endif //VULKAN_DUMP
    vkDestroyBuffer(device, buffer, pAllocator);
}

inline VkResult VulkanWrapper::CreateBufferView(VkDevice device, const VkBufferViewCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkBufferView* pView) noexcept
{
    return vkCreateBufferView(device, pCreateInfo, pAllocator, pView);
}

inline void VulkanWrapper::DestroyBufferView(VkDevice device, VkBufferView bufferView, const VkAllocationCallbacks* pAllocator) noexcept
{
    vkDestroyBufferView(device, bufferView, pAllocator);
}

inline VkResult VulkanWrapper::CreateImage(VkDevice device, const VkImageCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkImage* pImage) noexcept
{
#if VULKAN_DUMP
    VulkanDump::getInstance()->dumpPreCreateImage(device, pCreateInfo, pAllocator);
    VkResult result = vkCreateImage(device, pCreateInfo, pAllocator, pImage);
    VulkanDump::getInstance()->dumpPostCreateImage(result, pImage);
    return result;
#else
    return vkCreateImage(device, pCreateInfo, pAllocator, pImage);
#endif //VULKAN_DUMP
}

inline void VulkanWrapper::DestroyImage(VkDevice device, VkImage image, const VkAllocationCallbacks* pAllocator) noexcept
{
#if VULKAN_DUMP
    VulkanDump::getInstance()->dumpDestroyImage(device, image, pAllocator);
#endif //VULKAN_DUMP
    vkDestroyImage(device, image, pAllocator);
}

inline void VulkanWrapper::GetImageSubresourceLayout(VkDevice device, VkImage image, const VkImageSubresource* pSubresource, VkSubresourceLayout* pLayout) noexcept
{
    vkGetImageSubresourceLayout(device, image, pSubresource, pLayout);
}

inline VkResult VulkanWrapper::CreateImageView(VkDevice device, const VkImageViewCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkImageView* pView) noexcept
{
#if VULKAN_DUMP
    VulkanDump::getInstance()->dumpPreCreateImageView(device, pCreateInfo, pAllocator);
    VkResult result = vkCreateImageView(device, pCreateInfo, pAllocator, pView);
    VulkanDump::getInstance()->dumpPostCreateImageView(result, pView);
    return result;
#else
    return vkCreateImageView(device, pCreateInfo, pAllocator, pView);
#endif //VULKAN_DUMP
}

inline void VulkanWrapper::DestroyImageView(VkDevice device, VkImageView imageView, const VkAllocationCallbacks* pAllocator) noexcept
{
#if VULKAN_DUMP
    VulkanDump::getInstance()->dumpDestroyImageView(device, imageView, pAllocator);
#endif //VULKAN_DUMP
    vkDestroyImageView(device, imageView, pAllocator);
}

inline VkResult VulkanWrapper::CreateShaderModule(VkDevice device, const VkShaderModuleCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkShaderModule* pShaderModule) noexcept
{
    return vkCreateShaderModule(device, pCreateInfo, pAllocator, pShaderModule);
}

inline void VulkanWrapper::DestroyShaderModule(VkDevice device, VkShaderModule shaderModule, const VkAllocationCallbacks* pAllocator) noexcept
{
    vkDestroyShaderModule(device, shaderModule, pAllocator);
}

inline VkResult VulkanWrapper::CreatePipelineCache(VkDevice device, const VkPipelineCacheCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkPipelineCache* pPipelineCache) noexcept
{
    return vkCreatePipelineCache(device, pCreateInfo, pAllocator, pPipelineCache);
}

inline void VulkanWrapper::DestroyPipelineCache(VkDevice device, VkPipelineCache pipelineCache, const VkAllocationCallbacks* pAllocator) noexcept
{
    vkDestroyPipelineCache(device, pipelineCache, pAllocator);
}

inline VkResult VulkanWrapper::GetPipelineCacheData(VkDevice device, VkPipelineCache pipelineCache, size_t* pDataSize, void* pData) noexcept
{
    return vkGetPipelineCacheData(device, pipelineCache, pDataSize, pData);
}

inline VkResult VulkanWrapper::MergePipelineCaches(VkDevice device, VkPipelineCache dstCache, uint32_t srcCacheCount, const VkPipelineCache* pSrcCaches) noexcept
{
    return vkMergePipelineCaches(device, dstCache, srcCacheCount, pSrcCaches);
}

inline VkResult VulkanWrapper::CreateGraphicsPipelines(VkDevice device, VkPipelineCache pipelineCache, uint32_t createInfoCount, const VkGraphicsPipelineCreateInfo* pCreateInfos, const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines) noexcept
{
    return vkCreateGraphicsPipelines(device, pipelineCache, createInfoCount, pCreateInfos, pAllocator, pPipelines);
}

inline VkResult VulkanWrapper::CreateComputePipelines(VkDevice device, VkPipelineCache pipelineCache, uint32_t createInfoCount, const VkComputePipelineCreateInfo* pCreateInfos, const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines) noexcept
{
    return vkCreateComputePipelines(device, pipelineCache, createInfoCount, pCreateInfos, pAllocator, pPipelines);
}

inline void VulkanWrapper::DestroyPipeline(VkDevice device, VkPipeline pipeline, const VkAllocationCallbacks* pAllocator) noexcept
{
    vkDestroyPipeline(device, pipeline, pAllocator);
}

inline VkResult VulkanWrapper::CreatePipelineLayout(VkDevice device, const VkPipelineLayoutCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkPipelineLayout* pPipelineLayout) noexcept
{
    return vkCreatePipelineLayout(device, pCreateInfo, pAllocator, pPipelineLayout);
}

inline void VulkanWrapper::DestroyPipelineLayout(VkDevice device, VkPipelineLayout pipelineLayout, const VkAllocationCallbacks* pAllocator) noexcept
{
    vkDestroyPipelineLayout(device, pipelineLayout, pAllocator);
}

inline VkResult VulkanWrapper::CreateSampler(VkDevice device, const VkSamplerCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSampler* pSampler) noexcept
{
    return vkCreateSampler(device, pCreateInfo, pAllocator, pSampler);
}

inline void VulkanWrapper::DestroySampler(VkDevice device, VkSampler sampler, const VkAllocationCallbacks* pAllocator) noexcept
{
    vkDestroySampler(device, sampler, pAllocator);
}

inline VkResult VulkanWrapper::CreateDescriptorSetLayout(VkDevice device, const VkDescriptorSetLayoutCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDescriptorSetLayout* pSetLayout) noexcept
{
    return vkCreateDescriptorSetLayout(device, pCreateInfo, pAllocator, pSetLayout);
}

inline void VulkanWrapper::DestroyDescriptorSetLayout(VkDevice device, VkDescriptorSetLayout descriptorSetLayout, const VkAllocationCallbacks* pAllocator) noexcept
{
    vkDestroyDescriptorSetLayout(device, descriptorSetLayout, pAllocator);
}

inline VkResult VulkanWrapper::CreateDescriptorPool(VkDevice device, const VkDescriptorPoolCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDescriptorPool* pDescriptorPool) noexcept
{
    {
        TRACE_PROFILER_VULKAN_SCOPE("vkCreateDescriptorPool", color::rgba8::BLACK);
        return vkCreateDescriptorPool(device, pCreateInfo, pAllocator, pDescriptorPool);
    }
}

inline void VulkanWrapper::DestroyDescriptorPool(VkDevice device, VkDescriptorPool descriptorPool, const VkAllocationCallbacks* pAllocator) noexcept
{
    {
        TRACE_PROFILER_VULKAN_SCOPE("vkDestroyDescriptorPool", color::rgba8::BLACK);
        vkDestroyDescriptorPool(device, descriptorPool, pAllocator);
    }
}

inline VkResult VulkanWrapper::ResetDescriptorPool(VkDevice device, VkDescriptorPool descriptorPool, VkDescriptorPoolResetFlags flags) noexcept
{
    {
        TRACE_PROFILER_VULKAN_SCOPE("vkResetDescriptorPool", color::rgba8::BLACK);
        return vkResetDescriptorPool(device, descriptorPool, flags);
    }
}

inline VkResult VulkanWrapper::AllocateDescriptorSets(VkDevice device, const VkDescriptorSetAllocateInfo* pAllocateInfo, VkDescriptorSet* pDescriptorSets) noexcept
{
    {
        TRACE_PROFILER_VULKAN_SCOPE("vkAllocateDescriptorSets", color::rgba8::BLACK);
        return vkAllocateDescriptorSets(device, pAllocateInfo, pDescriptorSets);
    }
}

inline VkResult VulkanWrapper::FreeDescriptorSets(VkDevice device, VkDescriptorPool descriptorPool, uint32_t descriptorSetCount, const VkDescriptorSet* pDescriptorSets) noexcept
{
    {
        TRACE_PROFILER_VULKAN_SCOPE("vkFreeDescriptorSets", color::rgba8::BLACK);
        return vkFreeDescriptorSets(device, descriptorPool, descriptorSetCount, pDescriptorSets);
    }
}

inline void VulkanWrapper::UpdateDescriptorSets(VkDevice device, uint32_t descriptorWriteCount, const VkWriteDescriptorSet* pDescriptorWrites, uint32_t descriptorCopyCount, const VkCopyDescriptorSet* pDescriptorCopies) noexcept
{
    {
        TRACE_PROFILER_VULKAN_SCOPE("vkUpdateDescriptorSets", color::rgba8::BLACK);
        vkUpdateDescriptorSets(device, descriptorWriteCount, pDescriptorWrites, descriptorCopyCount, pDescriptorCopies);
    }
}

inline VkResult VulkanWrapper::CreateFramebuffer(VkDevice device, const VkFramebufferCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkFramebuffer* pFramebuffer) noexcept
{
    return vkCreateFramebuffer(device, pCreateInfo, pAllocator, pFramebuffer);
}

inline void VulkanWrapper::DestroyFramebuffer(VkDevice device, VkFramebuffer framebuffer, const VkAllocationCallbacks* pAllocator) noexcept
{
    vkDestroyFramebuffer(device, framebuffer, pAllocator);
}

inline VkResult VulkanWrapper::CreateRenderPass(VkDevice device, const VkRenderPassCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkRenderPass* pRenderPass) noexcept
{
    return vkCreateRenderPass(device, pCreateInfo, pAllocator, pRenderPass);
}

inline void VulkanWrapper::DestroyRenderPass(VkDevice device, VkRenderPass renderPass, const VkAllocationCallbacks* pAllocator) noexcept
{
    vkDestroyRenderPass(device, renderPass, pAllocator);
}

inline void VulkanWrapper::GetRenderAreaGranularity(VkDevice device, VkRenderPass renderPass, VkExtent2D* pGranularity) noexcept
{
    vkGetRenderAreaGranularity(device, renderPass, pGranularity);
}

inline VkResult VulkanWrapper::CreateCommandPool(VkDevice device, const VkCommandPoolCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkCommandPool* pCommandPool) noexcept
{
    return vkCreateCommandPool(device, pCreateInfo, pAllocator, pCommandPool);
}

inline void VulkanWrapper::DestroyCommandPool(VkDevice device, VkCommandPool commandPool, const VkAllocationCallbacks* pAllocator) noexcept
{
    vkDestroyCommandPool(device, commandPool, pAllocator);
}

inline VkResult VulkanWrapper::ResetCommandPool(VkDevice device, VkCommandPool commandPool, VkCommandPoolResetFlags flags) noexcept
{
    {
        TRACE_PROFILER_VULKAN_SCOPE("vkResetCommandPool", color::rgba8::BLACK);
        return vkResetCommandPool(device, commandPool, flags);
    }
}

inline VkResult VulkanWrapper::AllocateCommandBuffers(VkDevice device, const VkCommandBufferAllocateInfo* pAllocateInfo, VkCommandBuffer* pCommandBuffers) noexcept
{
    {
        TRACE_PROFILER_VULKAN_SCOPE("vkAllocateCommandBuffers", color::rgba8::BLACK);
        return vkAllocateCommandBuffers(device, pAllocateInfo, pCommandBuffers);
    }
}

inline void VulkanWrapper::FreeCommandBuffers(VkDevice device, VkCommandPool commandPool, uint32_t commandBufferCount, const VkCommandBuffer* pCommandBuffers) noexcept
{
    {
        TRACE_PROFILER_VULKAN_SCOPE("vkFreeCommandBuffers", color::rgba8::BLACK);
        vkFreeCommandBuffers(device, commandPool, commandBufferCount, pCommandBuffers);
    }
}

inline VkResult VulkanWrapper::BeginCommandBuffer(VkCommandBuffer commandBuffer, const VkCommandBufferBeginInfo* pBeginInfo) noexcept
{
    {
        TRACE_PROFILER_VULKAN_SCOPE("vkBeginCommandBuffer", color::rgba8::BLACK);
        return vkBeginCommandBuffer(commandBuffer, pBeginInfo);
    }
}

inline VkResult VulkanWrapper::EndCommandBuffer(VkCommandBuffer commandBuffer) noexcept
{
    {
        TRACE_PROFILER_VULKAN_SCOPE("vkEndCommandBuffer", color::rgba8::BLACK);
        return vkEndCommandBuffer(commandBuffer);
    }
}

inline VkResult VulkanWrapper::ResetCommandBuffer(VkCommandBuffer commandBuffer, VkCommandBufferResetFlags flags) noexcept
{
    {
        TRACE_PROFILER_VULKAN_SCOPE("vkResetCommandBuffer", color::rgba8::BLACK);
        return vkResetCommandBuffer(commandBuffer, flags);
    }
}

inline void VulkanWrapper::CmdBindPipeline(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint, VkPipeline pipeline) noexcept
{
    {
        TRACE_PROFILER_VULKAN_SCOPE("vkCmdBindPipeline", color::rgba8::BLACK);
        vkCmdBindPipeline(commandBuffer, pipelineBindPoint, pipeline);
    }
}

inline void VulkanWrapper::CmdSetViewport(VkCommandBuffer commandBuffer, uint32_t firstViewport, uint32_t viewportCount, const VkViewport* pViewports) noexcept
{
    vkCmdSetViewport(commandBuffer, firstViewport, viewportCount, pViewports);
}

inline void VulkanWrapper::CmdSetScissor(VkCommandBuffer commandBuffer, uint32_t firstScissor, uint32_t scissorCount, const VkRect2D* pScissors) noexcept
{
    vkCmdSetScissor(commandBuffer, firstScissor, scissorCount, pScissors);
}

inline void VulkanWrapper::CmdSetLineWidth(VkCommandBuffer commandBuffer, float lineWidth) noexcept
{
    vkCmdSetLineWidth(commandBuffer, lineWidth);
}

inline void VulkanWrapper::CmdSetDepthBias(VkCommandBuffer commandBuffer, float depthBiasConstantFactor, float depthBiasClamp, float depthBiasSlopeFactor) noexcept
{
    vkCmdSetDepthBias(commandBuffer, depthBiasConstantFactor, depthBiasClamp, depthBiasSlopeFactor);
}

inline void VulkanWrapper::CmdSetBlendConstants(VkCommandBuffer commandBuffer, const float blendConstants[4]) noexcept
{
    vkCmdSetBlendConstants(commandBuffer, blendConstants);
}

inline void VulkanWrapper::CmdSetDepthBounds(VkCommandBuffer commandBuffer, float minDepthBounds, float maxDepthBounds) noexcept
{
    vkCmdSetDepthBounds(commandBuffer, minDepthBounds, maxDepthBounds);
}

inline void VulkanWrapper::CmdSetStencilCompareMask(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask, uint32_t compareMask) noexcept
{
    vkCmdSetStencilCompareMask(commandBuffer, faceMask, compareMask);
}

inline void VulkanWrapper::CmdSetStencilWriteMask(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask, uint32_t writeMask) noexcept
{
    vkCmdSetStencilWriteMask(commandBuffer, faceMask, writeMask);
}

inline void VulkanWrapper::CmdSetStencilReference(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask, uint32_t reference) noexcept
{
    vkCmdSetStencilReference(commandBuffer, faceMask, reference);
}

inline void VulkanWrapper::CmdBindDescriptorSets(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint, VkPipelineLayout layout, uint32_t firstSet, uint32_t descriptorSetCount, const VkDescriptorSet* pDescriptorSets, uint32_t dynamicOffsetCount, const uint32_t* pDynamicOffsets) noexcept
{
    {
        TRACE_PROFILER_VULKAN_SCOPE("vkCmdBindDescriptorSets", color::rgba8::BLACK);
        vkCmdBindDescriptorSets(commandBuffer, pipelineBindPoint, layout, firstSet, descriptorSetCount, pDescriptorSets, dynamicOffsetCount, pDynamicOffsets);
    }
}

inline void VulkanWrapper::CmdBindIndexBuffer(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, VkIndexType indexType) noexcept
{
    {
        TRACE_PROFILER_VULKAN_SCOPE("vkCmdBindIndexBuffer", color::rgba8::BLACK);
        vkCmdBindIndexBuffer(commandBuffer, buffer, offset, indexType);
    }
}

inline void VulkanWrapper::CmdBindVertexBuffers(VkCommandBuffer commandBuffer, uint32_t firstBinding, uint32_t bindingCount, const VkBuffer* pBuffers, const VkDeviceSize* pOffsets) noexcept
{
    {
        TRACE_PROFILER_VULKAN_SCOPE("vkCmdBindVertexBuffers", color::rgba8::BLACK);
        vkCmdBindVertexBuffers(commandBuffer, firstBinding, bindingCount, pBuffers, pOffsets);
    }
}

inline void VulkanWrapper::CmdDraw(VkCommandBuffer commandBuffer, uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance) noexcept
{
    {
        TRACE_PROFILER_VULKAN_SCOPE("vkCmdDraw", color::rgba8::BLACK);
        vkCmdDraw(commandBuffer, vertexCount, instanceCount, firstVertex, firstInstance);
    }
}

inline void VulkanWrapper::CmdDrawIndexed(VkCommandBuffer commandBuffer, uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, int32_t vertexOffset, uint32_t firstInstance) noexcept
{
    {
        TRACE_PROFILER_VULKAN_SCOPE("vkCmdDrawIndexed", color::rgba8::BLACK);
        vkCmdDrawIndexed(commandBuffer, indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
    }
}

inline void VulkanWrapper::CmdDrawIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, uint32_t drawCount, uint32_t stride) noexcept
{
    {
        TRACE_PROFILER_VULKAN_SCOPE("vkCmdDrawIndirect", color::rgba8::BLACK);
        vkCmdDrawIndirect(commandBuffer, buffer, offset, drawCount, stride);
    }
}

inline void VulkanWrapper::CmdDrawIndexedIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, uint32_t drawCount, uint32_t stride) noexcept
{
    {
        TRACE_PROFILER_VULKAN_SCOPE("vkCmdDrawIndexedIndirect", color::rgba8::BLACK);
        vkCmdDrawIndexedIndirect(commandBuffer, buffer, offset, drawCount, stride);
    }
}

inline void VulkanWrapper::CmdDispatch(VkCommandBuffer commandBuffer, uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ) noexcept
{
    {
        TRACE_PROFILER_VULKAN_SCOPE("vkCmdDispatch", color::rgba8::BLACK);
        vkCmdDispatch(commandBuffer, groupCountX, groupCountY, groupCountZ);
    }
}

inline void VulkanWrapper::CmdDispatchIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset) noexcept
{
    {
        TRACE_PROFILER_VULKAN_SCOPE("vkCmdDispatchIndirect", color::rgba8::BLACK);
        vkCmdDispatchIndirect(commandBuffer, buffer, offset);
    }
}

inline void VulkanWrapper::CmdCopyBuffer(VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkBuffer dstBuffer, uint32_t regionCount, const VkBufferCopy* pRegions) noexcept
{
    vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, regionCount, pRegions);
}

inline void VulkanWrapper::CmdCopyImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout, VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount, const VkImageCopy* pRegions) noexcept
{
    vkCmdCopyImage(commandBuffer, srcImage, srcImageLayout, dstImage, dstImageLayout, regionCount, pRegions);
}

inline void VulkanWrapper::CmdBlitImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout, VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount, const VkImageBlit* pRegions, VkFilter filter) noexcept
{
    vkCmdBlitImage(commandBuffer, srcImage, srcImageLayout, dstImage, dstImageLayout, regionCount, pRegions, filter);
}

inline void VulkanWrapper::CmdCopyBufferToImage(VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount, const VkBufferImageCopy* pRegions) noexcept
{
    vkCmdCopyBufferToImage(commandBuffer, srcBuffer, dstImage, dstImageLayout, regionCount, pRegions);
}

inline void VulkanWrapper::CmdCopyImageToBuffer(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout, VkBuffer dstBuffer, uint32_t regionCount, const VkBufferImageCopy* pRegions) noexcept
{
    vkCmdCopyImageToBuffer(commandBuffer, srcImage, srcImageLayout, dstBuffer, regionCount, pRegions);
}

inline void VulkanWrapper::CmdUpdateBuffer(VkCommandBuffer commandBuffer, VkBuffer dstBuffer, VkDeviceSize dstOffset, VkDeviceSize dataSize, const void* pData) noexcept
{
    vkCmdUpdateBuffer(commandBuffer, dstBuffer, dstOffset, dataSize, pData);
}

inline void VulkanWrapper::CmdFillBuffer(VkCommandBuffer commandBuffer, VkBuffer dstBuffer, VkDeviceSize dstOffset, VkDeviceSize size, uint32_t data) noexcept
{
    vkCmdFillBuffer(commandBuffer, dstBuffer, dstOffset, size, data);
}

inline void VulkanWrapper::CmdClearColorImage(VkCommandBuffer commandBuffer, VkImage image, VkImageLayout imageLayout, const VkClearColorValue* pColor, uint32_t rangeCount, const VkImageSubresourceRange* pRanges) noexcept
{
    vkCmdClearColorImage(commandBuffer, image, imageLayout, pColor, rangeCount, pRanges);
}

inline void VulkanWrapper::CmdClearDepthStencilImage(VkCommandBuffer commandBuffer, VkImage image, VkImageLayout imageLayout, const VkClearDepthStencilValue* pDepthStencil, uint32_t rangeCount, const VkImageSubresourceRange* pRanges) noexcept
{
    vkCmdClearDepthStencilImage(commandBuffer, image, imageLayout, pDepthStencil, rangeCount, pRanges);
}

inline void VulkanWrapper::CmdClearAttachments(VkCommandBuffer commandBuffer, uint32_t attachmentCount, const VkClearAttachment* pAttachments, uint32_t rectCount, const VkClearRect* pRects) noexcept
{
    vkCmdClearAttachments(commandBuffer, attachmentCount, pAttachments, rectCount, pRects);
}

inline void VulkanWrapper::CmdResolveImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout, VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount, const VkImageResolve* pRegions) noexcept
{
    vkCmdResolveImage(commandBuffer, srcImage, srcImageLayout, dstImage, dstImageLayout, regionCount, pRegions);
}

inline void VulkanWrapper::CmdSetEvent(VkCommandBuffer commandBuffer, VkEvent event, VkPipelineStageFlags stageMask) noexcept
{
    vkCmdSetEvent(commandBuffer, event, stageMask);
}

inline void VulkanWrapper::CmdResetEvent(VkCommandBuffer commandBuffer, VkEvent event, VkPipelineStageFlags stageMask) noexcept
{
    vkCmdResetEvent(commandBuffer, event, stageMask);
}

inline void VulkanWrapper::CmdWaitEvents(VkCommandBuffer commandBuffer, uint32_t eventCount, const VkEvent* pEvents, VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask, uint32_t memoryBarrierCount, const VkMemoryBarrier* pMemoryBarriers, uint32_t bufferMemoryBarrierCount, const VkBufferMemoryBarrier* pBufferMemoryBarriers, uint32_t imageMemoryBarrierCount, const VkImageMemoryBarrier* pImageMemoryBarriers) noexcept
{
    vkCmdWaitEvents(commandBuffer, eventCount, pEvents, srcStageMask, dstStageMask, memoryBarrierCount, pMemoryBarriers, bufferMemoryBarrierCount, pBufferMemoryBarriers, imageMemoryBarrierCount, pImageMemoryBarriers);
}

inline void VulkanWrapper::CmdPipelineBarrier(VkCommandBuffer commandBuffer, VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask, VkDependencyFlags dependencyFlags, uint32_t memoryBarrierCount, const VkMemoryBarrier* pMemoryBarriers, uint32_t bufferMemoryBarrierCount, const VkBufferMemoryBarrier* pBufferMemoryBarriers, uint32_t imageMemoryBarrierCount, const VkImageMemoryBarrier* pImageMemoryBarriers) noexcept
{
    vkCmdPipelineBarrier(commandBuffer, srcStageMask, dstStageMask, dependencyFlags,
        memoryBarrierCount, pMemoryBarriers, bufferMemoryBarrierCount, pBufferMemoryBarriers, imageMemoryBarrierCount, pImageMemoryBarriers);
}

inline void VulkanWrapper::CmdBeginQuery(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t query, VkQueryControlFlags flags) noexcept
{
    vkCmdBeginQuery(commandBuffer, queryPool, query, flags);
}

inline void VulkanWrapper::CmdEndQuery(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t query) noexcept
{
    vkCmdEndQuery(commandBuffer, queryPool, query);
}

inline void VulkanWrapper::CmdResetQueryPool(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t firstQuery, uint32_t queryCount) noexcept
{
    vkCmdResetQueryPool(commandBuffer, queryPool, firstQuery, queryCount);
}

inline void VulkanWrapper::CmdWriteTimestamp(VkCommandBuffer commandBuffer, VkPipelineStageFlagBits pipelineStage, VkQueryPool queryPool, uint32_t query) noexcept
{
    vkCmdWriteTimestamp(commandBuffer, pipelineStage, queryPool, query);
}

inline void VulkanWrapper::CmdCopyQueryPoolResults(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t firstQuery, uint32_t queryCount, VkBuffer dstBuffer, VkDeviceSize dstOffset, VkDeviceSize stride, VkQueryResultFlags flags) noexcept
{
    vkCmdCopyQueryPoolResults(commandBuffer, queryPool, firstQuery, queryCount, dstBuffer, dstOffset, stride, flags);
}

inline void VulkanWrapper::CmdPushConstants(VkCommandBuffer commandBuffer, VkPipelineLayout layout, VkShaderStageFlags stageFlags, uint32_t offset, uint32_t size, const void* pValues) noexcept
{
    vkCmdPushConstants(commandBuffer, layout, stageFlags, offset, size, pValues);
}

inline void VulkanWrapper::CmdBeginRenderPass(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo* pRenderPassBegin, VkSubpassContents contents) noexcept
{
    {
        TRACE_PROFILER_VULKAN_SCOPE("vkCmdBeginRenderPass", color::rgba8::BLACK);
        vkCmdBeginRenderPass(commandBuffer, pRenderPassBegin, contents);
    }
}

inline void VulkanWrapper::CmdNextSubpass(VkCommandBuffer commandBuffer, VkSubpassContents contents) noexcept
{
    vkCmdNextSubpass(commandBuffer, contents);
}

inline void VulkanWrapper::CmdEndRenderPass(VkCommandBuffer commandBuffer) noexcept
{
    {
        TRACE_PROFILER_VULKAN_SCOPE("vkCmdEndRenderPass", color::rgba8::BLACK);
        vkCmdEndRenderPass(commandBuffer);
    }
}

inline void VulkanWrapper::CmdExecuteCommands(VkCommandBuffer commandBuffer, uint32_t commandBufferCount, const VkCommandBuffer* pCommandBuffers) noexcept
{
    {
        TRACE_PROFILER_VULKAN_SCOPE("vkCmdExecuteCommands", color::rgba8::BLACK);
        vkCmdExecuteCommands(commandBuffer, commandBufferCount, pCommandBuffers);
    }
}
//VULKAN_VERSION_1_0

/////////////////////////////////////////////////////////////////////////////////////////////////////

//VULKAN_VERSION_1_1
#ifdef VULKAN_VERSION_1_1
inline VkResult VulkanWrapper::EnumerateInstanceVersion(uint32_t* pApiVersion) noexcept
{
    return vkEnumerateInstanceVersion(pApiVersion);
}

inline VkResult VulkanWrapper::BindBufferMemory2(VkDevice device, uint32_t bindInfoCount, const VkBindBufferMemoryInfo* pBindInfos) noexcept
{
#if (VULKAN_CURRENT_VERSION < VULKAN_VERSION_1_1)
#   define vkBindBufferMemory2 vkBindBufferMemory2KHR
#endif
    return vkBindBufferMemory2(device, bindInfoCount, pBindInfos);
#if (VULKAN_CURRENT_VERSION < VULKAN_VERSION_1_1)
#   undef vkBindBufferMemory2
#endif
}

inline VkResult VulkanWrapper::BindImageMemory2(VkDevice device, uint32_t bindInfoCount, const VkBindImageMemoryInfo* pBindInfos) noexcept
{
#if (VULKAN_CURRENT_VERSION < VULKAN_VERSION_1_1)
#   define vkBindImageMemory2 vkBindImageMemory2KHR
#endif
    return vkBindImageMemory2(device, bindInfoCount, pBindInfos);
#if (VULKAN_CURRENT_VERSION < VULKAN_VERSION_1_1)
#   undef vkBindImageMemory2
#endif
}

inline void VulkanWrapper::GetDeviceGroupPeerMemoryFeatures(VkDevice device, uint32_t heapIndex, uint32_t localDeviceIndex, uint32_t remoteDeviceIndex, VkPeerMemoryFeatureFlags* pPeerMemoryFeatures) noexcept
{
#if (VULKAN_CURRENT_VERSION < VULKAN_VERSION_1_1)
#   define vkGetDeviceGroupPeerMemoryFeatures vkGetDeviceGroupPeerMemoryFeaturesKHR
#endif
    vkGetDeviceGroupPeerMemoryFeatures(device, heapIndex, localDeviceIndex, remoteDeviceIndex, pPeerMemoryFeatures);
#if (VULKAN_CURRENT_VERSION < VULKAN_VERSION_1_1)
#   undef vkGetDeviceGroupPeerMemoryFeatures
#endif
}

inline void VulkanWrapper::CmdSetDeviceMask(VkCommandBuffer commandBuffer, uint32_t deviceMask) noexcept
{
#if (VULKAN_CURRENT_VERSION < VULKAN_VERSION_1_1)
#   define vkCmdSetDeviceMask vkCmdSetDeviceMaskKHR
#endif
    vkCmdSetDeviceMask(commandBuffer, deviceMask);
#if (VULKAN_CURRENT_VERSION < VULKAN_VERSION_1_1)
#   undef vkCmdSetDeviceMask
#endif
}

inline void VulkanWrapper::CmdDispatchBase(VkCommandBuffer commandBuffer, uint32_t baseGroupX, uint32_t baseGroupY, uint32_t baseGroupZ, uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ) noexcept
{
#if (VULKAN_CURRENT_VERSION < VULKAN_VERSION_1_1)
#   define vkCmdDispatchBase vkCmdDispatchBaseKHR
#endif
    vkCmdDispatchBase(commandBuffer, baseGroupX, baseGroupY, baseGroupZ, groupCountX, groupCountY, groupCountZ);
#if (VULKAN_CURRENT_VERSION < VULKAN_VERSION_1_1)
#   undef vkCmdDispatchBase
#endif
}

inline VkResult VulkanWrapper::EnumeratePhysicalDeviceGroups(VkInstance instance, uint32_t* pPhysicalDeviceGroupCount, VkPhysicalDeviceGroupProperties* pPhysicalDeviceGroupProperties) noexcept
{
#if (VULKAN_CURRENT_VERSION < VULKAN_VERSION_1_1)
#   define vkEnumeratePhysicalDeviceGroups vkEnumeratePhysicalDeviceGroupsKHR
#endif
    return vkEnumeratePhysicalDeviceGroups(instance, pPhysicalDeviceGroupCount, pPhysicalDeviceGroupProperties);
#if (VULKAN_CURRENT_VERSION < VULKAN_VERSION_1_1)
#   undef vkEnumeratePhysicalDeviceGroups
#endif
}

inline void VulkanWrapper::GetImageMemoryRequirements2(VkDevice device, const VkImageMemoryRequirementsInfo2* pInfo, VkMemoryRequirements2* pMemoryRequirements) noexcept
{
#if (VULKAN_CURRENT_VERSION < VULKAN_VERSION_1_1)
#   define vkGetImageMemoryRequirements2 vkGetImageMemoryRequirements2KHR
#endif
#if VULKAN_DUMP
    VulkanDump::getInstance()->dumpPreGetImageMemoryRequirements(device, pInfo->image);
    vkGetImageMemoryRequirements2(device, pInfo, pMemoryRequirements);
    VulkanDump::getInstance()->dumpPostGetImageMemoryRequirements(&pMemoryRequirements->memoryRequirements);
#else
    vkGetImageMemoryRequirements2(device, pInfo, pMemoryRequirements);
#endif //VULKAN_DUMP
#if (VULKAN_CURRENT_VERSION < VULKAN_VERSION_1_1)
#   undef vkGetImageMemoryRequirements2
#endif
}

inline void VulkanWrapper::GetBufferMemoryRequirements2(VkDevice device, const VkBufferMemoryRequirementsInfo2* pInfo, VkMemoryRequirements2* pMemoryRequirements) noexcept
{
#if (VULKAN_CURRENT_VERSION < VULKAN_VERSION_1_1)
#   define vkGetBufferMemoryRequirements2 vkGetBufferMemoryRequirements2KHR
#endif
#if VULKAN_DUMP
    VulkanDump::getInstance()->dumpPreGetBufferMemoryRequirements(device, pInfo->buffer);
    vkGetBufferMemoryRequirements2(device, pInfo, pMemoryRequirements);
    VulkanDump::getInstance()->dumpPostGetBufferMemoryRequirements(&pMemoryRequirements->memoryRequirements);
#else
    vkGetBufferMemoryRequirements2(device, pInfo, pMemoryRequirements);
#endif //VULKAN_DUMP
#if (VULKAN_CURRENT_VERSION < VULKAN_VERSION_1_1)
#   undef vkGetBufferMemoryRequirements2
#endif
}

inline void VulkanWrapper::GetImageSparseMemoryRequirements2(VkDevice device, const VkImageSparseMemoryRequirementsInfo2* pInfo, uint32_t* pSparseMemoryRequirementCount, VkSparseImageMemoryRequirements2* pSparseMemoryRequirements) noexcept
{
#if (VULKAN_CURRENT_VERSION < VULKAN_VERSION_1_1)
#   define vkGetImageSparseMemoryRequirements2 vkGetImageSparseMemoryRequirements2KHR
#endif
    vkGetImageSparseMemoryRequirements2(device, pInfo, pSparseMemoryRequirementCount, pSparseMemoryRequirements);
#if (VULKAN_CURRENT_VERSION < VULKAN_VERSION_1_1)
#   undef vkGetImageSparseMemoryRequirements2
#endif
}

inline void VulkanWrapper::GetPhysicalDeviceFeatures2(VkPhysicalDevice physicalDevice, VkPhysicalDeviceFeatures2* pFeatures) noexcept
{
#if (VULKAN_CURRENT_VERSION < VULKAN_VERSION_1_1)
#   define vkGetPhysicalDeviceFeatures2 vkGetPhysicalDeviceFeatures2KHR
#endif
    vkGetPhysicalDeviceFeatures2(physicalDevice, pFeatures);
#if (VULKAN_CURRENT_VERSION < VULKAN_VERSION_1_1)
#   undef vkGetPhysicalDeviceFeatures2
#endif
}

inline void VulkanWrapper::GetPhysicalDeviceProperties2(VkPhysicalDevice physicalDevice, VkPhysicalDeviceProperties2* pProperties) noexcept
{
#if (VULKAN_CURRENT_VERSION < VULKAN_VERSION_1_1)
#   define vkGetPhysicalDeviceProperties2 vkGetPhysicalDeviceProperties2KHR
#endif
    vkGetPhysicalDeviceProperties2(physicalDevice, pProperties);
#if (VULKAN_CURRENT_VERSION < VULKAN_VERSION_1_1)
#   undef vkGetPhysicalDeviceProperties2
#endif
}

inline void VulkanWrapper::GetPhysicalDeviceFormatProperties2(VkPhysicalDevice physicalDevice, VkFormat format, VkFormatProperties2* pFormatProperties) noexcept
{
#if (VULKAN_CURRENT_VERSION < VULKAN_VERSION_1_1)
#   define vkGetPhysicalDeviceFormatProperties2 vkGetPhysicalDeviceFormatProperties2KHR
#endif
    vkGetPhysicalDeviceFormatProperties2(physicalDevice, format, pFormatProperties);
#if (VULKAN_CURRENT_VERSION < VULKAN_VERSION_1_1)
#   undef vkGetPhysicalDeviceFormatProperties2
#endif
}

inline VkResult VulkanWrapper::GetPhysicalDeviceImageFormatProperties2(VkPhysicalDevice physicalDevice, const VkPhysicalDeviceImageFormatInfo2* pImageFormatInfo, VkImageFormatProperties2* pImageFormatProperties) noexcept
{
#if (VULKAN_CURRENT_VERSION < VULKAN_VERSION_1_1)
#   define vkGetPhysicalDeviceImageFormatProperties2 vkGetPhysicalDeviceImageFormatProperties2KHR
#endif
    return vkGetPhysicalDeviceImageFormatProperties2(physicalDevice, pImageFormatInfo, pImageFormatProperties);
#if (VULKAN_CURRENT_VERSION < VULKAN_VERSION_1_1)
#   undef vkGetPhysicalDeviceImageFormatProperties2
#endif
}

inline void VulkanWrapper::GetPhysicalDeviceQueueFamilyProperties2(VkPhysicalDevice physicalDevice, uint32_t* pQueueFamilyPropertyCount, VkQueueFamilyProperties2* pQueueFamilyProperties) noexcept
{
#if (VULKAN_CURRENT_VERSION < VULKAN_VERSION_1_1)
#   define vkGetPhysicalDeviceQueueFamilyProperties2 vkGetPhysicalDeviceQueueFamilyProperties2KHR
#endif
    vkGetPhysicalDeviceQueueFamilyProperties2(physicalDevice, pQueueFamilyPropertyCount, pQueueFamilyProperties);
#if (VULKAN_CURRENT_VERSION < VULKAN_VERSION_1_1)
#   undef vkGetPhysicalDeviceQueueFamilyProperties2
#endif
}

inline void VulkanWrapper::GetPhysicalDeviceMemoryProperties2(VkPhysicalDevice physicalDevice, VkPhysicalDeviceMemoryProperties2* pMemoryProperties) noexcept
{
#if (VULKAN_CURRENT_VERSION < VULKAN_VERSION_1_1)
#   define vkGetPhysicalDeviceMemoryProperties2 vkGetPhysicalDeviceMemoryProperties2KHR
#endif
    vkGetPhysicalDeviceMemoryProperties2(physicalDevice, pMemoryProperties);
#if (VULKAN_CURRENT_VERSION < VULKAN_VERSION_1_1)
#   undef vkGetPhysicalDeviceMemoryProperties2
#endif
}

inline void VulkanWrapper::GetPhysicalDeviceSparseImageFormatProperties2(VkPhysicalDevice physicalDevice, const VkPhysicalDeviceSparseImageFormatInfo2* pFormatInfo, uint32_t* pPropertyCount, VkSparseImageFormatProperties2* pProperties) noexcept
{
#if (VULKAN_CURRENT_VERSION < VULKAN_VERSION_1_1)
#   define vkGetPhysicalDeviceSparseImageFormatProperties2 vkGetPhysicalDeviceSparseImageFormatProperties2KHR
#endif
    vkGetPhysicalDeviceSparseImageFormatProperties2(physicalDevice, pFormatInfo, pPropertyCount, pProperties);
#if (VULKAN_CURRENT_VERSION < VULKAN_VERSION_1_1)
#   undef vkGetPhysicalDeviceSparseImageFormatProperties2
#endif
}

inline void VulkanWrapper::TrimCommandPool(VkDevice device, VkCommandPool commandPool, VkCommandPoolTrimFlags flags) noexcept
{
#if (VULKAN_CURRENT_VERSION < VULKAN_VERSION_1_1)
#   define vkTrimCommandPool vkTrimCommandPoolKHR
#endif
    vkTrimCommandPool(device, commandPool, flags);
#if (VULKAN_CURRENT_VERSION < VULKAN_VERSION_1_1)
#   undef vkTrimCommandPool
#endif
}

inline void VulkanWrapper::GetDeviceQueue2(VkDevice device, const VkDeviceQueueInfo2* pQueueInfo, VkQueue* pQueue) noexcept
{
#if (VULKAN_CURRENT_VERSION < VULKAN_VERSION_1_1)
    NOT_IMPL;
#else
    vkGetDeviceQueue2(device, pQueueInfo, pQueue);
#endif
}

inline VkResult VulkanWrapper::CreateSamplerYcbcrConversion(VkDevice device, const VkSamplerYcbcrConversionCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSamplerYcbcrConversion* pYcbcrConversion) noexcept
{
#if (VULKAN_CURRENT_VERSION < VULKAN_VERSION_1_1)
#   define vkCreateSamplerYcbcrConversion vkCreateSamplerYcbcrConversionKHR
#endif
    return vkCreateSamplerYcbcrConversion(device, pCreateInfo, pAllocator, pYcbcrConversion);
#if (VULKAN_CURRENT_VERSION < VULKAN_VERSION_1_1)
#   undef vkCreateSamplerYcbcrConversion
#endif
}

inline void VulkanWrapper::DestroySamplerYcbcrConversion(VkDevice device, VkSamplerYcbcrConversion ycbcrConversion, const VkAllocationCallbacks* pAllocator) noexcept
{
#if (VULKAN_CURRENT_VERSION < VULKAN_VERSION_1_1)
#   define vkDestroySamplerYcbcrConversion vkDestroySamplerYcbcrConversionKHR
#endif
    vkDestroySamplerYcbcrConversion(device, ycbcrConversion, pAllocator);
#if (VULKAN_CURRENT_VERSION < VULKAN_VERSION_1_1)
#   undef vkDestroySamplerYcbcrConversion
#endif
}

inline VkResult VulkanWrapper::CreateDescriptorUpdateTemplate(VkDevice device, const VkDescriptorUpdateTemplateCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDescriptorUpdateTemplate* pDescriptorUpdateTemplate) noexcept
{
#if (VULKAN_CURRENT_VERSION < VULKAN_VERSION_1_1)
#   define vkCreateDescriptorUpdateTemplate vkCreateDescriptorUpdateTemplateKHR
#endif
    return vkCreateDescriptorUpdateTemplate(device, pCreateInfo, pAllocator, pDescriptorUpdateTemplate);
#if (VULKAN_CURRENT_VERSION < VULKAN_VERSION_1_1)
#   undef vkCreateDescriptorUpdateTemplate
#endif
}

inline void VulkanWrapper::DestroyDescriptorUpdateTemplate(VkDevice device, VkDescriptorUpdateTemplate descriptorUpdateTemplate, const VkAllocationCallbacks* pAllocator) noexcept
{
#if (VULKAN_CURRENT_VERSION < VULKAN_VERSION_1_1)
#   define vkDestroyDescriptorUpdateTemplate vkDestroyDescriptorUpdateTemplateKHR
#endif
    vkDestroyDescriptorUpdateTemplate(device, descriptorUpdateTemplate, pAllocator);
#if (VULKAN_CURRENT_VERSION < VULKAN_VERSION_1_1)
#   undef vkDestroyDescriptorUpdateTemplate
#endif
}

inline void VulkanWrapper::UpdateDescriptorSetWithTemplate(VkDevice device, VkDescriptorSet descriptorSet, VkDescriptorUpdateTemplate descriptorUpdateTemplate, const void* pData) noexcept
{
#if (VULKAN_CURRENT_VERSION < VULKAN_VERSION_1_1)
#   define vkUpdateDescriptorSetWithTemplate vkUpdateDescriptorSetWithTemplateKHR
#endif
    vkUpdateDescriptorSetWithTemplate(device, descriptorSet, descriptorUpdateTemplate, pData);
#if (VULKAN_CURRENT_VERSION < VULKAN_VERSION_1_1)
#   undef vkUpdateDescriptorSetWithTemplate
#endif
}

inline void VulkanWrapper::GetPhysicalDeviceExternalBufferProperties(VkPhysicalDevice physicalDevice, const VkPhysicalDeviceExternalBufferInfo* pExternalBufferInfo, VkExternalBufferProperties* pExternalBufferProperties) noexcept
{
#if (VULKAN_CURRENT_VERSION < VULKAN_VERSION_1_1)
#   define vkGetPhysicalDeviceExternalBufferProperties vkGetPhysicalDeviceExternalBufferPropertiesKHR
#endif
    vkGetPhysicalDeviceExternalBufferProperties(physicalDevice, pExternalBufferInfo, pExternalBufferProperties);
#if (VULKAN_CURRENT_VERSION < VULKAN_VERSION_1_1)
#   undef vkGetPhysicalDeviceExternalBufferProperties
#endif
}

inline void VulkanWrapper::GetPhysicalDeviceExternalFenceProperties(VkPhysicalDevice physicalDevice, const VkPhysicalDeviceExternalFenceInfo* pExternalFenceInfo, VkExternalFenceProperties* pExternalFenceProperties) noexcept
{
#if (VULKAN_CURRENT_VERSION < VULKAN_VERSION_1_1)
#   define vkGetPhysicalDeviceExternalFenceProperties vkGetPhysicalDeviceExternalFencePropertiesKHR
#endif
    vkGetPhysicalDeviceExternalFenceProperties(physicalDevice, pExternalFenceInfo, pExternalFenceProperties);
#if (VULKAN_CURRENT_VERSION < VULKAN_VERSION_1_1)
#   undef vkGetPhysicalDeviceExternalFenceProperties
#endif
}

inline void VulkanWrapper::GetPhysicalDeviceExternalSemaphoreProperties(VkPhysicalDevice physicalDevice, const VkPhysicalDeviceExternalSemaphoreInfo* pExternalSemaphoreInfo, VkExternalSemaphoreProperties* pExternalSemaphoreProperties) noexcept
{
#if (VULKAN_CURRENT_VERSION < VULKAN_VERSION_1_1)
#   define vkGetPhysicalDeviceExternalSemaphoreProperties vkGetPhysicalDeviceExternalSemaphorePropertiesKHR
#endif
    vkGetPhysicalDeviceExternalSemaphoreProperties(physicalDevice, pExternalSemaphoreInfo, pExternalSemaphoreProperties);
#if (VULKAN_CURRENT_VERSION < VULKAN_VERSION_1_1)
#   undef vkGetPhysicalDeviceExternalSemaphoreProperties
#endif
}

inline void VulkanWrapper::GetDescriptorSetLayoutSupport(VkDevice device, const VkDescriptorSetLayoutCreateInfo* pCreateInfo, VkDescriptorSetLayoutSupport* pSupport) noexcept
{
#if (VULKAN_CURRENT_VERSION < VULKAN_VERSION_1_1)
#   define vkGetDescriptorSetLayoutSupport vkGetDescriptorSetLayoutSupportKHR
#endif
    vkGetDescriptorSetLayoutSupport(device, pCreateInfo, pSupport);
#if (VULKAN_CURRENT_VERSION < VULKAN_VERSION_1_1)
#   undef vkGetDescriptorSetLayoutSupport
#endif
}
#endif //VULKAN_VERSION_1_1

/////////////////////////////////////////////////////////////////////////////////////////////////////

//VULKAN_VERSION_1_2
#ifdef VULKAN_VERSION_1_2
inline void VulkanWrapper::CmdDrawIndirectCount(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount, uint32_t stride) noexcept
{
#if (VULKAN_CURRENT_VERSION < VULKAN_VERSION_1_2)
#   define vkCmdDrawIndirectCount vkCmdDrawIndirectCountKHR
#endif
    vkCmdDrawIndirectCount(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride);
#if (VULKAN_CURRENT_VERSION < VULKAN_VERSION_1_2)
#   undef vkCmdDrawIndirectCount
#endif
}

inline void VulkanWrapper::CmdDrawIndexedIndirectCount(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount, uint32_t stride) noexcept
{
#if (VULKAN_CURRENT_VERSION < VULKAN_VERSION_1_2)
#   define vkCmdDrawIndexedIndirectCount vkCmdDrawIndexedIndirectCountKHR
#endif
    vkCmdDrawIndexedIndirectCount(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride);
#if (VULKAN_CURRENT_VERSION < VULKAN_VERSION_1_2)
#   undef vkCmdDrawIndexedIndirectCount
#endif
}

inline VkResult VulkanWrapper::CreateRenderPass2(VkDevice device, const VkRenderPassCreateInfo2* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkRenderPass* pRenderPass) noexcept
{
#if (VULKAN_CURRENT_VERSION < VULKAN_VERSION_1_2)
#   define vkCreateRenderPass2 vkCreateRenderPass2KHR
#endif
    return vkCreateRenderPass2(device, pCreateInfo, pAllocator, pRenderPass);
#if (VULKAN_CURRENT_VERSION < VULKAN_VERSION_1_2)
#   undef vkCreateRenderPass2
#endif
}

inline void VulkanWrapper::CmdBeginRenderPass2(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo* pRenderPassBegin, const VkSubpassBeginInfo* pSubpassBeginInfo) noexcept
{
#if (VULKAN_CURRENT_VERSION < VULKAN_VERSION_1_2)
#   define vkCmdBeginRenderPass2 vkCmdBeginRenderPass2KHR
#endif
    {
        TRACE_PROFILER_VULKAN_SCOPE("vkCmdBeginRenderPass2", color::rgba8::BLACK);
        vkCmdBeginRenderPass2(commandBuffer, pRenderPassBegin, pSubpassBeginInfo);
    }
#if (VULKAN_CURRENT_VERSION < VULKAN_VERSION_1_2)
#   undef vkCmdBeginRenderPass2
#endif
}

inline void VulkanWrapper::CmdNextSubpass2(VkCommandBuffer commandBuffer, const VkSubpassBeginInfo* pSubpassBeginInfo, const VkSubpassEndInfo* pSubpassEndInfo) noexcept
{
#if (VULKAN_CURRENT_VERSION < VULKAN_VERSION_1_2)
#   define vkCmdNextSubpass2 vkCmdNextSubpass2KHR
#endif
    {
        TRACE_PROFILER_VULKAN_SCOPE("vkCmdNextSubpass2", color::rgba8::BLACK);
        vkCmdNextSubpass2(commandBuffer, pSubpassBeginInfo, pSubpassEndInfo);
    }
#if (VULKAN_CURRENT_VERSION < VULKAN_VERSION_1_2)
#   undef vkCmdNextSubpass2
#endif
}

inline void VulkanWrapper::CmdEndRenderPass2(VkCommandBuffer commandBuffer, const VkSubpassEndInfo* pSubpassEndInfo) noexcept
{
#if (VULKAN_CURRENT_VERSION < VULKAN_VERSION_1_2)
#   define vkCmdEndRenderPass2 vkCmdEndRenderPass2KHR
#endif
    {
        TRACE_PROFILER_VULKAN_SCOPE("vkCmdEndRenderPass2", color::rgba8::BLACK);
        vkCmdEndRenderPass2(commandBuffer, pSubpassEndInfo);
    }
#if (VULKAN_CURRENT_VERSION < VULKAN_VERSION_1_2)
#   undef vkCmdEndRenderPass2
#endif
}

inline void VulkanWrapper::ResetQueryPool(VkDevice device, VkQueryPool queryPool, uint32_t firstQuery, uint32_t queryCount) noexcept
{
#if (VULKAN_CURRENT_VERSION < VULKAN_VERSION_1_2)
#   define vkResetQueryPool vkResetQueryPoolEXT
#endif
    {
        TRACE_PROFILER_VULKAN_SCOPE("vkResetQueryPool", color::rgba8::BLACK);
        vkResetQueryPool(device, queryPool, firstQuery, queryCount);
    }
#if (VULKAN_CURRENT_VERSION < VULKAN_VERSION_1_2)
#   undef vkResetQueryPool
#endif
}

inline VkResult VulkanWrapper::GetSemaphoreCounterValue(VkDevice device, VkSemaphore semaphore, uint64_t* pValue) noexcept
{
#if (VULKAN_CURRENT_VERSION < VULKAN_VERSION_1_2)
#   define vkGetSemaphoreCounterValue vkGetSemaphoreCounterValueKHR
#endif
    return vkGetSemaphoreCounterValue(device, semaphore, pValue);
#if (VULKAN_CURRENT_VERSION < VULKAN_VERSION_1_2)
#   undef vkGetSemaphoreCounterValue
#endif
}

inline VkResult VulkanWrapper::WaitSemaphores(VkDevice device, const VkSemaphoreWaitInfo* pWaitInfo, uint64_t timeout) noexcept
{
#if (VULKAN_CURRENT_VERSION < VULKAN_VERSION_1_2)
#   define vkWaitSemaphores vkWaitSemaphoresKHR
#endif
    {
        TRACE_PROFILER_VULKAN_SCOPE("vkWaitSemaphores", color::rgba8::BLACK);
        return vkWaitSemaphores(device, pWaitInfo, timeout);
    }
#if (VULKAN_CURRENT_VERSION < VULKAN_VERSION_1_2)
#   undef vkWaitSemaphores
#endif
}

inline VkResult VulkanWrapper::SignalSemaphore(VkDevice device, const VkSemaphoreSignalInfo* pSignalInfo) noexcept
{
#if (VULKAN_CURRENT_VERSION < VULKAN_VERSION_1_2)
#   define vkSignalSemaphore vkSignalSemaphoreKHR
#endif
    return vkSignalSemaphore(device, pSignalInfo);
#if (VULKAN_CURRENT_VERSION < VULKAN_VERSION_1_2)
#   undef vkSignalSemaphore
#endif
}

inline VkDeviceAddress VulkanWrapper::GetBufferDeviceAddress(VkDevice device, const VkBufferDeviceAddressInfo* pInfo) noexcept
{
#if (VULKAN_CURRENT_VERSION < VULKAN_VERSION_1_2)
#   define vkGetBufferDeviceAddress vkGetBufferDeviceAddressKHR
#endif
    return vkGetBufferDeviceAddress(device, pInfo);
#if (VULKAN_CURRENT_VERSION < VULKAN_VERSION_1_2)
#   undef vkGetBufferDeviceAddress
#endif
}

inline uint64_t VulkanWrapper::GetBufferOpaqueCaptureAddress(VkDevice device, const VkBufferDeviceAddressInfo* pInfo) noexcept
{
#if (VULKAN_CURRENT_VERSION < VULKAN_VERSION_1_2)
#   define vkGetBufferOpaqueCaptureAddress vkGetBufferOpaqueCaptureAddressKHR
#endif
    return vkGetBufferOpaqueCaptureAddress(device, pInfo);
#if (VULKAN_CURRENT_VERSION < VULKAN_VERSION_1_2)
#   undef vkGetBufferOpaqueCaptureAddress
#endif
}

inline uint64_t VulkanWrapper::GetDeviceMemoryOpaqueCaptureAddress(VkDevice device, const VkDeviceMemoryOpaqueCaptureAddressInfo* pInfo) noexcept
{
#if (VULKAN_CURRENT_VERSION < VULKAN_VERSION_1_2)
#   define vkGetDeviceMemoryOpaqueCaptureAddress vkGetDeviceMemoryOpaqueCaptureAddressKHR
#endif
    return vkGetDeviceMemoryOpaqueCaptureAddress(device, pInfo);
#if (VULKAN_CURRENT_VERSION < VULKAN_VERSION_1_2)
#   undef vkGetDeviceMemoryOpaqueCaptureAddress
#endif
}
#endif //VULKAN_VERSION_1_2

/////////////////////////////////////////////////////////////////////////////////////////////////////

//VULKAN_VERSION_1_3
#ifdef VULKAN_VERSION_1_3
inline VkResult VulkanWrapper::GetPhysicalDeviceToolProperties(VkPhysicalDevice physicalDevice, uint32_t* pToolCount, VkPhysicalDeviceToolProperties* pToolProperties) noexcept
{
#if (VULKAN_CURRENT_VERSION < VULKAN_VERSION_1_3)
#   define vkGetPhysicalDeviceToolProperties vkGetPhysicalDeviceToolPropertiesEXT
#endif
    return vkGetPhysicalDeviceToolProperties(physicalDevice, pToolCount, pToolProperties);
#if (VULKAN_CURRENT_VERSION < VULKAN_VERSION_1_3)
#   undef vkGetPhysicalDeviceToolProperties
#endif
}

inline VkResult VulkanWrapper::CreatePrivateDataSlot(VkDevice device, const VkPrivateDataSlotCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkPrivateDataSlot* pPrivateDataSlot) noexcept
{
#if (VULKAN_CURRENT_VERSION < VULKAN_VERSION_1_3)
#   define vkCreatePrivateDataSlot vkCreatePrivateDataSlotEXT
#endif
    return vkCreatePrivateDataSlot(device, pCreateInfo, pAllocator, pPrivateDataSlot);
#if (VULKAN_CURRENT_VERSION < VULKAN_VERSION_1_3)
#   undef vkCreatePrivateDataSlot
#endif
}

inline void VulkanWrapper::DestroyPrivateDataSlot(VkDevice device, VkPrivateDataSlot privateDataSlot, const VkAllocationCallbacks* pAllocator) noexcept
{
#if (VULKAN_CURRENT_VERSION < VULKAN_VERSION_1_3)
#   define vkDestroyPrivateDataSlot vkDestroyPrivateDataSlotEXT
#endif
    return vkDestroyPrivateDataSlot(device, privateDataSlot, pAllocator);
#if (VULKAN_CURRENT_VERSION < VULKAN_VERSION_1_3)
#   undef vkDestroyPrivateDataSlot
#endif
}

inline VkResult VulkanWrapper::SetPrivateData(VkDevice device, VkObjectType objectType, uint64_t objectHandle, VkPrivateDataSlot privateDataSlot, uint64_t data) noexcept
{
#if (VULKAN_CURRENT_VERSION < VULKAN_VERSION_1_3)
#   define vkSetPrivateData vkSetPrivateDataEXT
#endif
    return vkSetPrivateData(device, objectType, objectHandle, privateDataSlot, data);
#if (VULKAN_CURRENT_VERSION < VULKAN_VERSION_1_3)
#   undef vkSetPrivateData
#endif
}

inline void VulkanWrapper::GetPrivateData(VkDevice device, VkObjectType objectType, uint64_t objectHandle, VkPrivateDataSlot privateDataSlot, uint64_t* pData) noexcept
{
#if (VULKAN_CURRENT_VERSION < VULKAN_VERSION_1_3)
#   define vkGetPrivateData vkGetPrivateDataEXT
#endif
    return vkGetPrivateData(device, objectType, objectHandle, privateDataSlot, pData);
#if (VULKAN_CURRENT_VERSION < VULKAN_VERSION_1_3)
#   undef vkGetPrivateData
#endif
}

inline void VulkanWrapper::CmdSetEvent2(VkCommandBuffer commandBuffer, VkEvent event, const VkDependencyInfo* pDependencyInfo) noexcept
{
#if (VULKAN_CURRENT_VERSION < VULKAN_VERSION_1_3)
#   define vkCmdSetEvent2 vkCmdSetEvent2KHR
#endif
    return vkCmdSetEvent2(commandBuffer, event, pDependencyInfo);
#if (VULKAN_CURRENT_VERSION < VULKAN_VERSION_1_3)
#   undef vkCmdSetEvent2
#endif
}

inline void VulkanWrapper::CmdResetEvent2(VkCommandBuffer commandBuffer, VkEvent event, VkPipelineStageFlags2 stageMask) noexcept
{
#if (VULKAN_CURRENT_VERSION < VULKAN_VERSION_1_3)
#   define vkCmdResetEvent2 vkCmdResetEvent2KHR
#endif
    return vkCmdResetEvent2(commandBuffer, event, stageMask);
#if (VULKAN_CURRENT_VERSION < VULKAN_VERSION_1_3)
#   undef vkCmdResetEvent2
#endif
}

inline void VulkanWrapper::CmdWaitEvents2(VkCommandBuffer commandBuffer, uint32_t eventCount, const VkEvent* pEvents, const VkDependencyInfo* pDependencyInfos) noexcept
{
#if (VULKAN_CURRENT_VERSION < VULKAN_VERSION_1_3)
#   define vkCmdWaitEvents2 vkCmdWaitEvents2KHR
#endif
    return vkCmdWaitEvents2(commandBuffer, eventCount, pEvents, pDependencyInfos);
#if (VULKAN_CURRENT_VERSION < VULKAN_VERSION_1_3)
#   undef vkCmdWaitEvents2
#endif
}

inline void VulkanWrapper::CmdPipelineBarrier2(VkCommandBuffer commandBuffer, const VkDependencyInfo* pDependencyInfo) noexcept
{
#if (VULKAN_CURRENT_VERSION < VULKAN_VERSION_1_3)
#   define vkCmdPipelineBarrier2 vkCmdPipelineBarrier2KHR
#endif
    return vkCmdPipelineBarrier2(commandBuffer, pDependencyInfo);
#if (VULKAN_CURRENT_VERSION < VULKAN_VERSION_1_3)
#   undef vkCmdPipelineBarrier2
#endif
}

inline void VulkanWrapper::CmdWriteTimestamp2(VkCommandBuffer commandBuffer, VkPipelineStageFlags2 stage, VkQueryPool queryPool, uint32_t query) noexcept
{
#if (VULKAN_CURRENT_VERSION < VULKAN_VERSION_1_3)
#   define vkCmdWriteTimestamp2 vkCmdWriteTimestamp2KHR
#endif
    return vkCmdWriteTimestamp2(commandBuffer, stage, queryPool, query);
#if (VULKAN_CURRENT_VERSION < VULKAN_VERSION_1_3)
#   undef vkCmdWriteTimestamp2
#endif
}

inline VkResult VulkanWrapper::QueueSubmit2(VkQueue queue, uint32_t submitCount, const VkSubmitInfo2* pSubmits, VkFence fence)
{
#if (VULKAN_CURRENT_VERSION < VULKAN_VERSION_1_3)
#   define vkQueueSubmit2 vkQueueSubmit2KHR
#endif
    {
        TRACE_PROFILER_VULKAN_SCOPE("vkQueueSubmit2", color::rgba8::BLACK);
        return vkQueueSubmit2(queue, submitCount, pSubmits, fence);
    }
#if (VULKAN_CURRENT_VERSION < VULKAN_VERSION_1_3)
#   undef vkQueueSubmit2
#endif
}

inline void VulkanWrapper::CmdCopyBuffer2(VkCommandBuffer commandBuffer, const VkCopyBufferInfo2* pCopyBufferInfo) noexcept
{
#if (VULKAN_CURRENT_VERSION < VULKAN_VERSION_1_3)
#   define vkCmdCopyBuffer2 vkCmdCopyBuffer2KHR
#endif
    return vkCmdCopyBuffer2(commandBuffer, pCopyBufferInfo);
#if (VULKAN_CURRENT_VERSION < VULKAN_VERSION_1_3)
#   undef vkCmdCopyBuffer2
#endif
}

inline void VulkanWrapper::CmdCopyImage2(VkCommandBuffer commandBuffer, const VkCopyImageInfo2* pCopyImageInfo) noexcept
{
#if (VULKAN_CURRENT_VERSION < VULKAN_VERSION_1_3)
#   define vkCmdCopyImage2 vkCmdCopyImage2KHR
#endif
    return vkCmdCopyImage2(commandBuffer, pCopyImageInfo);
#if (VULKAN_CURRENT_VERSION < VULKAN_VERSION_1_3)
#   undef vkCmdCopyImage2
#endif
}

inline void VulkanWrapper::CmdCopyBufferToImage2(VkCommandBuffer commandBuffer, const VkCopyBufferToImageInfo2* pCopyBufferToImageInfo) noexcept
{
#if (VULKAN_CURRENT_VERSION < VULKAN_VERSION_1_3)
#   define vkCmdCopyBufferToImage2 vkCmdCopyBufferToImage2KHR
#endif
    return vkCmdCopyBufferToImage2(commandBuffer, pCopyBufferToImageInfo);
#if (VULKAN_CURRENT_VERSION < VULKAN_VERSION_1_3)
#   undef vkCmdCopyBufferToImage2KHR
#endif
}

inline void VulkanWrapper::CmdCopyImageToBuffer2(VkCommandBuffer commandBuffer, const VkCopyImageToBufferInfo2* pCopyImageToBufferInfo) noexcept
{
#if (VULKAN_CURRENT_VERSION < VULKAN_VERSION_1_3)
#   define vkCmdCopyImageToBuffer2 vkCmdCopyImageToBuffer2KHR
#endif
    return vkCmdCopyImageToBuffer2(commandBuffer, pCopyImageToBufferInfo);
#if (VULKAN_CURRENT_VERSION < VULKAN_VERSION_1_3)
#   undef vkCmdCopyImageToBuffer2
#endif
}

inline void VulkanWrapper::CmdBlitImage2(VkCommandBuffer commandBuffer, const VkBlitImageInfo2* pBlitImageInfo) noexcept
{
#if (VULKAN_CURRENT_VERSION < VULKAN_VERSION_1_3)
#   define vkCmdBlitImage2 vkCmdBlitImage2KHR
#endif
    return vkCmdBlitImage2(commandBuffer, pBlitImageInfo);
#if (VULKAN_CURRENT_VERSION < VULKAN_VERSION_1_3)
#   undef vkCmdBlitImage2
#endif
}

inline void VulkanWrapper::CmdResolveImage2(VkCommandBuffer commandBuffer, const VkResolveImageInfo2* pResolveImageInfo) noexcept
{
#if (VULKAN_CURRENT_VERSION < VULKAN_VERSION_1_3)
#   define vkCmdResolveImage2 vkCmdResolveImage2KHR
#endif
    return vkCmdResolveImage2(commandBuffer, pResolveImageInfo);
#if (VULKAN_CURRENT_VERSION < VULKAN_VERSION_1_3)
#   undef vkCmdResolveImage2
#endif
}

inline void VulkanWrapper::CmdBeginRendering(VkCommandBuffer commandBuffer, const VkRenderingInfo* pRenderingInfo) noexcept
{
#if (VULKAN_CURRENT_VERSION < VULKAN_VERSION_1_3)
#   define vkCmdBeginRendering vkCmdBeginRenderingKHR
#endif
    return vkCmdBeginRendering(commandBuffer, pRenderingInfo);
#if (VULKAN_CURRENT_VERSION < VULKAN_VERSION_1_3)
#   undef vkCmdBeginRendering
#endif
}

inline void VulkanWrapper::CmdEndRendering(VkCommandBuffer commandBuffer) noexcept
{
#if (VULKAN_CURRENT_VERSION < VULKAN_VERSION_1_3)
#   define vkCmdEndRendering vkCmdEndRenderingKHR
#endif
    return vkCmdEndRendering(commandBuffer);
#if (VULKAN_CURRENT_VERSION < VULKAN_VERSION_1_3)
#   undef vkCmdEndRendering
#endif
}

inline void VulkanWrapper::CmdSetCullMode(VkCommandBuffer commandBuffer, VkCullModeFlags cullMode) noexcept
{
#if (VULKAN_CURRENT_VERSION < VULKAN_VERSION_1_3)
#   define vkCmdSetCullMode vkCmdSetCullModeEXT
#endif
    return vkCmdSetCullMode(commandBuffer, cullMode);
#if (VULKAN_CURRENT_VERSION < VULKAN_VERSION_1_3)
#   undef vkCmdSetCullMode
#endif
}

inline void VulkanWrapper::CmdSetFrontFace(VkCommandBuffer commandBuffer, VkFrontFace frontFace) noexcept
{
#if (VULKAN_CURRENT_VERSION < VULKAN_VERSION_1_3)
#   define vkCmdSetFrontFace vkCmdSetFrontFaceEXT
#endif
    return vkCmdSetFrontFace(commandBuffer, frontFace);
#if (VULKAN_CURRENT_VERSION < VULKAN_VERSION_1_3)
#   undef vkCmdSetFrontFace
#endif
}

inline void VulkanWrapper::CmdSetPrimitiveTopology(VkCommandBuffer commandBuffer, VkPrimitiveTopology primitiveTopology) noexcept
{
#if (VULKAN_CURRENT_VERSION < VULKAN_VERSION_1_3)
#   define vkCmdSetPrimitiveTopology vkCmdSetPrimitiveTopologyEXT
#endif
    return vkCmdSetPrimitiveTopology(commandBuffer, primitiveTopology);
#if (VULKAN_CURRENT_VERSION < VULKAN_VERSION_1_3)
#   undef vkCmdSetPrimitiveTopology
#endif
}

inline void VulkanWrapper::CmdSetViewportWithCount(VkCommandBuffer commandBuffer, uint32_t viewportCount, const VkViewport* pViewports) noexcept
{
#if (VULKAN_CURRENT_VERSION < VULKAN_VERSION_1_3)
#   define vkCmdSetViewportWithCount vkCmdSetViewportWithCountEXT
#endif
    return vkCmdSetViewportWithCount(commandBuffer, viewportCount, pViewports);
#if (VULKAN_CURRENT_VERSION < VULKAN_VERSION_1_3)
#   undef vkCmdSetViewportWithCount
#endif
}

inline void VulkanWrapper::CmdSetScissorWithCount(VkCommandBuffer commandBuffer, uint32_t scissorCount, const VkRect2D* pScissors) noexcept
{
#if (VULKAN_CURRENT_VERSION < VULKAN_VERSION_1_3)
#   define vkCmdSetScissorWithCount vkCmdSetScissorWithCountEXT
#endif
    return vkCmdSetScissorWithCount(commandBuffer, scissorCount, pScissors);
#if (VULKAN_CURRENT_VERSION < VULKAN_VERSION_1_3)
#   undef vkCmdSetScissorWithCount
#endif
}

inline void VulkanWrapper::CmdBindVertexBuffers2(VkCommandBuffer commandBuffer, uint32_t firstBinding, uint32_t bindingCount, const VkBuffer* pBuffers, const VkDeviceSize* pOffsets, const VkDeviceSize* pSizes, const VkDeviceSize* pStrides) noexcept
{
#if (VULKAN_CURRENT_VERSION < VULKAN_VERSION_1_3)
#   define vkCmdBindVertexBuffers2 vkCmdBindVertexBuffers2EXT
#endif
    {
        TRACE_PROFILER_VULKAN_SCOPE("vkCmdBindVertexBuffers2", color::rgba8::BLACK);
        return vkCmdBindVertexBuffers2(commandBuffer, firstBinding, bindingCount, pBuffers, pOffsets, pSizes, pStrides);
    }
#if (VULKAN_CURRENT_VERSION < VULKAN_VERSION_1_3)
#   undef vkCmdBindVertexBuffers2
#endif
}

inline void VulkanWrapper::CmdSetDepthTestEnable(VkCommandBuffer commandBuffer, VkBool32 depthTestEnable) noexcept
{
#if (VULKAN_CURRENT_VERSION < VULKAN_VERSION_1_3)
#   define vkCmdSetDepthTestEnable vkCmdSetDepthTestEnableEXT
#endif
    return vkCmdSetDepthTestEnable(commandBuffer, depthTestEnable);
#if (VULKAN_CURRENT_VERSION < VULKAN_VERSION_1_3)
#   undef vkCmdSetDepthTestEnable
#endif
}

inline void VulkanWrapper::CmdSetDepthWriteEnable(VkCommandBuffer commandBuffer, VkBool32 depthWriteEnable) noexcept
{
#if (VULKAN_CURRENT_VERSION < VULKAN_VERSION_1_3)
#   define vkCmdSetDepthWriteEnable vkCmdSetDepthWriteEnableEXT
#endif
    return vkCmdSetDepthWriteEnable(commandBuffer, depthWriteEnable);
#if (VULKAN_CURRENT_VERSION < VULKAN_VERSION_1_3)
#   undef vkCmdSetDepthWriteEnable
#endif
}

inline void VulkanWrapper::CmdSetDepthCompareOp(VkCommandBuffer commandBuffer, VkCompareOp depthCompareOp) noexcept
{
#if (VULKAN_CURRENT_VERSION < VULKAN_VERSION_1_3)
#   define vkCmdSetDepthCompareOp vkCmdSetDepthCompareOpEXT
#endif
    return vkCmdSetDepthCompareOp(commandBuffer, depthCompareOp);
#if (VULKAN_CURRENT_VERSION < VULKAN_VERSION_1_3)
#   undef vkCmdSetDepthCompareOp
#endif
}

inline void VulkanWrapper::CmdSetDepthBoundsTestEnable(VkCommandBuffer commandBuffer, VkBool32 depthBoundsTestEnable) noexcept
{
#if (VULKAN_CURRENT_VERSION < VULKAN_VERSION_1_3)
#   define vkCmdSetDepthBoundsTestEnable vkCmdSetDepthBoundsTestEnableEXT
#endif
    return vkCmdSetDepthBoundsTestEnable(commandBuffer, depthBoundsTestEnable);
#if (VULKAN_CURRENT_VERSION < VULKAN_VERSION_1_3)
#   undef vkCmdSetDepthBoundsTestEnable
#endif
}

inline void VulkanWrapper::CmdSetStencilTestEnable(VkCommandBuffer commandBuffer, VkBool32 stencilTestEnable) noexcept
{
#if (VULKAN_CURRENT_VERSION < VULKAN_VERSION_1_3)
#   define vkCmdSetStencilTestEnable vkCmdSetStencilTestEnableEXT
#endif
    return vkCmdSetStencilTestEnable(commandBuffer, stencilTestEnable);
#if (VULKAN_CURRENT_VERSION < VULKAN_VERSION_1_3)
#   undef vkCmdSetStencilTestEnable
#endif
}

inline void VulkanWrapper::CmdSetStencilOp(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask, VkStencilOp failOp, VkStencilOp passOp, VkStencilOp depthFailOp, VkCompareOp compareOp) noexcept
{
#if (VULKAN_CURRENT_VERSION < VULKAN_VERSION_1_3)
#   define vkCmdSetStencilOp vkCmdSetStencilOpEXT
#endif
    return vkCmdSetStencilOp(commandBuffer, faceMask, failOp, passOp, depthFailOp, compareOp);
#if (VULKAN_CURRENT_VERSION < VULKAN_VERSION_1_3)
#   undef vkCmdSetStencilOp
#endif
}

inline void VulkanWrapper::CmdSetRasterizerDiscardEnable(VkCommandBuffer commandBuffer, VkBool32 rasterizerDiscardEnable) noexcept
{
#if (VULKAN_CURRENT_VERSION < VULKAN_VERSION_1_3)
#   define vkCmdSetRasterizerDiscardEnable vkCmdSetRasterizerDiscardEnableEXT
#endif
    return vkCmdSetRasterizerDiscardEnable(commandBuffer, rasterizerDiscardEnable);
#if (VULKAN_CURRENT_VERSION < VULKAN_VERSION_1_3)
#   undef vkCmdSetRasterizerDiscardEnable
#endif
}

inline void VulkanWrapper::CmdSetDepthBiasEnable(VkCommandBuffer commandBuffer, VkBool32 depthBiasEnable) noexcept
{
#if (VULKAN_CURRENT_VERSION < VULKAN_VERSION_1_3)
#   define vkCmdSetDepthBiasEnable vkCmdSetDepthBiasEnableEXT
#endif
    return vkCmdSetDepthBiasEnable(commandBuffer, depthBiasEnable);
#if (VULKAN_CURRENT_VERSION < VULKAN_VERSION_1_3)
#   undef vkCmdSetDepthBiasEnable
#endif
}

inline void VulkanWrapper::CmdSetPrimitiveRestartEnable(VkCommandBuffer commandBuffer, VkBool32 primitiveRestartEnable) noexcept
{
#if (VULKAN_CURRENT_VERSION < VULKAN_VERSION_1_3)
#   define vkCmdSetPrimitiveRestartEnable vkCmdSetPrimitiveRestartEnableEXT
#endif
    return vkCmdSetPrimitiveRestartEnable(commandBuffer, primitiveRestartEnable);
#if (VULKAN_CURRENT_VERSION < VULKAN_VERSION_1_3)
#   undef vkCmdSetPrimitiveRestartEnable
#endif
}

inline void VulkanWrapper::GetDeviceBufferMemoryRequirements(VkDevice device, const VkDeviceBufferMemoryRequirements* pInfo, VkMemoryRequirements2* pMemoryRequirements) noexcept
{
#if (VULKAN_CURRENT_VERSION < VULKAN_VERSION_1_3)
#   define vkGetDeviceBufferMemoryRequirements vkGetDeviceBufferMemoryRequirementsKHR
#endif
    return vkGetDeviceBufferMemoryRequirements(device, pInfo, pMemoryRequirements);
#if (VULKAN_CURRENT_VERSION < VULKAN_VERSION_1_3)
#   undef vkGetDeviceBufferMemoryRequirements
#endif
}

inline void VulkanWrapper::GetDeviceImageMemoryRequirements(VkDevice device, const VkDeviceImageMemoryRequirements* pInfo, VkMemoryRequirements2* pMemoryRequirements) noexcept
{
#if (VULKAN_CURRENT_VERSION < VULKAN_VERSION_1_3)
#   define vkGetDeviceImageMemoryRequirements vkGetDeviceImageMemoryRequirementsKHR
#endif
    return GetDeviceImageMemoryRequirements(device, pInfo, pMemoryRequirements);
#if (VULKAN_CURRENT_VERSION < VULKAN_VERSION_1_3)
#   undef GetDeviceImageMemoryRequirements
#endif
}
inline void VulkanWrapper::GetDeviceImageSparseMemoryRequirements(VkDevice device, const VkDeviceImageMemoryRequirements* pInfo, uint32_t* pSparseMemoryRequirementCount, VkSparseImageMemoryRequirements2* pSparseMemoryRequirements) noexcept
{
#if (VULKAN_CURRENT_VERSION < VULKAN_VERSION_1_3)
#   define vkGetDeviceImageSparseMemoryRequirements vkGetDeviceImageSparseMemoryRequirementsKHR
#endif
    return vkGetDeviceImageSparseMemoryRequirements(device, pInfo, pSparseMemoryRequirementCount, pSparseMemoryRequirements);
#if (VULKAN_CURRENT_VERSION < VULKAN_VERSION_1_3)
#   undef vkGetDeviceImageSparseMemoryRequirements
#endif
}
#endif //VULKAN_VERSION_1_3

/////////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef VK_KHR_surface
inline void VulkanWrapper::DestroySurface(VkInstance instance, VkSurfaceKHR surface, const VkAllocationCallbacks* pAllocator) noexcept
{
    vkDestroySurfaceKHR(instance, surface, pAllocator);
}

inline VkResult VulkanWrapper::GetPhysicalDeviceSurfaceSupport(VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex, VkSurfaceKHR surface, VkBool32* pSupported) noexcept
{
    return vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, queueFamilyIndex, surface, pSupported);
}

inline VkResult VulkanWrapper::GetPhysicalDeviceSurfaceCapabilities(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, VkSurfaceCapabilitiesKHR* pSurfaceCapabilities) noexcept
{
    return vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, pSurfaceCapabilities);
}

inline VkResult VulkanWrapper::GetPhysicalDeviceSurfaceFormats(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, uint32_t* pSurfaceFormatCount, VkSurfaceFormatKHR* pSurfaceFormats) noexcept
{
    return vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, pSurfaceFormatCount, pSurfaceFormats);
}

inline VkResult VulkanWrapper::GetPhysicalDeviceSurfacePresentModes(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, uint32_t* pPresentModeCount, VkPresentModeKHR* pPresentModes) noexcept
{
    return vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, pPresentModeCount, pPresentModes);
}
#endif //VK_KHR_surface

/////////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef VK_KHR_win32_surface
inline VkResult VulkanWrapper::CreateWin32Surface(VkInstance instance, const VkWin32SurfaceCreateInfoKHR* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface) noexcept
{
    return vkCreateWin32SurfaceKHR(instance, pCreateInfo, pAllocator, pSurface);
}
inline VkBool32 VulkanWrapper::GetPhysicalDeviceWin32PresentationSupport(VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex) noexcept
{
    return vkGetPhysicalDeviceWin32PresentationSupportKHR(physicalDevice, queueFamilyIndex);
}
#endif //VK_KHR_win32_surface

/////////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef VK_KHR_android_surface
inline VkResult VulkanWrapper::CreateAndroidSurface(VkInstance instance, const VkAndroidSurfaceCreateInfoKHR* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface) noexcept
{
    return vkCreateAndroidSurfaceKHR(instance, pCreateInfo, pAllocator, pSurface);
}
#endif //VK_KHR_android_surface

/////////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef VK_KHR_swapchain
inline VkResult VulkanWrapper::CreateSwapchain(VkDevice device, const VkSwapchainCreateInfoKHR* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSwapchainKHR* pSwapchain) noexcept
{
    return vkCreateSwapchainKHR(device, pCreateInfo, pAllocator, pSwapchain);
}

inline void VulkanWrapper::DestroySwapchain(VkDevice device, VkSwapchainKHR swapchain, const VkAllocationCallbacks* pAllocator) noexcept
{
    vkDestroySwapchainKHR(device, swapchain, pAllocator);
}

inline VkResult VulkanWrapper::GetSwapchainImages(VkDevice device, VkSwapchainKHR swapchain, uint32_t* pSwapchainImageCount, VkImage* pSwapchainImages) noexcept
{
    return vkGetSwapchainImagesKHR(device, swapchain, pSwapchainImageCount, pSwapchainImages);
}

inline VkResult VulkanWrapper::AcquireNextImage(VkDevice device, VkSwapchainKHR swapchain, uint64_t timeout, VkSemaphore semaphore, VkFence fence, uint32_t* pImageIndex) noexcept
{
    {
        TRACE_PROFILER_VULKAN_SCOPE("vkAcquireNextImageKHR", color::rgba8::BLACK);
        return vkAcquireNextImageKHR(device, swapchain, timeout, semaphore, fence, pImageIndex);
    }
}

inline VkResult VulkanWrapper::QueuePresent(VkQueue queue, const VkPresentInfoKHR* pPresentInfo) noexcept
{
    {
        TRACE_PROFILER_VULKAN_SCOPE("vkQueuePresentKHR", color::rgba8::BLACK);
        return vkQueuePresentKHR(queue, pPresentInfo);
    }
}

inline VkResult VulkanWrapper::GetDeviceGroupPresentCapabilities(VkDevice device, VkDeviceGroupPresentCapabilitiesKHR* pDeviceGroupPresentCapabilities) noexcept
{
    return vkGetDeviceGroupPresentCapabilitiesKHR(device, pDeviceGroupPresentCapabilities);
}

inline VkResult VulkanWrapper::GetDeviceGroupSurfacePresentModes(VkDevice device, VkSurfaceKHR surface, VkDeviceGroupPresentModeFlagsKHR* pModes) noexcept
{
    return vkGetDeviceGroupSurfacePresentModesKHR(device, surface, pModes);
}

inline VkResult VulkanWrapper::GetPhysicalDevicePresentRectangles(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, uint32_t* pRectCount, VkRect2D* pRects) noexcept
{
    return vkGetPhysicalDevicePresentRectanglesKHR(physicalDevice, surface, pRectCount, pRects);
}

inline VkResult VulkanWrapper::AcquireNextImage2(VkDevice device, const VkAcquireNextImageInfoKHR* pAcquireInfo, uint32_t* pImageIndex) noexcept
{
    {
        TRACE_PROFILER_VULKAN_SCOPE("vkAcquireNextImage2KHR", color::rgba8::BLACK);
        return vkAcquireNextImage2KHR(device, pAcquireInfo, pImageIndex);
    }
}
#endif //VK_KHR_swapchain

/////////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef VK_EXT_debug_report
inline VkResult VulkanWrapper::CreateDebugReportCallback(VkInstance instance, const VkDebugReportCallbackCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugReportCallbackEXT* pCallback)
{
    return vkCreateDebugReportCallbackEXT(instance, pCreateInfo, pAllocator, pCallback);
}

inline void VulkanWrapper::DestroyDebugReportCallback(VkInstance instance, VkDebugReportCallbackEXT callback, const VkAllocationCallbacks* pAllocator)
{
    vkDestroyDebugReportCallbackEXT(instance, callback, pAllocator);
}

inline void VulkanWrapper::DebugReportMessage(VkInstance instance, VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objectType, uint64_t object, size_t location, int32_t messageCode, const char* pLayerPrefix, const char* pMessage)
{
    vkDebugReportMessageEXT(instance, flags, objectType, object, location, messageCode, pLayerPrefix, pMessage);
}
#endif //VK_EXT_debug_report

/////////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef VK_EXT_debug_utils
inline VkResult VulkanWrapper::SetDebugUtilsObjectName(VkDevice device, const VkDebugUtilsObjectNameInfoEXT* pNameInfo) noexcept
{
    return vkSetDebugUtilsObjectNameEXT(device, pNameInfo);
}

inline VkResult VulkanWrapper::SetDebugUtilsObjectTag(VkDevice device, const VkDebugUtilsObjectTagInfoEXT* pTagInfo) noexcept
{
    return vkSetDebugUtilsObjectTagEXT(device, pTagInfo);
}

inline void VulkanWrapper::QueueBeginDebugUtilsLabel(VkQueue queue, const VkDebugUtilsLabelEXT* pLabelInfo) noexcept
{
    vkQueueBeginDebugUtilsLabelEXT(queue, pLabelInfo);
}

inline void VulkanWrapper::QueueEndDebugUtilsLabel(VkQueue queue) noexcept
{
    vkQueueEndDebugUtilsLabelEXT(queue);
}

inline void VulkanWrapper::QueueInsertDebugUtilsLabel(VkQueue queue, const VkDebugUtilsLabelEXT* pLabelInfo) noexcept
{
    vkQueueInsertDebugUtilsLabelEXT(queue, pLabelInfo);
}

inline void VulkanWrapper::CmdBeginDebugUtilsLabel(VkCommandBuffer commandBuffer, const VkDebugUtilsLabelEXT* pLabelInfo) noexcept
{
    vkCmdBeginDebugUtilsLabelEXT(commandBuffer, pLabelInfo);
}

inline void VulkanWrapper::CmdEndDebugUtilsLabel(VkCommandBuffer commandBuffer) noexcept
{
    vkCmdEndDebugUtilsLabelEXT(commandBuffer);
}

inline void VulkanWrapper::CmdInsertDebugUtilsLabel(VkCommandBuffer commandBuffer, const VkDebugUtilsLabelEXT* pLabelInfo) noexcept
{
    vkCmdInsertDebugUtilsLabelEXT(commandBuffer, pLabelInfo);
}

inline VkResult VulkanWrapper::CreateDebugUtilsMessenger(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pMessenger) noexcept
{
    return vkCreateDebugUtilsMessengerEXT(instance, pCreateInfo, pAllocator, pMessenger);
}

inline void VulkanWrapper::DestroyDebugUtilsMessenger(VkInstance instance, VkDebugUtilsMessengerEXT messenger, const VkAllocationCallbacks* pAllocator) noexcept
{
    vkDestroyDebugUtilsMessengerEXT(instance, messenger, pAllocator);
}

inline void VulkanWrapper::SubmitDebugUtilsMessage(VkInstance instance, VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageTypes, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData) noexcept
{
    vkSubmitDebugUtilsMessageEXT(instance, messageSeverity, messageTypes, pCallbackData);
}
#endif //VK_EXT_debug_utils

/////////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef VK_KHR_pipeline_executable_properties
inline VkResult VulkanWrapper::GetPipelineExecutableProperties(VkDevice device, const VkPipelineInfoKHR* pPipelineInfo, uint32_t* pExecutableCount, VkPipelineExecutablePropertiesKHR* pProperties) noexcept
{
    return vkGetPipelineExecutablePropertiesKHR(device, pPipelineInfo, pExecutableCount, pProperties);
}

inline VkResult VulkanWrapper::GetPipelineExecutableStatistics(VkDevice device, const VkPipelineExecutableInfoKHR* pExecutableInfo, uint32_t* pStatisticCount, VkPipelineExecutableStatisticKHR* pStatistics) noexcept
{
    return vkGetPipelineExecutableStatisticsKHR(device, pExecutableInfo, pStatisticCount, pStatistics);
}

inline VkResult VulkanWrapper::GetPipelineExecutableInternalRepresentations(VkDevice device, const VkPipelineExecutableInfoKHR* pExecutableInfo, uint32_t* pInternalRepresentationCount, VkPipelineExecutableInternalRepresentationKHR* pInternalRepresentations) noexcept
{
    return vkGetPipelineExecutableInternalRepresentationsKHR(device, pExecutableInfo, pInternalRepresentationCount, pInternalRepresentations);
}
#endif //VK_KHR_pipeline_executable_properties

} //namespace vk
} //namespace renderer
} //namespace v3d
#endif //VULKAN_RENDER