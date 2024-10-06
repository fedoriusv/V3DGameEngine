#include "VulkanConstantBuffer.h"

#ifdef VULKAN_RENDER
#   include "VulkanBuffer.h"
#   include "VulkanDeviceCaps.h"
#   include "VulkanDevice.h"

namespace v3d
{
namespace renderer
{
namespace vk
{
VulkanConstantBuffer::VulkanConstantBuffer() noexcept
    : m_buffer(nullptr)
    , m_offset(0U)
    , m_size(0U)
{
}

VulkanConstantBuffer::VulkanConstantBuffer(VulkanBuffer* buffer, u64 offset, u64 size) noexcept
    : m_buffer(buffer)
    , m_offset(offset)
    , m_size(size)
{
}

void VulkanConstantBuffer::set(VulkanBuffer* buffer, u64 offset, u64 size)
{
    m_buffer = buffer;
    m_offset = offset;
    m_size = size;
}

bool VulkanConstantBuffer::update(u32 offset, u32 size, const void* data)
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


VulkanConstantBufferManager::VulkanConstantBufferManager(VulkanDevice* device, VulkanResourceDeleter& resourceDeleter) noexcept
    : m_device(*device)
    , m_currentPoolBuffer(nullptr)

    , m_memoryManager(new SimpleVulkanMemoryAllocator(device->getDeviceInfo()._device))
    , m_resourceDeleter(resourceDeleter)
    , k_bufferPoolSize(m_device.getVulkanDeviceCaps()._useDynamicUniforms ?
        std::min<u32>(m_device.getVulkanDeviceCaps().getPhysicalDeviceLimits().maxUniformBufferRange, m_device.getVulkanDeviceCaps().getPhysicalDeviceMaintenance3Properties().maxMemoryAllocationSize) : 1 * 1024 * 1024)
{
}

VulkanConstantBufferManager::~VulkanConstantBufferManager()
{
    ASSERT(m_usedPoolBuffers.empty(), "still not empty");

    if (m_currentPoolBuffer)
    {
        bool result = VulkanConstantBufferManager::freeUniformBufferPool(m_currentPoolBuffer, true);
        ASSERT(result, "fail to delete uniform current pool. Still used");
        delete m_currentPoolBuffer;
        m_currentPoolBuffer = nullptr;
    }

    while (!m_freePoolBuffers.empty())
    {
        VulkanConstantBufferPool* poolUnifromBuffers = m_freePoolBuffers.front();
        m_freePoolBuffers.pop_front();

        VulkanConstantBufferManager::freeUniformBufferPool(poolUnifromBuffers, false);
        delete poolUnifromBuffers;
    }

    if (m_memoryManager)
    {
        delete m_memoryManager;
        m_memoryManager = nullptr;
    }
}

VulkanConstantBuffer* VulkanConstantBufferManager::acquireUnformBuffer(u32 requestedSize)
{
    u32 alingment = static_cast<u32>(m_device.getVulkanDeviceCaps().getPhysicalDeviceLimits().minUniformBufferOffsetAlignment);
    u32 requirementSize = math::alignUp(std::max<u32>(m_device.getVulkanDeviceCaps().getPhysicalDeviceLimits().minMemoryMapAlignment, requestedSize), alingment);
    u32 poolCount = k_bufferPoolSize / alingment;

    if (!m_currentPoolBuffer)
    {
        if (m_freePoolBuffers.empty())
        {
            ASSERT(requirementSize <= k_bufferPoolSize, "pool size less than reqested");
            m_currentPoolBuffer = VulkanConstantBufferManager::getNewPool(k_bufferPoolSize, poolCount);
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
            m_currentPoolBuffer = VulkanConstantBufferManager::getNewPool(k_bufferPoolSize, poolCount);
        }
    }

    VulkanConstantBuffer* newUnifromBuffer = m_currentPoolBuffer->prepareUniformBuffer(m_currentPoolBuffer->_buffer, m_currentPoolBuffer->_usedSize, requirementSize);
    return newUnifromBuffer;
}

void VulkanConstantBufferManager::markToUse(VulkanCommandBuffer* cmdBuffer, u64 frame)
{
    if (m_currentPoolBuffer && m_device.getVulkanDeviceCaps()._useDynamicUniforms)
    {
        m_currentPoolBuffer->_buffer->captureInsideCommandBuffer(cmdBuffer, frame);

        m_usedPoolBuffers.push_back(m_currentPoolBuffer);
        m_currentPoolBuffer = nullptr;
    }
}

void VulkanConstantBufferManager::updateUniformBuffers()
{
    for (auto iter = m_usedPoolBuffers.begin(); iter != m_usedPoolBuffers.end();)
    {
        VulkanConstantBufferPool* pool = (*iter);
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

bool VulkanConstantBufferManager::freeUniformBufferPool(VulkanConstantBufferPool* uniformPool, bool waitComplete)
{
    ASSERT(uniformPool, "nullptr");
    uniformPool->resetPool();

    VulkanBuffer* buffer = uniformPool->_buffer;
    ASSERT(buffer, "nullptr");

    if (buffer->isCaptured())
    {
        /*m_resourceDeleter.addResourceToDelete(buffer, [buffer](VulkanResource * resource) -> void
            {
                buffer->notifyObservers();

                buffer->destroy();
                delete buffer;

            }, waitComplete);*/

        return false;
    }
    else
    {
        //buffer->notifyObservers();

        buffer->destroy();
        delete buffer;

        return true;
    }

    return true;
}

VulkanConstantBufferManager::VulkanConstantBufferPool* VulkanConstantBufferManager::getNewPool(u64 size, u32 count)
{
    VulkanConstantBufferPool* newPool = new VulkanConstantBufferPool();
    //newPool->_buffer = new VulkanBuffer(m_memoryManager, m_device, Buffer::BufferType::ConstantBuffer, 0, size);
    newPool->_usedSize = 0;
    newPool->_freeSize = size;
    newPool->_poolSize = size;
    newPool->_uniforms.resize(count, VulkanConstantBuffer());

    if (!newPool->_buffer->create())
    {
        delete newPool;

        ASSERT(false, "buffer doesn't create");
        return nullptr;
    }
    return newPool;
}

void VulkanConstantBufferManager::VulkanConstantBufferPool::resetPool()
{
    _uniformIndex = 0;
#if VULKAN_DEBUG
    memset(_uniforms.data(), 0, _uniforms.size());
#endif

    _usedSize = 0;
    _freeSize = _poolSize;
}

VulkanConstantBuffer* VulkanConstantBufferManager::VulkanConstantBufferPool::prepareUniformBuffer(VulkanBuffer* buffer, u32 offset, u32 size)
{
    _freeSize -= size;
    _usedSize += size;

    //ASSERT(_uniformIndex < _uniforms.size(), "out of range");
    //VulkanUniformBuffer& uniformBuffer = _uniforms[_uniformIndex];
    //uniformBuffer.set(buffer, offset, size);
    //++_uniformIndex;

    //return &uniformBuffer;

    return nullptr;
}

} //namespace vk
} //namespace renderer
} //namespace v3d
#endif //VULKAN_RENDER
