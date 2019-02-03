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
    : m_buffer(buffer)
    , m_offset(offset)
    , m_size(size)
{
}

VulkanUniformBuffer::~VulkanUniformBuffer()
{
}

VulkanBuffer * VulkanUniformBuffer::getBuffer() const
{
    return m_buffer;
}

u64 VulkanUniformBuffer::getOffset() const
{
    return m_offset;
}

u64 VulkanUniformBuffer::getSize() const
{
    return m_size;
}

bool VulkanUniformBuffer::update(u32 offset, u32 size, const void * data)
{
    ASSERT(data, "nullptr");
    void* originBuffer = m_buffer->map();
    ASSERT(originBuffer, "nullptr");
    u8* buffer = reinterpret_cast<u8*>(originBuffer) + m_offset;
    
    bool checkContent = true;
    if (checkContent)
    {
        u32 result = memcmp(buffer, data, size);
        if (!result)
        {
            return false;
        }
    }

    memcpy(buffer + offset, data, size);
    m_buffer->unmap();

    return true;
}

VulkanUniformBufferManager::VulkanUniformBufferManager(VkDevice device)
    : m_device(device)
    , m_memoryManager(new SimpleVulkanMemoryAllocator(device))

    , m_currentPoolBuffer(nullptr)
{
}

VulkanUniformBufferManager::~VulkanUniformBufferManager()
{
    ASSERT(m_usedPoolBuffers.empty(), "still not empty");

    if (m_memoryManager)
    {
        delete m_memoryManager;
        m_memoryManager = nullptr;
    }

    while (!m_freePoolBuffers.empty())
    {
        VulkanUniformBufferPool* poolUnifromBuffers = m_freePoolBuffers.front();
        m_freePoolBuffers.pop_front();

        for (auto& uniform : poolUnifromBuffers->_uniformList)
        {
            delete uniform;
        }
        poolUnifromBuffers->_uniformList.clear();

        poolUnifromBuffers->_buffer->destroy();
        delete poolUnifromBuffers->_buffer;

        delete poolUnifromBuffers;
    }
}

VulkanUniformBuffer * VulkanUniformBufferManager::acquireUnformBuffer(u32 requestedSize)
{
    if (!m_currentPoolBuffer)
    {
        m_currentPoolBuffer = VulkanUniformBufferManager::getNewPool();
    }
    else
    {
        m_usedPoolBuffers.push_back(m_currentPoolBuffer);
        if (m_currentPoolBuffer->_freeSize < requestedSize)
        {
            if (!m_freePoolBuffers.empty())
            {
                m_currentPoolBuffer = m_freePoolBuffers.front();
                m_freePoolBuffers.pop_front();
            }
            else
            {
                m_currentPoolBuffer = VulkanUniformBufferManager::getNewPool();
            }
        }
    }

    VulkanUniformBuffer * newUnifromBuffer = new VulkanUniformBuffer(m_currentPoolBuffer->_buffer, m_currentPoolBuffer->_usedSize, requestedSize);
    m_currentPoolBuffer->addUniformBuffer(newUnifromBuffer, requestedSize);

    return newUnifromBuffer;
}

VulkanUniformBuffer * VulkanUniformBufferManager::findUniformBuffer(const VulkanBuffer * buffer, u32 requestedSize)
{


    return nullptr;
}

void VulkanUniformBufferManager::updateUniformBuffers()
{
    for (auto iter = m_usedPoolBuffers.begin(); iter != m_usedPoolBuffers.end();)
    {
        VulkanUniformBufferPool* pool = (*iter);
        if (!pool->_buffer->isCaptured())
        {
            m_freePoolBuffers.push_back(pool);
            m_usedPoolBuffers.erase(iter);
        }
    }
}

VulkanUniformBufferManager::VulkanUniformBufferPool * VulkanUniformBufferManager::getNewPool()
{
    VulkanUniformBufferPool* newPool = new VulkanUniformBufferPool();
    newPool->_buffer = new VulkanBuffer(m_memoryManager, m_device, Buffer::BufferType::BufferType_UniformBuffer, 0, k_bufferPoolSize);
    newPool->_usedSize = 0;
    newPool->_freeSize = k_bufferPoolSize;

    if (!newPool->_buffer->create())
    {
        delete newPool;

        ASSERT(false, "buffer doesn't create");
        return nullptr;
    }
    return newPool;
}

void VulkanUniformBufferManager::VulkanUniformBufferPool::addUniformBuffer(VulkanUniformBuffer * uniformBuffer, u64 size)
{
    _freeSize -= size;
    _usedSize += size;
    _uniformList.push_back(uniformBuffer);
}

} //namespace vk
} //namespace renderer
} //namespace v3d
#endif //VULKAN_RENDER
