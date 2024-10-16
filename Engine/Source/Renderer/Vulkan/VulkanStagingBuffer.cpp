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
    : m_buffer(V3D_NEW(VulkanBuffer, memory::MemoryLabel::MemoryRenderCore)(device, memory, RenderBuffer::Type::StagingBuffer, usageFlag, size))
{
}

VulkanStagingBuffer::~VulkanStagingBuffer()
{
    if (m_buffer)
    {
        m_buffer->destroy();

        V3D_DELETE(m_buffer, memory::MemoryLabel::MemoryRenderCore);
        m_buffer = nullptr;
    }
}

VulkanStagingBufferManager::VulkanStagingBufferManager(VulkanDevice* device) noexcept
    : m_device(*device)
    , m_memoryManager(V3D_NEW(SimpleVulkanMemoryAllocator, memory::MemoryLabel::MemoryRenderCore)(device))
{
}

VulkanStagingBufferManager::~VulkanStagingBufferManager()
{
    if (m_memoryManager)
    {
        V3D_DELETE(m_memoryManager, memory::MemoryLabel::MemoryRenderCore);
        m_memoryManager = nullptr;
    }
}

VulkanStagingBuffer* VulkanStagingBufferManager::createStagingBuffer(u64 size, BufferUsageFlags usageFlag) const
{
    VulkanStagingBuffer* stagingBuffer = V3D_NEW(VulkanStagingBuffer, memory::MemoryLabel::MemoryRenderCore)(&m_device, m_memoryManager, size, usageFlag);
    ASSERT(stagingBuffer, "nullptr");
    
    if (!stagingBuffer->create())
    {
        ASSERT(false, "fail");
        V3D_DELETE(stagingBuffer, memory::MemoryLabel::MemoryRenderCore);
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
        V3D_DELETE(buff, memory::MemoryLabel::MemoryRenderCore);
    }
}

} //namespace vk
} //namespace renderer
} //namespace v3d
#endif //VULKAN_RENDER
