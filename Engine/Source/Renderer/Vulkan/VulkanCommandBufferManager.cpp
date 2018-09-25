#include "VulkanCommandBufferManager.h"
#include "VulkanCommandBuffer.h"
#include "VulkanDebug.h"
#include "VulkanGraphicContext.h"
#include "Utils/Logger.h"

#ifdef VULKAN_RENDER
namespace v3d
{
namespace renderer
{
namespace vk
{

 VulkanCommandBufferManager::VulkanCommandBufferManager(const struct DeviceInfo* info, VkQueue queue)
     : m_device(info->_device)
     , m_queue(queue)
     , m_familyIndex(info->_queueFamilyIndex)

     , m_poolFlag(VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT) //TODO:
 {
     VkCommandPool pool = createCommandPool(m_poolFlag, m_familyIndex);
     m_commandPools.push_back(pool);
 }

 VulkanCommandBufferManager::~VulkanCommandBufferManager()
 {
     for (auto& pool : m_commandPools)
     {
         destoryCommandPool(pool);
     }
     m_commandPools.clear();
 }

 VulkanCommandBuffer * VulkanCommandBufferManager::acquireNewCmdBuffer(CommandTargetType type)
 {

     return nullptr;
 }

 bool VulkanCommandBufferManager::submit(VulkanCommandBuffer* buffer, std::vector<VkSemaphore>& signalSemaphores)
 {
     ASSERT(buffer, "buffer is nullptr");
     if (buffer->getStatus() != VulkanCommandBuffer::CommandBufferStatus::EndBuffer)
     {
         LOG_ERROR("VulkanCommandBufferManager::submit buffer current status not EndBuffer. skip submit");
         return false;
     }

     VkCommandBuffer cmdBuffer = buffer->getHandle();

     VkSubmitInfo submitInfo = {};
     submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
     submitInfo.pNext = nullptr;
     submitInfo.waitSemaphoreCount = static_cast<u32>(buffer->m_semaphores.size());
     submitInfo.pWaitSemaphores = buffer->m_semaphores.data();
     submitInfo.pWaitDstStageMask = buffer->m_stageMasks.data();
     submitInfo.commandBufferCount = 1;
     submitInfo.pCommandBuffers = &cmdBuffer;
     submitInfo.signalSemaphoreCount = static_cast<u32>(signalSemaphores.size());
     submitInfo.pSignalSemaphores = signalSemaphores.data();

     VkResult result = VulkanWrapper::QueueSubmit(m_queue, 1, &submitInfo, buffer->m_fence);
     if (result != VK_SUCCESS)
     {
         LOG_ERROR("VulkanCommandBufferManager::submit vkQueueSubmit. Error %s", ErrorString(result).c_str());
         //TODO:

         buffer->m_status = VulkanCommandBuffer::CommandBufferStatus::Invalid;
         return false;
     }


     buffer->m_status = VulkanCommandBuffer::CommandBufferStatus::ExecuteBuffer;
     return true;
 }

 void VulkanCommandBufferManager::waitCompete()
 {
 }

 void VulkanCommandBufferManager::resetPools()
 {
     VkCommandPoolResetFlags flag = (m_poolFlag == VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT) ? 0 : VK_COMMAND_POOL_RESET_RELEASE_RESOURCES_BIT;
     for (auto pool : m_commandPools)
     {
         vkResetCommandPool(m_device, pool, VK_COMMAND_POOL_RESET_RELEASE_RESOURCES_BIT);
     }
 }

 VkCommandPool VulkanCommandBufferManager::createCommandPool(VkCommandPoolCreateFlags flag, u32 familyIndex)
 {
     VkCommandPoolCreateInfo commandPoolCreateInfo = {};
     commandPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
     commandPoolCreateInfo.pNext = nullptr;
     commandPoolCreateInfo.flags = flag;
     commandPoolCreateInfo.queueFamilyIndex = familyIndex;

     VkCommandPool pool = VK_NULL_HANDLE;
     VkResult result = VulkanWrapper::CreateCommandPool(m_device, &commandPoolCreateInfo, VULKAN_ALLOCATOR, &pool);
     if (result != VK_SUCCESS)
     {
         LOG_ERROR("VulkanCommandBufferManager::createCommandPool vkCreateCommandPool. Error %s", ErrorString(result).c_str());
         return VK_NULL_HANDLE;
     }

     return pool;
 }

 void VulkanCommandBufferManager::destoryCommandPool(VkCommandPool pool)
 {
     vkDestroyCommandPool(m_device, pool, VULKAN_ALLOCATOR);
 }

} //namespace vk
} //namespace renderer
} //namespace v3d
#endif //VULKAN_RENDER
