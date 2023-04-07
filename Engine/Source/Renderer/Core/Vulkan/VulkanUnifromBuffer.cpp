#include "VulkanUnifromBuffer.h"

#ifdef VULKAN_RENDER
#include "VulkanBuffer.h"
#include "VulkanDeviceCaps.h"
#include "VulkanContext.h"

namespace v3d
{
namespace renderer
{
namespace vk
{
VulkanUniformBuffer::VulkanUniformBuffer() noexcept
    : m_buffer(nullptr)
    , m_offset(0U)
    , m_size(0U)
{
}

VulkanUniformBuffer::VulkanUniformBuffer(VulkanBuffer* buffer, u64 offset, u64 size) noexcept
    : m_buffer(buffer)
    , m_offset(offset)
    , m_size(size)
{
}

void VulkanUniformBuffer::set(VulkanBuffer* buffer, u64 offset, u64 size)
{
    m_buffer = buffer;
    m_offset = offset;
    m_size = size;
}

bool VulkanUniformBuffer::update(u32 offset, u32 size, const void* data)
{
    ASSERT(data, "nullptr");
    void* originBuffer = m_buffer->map();
    ASSERT(originBuffer, "nullptr");
    u8* buffer = reinterpret_cast<u8*>(originBuffer) + m_offset;
    
#if VULKAN_DEBUG
    bool checkContent = false;
    if (checkContent)
    {
        u32 result = memcmp(buffer, data, size);
        if (!result)
        {
            m_buffer->unmap();
            return false;
        }
    }
#endif //VULKAN_DEBUG

    memcpy(buffer + offset, data, size);
    m_buffer->unmap();

    return true;
}


VulkanUniformBufferManager::VulkanUniformBufferManager(VkDevice device, VulkanResourceDeleter& resourceDeleter) noexcept
    : m_device(device)
    , m_currentPoolBuffer(nullptr)

    , m_memoryManager(new SimpleVulkanMemoryAllocator(device))
    , m_resourceDeleter(resourceDeleter)
    , k_bufferPoolSize(VulkanDeviceCaps::getInstance()->useDynamicUniforms ? 
        std::min<u32>(VulkanDeviceCaps::getInstance()->getPhysicalDeviceLimits().maxUniformBufferRange, VulkanDeviceCaps::getInstance()->getPhysicalDeviceMaintenance3Properties().maxMemoryAllocationSize) : 1 * 1024 * 1024)
{
}

VulkanUniformBufferManager::~VulkanUniformBufferManager()
{
    ASSERT(m_usedPoolBuffers.empty(), "still not empty");

    if (m_currentPoolBuffer)
    {
        bool result = VulkanUniformBufferManager::freeUniformBufferPool(m_currentPoolBuffer, true);
        ASSERT(result, "fail to delete uniform current pool. Still used");
        delete m_currentPoolBuffer;
        m_currentPoolBuffer = nullptr;
    }

    while (!m_freePoolBuffers.empty())
    {
        VulkanUniformBufferPool* poolUnifromBuffers = m_freePoolBuffers.front();
        m_freePoolBuffers.pop_front();

        VulkanUniformBufferManager::freeUniformBufferPool(poolUnifromBuffers, false);
        delete poolUnifromBuffers;
    }

    if (m_memoryManager)
    {
        delete m_memoryManager;
        m_memoryManager = nullptr;
    }
}

VulkanUniformBuffer* VulkanUniformBufferManager::acquireUnformBuffer(u32 requestedSize)
{
    u32 alingment = static_cast<u32>(VulkanDeviceCaps::getInstance()->getPhysicalDeviceLimits().minUniformBufferOffsetAlignment);
    u32 requirementSize = math::alignUp(std::max<u32>(VulkanDeviceCaps::getInstance()->getPhysicalDeviceLimits().minMemoryMapAlignment, requestedSize), alingment);
    u32 poolCount = k_bufferPoolSize / alingment;

    if (!m_currentPoolBuffer)
    {
        if (m_freePoolBuffers.empty())
        {
            ASSERT(requirementSize <= k_bufferPoolSize, "pool size less than reqested");
            m_currentPoolBuffer = VulkanUniformBufferManager::getNewPool(k_bufferPoolSize, poolCount);
        }
        else
        {
            m_currentPoolBuffer = m_freePoolBuffers.front();
            m_freePoolBuffers.pop_front();
        }
    }

    if (m_currentPoolBuffer->_freeSize < requirementSize)
    {
        m_usedPoolBuffers.push_back(m_currentPoolBuffer);
        if (!m_freePoolBuffers.empty())
        {
            m_currentPoolBuffer = m_freePoolBuffers.front();
            m_freePoolBuffers.pop_front();
        }
        else
        {
            ASSERT(requirementSize <= k_bufferPoolSize, "pool size less than reqested");
            m_currentPoolBuffer = VulkanUniformBufferManager::getNewPool(k_bufferPoolSize, poolCount);
        }
    }

    VulkanUniformBuffer* newUnifromBuffer = m_currentPoolBuffer->prepareUniformBuffer(m_currentPoolBuffer->_buffer, m_currentPoolBuffer->_usedSize, requirementSize);
    return newUnifromBuffer;
}

void VulkanUniformBufferManager::markToUse(VulkanCommandBuffer* cmdBuffer, u64 frame)
{
    if (m_currentPoolBuffer && VulkanDeviceCaps::getInstance()->useDynamicUniforms)
    {
        m_currentPoolBuffer->_buffer->captureInsideCommandBuffer(cmdBuffer, frame);

        m_usedPoolBuffers.push_back(m_currentPoolBuffer);
        m_currentPoolBuffer = nullptr;
    }
}

void VulkanUniformBufferManager::updateUniformBuffers()
{
    for (auto iter = m_usedPoolBuffers.begin(); iter != m_usedPoolBuffers.end();)
    {
        VulkanUniformBufferPool* pool = (*iter);
        if (!pool->_buffer->isCaptured())
        {
            m_freePoolBuffers.push_back(pool);
            iter = m_usedPoolBuffers.erase(iter);

            pool->resetPool();
        }
        else
        {
            ++iter;
        }
    }
}

bool VulkanUniformBufferManager::freeUniformBufferPool(VulkanUniformBufferPool* uniformPool, bool waitComplete)
{
    ASSERT(uniformPool, "nullptr");
    uniformPool->resetPool();

    VulkanBuffer* buffer = uniformPool->_buffer;
    ASSERT(buffer, "nullptr");

    if (buffer->isCaptured())
    {
        m_resourceDeleter.addResourceToDelete(buffer, [buffer](VulkanResource * resource) -> void
            {
                buffer->notifyObservers();

                buffer->destroy();
                delete buffer;

            }, waitComplete);

        return false;
    }
    else
    {
        buffer->notifyObservers();

        buffer->destroy();
        delete buffer;

        return true;
    }

    return true;
}

VulkanUniformBufferManager::VulkanUniformBufferPool * VulkanUniformBufferManager::getNewPool(u64 size, u32 count)
{
    VulkanUniformBufferPool* newPool = new VulkanUniformBufferPool();
    newPool->_buffer = new VulkanBuffer(m_memoryManager, m_device, Buffer::BufferType::ConstantBuffer, 0, size);
    newPool->_usedSize = 0;
    newPool->_freeSize = size;
    newPool->_poolSize = size;
    newPool->_uniforms.resize(count, VulkanUniformBuffer());

    if (!newPool->_buffer->create())
    {
        delete newPool;

        ASSERT(false, "buffer doesn't create");
        return nullptr;
    }
    return newPool;
}

void VulkanUniformBufferManager::VulkanUniformBufferPool::resetPool()
{
    _uniformIndex = 0;
#if VULKAN_DEBUG
    memset(_uniforms.data(), 0, _uniforms.size());
#endif

    _usedSize = 0;
    _freeSize = _poolSize;
}

VulkanUniformBuffer* VulkanUniformBufferManager::VulkanUniformBufferPool::prepareUniformBuffer(VulkanBuffer* buffer, u32 offset, u32 size)
{
    _freeSize -= size;
    _usedSize += size;

    ASSERT(_uniformIndex < _uniforms.size(), "out of range");
    VulkanUniformBuffer& uniformBuffer = _uniforms[_uniformIndex];
    uniformBuffer.set(buffer, offset, size);
    ++_uniformIndex;

    return &uniformBuffer;
}

} //namespace vk
} //namespace renderer
} //namespace v3d
#endif //VULKAN_RENDER
