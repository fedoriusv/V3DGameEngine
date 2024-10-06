#include "VulkanStagingBuffer.h"

#ifdef VULKAN_RENDER
#   include "VulkanMemory.h"
#   include "VulkanDevice.h"

namespace v3d
{
namespace renderer
{
namespace vk
{

VulkanStagingBuffer::VulkanStagingBuffer(VulkanDevice* device, VulkanMemory::VulkanMemoryAllocator* memory, u64 size, BufferUsageFlags usageFlag) noexcept
    : m_buffer(new VulkanBuffer(device, memory, RenderBuffer::Type::StagingBuffer, usageFlag, size))
{
}

VulkanStagingBuffer::~VulkanStagingBuffer()
{
    if (m_buffer)
    {
        m_buffer->destroy();

        delete m_buffer;
        m_buffer = nullptr;
    }
}

VulkanStagingBufferManager::VulkanStagingBufferManager(VulkanDevice* device) noexcept
    : m_device(*device)
    , m_memoryManager(new SimpleVulkanMemoryAllocator(device->getDeviceInfo()._device))
{
}

VulkanStagingBufferManager::~VulkanStagingBufferManager()
{
    if (m_memoryManager)
    {
        delete m_memoryManager;
        m_memoryManager = nullptr;
    }
}

VulkanStagingBuffer* VulkanStagingBufferManager::createStagingBuffer(u64 size, BufferUsageFlags usageFlag) const
{
    VulkanStagingBuffer* stagingBuffer = new VulkanStagingBuffer(&m_device, m_memoryManager, size, usageFlag);
    ASSERT(stagingBuffer, "nullptr");
    
    if (!stagingBuffer->create())
    {
        ASSERT(false, "fail");
        return nullptr;
    }

    return stagingBuffer;
}

void VulkanStagingBufferManager::destroyAfterUse(VulkanStagingBuffer* buffer)
{
    m_stagingBuffers.push_back(buffer);
}

void VulkanStagingBufferManager::destroyStagingBuffers()
{
    for (auto iter = m_stagingBuffers.begin(); iter != m_stagingBuffers.end();)
    {
        if ((*iter)->getBuffer()->isCaptured())
        {
            //ASSERT(false, "captured");
            //buff->getBuffer()->waitComplete();

            ++iter;
            continue;
        }

        VulkanStagingBuffer* buff = *iter;
        iter = m_stagingBuffers.erase(iter);

        buff->destroy();
        delete buff;
    }
}

} //namespace vk
} //namespace renderer
} //namespace v3d
#endif //VULKAN_RENDER
