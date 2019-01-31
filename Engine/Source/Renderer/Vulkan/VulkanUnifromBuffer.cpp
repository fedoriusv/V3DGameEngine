#include "VulkanUnifromBuffer.h"

#ifdef VULKAN_RENDER
#include "VulkanBuffer.h"

namespace v3d
{
namespace renderer
{
namespace vk
{

VulkanUniformBufferManager::VulkanUniformBufferManager(VkDevice device)
    : m_device(device)
    , m_memoryManager(new SimpleVulkanMemoryAllocator(device))
{
}

VulkanUniformBufferManager::~VulkanUniformBufferManager()
{
    if (m_memoryManager)
    {
        delete m_memoryManager;
        m_memoryManager = nullptr;
    }
}

VulkanUnifromBuffer * VulkanUniformBufferManager::acquireUnformBuffer(u32 size)
{
    //TODO
    if (!m_currentBuffer)
    {
        m_currentBuffer = new VulkanBuffer(m_memoryManager, m_device, Buffer::BufferType::BufferType_UniformBuffer, 0, 1 * 1024 * 1024);
    }

    VulkanUnifromBuffer* newBuffer = new VulkanUnifromBuffer();
    newBuffer->_buffer = m_currentBuffer;
    newBuffer->_size = size;
    newBuffer->_offset = 0;

    m_uniformBuffers.push_back(newBuffer);

    return newBuffer;
}

} //namespace vk
} //namespace renderer
} //namespace v3d
#endif //VULKAN_RENDER
