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
VulkanConstantBuffer::VulkanConstantBuffer(VulkanDevice* device, VulkanMemory::VulkanMemoryAllocator* alloc, u64 size, const std::string& name) noexcept
    : m_buffer(V3D_NEW(VulkanBuffer, memory::MemoryLabel::MemoryRenderCore)(device, alloc, RenderBuffer::Type::ConstantBuffer, 0, size, name))
    , m_offset(0U)
    , m_size(size)
{
}

bool VulkanConstantBuffer::create()
{
    ASSERT(m_buffer, "nullptr");
    return m_buffer->create();
}

void VulkanConstantBuffer::destroy()
{
    ASSERT(m_buffer, "nullptr");
    return m_buffer->destroy();
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


VulkanConstantBufferManager::VulkanConstantBufferManager(VulkanDevice* device) noexcept
    : m_device(*device)
    , m_memoryManager(V3D_NEW(SimpleVulkanMemoryAllocator, memory::MemoryLabel::MemoryRenderCore)(device))

    , m_currentConstantBuffer({ nullptr, 0, 0})
{
}

VulkanConstantBufferManager::~VulkanConstantBufferManager()
{
    ASSERT(m_usedConstantBuffers.empty(), "must be empty");

    if (m_currentConstantBuffer._CB)
    {
        m_currentConstantBuffer._CB->destroy();
        V3D_DELETE(m_currentConstantBuffer._CB, memory::MemoryLabel::MemoryRenderCore);
        m_currentConstantBuffer = { nullptr, 0, 0 };
    }

    while (!m_freeConstantBuffers.empty())
    {
        VulkanConstantBuffer* constantBuffer = m_freeConstantBuffers.front();
        m_freeConstantBuffers.pop_front();

        constantBuffer->destroy();
        V3D_DELETE(constantBuffer, memory::MemoryLabel::MemoryRenderCore);
    }

    if (m_memoryManager)
    {
        V3D_DELETE(m_memoryManager, memory::MemoryLabel::MemoryRenderCore);
        m_memoryManager = nullptr;
    }
}

ConstantBufferRange VulkanConstantBufferManager::acquireUnformBuffer(u32 requestedSize)
{
    ASSERT(requestedSize == math::alignUp<u32>(requestedSize, m_device.getVulkanDeviceCaps().getPhysicalDeviceLimits().minMemoryMapAlignment), "must be alignment");
    u32 desiredSize = std::max(m_device.getVulkanDeviceCaps()._constantBufferSize, requestedSize);

    if (m_currentConstantBuffer._CB) //current
    {
        if (m_currentConstantBuffer._CB->getSize() < requestedSize)
        {
            desiredSize = requestedSize;
        }

        if (requestedSize < m_currentConstantBuffer._freeSize)
        {
            m_currentConstantBuffer._offset = m_currentConstantBuffer._CB->getSize() - m_currentConstantBuffer._freeSize;
            ASSERT(m_currentConstantBuffer._offset < m_currentConstantBuffer._CB->getSize(), ("can't be more than buffer size"));
            m_currentConstantBuffer._freeSize -= requestedSize;
            return m_currentConstantBuffer;
        }
        else
        {
            m_usedConstantBuffers.push_back(m_currentConstantBuffer._CB);
        }
    }

    if (!m_freeConstantBuffers.empty()) //free
    {
        VulkanConstantBuffer* buffer = m_freeConstantBuffers.front();
        if (buffer->getSize() < requestedSize)
        {
            desiredSize = requestedSize;
        }
        else
        {
            m_freeConstantBuffers.pop_front();
            m_currentConstantBuffer._CB = buffer;
            m_currentConstantBuffer._offset = 0;
            m_currentConstantBuffer._freeSize = buffer->getSize() - requestedSize;

            return m_currentConstantBuffer;
        }
    }

    //new
    ASSERT(requestedSize <= desiredSize, ("has small size"));
    VulkanConstantBuffer* buffer = V3D_NEW(VulkanConstantBuffer, memory::MemoryLabel::MemoryRenderCore)(&m_device, m_memoryManager, desiredSize, "ConstantBuffer");
    if (!buffer->create())
    {
        V3D_DELETE(buffer, memory::MemoryLabel::MemoryRenderCore);
        ASSERT(false, ("Can't create CB"));
        return { nullptr, 0, 0 };
    }

    m_currentConstantBuffer._CB = buffer;
    m_currentConstantBuffer._offset = 0;
    m_currentConstantBuffer._freeSize = buffer->getSize() - requestedSize;

    return m_currentConstantBuffer;
}

void VulkanConstantBufferManager::markToUse(VulkanCommandBuffer* cmdBuffer, u64 frame)
{
    if (m_currentConstantBuffer._CB && m_device.getVulkanDeviceCaps()._useDynamicUniforms)
    {
        cmdBuffer->captureResource(m_currentConstantBuffer._CB->getBuffer(), frame);

        m_usedConstantBuffers.push_back(m_currentConstantBuffer._CB);
        m_currentConstantBuffer = { nullptr, 0, 0 };
    }
}

void VulkanConstantBufferManager::updateStatus()
{
    if (m_currentConstantBuffer._CB)
    {
        m_usedConstantBuffers.push_back(m_currentConstantBuffer._CB);
        m_currentConstantBuffer = { nullptr, 0, 0 };
    }

    for (auto iter = m_usedConstantBuffers.begin(); iter != m_usedConstantBuffers.end();)
    {
        VulkanConstantBuffer* CBO = (*iter);
        if (CBO->getBuffer()->isUsed())
        {
            ++iter;
        }
        else
        {
            iter = m_usedConstantBuffers.erase(iter);
            m_freeConstantBuffers.push_back(*iter);
        }
    }
}

} //namespace vk
} //namespace renderer
} //namespace v3d
#endif //VULKAN_RENDER
