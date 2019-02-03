#include "VulkanWrapper.h"

#ifdef VULKAN_RENDER
namespace v3d
{
namespace renderer
{
namespace vk
{

VkResult VulkanWrapper::CreateInstance(const VkInstanceCreateInfo * pCreateInfo, const VkAllocationCallbacks * pAllocator, VkInstance * pInstance) noexcept
{
    return vkCreateInstance(pCreateInfo, pAllocator, pInstance);
}

void VulkanWrapper::DestroyInstance(VkInstance instance, const VkAllocationCallbacks * pAllocator) noexcept
{
    vkDestroyInstance(instance, pAllocator);
}

VkResult VulkanWrapper::EnumeratePhysicalDevices(VkInstance instance, uint32_t * pPhysicalDeviceCount, VkPhysicalDevice * pPhysicalDevices) noexcept
{
    return vkEnumeratePhysicalDevices(instance, pPhysicalDeviceCount, pPhysicalDevices);
}

void VulkanWrapper::GetPhysicalDeviceFeatures(VkPhysicalDevice physicalDevice, VkPhysicalDeviceFeatures * pFeatures) noexcept
{
    vkGetPhysicalDeviceFeatures(physicalDevice, pFeatures);
}

void VulkanWrapper::GetPhysicalDeviceFormatProperties(VkPhysicalDevice physicalDevice, VkFormat format, VkFormatProperties * pFormatProperties) noexcept
{
    vkGetPhysicalDeviceFormatProperties(physicalDevice, format, pFormatProperties);
}

VkResult VulkanWrapper::GetPhysicalDeviceImageFormatProperties(VkPhysicalDevice physicalDevice, VkFormat format, VkImageType type, VkImageTiling tiling, VkImageUsageFlags usage, VkImageCreateFlags flags, VkImageFormatProperties * pImageFormatProperties) noexcept
{
    return vkGetPhysicalDeviceImageFormatProperties(physicalDevice, format, type, tiling, usage, flags, pImageFormatProperties);
}

void VulkanWrapper::GetPhysicalDeviceProperties(VkPhysicalDevice physicalDevice, VkPhysicalDeviceProperties * pProperties) noexcept
{
    vkGetPhysicalDeviceProperties(physicalDevice, pProperties);
}

void VulkanWrapper::GetPhysicalDeviceQueueFamilyProperties(VkPhysicalDevice physicalDevice, uint32_t * pQueueFamilyPropertyCount, VkQueueFamilyProperties * pQueueFamilyProperties) noexcept
{
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, pQueueFamilyPropertyCount, pQueueFamilyProperties);
}

void VulkanWrapper::GetPhysicalDeviceMemoryProperties(VkPhysicalDevice physicalDevice, VkPhysicalDeviceMemoryProperties * pMemoryProperties) noexcept
{
    vkGetPhysicalDeviceMemoryProperties(physicalDevice, pMemoryProperties);
}

PFN_vkVoidFunction VulkanWrapper::GetInstanceProcAddr(VkInstance instance, const char * pName) noexcept
{
    return vkGetInstanceProcAddr(instance, pName);
}

PFN_vkVoidFunction VulkanWrapper::GetDeviceProcAddr(VkDevice device, const char * pName) noexcept
{
    return vkGetDeviceProcAddr(device, pName);
}

VkResult VulkanWrapper::CreateDevice(VkPhysicalDevice physicalDevice, const VkDeviceCreateInfo * pCreateInfo, const VkAllocationCallbacks * pAllocator, VkDevice * pDevice) noexcept
{
    return vkCreateDevice(physicalDevice, pCreateInfo, pAllocator, pDevice);
}

void VulkanWrapper::DestroyDevice(VkDevice device, const VkAllocationCallbacks * pAllocator) noexcept
{
    vkDestroyDevice(device, pAllocator);
}

VkResult VulkanWrapper::EnumerateInstanceExtensionProperties(const char * pLayerName, uint32_t * pPropertyCount, VkExtensionProperties * pProperties) noexcept
{
    return vkEnumerateInstanceExtensionProperties(pLayerName, pPropertyCount, pProperties);
}

VkResult VulkanWrapper::EnumerateDeviceExtensionProperties(VkPhysicalDevice physicalDevice, const char * pLayerName, uint32_t * pPropertyCount, VkExtensionProperties * pProperties) noexcept
{
    return vkEnumerateDeviceExtensionProperties(physicalDevice, pLayerName, pPropertyCount, pProperties);
}

VkResult VulkanWrapper::EnumerateInstanceLayerProperties(uint32_t * pPropertyCount, VkLayerProperties * pProperties) noexcept
{
    return vkEnumerateInstanceLayerProperties(pPropertyCount, pProperties);
}

VkResult VulkanWrapper::EnumerateDeviceLayerProperties(VkPhysicalDevice physicalDevice, uint32_t * pPropertyCount, VkLayerProperties * pProperties) noexcept
{
    return vkEnumerateDeviceLayerProperties(physicalDevice, pPropertyCount, pProperties);
}

void VulkanWrapper::GetDeviceQueue(VkDevice device, uint32_t queueFamilyIndex, uint32_t queueIndex, VkQueue * pQueue) noexcept
{
    vkGetDeviceQueue(device, queueFamilyIndex, queueIndex, pQueue);
}

VkResult VulkanWrapper::QueueSubmit(VkQueue queue, uint32_t submitCount, const VkSubmitInfo * pSubmits, VkFence fence) noexcept
{
    return vkQueueSubmit(queue, submitCount, pSubmits, fence);
}

VkResult VulkanWrapper::QueueWaitIdle(VkQueue queue) noexcept
{
    return vkQueueWaitIdle(queue);
}

VkResult VulkanWrapper::DeviceWaitIdle(VkDevice device) noexcept
{
    return vkDeviceWaitIdle(device);
}

VkResult VulkanWrapper::AllocateMemory(VkDevice device, const VkMemoryAllocateInfo * pAllocateInfo, const VkAllocationCallbacks * pAllocator, VkDeviceMemory * pMemory) noexcept
{
    return vkAllocateMemory(device, pAllocateInfo, pAllocator, pMemory);
}

void VulkanWrapper::FreeMemory(VkDevice device, VkDeviceMemory memory, const VkAllocationCallbacks * pAllocator) noexcept
{
    vkFreeMemory(device, memory, pAllocator);
}

VkResult VulkanWrapper::MapMemory(VkDevice device, VkDeviceMemory memory, VkDeviceSize offset, VkDeviceSize size, VkMemoryMapFlags flags, void ** ppData) noexcept
{
    return vkMapMemory(device, memory, offset, size, flags, ppData);
}

void VulkanWrapper::UnmapMemory(VkDevice device, VkDeviceMemory memory) noexcept
{
    vkUnmapMemory(device, memory);
}

VkResult VulkanWrapper::FlushMappedMemoryRanges(VkDevice device, uint32_t memoryRangeCount, const VkMappedMemoryRange * pMemoryRanges) noexcept
{
    return vkFlushMappedMemoryRanges(device, memoryRangeCount, pMemoryRanges);
}

VkResult VulkanWrapper::InvalidateMappedMemoryRanges(VkDevice device, uint32_t memoryRangeCount, const VkMappedMemoryRange * pMemoryRanges) noexcept
{
    return vkInvalidateMappedMemoryRanges(device, memoryRangeCount, pMemoryRanges);
}

void VulkanWrapper::GetDeviceMemoryCommitment(VkDevice device, VkDeviceMemory memory, VkDeviceSize * pCommittedMemoryInBytes) noexcept
{
    vkGetDeviceMemoryCommitment(device, memory, pCommittedMemoryInBytes);
}

VkResult VulkanWrapper::BindBufferMemory(VkDevice device, VkBuffer buffer, VkDeviceMemory memory, VkDeviceSize memoryOffset) noexcept
{
    return vkBindBufferMemory(device, buffer, memory, memoryOffset);
}

VkResult VulkanWrapper::BindImageMemory(VkDevice device, VkImage image, VkDeviceMemory memory, VkDeviceSize memoryOffset) noexcept
{
    return vkBindImageMemory(device, image, memory, memoryOffset);
}

void VulkanWrapper::GetBufferMemoryRequirements(VkDevice device, VkBuffer buffer, VkMemoryRequirements * pMemoryRequirements) noexcept
{
    vkGetBufferMemoryRequirements(device, buffer, pMemoryRequirements);
}

void VulkanWrapper::GetImageMemoryRequirements(VkDevice device, VkImage image, VkMemoryRequirements * pMemoryRequirements) noexcept
{
    vkGetImageMemoryRequirements(device, image, pMemoryRequirements);
}

void VulkanWrapper::GetImageSparseMemoryRequirements(VkDevice device, VkImage image, uint32_t * pSparseMemoryRequirementCount, VkSparseImageMemoryRequirements * pSparseMemoryRequirements) noexcept
{
    vkGetImageSparseMemoryRequirements(device, image, pSparseMemoryRequirementCount, pSparseMemoryRequirements);
}

void VulkanWrapper::GetPhysicalDeviceSparseImageFormatProperties(VkPhysicalDevice physicalDevice, VkFormat format, VkImageType type, VkSampleCountFlagBits samples, VkImageUsageFlags usage, VkImageTiling tiling, uint32_t * pPropertyCount, VkSparseImageFormatProperties * pProperties) noexcept
{
    vkGetPhysicalDeviceSparseImageFormatProperties(physicalDevice, format, type, samples, usage, tiling, pPropertyCount, pProperties);
}

VkResult VulkanWrapper::QueueBindSparse(VkQueue queue, uint32_t bindInfoCount, const VkBindSparseInfo * pBindInfo, VkFence fence) noexcept
{
    return vkQueueBindSparse(queue, bindInfoCount, pBindInfo, fence);
}

VkResult VulkanWrapper::CreateFence(VkDevice device, const VkFenceCreateInfo * pCreateInfo, const VkAllocationCallbacks * pAllocator, VkFence * pFence) noexcept
{
    return vkCreateFence(device, pCreateInfo, pAllocator, pFence);
}

void VulkanWrapper::DestroyFence(VkDevice device, VkFence fence, const VkAllocationCallbacks * pAllocator) noexcept
{
    vkDestroyFence(device, fence, pAllocator);
}

VkResult VulkanWrapper::ResetFences(VkDevice device, uint32_t fenceCount, const VkFence * pFences) noexcept
{
    return vkResetFences(device, fenceCount, pFences);
}

VkResult VulkanWrapper::GetFenceStatus(VkDevice device, VkFence fence) noexcept
{
    return vkGetFenceStatus(device, fence);
}

VkResult VulkanWrapper::WaitForFences(VkDevice device, uint32_t fenceCount, const VkFence * pFences, VkBool32 waitAll, uint64_t timeout) noexcept
{
    return vkWaitForFences(device, fenceCount, pFences, waitAll, timeout);
}

VkResult VulkanWrapper::CreateSemaphore(VkDevice device, const VkSemaphoreCreateInfo * pCreateInfo, const VkAllocationCallbacks * pAllocator, VkSemaphore * pSemaphore) noexcept
{
    return vkCreateSemaphore(device, pCreateInfo, pAllocator, pSemaphore);
}

void VulkanWrapper::DestroySemaphore(VkDevice device, VkSemaphore semaphore, const VkAllocationCallbacks * pAllocator) noexcept
{
    vkDestroySemaphore(device, semaphore, pAllocator);
}

VkResult VulkanWrapper::CreateEvent(VkDevice device, const VkEventCreateInfo * pCreateInfo, const VkAllocationCallbacks * pAllocator, VkEvent * pEvent) noexcept
{
    return vkCreateEvent(device, pCreateInfo, pAllocator, pEvent);
}

void VulkanWrapper::DestroyEvent(VkDevice device, VkEvent event, const VkAllocationCallbacks * pAllocator) noexcept
{
    vkDestroyEvent(device, event, pAllocator);
}

VkResult VulkanWrapper::GetEventStatus(VkDevice device, VkEvent event) noexcept
{
    return vkGetEventStatus(device, event);
}

VkResult VulkanWrapper::SetEvent(VkDevice device, VkEvent event) noexcept
{
    return vkSetEvent(device, event);
}

VkResult VulkanWrapper::ResetEvent(VkDevice device, VkEvent event) noexcept
{
    return vkResetEvent(device, event);
}

VkResult VulkanWrapper::CreateQueryPool(VkDevice device, const VkQueryPoolCreateInfo * pCreateInfo, const VkAllocationCallbacks * pAllocator, VkQueryPool * pQueryPool) noexcept
{
    return vkCreateQueryPool(device, pCreateInfo, pAllocator, pQueryPool);
}

void VulkanWrapper::DestroyQueryPool(VkDevice device, VkQueryPool queryPool, const VkAllocationCallbacks * pAllocator) noexcept
{
    vkDestroyQueryPool(device, queryPool, pAllocator);
}

VkResult VulkanWrapper::GetQueryPoolResults(VkDevice device, VkQueryPool queryPool, uint32_t firstQuery, uint32_t queryCount, size_t dataSize, void * pData, VkDeviceSize stride, VkQueryResultFlags flags) noexcept
{
    return vkGetQueryPoolResults(device, queryPool, firstQuery, queryCount, dataSize, pData, stride, flags);
}

VkResult VulkanWrapper::CreateBuffer(VkDevice device, const VkBufferCreateInfo * pCreateInfo, const VkAllocationCallbacks * pAllocator, VkBuffer * pBuffer) noexcept
{
    return vkCreateBuffer(device, pCreateInfo, pAllocator, pBuffer);
}

void VulkanWrapper::DestroyBuffer(VkDevice device, VkBuffer buffer, const VkAllocationCallbacks * pAllocator) noexcept
{
    vkDestroyBuffer(device, buffer, pAllocator);
}

VkResult VulkanWrapper::CreateBufferView(VkDevice device, const VkBufferViewCreateInfo * pCreateInfo, const VkAllocationCallbacks * pAllocator, VkBufferView * pView) noexcept
{
    return vkCreateBufferView(device, pCreateInfo, pAllocator, pView);
}

void VulkanWrapper::DestroyBufferView(VkDevice device, VkBufferView bufferView, const VkAllocationCallbacks * pAllocator) noexcept
{
    vkDestroyBufferView(device, bufferView, pAllocator);
}

VkResult VulkanWrapper::CreateImage(VkDevice device, const VkImageCreateInfo * pCreateInfo, const VkAllocationCallbacks * pAllocator, VkImage * pImage) noexcept
{
    return vkCreateImage(device, pCreateInfo, pAllocator, pImage);
}

void VulkanWrapper::DestroyImage(VkDevice device, VkImage image, const VkAllocationCallbacks * pAllocator) noexcept
{
    vkDestroyImage(device, image, pAllocator);
}

void VulkanWrapper::GetImageSubresourceLayout(VkDevice device, VkImage image, const VkImageSubresource * pSubresource, VkSubresourceLayout * pLayout) noexcept
{
    vkGetImageSubresourceLayout(device, image, pSubresource, pLayout);
}

VkResult VulkanWrapper::CreateImageView(VkDevice device, const VkImageViewCreateInfo * pCreateInfo, const VkAllocationCallbacks * pAllocator, VkImageView * pView) noexcept
{
    return vkCreateImageView(device, pCreateInfo, pAllocator, pView);
}

void VulkanWrapper::DestroyImageView(VkDevice device, VkImageView imageView, const VkAllocationCallbacks * pAllocator) noexcept
{
    vkDestroyImageView(device, imageView, pAllocator);
}

VkResult VulkanWrapper::CreateShaderModule(VkDevice device, const VkShaderModuleCreateInfo * pCreateInfo, const VkAllocationCallbacks * pAllocator, VkShaderModule * pShaderModule) noexcept
{
    return vkCreateShaderModule(device, pCreateInfo, pAllocator, pShaderModule);
}

void VulkanWrapper::DestroyShaderModule(VkDevice device, VkShaderModule shaderModule, const VkAllocationCallbacks * pAllocator) noexcept
{
    vkDestroyShaderModule(device, shaderModule, pAllocator);
}

VkResult VulkanWrapper::CreatePipelineCache(VkDevice device, const VkPipelineCacheCreateInfo * pCreateInfo, const VkAllocationCallbacks * pAllocator, VkPipelineCache * pPipelineCache) noexcept
{
    return vkCreatePipelineCache(device, pCreateInfo, pAllocator, pPipelineCache);
}

void VulkanWrapper::DestroyPipelineCache(VkDevice device, VkPipelineCache pipelineCache, const VkAllocationCallbacks * pAllocator) noexcept
{
    vkDestroyPipelineCache(device, pipelineCache, pAllocator);
}

VkResult VulkanWrapper::GetPipelineCacheData(VkDevice device, VkPipelineCache pipelineCache, size_t * pDataSize, void * pData) noexcept
{
    return vkGetPipelineCacheData(device, pipelineCache, pDataSize, pData);
}

VkResult VulkanWrapper::MergePipelineCaches(VkDevice device, VkPipelineCache dstCache, uint32_t srcCacheCount, const VkPipelineCache * pSrcCaches) noexcept
{
    return vkMergePipelineCaches(device, dstCache, srcCacheCount, pSrcCaches);
}

VkResult VulkanWrapper::CreateGraphicsPipelines(VkDevice device, VkPipelineCache pipelineCache, uint32_t createInfoCount, const VkGraphicsPipelineCreateInfo * pCreateInfos, const VkAllocationCallbacks * pAllocator, VkPipeline * pPipelines) noexcept
{
    return vkCreateGraphicsPipelines(device, pipelineCache, createInfoCount, pCreateInfos, pAllocator, pPipelines);
}

VkResult VulkanWrapper::CreateComputePipelines(VkDevice device, VkPipelineCache pipelineCache, uint32_t createInfoCount, const VkComputePipelineCreateInfo * pCreateInfos, const VkAllocationCallbacks * pAllocator, VkPipeline * pPipelines) noexcept
{
    return vkCreateComputePipelines(device, pipelineCache, createInfoCount, pCreateInfos, pAllocator, pPipelines);
}

void VulkanWrapper::DestroyPipeline(VkDevice device, VkPipeline pipeline, const VkAllocationCallbacks * pAllocator) noexcept
{
    vkDestroyPipeline(device, pipeline, pAllocator);
}

VkResult VulkanWrapper::CreatePipelineLayout(VkDevice device, const VkPipelineLayoutCreateInfo * pCreateInfo, const VkAllocationCallbacks * pAllocator, VkPipelineLayout * pPipelineLayout) noexcept
{
    return vkCreatePipelineLayout(device, pCreateInfo, pAllocator, pPipelineLayout);
}

void VulkanWrapper::DestroyPipelineLayout(VkDevice device, VkPipelineLayout pipelineLayout, const VkAllocationCallbacks * pAllocator) noexcept
{
    vkDestroyPipelineLayout(device, pipelineLayout, pAllocator);
}

VkResult VulkanWrapper::CreateSampler(VkDevice device, const VkSamplerCreateInfo * pCreateInfo, const VkAllocationCallbacks * pAllocator, VkSampler * pSampler) noexcept
{
    return vkCreateSampler(device, pCreateInfo, pAllocator, pSampler);
}

void VulkanWrapper::DestroySampler(VkDevice device, VkSampler sampler, const VkAllocationCallbacks * pAllocator) noexcept
{
    vkDestroySampler(device, sampler, pAllocator);
}

VkResult VulkanWrapper::CreateDescriptorSetLayout(VkDevice device, const VkDescriptorSetLayoutCreateInfo * pCreateInfo, const VkAllocationCallbacks * pAllocator, VkDescriptorSetLayout * pSetLayout) noexcept
{
    return vkCreateDescriptorSetLayout(device, pCreateInfo, pAllocator, pSetLayout);
}

void VulkanWrapper::DestroyDescriptorSetLayout(VkDevice device, VkDescriptorSetLayout descriptorSetLayout, const VkAllocationCallbacks * pAllocator) noexcept
{
    vkDestroyDescriptorSetLayout(device, descriptorSetLayout, pAllocator);
}

VkResult VulkanWrapper::CreateDescriptorPool(VkDevice device, const VkDescriptorPoolCreateInfo * pCreateInfo, const VkAllocationCallbacks * pAllocator, VkDescriptorPool * pDescriptorPool) noexcept
{
    return vkCreateDescriptorPool(device, pCreateInfo, pAllocator, pDescriptorPool);
}

void VulkanWrapper::DestroyDescriptorPool(VkDevice device, VkDescriptorPool descriptorPool, const VkAllocationCallbacks * pAllocator) noexcept
{
    vkDestroyDescriptorPool(device, descriptorPool, pAllocator);
}

VkResult VulkanWrapper::ResetDescriptorPool(VkDevice device, VkDescriptorPool descriptorPool, VkDescriptorPoolResetFlags flags) noexcept
{
    return vkResetDescriptorPool(device, descriptorPool, flags);
}

VkResult VulkanWrapper::AllocateDescriptorSets(VkDevice device, const VkDescriptorSetAllocateInfo * pAllocateInfo, VkDescriptorSet * pDescriptorSets) noexcept
{
    return vkAllocateDescriptorSets(device, pAllocateInfo, pDescriptorSets);
}

VkResult VulkanWrapper::FreeDescriptorSets(VkDevice device, VkDescriptorPool descriptorPool, uint32_t descriptorSetCount, const VkDescriptorSet * pDescriptorSets) noexcept
{
    return vkFreeDescriptorSets(device, descriptorPool, descriptorSetCount, pDescriptorSets);
}

void VulkanWrapper::UpdateDescriptorSets(VkDevice device, uint32_t descriptorWriteCount, const VkWriteDescriptorSet * pDescriptorWrites, uint32_t descriptorCopyCount, const VkCopyDescriptorSet * pDescriptorCopies) noexcept
{
    vkUpdateDescriptorSets(device, descriptorWriteCount, pDescriptorWrites, descriptorCopyCount, pDescriptorCopies);
}

VkResult VulkanWrapper::CreateFramebuffer(VkDevice device, const VkFramebufferCreateInfo * pCreateInfo, const VkAllocationCallbacks * pAllocator, VkFramebuffer * pFramebuffer) noexcept
{
    return vkCreateFramebuffer(device, pCreateInfo, pAllocator, pFramebuffer);
}

void VulkanWrapper::DestroyFramebuffer(VkDevice device, VkFramebuffer framebuffer, const VkAllocationCallbacks * pAllocator) noexcept
{
    vkDestroyFramebuffer(device, framebuffer, pAllocator);
}

VkResult VulkanWrapper::CreateRenderPass(VkDevice device, const VkRenderPassCreateInfo * pCreateInfo, const VkAllocationCallbacks * pAllocator, VkRenderPass * pRenderPass) noexcept
{
    return vkCreateRenderPass(device, pCreateInfo, pAllocator, pRenderPass);
}

void VulkanWrapper::DestroyRenderPass(VkDevice device, VkRenderPass renderPass, const VkAllocationCallbacks * pAllocator) noexcept
{
    vkDestroyRenderPass(device, renderPass, pAllocator);
}

void VulkanWrapper::GetRenderAreaGranularity(VkDevice device, VkRenderPass renderPass, VkExtent2D * pGranularity) noexcept
{
    vkGetRenderAreaGranularity(device, renderPass, pGranularity);
}

VkResult VulkanWrapper::CreateCommandPool(VkDevice device, const VkCommandPoolCreateInfo * pCreateInfo, const VkAllocationCallbacks * pAllocator, VkCommandPool * pCommandPool) noexcept
{
    return vkCreateCommandPool(device, pCreateInfo, pAllocator, pCommandPool);
}

void VulkanWrapper::DestroyCommandPool(VkDevice device, VkCommandPool commandPool, const VkAllocationCallbacks * pAllocator) noexcept
{
    vkDestroyCommandPool(device, commandPool, pAllocator);
}

VkResult VulkanWrapper::ResetCommandPool(VkDevice device, VkCommandPool commandPool, VkCommandPoolResetFlags flags) noexcept
{
    return vkResetCommandPool(device, commandPool, flags);
}

VkResult VulkanWrapper::AllocateCommandBuffers(VkDevice device, const VkCommandBufferAllocateInfo * pAllocateInfo, VkCommandBuffer * pCommandBuffers) noexcept
{
    return vkAllocateCommandBuffers(device, pAllocateInfo, pCommandBuffers);
}

void VulkanWrapper::FreeCommandBuffers(VkDevice device, VkCommandPool commandPool, uint32_t commandBufferCount, const VkCommandBuffer * pCommandBuffers) noexcept
{
    vkFreeCommandBuffers(device, commandPool, commandBufferCount, pCommandBuffers);
}

VkResult VulkanWrapper::BeginCommandBuffer(VkCommandBuffer commandBuffer, const VkCommandBufferBeginInfo * pBeginInfo) noexcept
{
    return vkBeginCommandBuffer(commandBuffer, pBeginInfo);
}

VkResult VulkanWrapper::EndCommandBuffer(VkCommandBuffer commandBuffer) noexcept
{
    return vkEndCommandBuffer(commandBuffer);
}

VkResult VulkanWrapper::ResetCommandBuffer(VkCommandBuffer commandBuffer, VkCommandBufferResetFlags flags) noexcept
{
    return vkResetCommandBuffer(commandBuffer, flags);
}

void VulkanWrapper::CmdBindPipeline(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint, VkPipeline pipeline) noexcept
{
    vkCmdBindPipeline(commandBuffer, pipelineBindPoint, pipeline);
}

void VulkanWrapper::CmdSetViewport(VkCommandBuffer commandBuffer, uint32_t firstViewport, uint32_t viewportCount, const VkViewport * pViewports) noexcept
{
    vkCmdSetViewport(commandBuffer, firstViewport, viewportCount, pViewports);
}

void VulkanWrapper::CmdSetScissor(VkCommandBuffer commandBuffer, uint32_t firstScissor, uint32_t scissorCount, const VkRect2D * pScissors) noexcept
{
    vkCmdSetScissor(commandBuffer, firstScissor, scissorCount, pScissors);
}

void VulkanWrapper::CmdSetLineWidth(VkCommandBuffer commandBuffer, float lineWidth) noexcept
{
    vkCmdSetLineWidth(commandBuffer, lineWidth);
}

void VulkanWrapper::CmdSetDepthBias(VkCommandBuffer commandBuffer, float depthBiasConstantFactor, float depthBiasClamp, float depthBiasSlopeFactor) noexcept
{
    vkCmdSetDepthBias(commandBuffer, depthBiasConstantFactor, depthBiasClamp, depthBiasSlopeFactor);
}

void VulkanWrapper::CmdSetBlendConstants(VkCommandBuffer commandBuffer, const float blendConstants[4]) noexcept
{
    vkCmdSetBlendConstants(commandBuffer, blendConstants);
}

void VulkanWrapper::CmdSetDepthBounds(VkCommandBuffer commandBuffer, float minDepthBounds, float maxDepthBounds) noexcept
{
    vkCmdSetDepthBounds(commandBuffer, minDepthBounds, maxDepthBounds);
}

void VulkanWrapper::CmdSetStencilCompareMask(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask, uint32_t compareMask) noexcept
{
    vkCmdSetStencilCompareMask(commandBuffer, faceMask, compareMask);
}

void VulkanWrapper::CmdSetStencilWriteMask(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask, uint32_t writeMask) noexcept
{
    vkCmdSetStencilWriteMask(commandBuffer, faceMask, writeMask);
}

void VulkanWrapper::CmdSetStencilReference(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask, uint32_t reference) noexcept
{
    vkCmdSetStencilReference(commandBuffer, faceMask, reference);
}

void VulkanWrapper::CmdBindDescriptorSets(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint, VkPipelineLayout layout, uint32_t firstSet, uint32_t descriptorSetCount, const VkDescriptorSet * pDescriptorSets, uint32_t dynamicOffsetCount, const uint32_t * pDynamicOffsets) noexcept
{
    vkCmdBindDescriptorSets(commandBuffer, pipelineBindPoint, layout, firstSet, descriptorSetCount, pDescriptorSets, dynamicOffsetCount, pDynamicOffsets);
}

void VulkanWrapper::CmdBindIndexBuffer(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, VkIndexType indexType) noexcept
{
    vkCmdBindIndexBuffer(commandBuffer, buffer, offset, indexType);
}

void VulkanWrapper::CmdBindVertexBuffers(VkCommandBuffer commandBuffer, uint32_t firstBinding, uint32_t bindingCount, const VkBuffer * pBuffers, const VkDeviceSize * pOffsets) noexcept
{
    vkCmdBindVertexBuffers(commandBuffer, firstBinding, bindingCount, pBuffers, pOffsets);
}

void VulkanWrapper::CmdDraw(VkCommandBuffer commandBuffer, uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance) noexcept
{
    vkCmdDraw(commandBuffer, vertexCount, instanceCount, firstVertex, firstInstance);
}

void VulkanWrapper::CmdDrawIndexed(VkCommandBuffer commandBuffer, uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, int32_t vertexOffset, uint32_t firstInstance) noexcept
{
    vkCmdDrawIndexed(commandBuffer, indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
}

void VulkanWrapper::CmdDrawIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, uint32_t drawCount, uint32_t stride) noexcept
{
    vkCmdDrawIndirect(commandBuffer, buffer, offset, drawCount, stride);
}

void VulkanWrapper::CmdDrawIndexedIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, uint32_t drawCount, uint32_t stride) noexcept
{
    vkCmdDrawIndexedIndirect(commandBuffer, buffer, offset, drawCount, stride);
}

void VulkanWrapper::CmdDispatch(VkCommandBuffer commandBuffer, uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ) noexcept
{
    vkCmdDispatch(commandBuffer, groupCountX, groupCountY, groupCountZ);
}

void VulkanWrapper::CmdDispatchIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset) noexcept
{
    vkCmdDispatchIndirect(commandBuffer, buffer, offset);
}

void VulkanWrapper::CmdCopyBuffer(VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkBuffer dstBuffer, uint32_t regionCount, const VkBufferCopy * pRegions) noexcept
{
    vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, regionCount, pRegions);
}

void VulkanWrapper::CmdCopyImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout, VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount, const VkImageCopy * pRegions) noexcept
{
    vkCmdCopyImage(commandBuffer, srcImage, srcImageLayout, dstImage, dstImageLayout, regionCount, pRegions);
}

void VulkanWrapper::CmdBlitImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout, VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount, const VkImageBlit * pRegions, VkFilter filter) noexcept
{
    vkCmdBlitImage(commandBuffer, srcImage, srcImageLayout, dstImage, dstImageLayout, regionCount, pRegions, filter);
}

void VulkanWrapper::CmdCopyBufferToImage(VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount, const VkBufferImageCopy * pRegions) noexcept
{
    vkCmdCopyBufferToImage(commandBuffer, srcBuffer, dstImage, dstImageLayout, regionCount, pRegions);
}

void VulkanWrapper::CmdCopyImageToBuffer(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout, VkBuffer dstBuffer, uint32_t regionCount, const VkBufferImageCopy * pRegions) noexcept
{
    vkCmdCopyImageToBuffer(commandBuffer, srcImage, srcImageLayout, dstBuffer, regionCount, pRegions);
}

void VulkanWrapper::CmdUpdateBuffer(VkCommandBuffer commandBuffer, VkBuffer dstBuffer, VkDeviceSize dstOffset, VkDeviceSize dataSize, const void * pData) noexcept
{
    vkCmdUpdateBuffer(commandBuffer, dstBuffer, dstOffset, dataSize, pData);
}

void VulkanWrapper::CmdFillBuffer(VkCommandBuffer commandBuffer, VkBuffer dstBuffer, VkDeviceSize dstOffset, VkDeviceSize size, uint32_t data) noexcept
{
    vkCmdFillBuffer(commandBuffer, dstBuffer, dstOffset, size, data);
}

void VulkanWrapper::CmdClearColorImage(VkCommandBuffer commandBuffer, VkImage image, VkImageLayout imageLayout, const VkClearColorValue * pColor, uint32_t rangeCount, const VkImageSubresourceRange * pRanges) noexcept
{
    vkCmdClearColorImage(commandBuffer, image, imageLayout, pColor, rangeCount, pRanges);
}

void VulkanWrapper::CmdClearDepthStencilImage(VkCommandBuffer commandBuffer, VkImage image, VkImageLayout imageLayout, const VkClearDepthStencilValue * pDepthStencil, uint32_t rangeCount, const VkImageSubresourceRange * pRanges) noexcept
{
    vkCmdClearDepthStencilImage(commandBuffer, image, imageLayout, pDepthStencil, rangeCount, pRanges);
}

void VulkanWrapper::CmdClearAttachments(VkCommandBuffer commandBuffer, uint32_t attachmentCount, const VkClearAttachment * pAttachments, uint32_t rectCount, const VkClearRect * pRects) noexcept
{
    vkCmdClearAttachments(commandBuffer, attachmentCount, pAttachments, rectCount, pRects);
}

void VulkanWrapper::CmdResolveImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout, VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount, const VkImageResolve * pRegions) noexcept
{
    vkCmdResolveImage(commandBuffer, srcImage, srcImageLayout, dstImage, dstImageLayout, regionCount, pRegions);
}

void VulkanWrapper::CmdSetEvent(VkCommandBuffer commandBuffer, VkEvent event, VkPipelineStageFlags stageMask) noexcept
{
    vkCmdSetEvent(commandBuffer, event, stageMask);
}

void VulkanWrapper::CmdResetEvent(VkCommandBuffer commandBuffer, VkEvent event, VkPipelineStageFlags stageMask) noexcept
{
    vkCmdResetEvent(commandBuffer, event, stageMask);
}

void VulkanWrapper::CmdWaitEvents(VkCommandBuffer commandBuffer, uint32_t eventCount, const VkEvent * pEvents, VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask, uint32_t memoryBarrierCount, const VkMemoryBarrier * pMemoryBarriers, uint32_t bufferMemoryBarrierCount, const VkBufferMemoryBarrier * pBufferMemoryBarriers, uint32_t imageMemoryBarrierCount, const VkImageMemoryBarrier * pImageMemoryBarriers) noexcept
{
    vkCmdWaitEvents(commandBuffer, eventCount, pEvents, srcStageMask, dstStageMask, memoryBarrierCount, pMemoryBarriers, bufferMemoryBarrierCount, pBufferMemoryBarriers, imageMemoryBarrierCount, pImageMemoryBarriers);
}

void VulkanWrapper::CmdPipelineBarrier(VkCommandBuffer commandBuffer, VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask, VkDependencyFlags dependencyFlags, uint32_t memoryBarrierCount, const VkMemoryBarrier * pMemoryBarriers, uint32_t bufferMemoryBarrierCount, const VkBufferMemoryBarrier * pBufferMemoryBarriers, uint32_t imageMemoryBarrierCount, const VkImageMemoryBarrier * pImageMemoryBarriers) noexcept
{
    vkCmdPipelineBarrier(commandBuffer, srcStageMask, dstStageMask, dependencyFlags, 
        memoryBarrierCount, pMemoryBarriers, bufferMemoryBarrierCount, pBufferMemoryBarriers, imageMemoryBarrierCount, pImageMemoryBarriers);
}

void VulkanWrapper::CmdBeginQuery(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t query, VkQueryControlFlags flags) noexcept
{
    vkCmdBeginQuery(commandBuffer, queryPool, query, flags);
}

void VulkanWrapper::CmdEndQuery(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t query) noexcept
{
    vkCmdEndQuery(commandBuffer, queryPool, query);
}

void VulkanWrapper::CmdResetQueryPool(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t firstQuery, uint32_t queryCount) noexcept
{
    vkCmdResetQueryPool(commandBuffer, queryPool, firstQuery, queryCount);
}

void VulkanWrapper::CmdWriteTimestamp(VkCommandBuffer commandBuffer, VkPipelineStageFlagBits pipelineStage, VkQueryPool queryPool, uint32_t query) noexcept
{
    vkCmdWriteTimestamp(commandBuffer, pipelineStage, queryPool, query);
}

void VulkanWrapper::CmdCopyQueryPoolResults(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t firstQuery, uint32_t queryCount, VkBuffer dstBuffer, VkDeviceSize dstOffset, VkDeviceSize stride, VkQueryResultFlags flags) noexcept
{
    vkCmdCopyQueryPoolResults(commandBuffer, queryPool, firstQuery, queryCount, dstBuffer, dstOffset, stride, flags);
}

void VulkanWrapper::CmdPushConstants(VkCommandBuffer commandBuffer, VkPipelineLayout layout, VkShaderStageFlags stageFlags, uint32_t offset, uint32_t size, const void * pValues) noexcept
{
    vkCmdPushConstants(commandBuffer, layout, stageFlags, offset, size, pValues);
}

void VulkanWrapper::CmdBeginRenderPass(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo * pRenderPassBegin, VkSubpassContents contents) noexcept
{
    vkCmdBeginRenderPass(commandBuffer, pRenderPassBegin, contents);
}

void VulkanWrapper::CmdNextSubpass(VkCommandBuffer commandBuffer, VkSubpassContents contents) noexcept
{
    vkCmdNextSubpass(commandBuffer, contents);
}

void VulkanWrapper::CmdEndRenderPass(VkCommandBuffer commandBuffer) noexcept
{
    vkCmdEndRenderPass(commandBuffer);
}

void VulkanWrapper::CmdExecuteCommands(VkCommandBuffer commandBuffer, uint32_t commandBufferCount, const VkCommandBuffer * pCommandBuffers) noexcept
{
    vkCmdExecuteCommands(commandBuffer, commandBufferCount, pCommandBuffers);
}

#ifdef VK_KHR_surface
void VulkanWrapper::DestroySurfaceKHR(VkInstance instance, VkSurfaceKHR surface, const VkAllocationCallbacks * pAllocator) noexcept
{
    vkDestroySurfaceKHR(instance, surface, pAllocator);
}

VkResult VulkanWrapper::GetPhysicalDeviceSurfaceSupportKHR(VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex, VkSurfaceKHR surface, VkBool32 * pSupported) noexcept
{
    return vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, queueFamilyIndex, surface, pSupported);
}

VkResult VulkanWrapper::GetPhysicalDeviceSurfaceCapabilitiesKHR(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, VkSurfaceCapabilitiesKHR * pSurfaceCapabilities) noexcept
{
    return vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, pSurfaceCapabilities);
}

VkResult VulkanWrapper::GetPhysicalDeviceSurfaceFormatsKHR(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, uint32_t * pSurfaceFormatCount, VkSurfaceFormatKHR * pSurfaceFormats) noexcept
{
    return vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, pSurfaceFormatCount, pSurfaceFormats);
}

VkResult VulkanWrapper::GetPhysicalDeviceSurfacePresentModesKHR(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, uint32_t * pPresentModeCount, VkPresentModeKHR * pPresentModes) noexcept
{
    return vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, pPresentModeCount, pPresentModes);
}
#endif //VK_KHR_surface

#ifdef VK_KHR_win32_surface
VkResult VulkanWrapper::CreateWin32SurfaceKHR(VkInstance instance, const VkWin32SurfaceCreateInfoKHR * pCreateInfo, const VkAllocationCallbacks * pAllocator, VkSurfaceKHR * pSurface) noexcept
{
    return vkCreateWin32SurfaceKHR(instance, pCreateInfo, pAllocator, pSurface);
}
VkBool32 VulkanWrapper::GetPhysicalDeviceWin32PresentationSupportKHR(VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex) noexcept
{
    return vkGetPhysicalDeviceWin32PresentationSupportKHR(physicalDevice, queueFamilyIndex);
}
#endif //VK_KHR_win32_surface

#ifdef VK_KHR_swapchain
VkResult VulkanWrapper::CreateSwapchainKHR(VkDevice device, const VkSwapchainCreateInfoKHR * pCreateInfo, const VkAllocationCallbacks * pAllocator, VkSwapchainKHR * pSwapchain) noexcept
{
    return vkCreateSwapchainKHR(device, pCreateInfo, pAllocator, pSwapchain);
}

void VulkanWrapper::DestroySwapchainKHR(VkDevice device, VkSwapchainKHR swapchain, const VkAllocationCallbacks * pAllocator) noexcept
{
    vkDestroySwapchainKHR(device, swapchain, pAllocator);
}

VkResult VulkanWrapper::GetSwapchainImagesKHR(VkDevice device, VkSwapchainKHR swapchain, uint32_t * pSwapchainImageCount, VkImage * pSwapchainImages) noexcept
{
    return vkGetSwapchainImagesKHR(device, swapchain, pSwapchainImageCount, pSwapchainImages);
}

VkResult VulkanWrapper::AcquireNextImageKHR(VkDevice device, VkSwapchainKHR swapchain, uint64_t timeout, VkSemaphore semaphore, VkFence fence, uint32_t * pImageIndex) noexcept
{
    return vkAcquireNextImageKHR(device, swapchain, timeout, semaphore, fence, pImageIndex);
}

VkResult VulkanWrapper::QueuePresentKHR(VkQueue queue, const VkPresentInfoKHR * pPresentInfo) noexcept
{
    return vkQueuePresentKHR(queue, pPresentInfo);
}

VkResult VulkanWrapper::GetDeviceGroupPresentCapabilitiesKHR(VkDevice device, VkDeviceGroupPresentCapabilitiesKHR * pDeviceGroupPresentCapabilities) noexcept
{
    return vkGetDeviceGroupPresentCapabilitiesKHR(device, pDeviceGroupPresentCapabilities);
}

VkResult VulkanWrapper::GetDeviceGroupSurfacePresentModesKHR(VkDevice device, VkSurfaceKHR surface, VkDeviceGroupPresentModeFlagsKHR * pModes) noexcept
{
    return vkGetDeviceGroupSurfacePresentModesKHR(device, surface, pModes);
}

VkResult VulkanWrapper::GetPhysicalDevicePresentRectanglesKHR(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, uint32_t * pRectCount, VkRect2D * pRects) noexcept
{
    return vkGetPhysicalDevicePresentRectanglesKHR(physicalDevice, surface, pRectCount, pRects);
}

VkResult VulkanWrapper::AcquireNextImage2KHR(VkDevice device, const VkAcquireNextImageInfoKHR * pAcquireInfo, uint32_t * pImageIndex) noexcept
{
    return vkAcquireNextImage2KHR(device, pAcquireInfo, pImageIndex);
}
#endif //VK_KHR_swapchain

#ifdef VK_EXT_debug_utils
VkResult VulkanWrapper::SetDebugUtilsObjectNameEXT(VkDevice device, const VkDebugUtilsObjectNameInfoEXT * pNameInfo) noexcept
{
    return vkSetDebugUtilsObjectNameEXT(device, pNameInfo);
}

VkResult VulkanWrapper::SetDebugUtilsObjectTagEXT(VkDevice device, const VkDebugUtilsObjectTagInfoEXT * pTagInfo) noexcept
{
    return vkSetDebugUtilsObjectTagEXT(device, pTagInfo);
}

void VulkanWrapper::QueueBeginDebugUtilsLabelEXT(VkQueue queue, const VkDebugUtilsLabelEXT * pLabelInfo) noexcept
{
    vkQueueBeginDebugUtilsLabelEXT(queue, pLabelInfo);
}

void VulkanWrapper::QueueEndDebugUtilsLabelEXT(VkQueue queue) noexcept
{
    vkQueueEndDebugUtilsLabelEXT(queue);
}

void VulkanWrapper::QueueInsertDebugUtilsLabelEXT(VkQueue queue, const VkDebugUtilsLabelEXT * pLabelInfo) noexcept
{
    vkQueueInsertDebugUtilsLabelEXT(queue, pLabelInfo);
}

void VulkanWrapper::CmdBeginDebugUtilsLabelEXT(VkCommandBuffer commandBuffer, const VkDebugUtilsLabelEXT * pLabelInfo) noexcept
{
    vkCmdBeginDebugUtilsLabelEXT(commandBuffer, pLabelInfo);
}

void VulkanWrapper::CmdEndDebugUtilsLabelEXT(VkCommandBuffer commandBuffer) noexcept
{
    vkCmdEndDebugUtilsLabelEXT(commandBuffer);
}

void VulkanWrapper::CmdInsertDebugUtilsLabelEXT(VkCommandBuffer commandBuffer, const VkDebugUtilsLabelEXT * pLabelInfo) noexcept
{
    vkCmdInsertDebugUtilsLabelEXT(commandBuffer, pLabelInfo);
}

VkResult VulkanWrapper::CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT * pCreateInfo, const VkAllocationCallbacks * pAllocator, VkDebugUtilsMessengerEXT * pMessenger) noexcept
{
    return vkCreateDebugUtilsMessengerEXT(instance, pCreateInfo, pAllocator, pMessenger);
}

void VulkanWrapper::DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT messenger, const VkAllocationCallbacks * pAllocator) noexcept
{
    vkDestroyDebugUtilsMessengerEXT(instance, messenger, pAllocator);
}

void VulkanWrapper::SubmitDebugUtilsMessageEXT(VkInstance instance, VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageTypes, const VkDebugUtilsMessengerCallbackDataEXT * pCallbackData) noexcept
{
    vkSubmitDebugUtilsMessageEXT(instance, messageSeverity, messageTypes, pCallbackData);
}
#endif //VK_EXT_debug_utils

} //namespace vk
} //namespace renderer
} //namespace v3d
#endif //VULKAN_RENDER
