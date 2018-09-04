#include "VulkanCommandBufferManager.h"

#ifdef VULKAN_RENDER
namespace v3d
{
namespace renderer
{
namespace vk
{

 VulkanCommandBufferManager::VulkanCommandBufferManager(VkQueue queue)
 {
 }

 VulkanCommandBufferManager::~VulkanCommandBufferManager()
 {
 }

 VulkanCommandBuffer * VulkanCommandBufferManager::acquireNewCmdBuffer(CommandTargetType type)
 {
     return nullptr;
 }

 void VulkanCommandBufferManager::submit(VulkanCommandBuffer * buffer)
 {
 }

 void VulkanCommandBufferManager::waitCompete()
 {
 }

 bool VulkanCommandBufferManager::createCommandPool()
 {
     return false;
 }

} //namespace vk
} //namespace renderer
} //namespace v3d
#endif //VULKAN_RENDER
