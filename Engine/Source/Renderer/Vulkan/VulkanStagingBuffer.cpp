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

VulkanStagingBufferManager::VulkanStagingBufferManager(VulkanDevice* device) noexcept
    : m_device(*device)
    , m_memoryManager(V3D_NEW(SimpleVulkanMemoryAllocator, memory::MemoryLabel::MemoryRenderCore)(device))
{
}

VulkanStagingBufferManager::~VulkanStagingBufferManager()
{
    ASSERT(m_stagingBuffers.empty(), "must be empty");
    if (m_memoryManager)
    {
        V3D_DELETE(m_memoryManager, memory::MemoryLabel::MemoryRenderCore);
        m_memoryManager = nullptr;
    }
}

VulkanBuffer* VulkanStagingBufferManager::createStagingBuffer(u64 size) const
{
    VulkanBuffer* stagingBuffer = V3D_NEW(VulkanBuffer, memory::MemoryLabel::MemoryRenderCore)(&m_device, m_memoryManager, RenderBuffer::Type::StagingBuffer, size, 0, "StagingBuffer");
    ASSERT(stagingBuffer, "nullptr");
    
    if (!stagingBuffer->create())
    {
        ASSERT(false, "fail");
        V3D_DELETE(stagingBuffer, memory::MemoryLabel::MemoryRenderCore);
        return nullptr;
    }

    return stagingBuffer;
}

void VulkanStagingBufferManager::destroyAfterUse(VulkanBuffer* buffer)
{
    std::lock_guard lock(m_mutex);

    m_stagingBuffers.push_back(buffer);
}

void VulkanStagingBufferManager::destroyStagingBuffers()
{
    std::lock_guard lock(m_mutex);

    for (auto iter = m_stagingBuffers.begin(); iter != m_stagingBuffers.end();)
    {
        if ((*iter)->isUsed())
        {
            ++iter;
            continue;
        }

        VulkanBuffer* buff = *iter;
        iter = m_stagingBuffers.erase(iter);

        buff->destroy();
        V3D_DELETE(buff, memory::MemoryLabel::MemoryRenderCore);
    }
}

} //namespace vk
} //namespace renderer
} //namespace v3d
#endif //VULKAN_RENDER
