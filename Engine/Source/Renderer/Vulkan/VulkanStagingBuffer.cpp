#include "VulkanStagingBuffer.h"

#ifdef VULKAN_RENDER
#include "VulkanMemory.h"

namespace v3d
{
namespace renderer
{
namespace vk
{

VulkanStaginBuffer::VulkanStaginBuffer(VulkanMemory* memory, VkDevice device, u64 size, u16 usageFlag) noexcept
    : m_device(device)
    , m_buffer(nullptr)
    , m_memoryManager(memory)
{
    m_buffer = new VulkanBuffer(m_memoryManager, m_device, Buffer::BufferType::BufferType_StagingBuffer, usageFlag, size);
}

VulkanStaginBuffer::~VulkanStaginBuffer()
{
    if (m_buffer)
    {
        m_buffer->destroy();

        delete m_buffer;
        m_buffer = nullptr;
    }
}

bool VulkanStaginBuffer::create()
{
    ASSERT(m_buffer, "nullptr");
    return m_buffer->create();
}

void VulkanStaginBuffer::destroy()
{
    ASSERT(m_buffer, "nullptr");
    m_buffer->destroy();
}

void * VulkanStaginBuffer::map()
{
    ASSERT(m_buffer, "nullptr");
    return m_buffer->map();
}

void VulkanStaginBuffer::unmap()
{
    ASSERT(m_buffer, "nullptr");
    m_buffer->unmap();
}

VulkanBuffer * VulkanStaginBuffer::getBuffer() const
{
    ASSERT(m_buffer, "nullptr");
    return m_buffer;
}



VulkanStaginBufferManager::VulkanStaginBufferManager(VkDevice device)
    : m_device(device)
    , m_memoryManager(new VulkanMemory(device))
{
}

VulkanStaginBufferManager::~VulkanStaginBufferManager()
{
    if (m_memoryManager)
    {
        delete m_memoryManager;
        m_memoryManager = nullptr;
    }
}

VulkanStaginBuffer * VulkanStaginBufferManager::createStagingBuffer(u64 size, u16 usageFlag) const
{
    VulkanStaginBuffer * stagingBuffer = new VulkanStaginBuffer(m_memoryManager, m_device, size, usageFlag);
    ASSERT(stagingBuffer, "nullptr");
    
    if (!stagingBuffer->create())
    {
        ASSERT(false, "fail");
        return nullptr;
    }

    return stagingBuffer;
}

} //namespace vk
} //namespace renderer
} //namespace v3d
#endif //VULKAN_RENDER
