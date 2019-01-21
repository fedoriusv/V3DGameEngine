#include "VulkanBuffer.h"

#include "Utils/Logger.h"

#ifdef VULKAN_RENDER
#include "VulkanDebug.h"

namespace v3d
{
namespace renderer
{
namespace vk
{

VulkanMemory::VulkanMemoryAllocator* VulkanBuffer::s_memoryAllocator = new SimpleVulkanMemoryAllocator();

VulkanBuffer::VulkanBuffer(VulkanMemory* memory, VkDevice device, u16 usageFlag)
    : m_device(device)
    , m_memoryManager(memory)
    , m_usageFlags(usageFlag)
{
}

VulkanBuffer::~VulkanBuffer()
{
}

bool VulkanBuffer::create()
{
    //VulkanWrapper::CreateBuffer(m_device, , VULKAN_ALLOCATOR, )

    return false;
}

void VulkanBuffer::destroy()
{
}

} //namespace vk
} //namespace renderer
} //namespace v3d
#endif //VULKAN_RENDER
