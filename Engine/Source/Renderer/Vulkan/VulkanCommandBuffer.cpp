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
    , m_command(VK_NULL_HANDLE)
{
}

VulkanCommandBuffer::~VulkanCommandBuffer()
{
}

VkCommandBuffer VulkanCommandBuffer::getHandle() const
{
    return m_command;
}

void VulkanCommandBuffer::beginCommandBuffer()
{
}

void VulkanCommandBuffer::endCommandBuffer()
{
}

void VulkanCommandBuffer::cmdDraw()
{
    if (m_type == CommandBufferType::PrimaryBuffer)
    {
        //VulkanWrapper::CmdDraw(m_command, );
    }
    else
    {
        //create cmdDraw task
    }
}

void VulkanCommandBuffer::cmdCopyBufferToImage()
{
    //VulkanWrapper::CmdCopyBufferToImage(m_command, , , )
}

} //namespace vk
} //namespace renderer
} //namespace v3d
#endif //VULKAN_RENDER
