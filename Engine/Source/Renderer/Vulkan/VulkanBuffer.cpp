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

#if DEBUG_OBJECT_MEMORY
std::set<VulkanBuffer*> VulkanBuffer::s_objects;
#endif //DEBUG_OBJECT_MEMORY

VulkanBuffer::VulkanBuffer(VulkanMemory::VulkanMemoryAllocator* memory, VkDevice device, Buffer::BufferType type, StreamBufferUsageFlags usageFlag, u64 size, const std::string& name) noexcept
    : m_device(device)

    , m_memory(VulkanMemory::s_invalidMemory)
    , m_memoryAllocator(memory)

    , m_usageFlags(usageFlag)
    , m_type(type)

    , m_size(size) //Check aligment
    , m_buffer(VK_NULL_HANDLE)
    
    , m_mapped(false)
{
    LOG_DEBUG("VulkanBuffer::VulkanBuffer constructor %llx", this);
#if VULKAN_DEBUG_MARKERS
    m_debugName = name.empty() ? std::to_string(reinterpret_cast<const u64>(this)) : name;
#endif //VULKAN_DEBUG_MARKERS
#if DEBUG_OBJECT_MEMORY
    s_objects.insert(this);
#endif //DEBUG_OBJECT_MEMORY
}

VulkanBuffer::~VulkanBuffer()
{
    LOG_DEBUG("VulkanBuffer::VulkanBuffer destructor %llx", this);
#if DEBUG_OBJECT_MEMORY
    s_objects.erase(this);
#endif //DEBUG_OBJECT_MEMORY
    ASSERT(!m_buffer, "m_buffer not nullptr");
}

bool VulkanBuffer::create()
{
    ASSERT(!m_buffer, "m_buffer already created");
    ASSERT(m_size, "m_size");

    VkBufferUsageFlags usageBuffer = 0;
    VkMemoryPropertyFlags memoryFlags = 0;

    switch (m_type)
    {
    case Buffer::BufferType::BufferType_VertexBuffer:
    {
        usageBuffer |= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;

        if (VulkanBuffer::isPresentingBufferUsageFlag(StreamBuffer_Dynamic))
        {
            memoryFlags |= VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
            if (VulkanBuffer::isPresentingBufferUsageFlag(StreamBufferUsage::StreamBuffer_Read))
            {
                memoryFlags |= VK_MEMORY_PROPERTY_HOST_CACHED_BIT;
                ASSERT(VulkanDeviceCaps::getInstance()->supportHostCacheMemory, "unsupport coherent memory");

            }
            else if (VulkanBuffer::isPresentingBufferUsageFlag(StreamBufferUsage::StreamBuffer_Write))
            {
                memoryFlags |= VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
                ASSERT(VulkanDeviceCaps::getInstance()->supportHostCoherentMemory, "unsupport coherent memory");
            }
            else
            {
                ASSERT(false, "fail");
            }
        }
        else
        {
            memoryFlags |= VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
            if (VulkanDeviceCaps::getInstance()->supportDeviceCoherentMemory)
            {
                memoryFlags |= VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
            }
            else if (VulkanDeviceCaps::getInstance()->supportDeviceCacheMemory)
            {
                memoryFlags |= VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_CACHED_BIT;
            }
        }

        break;
    }

    case Buffer::BufferType::BufferType_IndexBuffer:
    {
        ASSERT(m_usageFlags & ~StreamBuffer_Dynamic, "not support");
        usageBuffer |= VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
        memoryFlags |= VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
        if (VulkanDeviceCaps::getInstance()->supportDeviceCoherentMemory)
        {
            memoryFlags |= VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
        }
        else if (VulkanDeviceCaps::getInstance()->supportDeviceCacheMemory)
        {
            memoryFlags |= VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_CACHED_BIT;
        }

        break;
    }

    case Buffer::BufferType::BufferType_UniformBuffer:
    {
        usageBuffer |= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
        memoryFlags |= VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
        ASSERT(VulkanDeviceCaps::getInstance()->supportHostCoherentMemory, "unsupport coherent memory");

        break;
    }

    case Buffer::BufferType::BufferType_StagingBuffer:
    {
        memoryFlags |= VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
        break;
    }

    default:
        ASSERT(false, "not impl");
    }

    if (VulkanBuffer::isPresentingBufferUsageFlag(StreamBufferUsage::StreamBuffer_Read))
    {
        usageBuffer |= VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    }

    if (VulkanBuffer::isPresentingBufferUsageFlag(StreamBufferUsage::StreamBuffer_Write))
    {
        usageBuffer |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    }
    
#if VULKAN_DEBUG
    LOG_DEBUG("vkCreateBuffer: size %u; flags %u; usage %u, memoryFlags %u", m_size, 0, usageBuffer, memoryFlags);
#endif
    VkBufferCreateInfo bufferCreateInfo = {};
    bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferCreateInfo.pNext = nullptr;
    bufferCreateInfo.flags = 0; //sparse
    bufferCreateInfo.usage = usageBuffer;
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

#if VULKAN_DEBUG_MARKERS
    if (VulkanDeviceCaps::getInstance()->debugUtilsObjectNameEnabled)
    {
        VkDebugUtilsObjectNameInfoEXT debugUtilsObjectNameInfo = {};
        debugUtilsObjectNameInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
        debugUtilsObjectNameInfo.pNext = nullptr;
        debugUtilsObjectNameInfo.objectType = VK_OBJECT_TYPE_BUFFER;
        debugUtilsObjectNameInfo.objectHandle = reinterpret_cast<u64>(m_buffer);
        debugUtilsObjectNameInfo.pObjectName = m_debugName.c_str();

        VulkanWrapper::SetDebugUtilsObjectName(m_device, &debugUtilsObjectNameInfo);
    }
#endif //VULKAN_DEBUG_MARKERS

    m_memory = VulkanMemory::allocateBufferMemory(*m_memoryAllocator, m_buffer, memoryFlags);
    if (m_memory == VulkanMemory::s_invalidMemory)
    {
        VulkanBuffer::destroy();

        LOG_ERROR("VulkanBuffer::VulkanBuffer::create() is failed");
        return false;
    }

//TODO
//Create buffer view
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

    //TODO
    //destroy buffer view

    if (m_buffer)
    {
        VulkanWrapper::DestroyBuffer(m_device, m_buffer, VULKAN_ALLOCATOR);
        m_buffer = VK_NULL_HANDLE;
    }

    if (m_memory != VulkanMemory::s_invalidMemory)
    {
        VulkanMemory::freeMemory(*m_memoryAllocator, m_memory);
    }
}

bool VulkanBuffer::upload(Context* context, u32 offset, u64 size, const void * data)
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

    if (!VulkanBuffer::isPresentingBufferUsageFlag(StreamBufferUsage::StreamBuffer_Dynamic))
    {
        VulkanGraphicContext* vkContext = static_cast<VulkanGraphicContext*>(context);
        VulkanCommandBuffer* uploadBuffer = vkContext->getOrCreateAndStartCommandBuffer(CommandTargetType::CmdUploadBuffer);
        if (m_size <= 65'536)
        {
            ASSERT(!VulkanResource::isCaptured(), "still submitted");
            uploadBuffer->cmdUpdateBuffer(this, offset, m_size, data);
        }
        else
        {
            VulkanStagingBuffer* stagingBuffer = vkContext->getStagingManager()->createStagingBuffer(size, StreamBufferUsage::StreamBuffer_Read);
            if (!stagingBuffer)
            {
                ASSERT(false, "staginBuffer is nullptr");
                return false;
            }
            void* stagingData = stagingBuffer->map();
            ASSERT(stagingData, "stagingData is nullptr");
            memcpy(stagingData, data, size);
            stagingBuffer->unmap();

            ASSERT(!VulkanResource::isCaptured(), "still submitted");
            vkContext->getStagingManager()->destroyAfterUse(stagingBuffer);

            VkBufferCopy bufferCopy = {};
            bufferCopy.srcOffset = 0;
            bufferCopy.dstOffset = offset;
            bufferCopy.size = size;

            //TODO memory barrier
            uploadBuffer->cmdCopyBufferToBuffer(stagingBuffer->getBuffer(), this, { bufferCopy });
            //TODO memory barrier

            u32 immediateResourceSubmit = VulkanDeviceCaps::getInstance()->immediateResourceSubmit;
            if (immediateResourceSubmit > 0)
            {
                vkContext->submit(immediateResourceSubmit == 2 ? true : false);
            }
        }
    }
    else
    {
        if (VulkanResource::isCaptured())
        {
            //ASSERT(false, "still submitted");
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
    ASSERT(m_buffer != VK_NULL_HANDLE, "nullptr");
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

bool VulkanBuffer::isPresentingBufferUsageFlag(StreamBufferUsage flag)
{
    return m_usageFlags & flag;
}

} //namespace vk
} //namespace renderer
} //namespace v3d
#endif //VULKAN_RENDER
