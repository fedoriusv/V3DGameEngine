#include "VulkanWrapper.h"

#ifdef VULKAN_RENDER
namespace v3d
{
namespace renderer
{
namespace vk
{

VkResult VulkanWrapper::CreateInstance(const VkInstanceCreateInfo * pCreateInfo, const VkAllocationCallbacks * pAllocator, VkInstance * pInstance)
{
    return vkCreateInstance(pCreateInfo, pAllocator, pInstance);
}

void VulkanWrapper::DestroyInstance(VkInstance instance, const VkAllocationCallbacks * pAllocator)
{
    vkDestroyInstance(instance, pAllocator);
}

VkResult VulkanWrapper::EnumeratePhysicalDevices(VkInstance instance, uint32_t * pPhysicalDeviceCount, VkPhysicalDevice * pPhysicalDevices)
{
    return vkEnumeratePhysicalDevices(instance, pPhysicalDeviceCount, pPhysicalDevices);
}

void VulkanWrapper::GetPhysicalDeviceFeatures(VkPhysicalDevice physicalDevice, VkPhysicalDeviceFeatures * pFeatures)
{
    vkGetPhysicalDeviceFeatures(physicalDevice, pFeatures);
}

void VulkanWrapper::GetPhysicalDeviceFormatProperties(VkPhysicalDevice physicalDevice, VkFormat format, VkFormatProperties * pFormatProperties)
{
    vkGetPhysicalDeviceFormatProperties(physicalDevice, format, pFormatProperties);
}

VkResult VulkanWrapper::GetPhysicalDeviceImageFormatProperties(VkPhysicalDevice physicalDevice, VkFormat format, VkImageType type, VkImageTiling tiling, VkImageUsageFlags usage, VkImageCreateFlags flags, VkImageFormatProperties * pImageFormatProperties)
{
    return vkGetPhysicalDeviceImageFormatProperties(physicalDevice, format, type, tiling, usage, flags, pImageFormatProperties);
}

void VulkanWrapper::GetPhysicalDeviceProperties(VkPhysicalDevice physicalDevice, VkPhysicalDeviceProperties * pProperties)
{
    vkGetPhysicalDeviceProperties(physicalDevice, pProperties);
}

void VulkanWrapper::GetPhysicalDeviceQueueFamilyProperties(VkPhysicalDevice physicalDevice, uint32_t * pQueueFamilyPropertyCount, VkQueueFamilyProperties * pQueueFamilyProperties)
{
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, pQueueFamilyPropertyCount, pQueueFamilyProperties);
}

void VulkanWrapper::GetPhysicalDeviceMemoryProperties(VkPhysicalDevice physicalDevice, VkPhysicalDeviceMemoryProperties * pMemoryProperties)
{
    vkGetPhysicalDeviceMemoryProperties(physicalDevice, pMemoryProperties);
}

PFN_vkVoidFunction VulkanWrapper::GetInstanceProcAddr(VkInstance instance, const char * pName)
{
    return vkGetInstanceProcAddr(instance, pName);
}

PFN_vkVoidFunction VulkanWrapper::GetDeviceProcAddr(VkDevice device, const char * pName)
{
    return vkGetDeviceProcAddr(device, pName);
}

VkResult VulkanWrapper::CreateDevice(VkPhysicalDevice physicalDevice, const VkDeviceCreateInfo * pCreateInfo, const VkAllocationCallbacks * pAllocator, VkDevice * pDevice)
{
    return vkCreateDevice(physicalDevice, pCreateInfo, pAllocator, pDevice);
}

void VulkanWrapper::DestroyDevice(VkDevice device, const VkAllocationCallbacks * pAllocator)
{
    vkDestroyDevice(device, pAllocator);
}

VkResult VulkanWrapper::EnumerateInstanceExtensionProperties(const char * pLayerName, uint32_t * pPropertyCount, VkExtensionProperties * pProperties)
{
    return vkEnumerateInstanceExtensionProperties(pLayerName, pPropertyCount, pProperties);
}

VkResult VulkanWrapper::EnumerateDeviceExtensionProperties(VkPhysicalDevice physicalDevice, const char * pLayerName, uint32_t * pPropertyCount, VkExtensionProperties * pProperties)
{
    return vkEnumerateDeviceExtensionProperties(physicalDevice, pLayerName, pPropertyCount, pProperties);
}

VkResult VulkanWrapper::EnumerateInstanceLayerProperties(uint32_t * pPropertyCount, VkLayerProperties * pProperties)
{
    return vkEnumerateInstanceLayerProperties(pPropertyCount, pProperties);
}

VkResult VulkanWrapper::EnumerateDeviceLayerProperties(VkPhysicalDevice physicalDevice, uint32_t * pPropertyCount, VkLayerProperties * pProperties)
{
    return vkEnumerateDeviceLayerProperties(physicalDevice, pPropertyCount, pProperties);
}

void VulkanWrapper::GetDeviceQueue(VkDevice device, uint32_t queueFamilyIndex, uint32_t queueIndex, VkQueue * pQueue)
{
    vkGetDeviceQueue(device, queueFamilyIndex, queueIndex, pQueue);
}

VkResult VulkanWrapper::QueueSubmit(VkQueue queue, uint32_t submitCount, const VkSubmitInfo * pSubmits, VkFence fence)
{
    return vkQueueSubmit(queue, submitCount, pSubmits, fence);
}

VkResult VulkanWrapper::QueueWaitIdle(VkQueue queue)
{
    return vkQueueWaitIdle(queue);
}

VkResult VulkanWrapper::DeviceWaitIdle(VkDevice device)
{
    return vkDeviceWaitIdle(device);
}

//////////////////////
VkResult VulkanWrapper::AllocateMemory(VkDevice device, const VkMemoryAllocateInfo * pAllocateInfo, const VkAllocationCallbacks * pAllocator, VkDeviceMemory * pMemory)
{
    return VK_NOT_READY;
}

void VulkanWrapper::FreeMemory(VkDevice device, VkDeviceMemory memory, const VkAllocationCallbacks * pAllocator)
{
}

VkResult VulkanWrapper::MapMemory(VkDevice device, VkDeviceMemory memory, VkDeviceSize offset, VkDeviceSize size, VkMemoryMapFlags flags, void ** ppData)
{
    return VK_NOT_READY;
}

void VulkanWrapper::UnmapMemory(VkDevice device, VkDeviceMemory memory)
{
}

VkResult VulkanWrapper::FlushMappedMemoryRanges(VkDevice device, uint32_t memoryRangeCount, const VkMappedMemoryRange * pMemoryRanges)
{
    return VK_NOT_READY;
}

VkResult VulkanWrapper::InvalidateMappedMemoryRanges(VkDevice device, uint32_t memoryRangeCount, const VkMappedMemoryRange * pMemoryRanges)
{
    return VK_NOT_READY;
}

void VulkanWrapper::GetDeviceMemoryCommitment(VkDevice device, VkDeviceMemory memory, VkDeviceSize * pCommittedMemoryInBytes)
{
}

VkResult VulkanWrapper::BindBufferMemory(VkDevice device, VkBuffer buffer, VkDeviceMemory memory, VkDeviceSize memoryOffset)
{
    return VK_NOT_READY;
}

VkResult VulkanWrapper::BindImageMemory(VkDevice device, VkImage image, VkDeviceMemory memory, VkDeviceSize memoryOffset)
{
    return VK_NOT_READY;
}

void VulkanWrapper::GetBufferMemoryRequirements(VkDevice device, VkBuffer buffer, VkMemoryRequirements * pMemoryRequirements)
{
}

void VulkanWrapper::GetImageMemoryRequirements(VkDevice device, VkImage image, VkMemoryRequirements * pMemoryRequirements)
{
}

void VulkanWrapper::GetImageSparseMemoryRequirements(VkDevice device, VkImage image, uint32_t * pSparseMemoryRequirementCount, VkSparseImageMemoryRequirements * pSparseMemoryRequirements)
{
}

void VulkanWrapper::GetPhysicalDeviceSparseImageFormatProperties(VkPhysicalDevice physicalDevice, VkFormat format, VkImageType type, VkSampleCountFlagBits samples, VkImageUsageFlags usage, VkImageTiling tiling, uint32_t * pPropertyCount, VkSparseImageFormatProperties * pProperties)
{
}

VkResult VulkanWrapper::QueueBindSparse(VkQueue queue, uint32_t bindInfoCount, const VkBindSparseInfo * pBindInfo, VkFence fence)
{
    return VK_NOT_READY;
}

VkResult VulkanWrapper::CreateFence(VkDevice device, const VkFenceCreateInfo * pCreateInfo, const VkAllocationCallbacks * pAllocator, VkFence * pFence)
{
    return VK_NOT_READY;
}

void VulkanWrapper::DestroyFence(VkDevice device, VkFence fence, const VkAllocationCallbacks * pAllocator)
{
}

VkResult VulkanWrapper::ResetFences(VkDevice device, uint32_t fenceCount, const VkFence * pFences)
{
    return VK_NOT_READY;
}

VkResult VulkanWrapper::GetFenceStatus(VkDevice device, VkFence fence)
{
    return VK_NOT_READY;
}

VkResult VulkanWrapper::WaitForFences(VkDevice device, uint32_t fenceCount, const VkFence * pFences, VkBool32 waitAll, uint64_t timeout)
{
    return VK_NOT_READY;
}

VkResult VulkanWrapper::CreateSemaphore(VkDevice device, const VkSemaphoreCreateInfo * pCreateInfo, const VkAllocationCallbacks * pAllocator, VkSemaphore * pSemaphore)
{
    return VK_NOT_READY;
}

void VulkanWrapper::DestroySemaphore(VkDevice device, VkSemaphore semaphore, const VkAllocationCallbacks * pAllocator)
{
}

VkResult VulkanWrapper::CreateEvent(VkDevice device, const VkEventCreateInfo * pCreateInfo, const VkAllocationCallbacks * pAllocator, VkEvent * pEvent)
{
    return VK_NOT_READY;
}

void VulkanWrapper::DestroyEvent(VkDevice device, VkEvent event, const VkAllocationCallbacks * pAllocator)
{
}

VkResult VulkanWrapper::GetEventStatus(VkDevice device, VkEvent event)
{
    return VK_NOT_READY;
}

VkResult VulkanWrapper::SetEvent(VkDevice device, VkEvent event)
{
    return VK_NOT_READY;
}

VkResult VulkanWrapper::ResetEvent(VkDevice device, VkEvent event)
{
    return VK_NOT_READY;
}

VkResult VulkanWrapper::CreateQueryPool(VkDevice device, const VkQueryPoolCreateInfo * pCreateInfo, const VkAllocationCallbacks * pAllocator, VkQueryPool * pQueryPool)
{
    return VK_NOT_READY;
}

void VulkanWrapper::DestroyQueryPool(VkDevice device, VkQueryPool queryPool, const VkAllocationCallbacks * pAllocator)
{
}

VkResult VulkanWrapper::GetQueryPoolResults(VkDevice device, VkQueryPool queryPool, uint32_t firstQuery, uint32_t queryCount, size_t dataSize, void * pData, VkDeviceSize stride, VkQueryResultFlags flags)
{
    return VK_NOT_READY;
}

VkResult VulkanWrapper::CreateBuffer(VkDevice device, const VkBufferCreateInfo * pCreateInfo, const VkAllocationCallbacks * pAllocator, VkBuffer * pBuffer)
{
    return VK_NOT_READY;
}

void VulkanWrapper::DestroyBuffer(VkDevice device, VkBuffer buffer, const VkAllocationCallbacks * pAllocator)
{
}

VkResult VulkanWrapper::CreateBufferView(VkDevice device, const VkBufferViewCreateInfo * pCreateInfo, const VkAllocationCallbacks * pAllocator, VkBufferView * pView)
{
    return VK_NOT_READY;
}

void VulkanWrapper::DestroyBufferView(VkDevice device, VkBufferView bufferView, const VkAllocationCallbacks * pAllocator)
{
}

VkResult VulkanWrapper::CreateImage(VkDevice device, const VkImageCreateInfo * pCreateInfo, const VkAllocationCallbacks * pAllocator, VkImage * pImage)
{
    return VK_NOT_READY;
}

void VulkanWrapper::DestroyImage(VkDevice device, VkImage image, const VkAllocationCallbacks * pAllocator)
{
}

void VulkanWrapper::GetImageSubresourceLayout(VkDevice device, VkImage image, const VkImageSubresource * pSubresource, VkSubresourceLayout * pLayout)
{
}

VkResult VulkanWrapper::CreateImageView(VkDevice device, const VkImageViewCreateInfo * pCreateInfo, const VkAllocationCallbacks * pAllocator, VkImageView * pView)
{
    return VK_NOT_READY;
}

void VulkanWrapper::DestroyImageView(VkDevice device, VkImageView imageView, const VkAllocationCallbacks * pAllocator)
{
}

VkResult VulkanWrapper::CreateShaderModule(VkDevice device, const VkShaderModuleCreateInfo * pCreateInfo, const VkAllocationCallbacks * pAllocator, VkShaderModule * pShaderModule)
{
    return VK_NOT_READY;
}

void VulkanWrapper::DestroyShaderModule(VkDevice device, VkShaderModule shaderModule, const VkAllocationCallbacks * pAllocator)
{
}

VkResult VulkanWrapper::CreatePipelineCache(VkDevice device, const VkPipelineCacheCreateInfo * pCreateInfo, const VkAllocationCallbacks * pAllocator, VkPipelineCache * pPipelineCache)
{
    return VK_NOT_READY;
}

void VulkanWrapper::DestroyPipelineCache(VkDevice device, VkPipelineCache pipelineCache, const VkAllocationCallbacks * pAllocator)
{
}

VkResult VulkanWrapper::GetPipelineCacheData(VkDevice device, VkPipelineCache pipelineCache, size_t * pDataSize, void * pData)
{
    return VK_NOT_READY;
}

VkResult VulkanWrapper::MergePipelineCaches(VkDevice device, VkPipelineCache dstCache, uint32_t srcCacheCount, const VkPipelineCache * pSrcCaches)
{
    return VK_NOT_READY;
}

VkResult VulkanWrapper::CreateGraphicsPipelines(VkDevice device, VkPipelineCache pipelineCache, uint32_t createInfoCount, const VkGraphicsPipelineCreateInfo * pCreateInfos, const VkAllocationCallbacks * pAllocator, VkPipeline * pPipelines)
{
    return VK_NOT_READY;
}

VkResult VulkanWrapper::CreateComputePipelines(VkDevice device, VkPipelineCache pipelineCache, uint32_t createInfoCount, const VkComputePipelineCreateInfo * pCreateInfos, const VkAllocationCallbacks * pAllocator, VkPipeline * pPipelines)
{
    return VK_NOT_READY;
}

void VulkanWrapper::DestroyPipeline(VkDevice device, VkPipeline pipeline, const VkAllocationCallbacks * pAllocator)
{
}

VkResult VulkanWrapper::CreatePipelineLayout(VkDevice device, const VkPipelineLayoutCreateInfo * pCreateInfo, const VkAllocationCallbacks * pAllocator, VkPipelineLayout * pPipelineLayout)
{
    return VK_NOT_READY;
}

void VulkanWrapper::DestroyPipelineLayout(VkDevice device, VkPipelineLayout pipelineLayout, const VkAllocationCallbacks * pAllocator)
{
}

VkResult VulkanWrapper::CreateSampler(VkDevice device, const VkSamplerCreateInfo * pCreateInfo, const VkAllocationCallbacks * pAllocator, VkSampler * pSampler)
{
    return VK_NOT_READY;
}

void VulkanWrapper::DestroySampler(VkDevice device, VkSampler sampler, const VkAllocationCallbacks * pAllocator)
{
}

VkResult VulkanWrapper::CreateDescriptorSetLayout(VkDevice device, const VkDescriptorSetLayoutCreateInfo * pCreateInfo, const VkAllocationCallbacks * pAllocator, VkDescriptorSetLayout * pSetLayout)
{
    return VK_NOT_READY;
}

void VulkanWrapper::DestroyDescriptorSetLayout(VkDevice device, VkDescriptorSetLayout descriptorSetLayout, const VkAllocationCallbacks * pAllocator)
{
}

VkResult VulkanWrapper::CreateDescriptorPool(VkDevice device, const VkDescriptorPoolCreateInfo * pCreateInfo, const VkAllocationCallbacks * pAllocator, VkDescriptorPool * pDescriptorPool)
{
    return VK_NOT_READY;
}

void VulkanWrapper::DestroyDescriptorPool(VkDevice device, VkDescriptorPool descriptorPool, const VkAllocationCallbacks * pAllocator)
{
}

VkResult VulkanWrapper::ResetDescriptorPool(VkDevice device, VkDescriptorPool descriptorPool, VkDescriptorPoolResetFlags flags)
{
    return VK_NOT_READY;
}

VkResult VulkanWrapper::AllocateDescriptorSets(VkDevice device, const VkDescriptorSetAllocateInfo * pAllocateInfo, VkDescriptorSet * pDescriptorSets)
{
    return VK_NOT_READY;
}

VkResult VulkanWrapper::FreeDescriptorSets(VkDevice device, VkDescriptorPool descriptorPool, uint32_t descriptorSetCount, const VkDescriptorSet * pDescriptorSets)
{
    return VK_NOT_READY;
}

void VulkanWrapper::UpdateDescriptorSets(VkDevice device, uint32_t descriptorWriteCount, const VkWriteDescriptorSet * pDescriptorWrites, uint32_t descriptorCopyCount, const VkCopyDescriptorSet * pDescriptorCopies)
{
}

VkResult VulkanWrapper::CreateFramebuffer(VkDevice device, const VkFramebufferCreateInfo * pCreateInfo, const VkAllocationCallbacks * pAllocator, VkFramebuffer * pFramebuffer)
{
    return VK_NOT_READY;
}

void VulkanWrapper::DestroyFramebuffer(VkDevice device, VkFramebuffer framebuffer, const VkAllocationCallbacks * pAllocator)
{
}

VkResult VulkanWrapper::CreateRenderPass(VkDevice device, const VkRenderPassCreateInfo * pCreateInfo, const VkAllocationCallbacks * pAllocator, VkRenderPass * pRenderPass)
{
    return VK_NOT_READY;
}

void VulkanWrapper::DestroyRenderPass(VkDevice device, VkRenderPass renderPass, const VkAllocationCallbacks * pAllocator)
{
}

void VulkanWrapper::GetRenderAreaGranularity(VkDevice device, VkRenderPass renderPass, VkExtent2D * pGranularity)
{
}

VkResult VulkanWrapper::CreateCommandPool(VkDevice device, const VkCommandPoolCreateInfo * pCreateInfo, const VkAllocationCallbacks * pAllocator, VkCommandPool * pCommandPool)
{
    return VK_NOT_READY;
}

void VulkanWrapper::DestroyCommandPool(VkDevice device, VkCommandPool commandPool, const VkAllocationCallbacks * pAllocator)
{
}

VkResult VulkanWrapper::ResetCommandPool(VkDevice device, VkCommandPool commandPool, VkCommandPoolResetFlags flags)
{
    return VK_NOT_READY;
}

VkResult VulkanWrapper::AllocateCommandBuffers(VkDevice device, const VkCommandBufferAllocateInfo * pAllocateInfo, VkCommandBuffer * pCommandBuffers)
{
    return VK_NOT_READY;
}

void VulkanWrapper::FreeCommandBuffers(VkDevice device, VkCommandPool commandPool, uint32_t commandBufferCount, const VkCommandBuffer * pCommandBuffers)
{
}

VkResult VulkanWrapper::BeginCommandBuffer(VkCommandBuffer commandBuffer, const VkCommandBufferBeginInfo * pBeginInfo)
{
    return VK_NOT_READY;
}

VkResult VulkanWrapper::EndCommandBuffer(VkCommandBuffer commandBuffer)
{
    return VK_NOT_READY;
}

VkResult VulkanWrapper::ResetCommandBuffer(VkCommandBuffer commandBuffer, VkCommandBufferResetFlags flags)
{
    return VK_NOT_READY;
}

void VulkanWrapper::CmdBindPipeline(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint, VkPipeline pipeline)
{
}

void VulkanWrapper::CmdSetViewport(VkCommandBuffer commandBuffer, uint32_t firstViewport, uint32_t viewportCount, const VkViewport * pViewports)
{
}

void VulkanWrapper::CmdSetScissor(VkCommandBuffer commandBuffer, uint32_t firstScissor, uint32_t scissorCount, const VkRect2D * pScissors)
{
}

void VulkanWrapper::CmdSetLineWidth(VkCommandBuffer commandBuffer, float lineWidth)
{
}

void VulkanWrapper::CmdSetDepthBias(VkCommandBuffer commandBuffer, float depthBiasConstantFactor, float depthBiasClamp, float depthBiasSlopeFactor)
{
}

void VulkanWrapper::CmdSetBlendConstants(VkCommandBuffer commandBuffer, const float blendConstants[4])
{
}

void VulkanWrapper::CmdSetDepthBounds(VkCommandBuffer commandBuffer, float minDepthBounds, float maxDepthBounds)
{
}

void VulkanWrapper::CmdSetStencilCompareMask(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask, uint32_t compareMask)
{
}

void VulkanWrapper::CmdSetStencilWriteMask(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask, uint32_t writeMask)
{
}

void VulkanWrapper::CmdSetStencilReference(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask, uint32_t reference)
{
}

void VulkanWrapper::CmdBindDescriptorSets(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint, VkPipelineLayout layout, uint32_t firstSet, uint32_t descriptorSetCount, const VkDescriptorSet * pDescriptorSets, uint32_t dynamicOffsetCount, const uint32_t * pDynamicOffsets)
{
}

void VulkanWrapper::CmdBindIndexBuffer(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, VkIndexType indexType)
{
}

void VulkanWrapper::CmdBindVertexBuffers(VkCommandBuffer commandBuffer, uint32_t firstBinding, uint32_t bindingCount, const VkBuffer * pBuffers, const VkDeviceSize * pOffsets)
{
}

void VulkanWrapper::CmdDraw(VkCommandBuffer commandBuffer, uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance)
{
}

void VulkanWrapper::CmdDrawIndexed(VkCommandBuffer commandBuffer, uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, int32_t vertexOffset, uint32_t firstInstance)
{
}

void VulkanWrapper::CmdDrawIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, uint32_t drawCount, uint32_t stride)
{
}

void VulkanWrapper::CmdDrawIndexedIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, uint32_t drawCount, uint32_t stride)
{
}

void VulkanWrapper::CmdDispatch(VkCommandBuffer commandBuffer, uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ)
{
}

void VulkanWrapper::CmdDispatchIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset)
{
}

void VulkanWrapper::CmdCopyBuffer(VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkBuffer dstBuffer, uint32_t regionCount, const VkBufferCopy * pRegions)
{
}

void VulkanWrapper::CmdCopyImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout, VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount, const VkImageCopy * pRegions)
{
}

void VulkanWrapper::CmdBlitImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout, VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount, const VkImageBlit * pRegions, VkFilter filter)
{
}

void VulkanWrapper::CmdCopyBufferToImage(VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount, const VkBufferImageCopy * pRegions)
{
}

void VulkanWrapper::CmdCopyImageToBuffer(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout, VkBuffer dstBuffer, uint32_t regionCount, const VkBufferImageCopy * pRegions)
{
}

void VulkanWrapper::CmdUpdateBuffer(VkCommandBuffer commandBuffer, VkBuffer dstBuffer, VkDeviceSize dstOffset, VkDeviceSize dataSize, const void * pData)
{
}

void VulkanWrapper::CmdFillBuffer(VkCommandBuffer commandBuffer, VkBuffer dstBuffer, VkDeviceSize dstOffset, VkDeviceSize size, uint32_t data)
{
}

void VulkanWrapper::CmdClearColorImage(VkCommandBuffer commandBuffer, VkImage image, VkImageLayout imageLayout, const VkClearColorValue * pColor, uint32_t rangeCount, const VkImageSubresourceRange * pRanges)
{
}

void VulkanWrapper::CmdClearDepthStencilImage(VkCommandBuffer commandBuffer, VkImage image, VkImageLayout imageLayout, const VkClearDepthStencilValue * pDepthStencil, uint32_t rangeCount, const VkImageSubresourceRange * pRanges)
{
}

void VulkanWrapper::CmdClearAttachments(VkCommandBuffer commandBuffer, uint32_t attachmentCount, const VkClearAttachment * pAttachments, uint32_t rectCount, const VkClearRect * pRects)
{
}

void VulkanWrapper::CmdResolveImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout, VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount, const VkImageResolve * pRegions)
{
}

void VulkanWrapper::CmdSetEvent(VkCommandBuffer commandBuffer, VkEvent event, VkPipelineStageFlags stageMask)
{
}

void VulkanWrapper::CmdResetEvent(VkCommandBuffer commandBuffer, VkEvent event, VkPipelineStageFlags stageMask)
{
}

void VulkanWrapper::CmdWaitEvents(VkCommandBuffer commandBuffer, uint32_t eventCount, const VkEvent * pEvents, VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask, uint32_t memoryBarrierCount, const VkMemoryBarrier * pMemoryBarriers, uint32_t bufferMemoryBarrierCount, const VkBufferMemoryBarrier * pBufferMemoryBarriers, uint32_t imageMemoryBarrierCount, const VkImageMemoryBarrier * pImageMemoryBarriers)
{
}

void VulkanWrapper::CmdPipelineBarrier(VkCommandBuffer commandBuffer, VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask, VkDependencyFlags dependencyFlags, uint32_t memoryBarrierCount, const VkMemoryBarrier * pMemoryBarriers, uint32_t bufferMemoryBarrierCount, const VkBufferMemoryBarrier * pBufferMemoryBarriers, uint32_t imageMemoryBarrierCount, const VkImageMemoryBarrier * pImageMemoryBarriers)
{
}

void VulkanWrapper::CmdBeginQuery(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t query, VkQueryControlFlags flags)
{
}

void VulkanWrapper::CmdEndQuery(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t query)
{
}

void VulkanWrapper::CmdResetQueryPool(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t firstQuery, uint32_t queryCount)
{
}

void VulkanWrapper::CmdWriteTimestamp(VkCommandBuffer commandBuffer, VkPipelineStageFlagBits pipelineStage, VkQueryPool queryPool, uint32_t query)
{
}

void VulkanWrapper::CmdCopyQueryPoolResults(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t firstQuery, uint32_t queryCount, VkBuffer dstBuffer, VkDeviceSize dstOffset, VkDeviceSize stride, VkQueryResultFlags flags)
{
}

void VulkanWrapper::CmdPushConstants(VkCommandBuffer commandBuffer, VkPipelineLayout layout, VkShaderStageFlags stageFlags, uint32_t offset, uint32_t size, const void * pValues)
{
}

void VulkanWrapper::CmdBeginRenderPass(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo * pRenderPassBegin, VkSubpassContents contents)
{
}

void VulkanWrapper::CmdNextSubpass(VkCommandBuffer commandBuffer, VkSubpassContents contents)
{
}

void VulkanWrapper::CmdEndRenderPass(VkCommandBuffer commandBuffer)
{
}

void VulkanWrapper::CmdExecuteCommands(VkCommandBuffer commandBuffer, uint32_t commandBufferCount, const VkCommandBuffer * pCommandBuffers)
{
}

} //namespace vk
} //namespace renderer
} //namespace v3d
#endif //VULKAN_RENDER
