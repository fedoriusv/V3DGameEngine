#include "VulkanUnifromBuffer.h"

#ifdef VULKAN_RENDER
#include "VulkanBuffer.h"

namespace v3d
{
namespace renderer
{
namespace vk
{

VulkanUniformBuffer::VulkanUniformBuffer(VulkanBuffer* buffer, u64 offset, u64 size) noexcept
    : _buffer(buffer)
    , _offset(offset)
    , _size(size)
{

}

bool VulkanUniformBuffer::update(u32 offset, u32 size, const void * data)
{
    void* originBuffer = _buffer->map();
    ASSERT(originBuffer, "nullptr");
    u8* buffer = reinterpret_cast<u8*>(originBuffer) + _offset;
    
    bool checkContent = true;
    if (checkContent)
    {
        u32 result = memcmp(buffer, data, _size);
        if (!result)
        {
            return false;
        }
    }

    memcpy(buffer + offset, data, size);
    _buffer->unmap();

    return true;
}

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

VulkanUniformBuffer * VulkanUniformBufferManager::acquireUnformBuffer(u32 requestedSize)
{
    VulkanUniformBufferPool* pool = VulkanUniformBufferManager::getFreePool();
    if (pool->_freeSize < requestedSize)
    {
        //create new pool
    }

    VulkanUniformBuffer * newUnifromBuffer = new VulkanUniformBuffer(pool->_buffer, pool->_usedSize, requestedSize);
    pool->_freeSize -= requestedSize;
    pool->_usedSize += requestedSize;

    return newUnifromBuffer;
}

VulkanUniformBuffer * VulkanUniformBufferManager::findUniformBuffer(const VulkanBuffer * buffer, u32 requestedSize)
{
    return nullptr;
}

VulkanUniformBufferManager::VulkanUniformBufferPool * VulkanUniformBufferManager::getFreePool()
{
    //TODO
    if (!m_currentBuffer)
    {
        m_currentBuffer = new VulkanUniformBufferPool();
        m_currentBuffer->_buffer = new VulkanBuffer(m_memoryManager, m_device, Buffer::BufferType::BufferType_UniformBuffer, 0, k_bufferPoolSize);
        m_currentBuffer->_usedSize = 0;
        m_currentBuffer->_freeSize = k_bufferPoolSize;
    }

    return m_currentBuffer;
}

} //namespace vk
} //namespace renderer
} //namespace v3d
#endif //VULKAN_RENDER
