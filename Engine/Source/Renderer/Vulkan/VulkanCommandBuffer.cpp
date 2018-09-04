#include "VulkanCommandBuffer.h"

#ifdef VULKAN_RENDER
namespace v3d
{
namespace renderer
{
namespace vk
{

VulkanCommandBuffer::VulkanCommandBuffer(VkDevice device, VkCommandPool pool)
    : m_device(device)
    , m_pool(pool)
    , m_buffer(VK_NULL_HANDLE)
{
}

VulkanCommandBuffer::~VulkanCommandBuffer()
{
}

VkCommandBuffer VulkanCommandBuffer::getHandle() const
{
    return m_buffer;
}

void VulkanCommandBuffer::beginCommandBuffer()
{
}

void VulkanCommandBuffer::endCommandBuffer()
{
}

} //namespace vk
} //namespace renderer
} //namespace v3d
#endif //VULKAN_RENDER
