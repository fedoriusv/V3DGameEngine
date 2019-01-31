#include "VulkanBuffer.h"

#include "Utils/Logger.h"
#include "Renderer/Object/StreamBuffer.h"

#ifdef VULKAN_RENDER
#include "VulkanDebug.h"
#include "VulkanDeviceCaps.h"
#include "VulkanGraphicContext.h"
#include "VulkanStagingBuffer.h"

namespace v3d
{
namespace renderer
{
namespace vk
{

VulkanBuffer::VulkanBuffer(VulkanMemory::VulkanMemoryAllocator* memory, VkDevice device, Buffer::BufferType type, StreamBufferUsageFlags usageFlag, u64 size)
    : m_device(device)

    , m_memory(VulkanMemory::s_invalidMemory)
    , m_memoryAllocator(memory)

    , m_usageFlags(usageFlag)
    , m_type(type)

    , m_size(size) //Check aligment

    , m_buffer(VK_NULL_HANDLE)
{
    LOG_DEBUG("VulkanBuffer::VulkanBuffer constructor %llx", this);
}

VulkanBuffer::~VulkanBuffer()
{
    LOG_DEBUG("VulkanBuffer::VulkanBuffer destructor %llx", this);

    ASSERT(!m_buffer, "m_buffer not nullptr");
}

bool VulkanBuffer::create()
{
    ASSERT(!m_buffer, "m_buffer already created");

    VkBufferUsageFlags usage = 0;
    VkMemoryPropertyFlags flag = 0;
    if (m_type == Buffer::BufferType::BufferType_VertexBuffer)
    {
        usage |= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
        flag |= VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

        if (!VulkanDeviceCaps::getInstance()->useStagingBuffers)
        {
            flag |= VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
            flag |= VulkanDeviceCaps::getInstance()->supportCoherentMemory ? VK_MEMORY_PROPERTY_HOST_COHERENT_BIT : VK_MEMORY_PROPERTY_HOST_CACHED_BIT;
        }
    }
    else if (m_type == Buffer::BufferType::BufferType_IndexBuffer)
    {
        usage |= VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
        flag |= VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

        if (!VulkanDeviceCaps::getInstance()->useStagingBuffers)
        {
            flag |= VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
            flag |= VulkanDeviceCaps::getInstance()->supportCoherentMemory ? VK_MEMORY_PROPERTY_HOST_COHERENT_BIT : VK_MEMORY_PROPERTY_HOST_CACHED_BIT;
        }
    }
    else if (m_type == Buffer::BufferType::BufferType_UniformBuffer)
    {
        usage |= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
        flag |= VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
        flag |= VulkanDeviceCaps::getInstance()->supportCoherentMemory ? VK_MEMORY_PROPERTY_HOST_COHERENT_BIT : VK_MEMORY_PROPERTY_HOST_CACHED_BIT;
    }
    else if (m_type == Buffer::BufferType::BufferType_StagingBuffer)
    {
        flag |= VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    }

    if (m_usageFlags & StreamBufferUsage::StreamBuffer_Read)
    {
        usage |= VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    }

    if (m_usageFlags & StreamBufferUsage::StreamBuffer_Write)
    {
        usage |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    }
    
    VkBufferCreateInfo bufferCreateInfo = {};
    bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferCreateInfo.pNext = nullptr;
    bufferCreateInfo.flags = 0; //sparse
    bufferCreateInfo.usage = usage;
    bufferCreateInfo.size = m_size;
    bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    bufferCreateInfo.queueFamilyIndexCount = 1;
    bufferCreateInfo.pQueueFamilyIndices = nullptr;

    VkResult result = VulkanWrapper::CreateBuffer(m_device, &bufferCreateInfo, VULKAN_ALLOCATOR, &m_buffer);
    if (result != VK_SUCCESS)
    {
        LOG_ERROR("VulkanBuffer::create vkCreateBuffer is failed. Error: %s", ErrorString(result).c_str());
        return false;
    }

    m_memory = VulkanMemory::allocateBufferMemory(*m_memoryAllocator, m_buffer, flag);
    if (m_memory == VulkanMemory::s_invalidMemory)
    {
        VulkanBuffer::destroy();

        LOG_ERROR("VulkanBuffer::VulkanBuffer::create() is failed");
        return false;
    }

    /*if (!createViewBuffer())
    {
        VulkanBuffer::destroy();

        LOG_ERROR("VulkanBuffer::VulkanBuffer::create() is failed");
        return false;
    }*/

    return true;
}

void VulkanBuffer::destroy()
{
    ASSERT(!m_mapped, "mapped");

    /*if (m_imageView)
    {
        VulkanWrapper::DestroyImageView(m_device, m_imageView, VULKAN_ALLOCATOR);
        m_imageView = VK_NULL_HANDLE;
    }*/

    VulkanMemory::freeMemory(*m_memoryAllocator, m_memory);
    if (m_buffer)
    {
        VulkanWrapper::DestroyBuffer(m_device, m_buffer, VULKAN_ALLOCATOR);
        m_buffer = VK_NULL_HANDLE;
    }
}

bool VulkanBuffer::upload(Context* context, u32 offset, u64 size, void * data)
{
    if (!m_buffer)
    {
        ASSERT(false, "nullptr");
        return false;
    }

    if (size == 0 || !data)
    {
        return false;
    }

    if (m_size != size && offset == 0)
    {
        if (m_usageFlags & ~StreamBufferUsage::StreamBuffer_Dynamic)
        {
            ASSERT(false, "different size in non dynamic data");
            return false;
        }

        m_size = size;
        if (!VulkanBuffer::recreate())
        {
            VulkanBuffer::destroy();
            return false;
        }
    }

    if (VulkanDeviceCaps::getInstance()->useStagingBuffers)
    {
        VulkanGraphicContext* vkContext = static_cast<VulkanGraphicContext*>(context);
        VulkanCommandBuffer* updateBuffer = vkContext->getOrCreateAndStartCommandBuffer(CommandTargetType::CmdUploadBuffer);
        if (m_size <= 65536)
        {
            ASSERT(!VulkanResource::isCaptured(), "still submitted");
            //TODO memory barrier
            updateBuffer->cmdUpdateBuffer(this, offset, m_size, data);
            //TODO memory barrier
        }
        else
        {
            VulkanStaginBuffer* staginBuffer = vkContext->getStagingManager()->createStagingBuffer(size, StreamBufferUsage::StreamBuffer_Read);
            if (!staginBuffer)
            {
                ASSERT(false, "staginBuffer is nullptr");
                return false;
            }
            void* stagingData = staginBuffer->map();
            ASSERT(stagingData, "stagingData is nullptr");
            memcpy(stagingData, data, size);
            staginBuffer->unmap();

            VkBufferCopy bufferCopy = {};
            bufferCopy.srcOffset = 0;
            bufferCopy.dstOffset = offset;
            bufferCopy.size = size;

            ASSERT(!VulkanResource::isCaptured(), "still submitted");
            //TODO memory barrier
            updateBuffer->cmdCopyBufferToBuffer(this, staginBuffer->getBuffer(), bufferCopy);
            //TODO memory barrier

            vkContext->getStagingManager()->destroyAfterUse(staginBuffer);
        }
    }
    else
    {
        if (VulkanResource::isCaptured())
        {
            ASSERT(false, "still submitted");
            VulkanResource::waitComplete();
        }

        void* srcData = VulkanBuffer::map();
        memcpy(srcData, data, size);
        VulkanBuffer::unmap();

        return true;
    }

    return false;
}

VkBuffer VulkanBuffer::getHandle() const
{
    return m_buffer;
}

void * VulkanBuffer::map()
{
    if (!m_buffer)
    {
        ASSERT(false, "nullptr");
        return nullptr;
    }

    if (m_mapped)
    {
        ASSERT(false, "already mappped");
    }

    ASSERT(m_memory._mapped, "m_memory._mapped can't map");
    if (m_memory._flag & VK_MEMORY_PROPERTY_HOST_CACHED_BIT)
    {
        VkMappedMemoryRange mappedMemoryRange = {};
        mappedMemoryRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
        mappedMemoryRange.pNext = nullptr;
        mappedMemoryRange.memory = m_memory._memory;
        mappedMemoryRange.size = m_memory._size;
        mappedMemoryRange.offset = m_memory._offset;

        VkResult result = VulkanWrapper::InvalidateMappedMemoryRanges(m_device, 1, &mappedMemoryRange);
        if (result != VK_SUCCESS)
        {
            ASSERT(false, "error");
            return nullptr;
        }
    }
    m_mapped = true;


    return m_memory._mapped;
}

void VulkanBuffer::unmap()
{
    if (!m_buffer)
    {
        ASSERT(false, "nullptr");
        return;
    }

    if (!m_mapped)
    {
        return;
    }

    ASSERT(m_memory._mapped, "m_memory._mapped can't map");
    if (m_memory._flag & VK_MEMORY_PROPERTY_HOST_CACHED_BIT)
    {
        VkMappedMemoryRange mappedMemoryRange = {};
        mappedMemoryRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
        mappedMemoryRange.pNext = nullptr;
        mappedMemoryRange.memory = m_memory._memory;
        mappedMemoryRange.size = m_memory._size;
        mappedMemoryRange.offset = m_memory._offset;

        VkResult result = VulkanWrapper::FlushMappedMemoryRanges(m_device, 1, &mappedMemoryRange);
        if (result != VK_SUCCESS)
        {
            ASSERT(false, "error");
        }
    }
    m_mapped = false;
}

bool VulkanBuffer::recreate()
{
    if (!m_buffer)
    {
        ASSERT(false, "nullptr");
        return false;
    }

    ASSERT(false, "not implementing");
    return false;
}

} //namespace vk
} //namespace renderer
} //namespace v3d
#endif //VULKAN_RENDER
