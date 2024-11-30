#include "VulkanBuffer.h"

#include "Utils/Logger.h"

#ifdef VULKAN_RENDER
#   include "VulkanDebug.h"
#   include "VulkanDevice.h"
#   include "VulkanStagingBuffer.h"

namespace v3d
{
namespace renderer
{
namespace vk
{

#if DEBUG_OBJECT_MEMORY
std::set<VulkanBuffer*> VulkanBuffer::s_objects;
#endif //DEBUG_OBJECT_MEMORY

VulkanBuffer::VulkanBuffer(VulkanDevice* device, VulkanMemory::VulkanMemoryAllocator* alloc, RenderBuffer::Type type, u64 size, BufferUsageFlags usageFlags, const std::string& name) noexcept
    : m_device(*device)
    , m_memoryAllocator(alloc)

    , m_buffer(VK_NULL_HANDLE)
    , m_memory(VulkanMemory::s_invalidMemory)

    , m_type(type)
    , m_size(size) //TODO: check aligment
    , m_usageFlags(usageFlags)

    , m_mapped(false)
{
    LOG_DEBUG("VulkanBuffer::VulkanBuffer constructor %llx", this);
#if VULKAN_DEBUG_MARKERS
    m_debugName = name.empty() ? "Buffer" : name;
    m_debugName.append(VulkanDebugUtils::k_addressPreffix);
    m_debugName.append(std::to_string(reinterpret_cast<const u64>(this)));
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
    case RenderBuffer::Type::VertexBuffer:
    {
        usageBuffer |= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
        usageBuffer |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;

        if (VulkanBuffer::hasUsageFlag(BufferUsage::Buffer_Dynamic))
        {
            memoryFlags |= VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
            if (VulkanBuffer::hasUsageFlag(BufferUsage::Buffer_GPUWrite) && VulkanBuffer::hasUsageFlag(BufferUsage::Buffer_GPURead))
            {
                memoryFlags |= VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
                ASSERT(m_device.getVulkanDeviceCaps()._supportHostCoherentMemory, "unsupport coherent memory");
            }
            else if (VulkanBuffer::hasUsageFlag(BufferUsage::Buffer_GPUWrite))
            {
                memoryFlags |= VK_MEMORY_PROPERTY_HOST_CACHED_BIT;
                ASSERT(m_device.getVulkanDeviceCaps()._supportHostCacheMemory, "unsupport coherent memory");
            }
            else
            {
                ASSERT(false, "fail");
            }
        }
        else
        {
            memoryFlags |= VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
        }

        break;
    }

    case RenderBuffer::Type::IndexBuffer:
    {
        usageBuffer |= VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
        usageBuffer |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;

        if (VulkanBuffer::hasUsageFlag(BufferUsage::Buffer_Dynamic))
        {
            memoryFlags |= VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
            if (VulkanBuffer::hasUsageFlag(BufferUsage::Buffer_GPUWrite) && VulkanBuffer::hasUsageFlag(BufferUsage::Buffer_GPURead))
            {
                memoryFlags |= VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
                ASSERT(m_device.getVulkanDeviceCaps()._supportHostCoherentMemory, "unsupport coherent memory");
            }
            else if (VulkanBuffer::hasUsageFlag(BufferUsage::Buffer_GPUWrite))
            {
                memoryFlags |= VK_MEMORY_PROPERTY_HOST_CACHED_BIT;
                ASSERT(m_device.getVulkanDeviceCaps()._supportHostCacheMemory, "unsupport coherent memory");
            }
            else
            {
                ASSERT(false, "fail");
            }
        }
        else
        {
            memoryFlags |= VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
        }

        break;
    }

    case RenderBuffer::Type::ConstantBuffer:
    {
        usageBuffer |= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
        memoryFlags |= VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
        ASSERT(m_device.getVulkanDeviceCaps()._supportHostCoherentMemory, "unsupport coherent memory");

        break;
    }

    case RenderBuffer::Type::StagingBuffer:
    {
        usageBuffer |= VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
        memoryFlags |= VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;

        break;
    }

    case RenderBuffer::Type::StorageBuffer:
    {
        ASSERT(false, "not impl");
        memoryFlags |= VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;

        break;
    }

    case RenderBuffer::Type::ReadbackBuffer:
    {
        ASSERT(false, "not impl");
        usageBuffer |= VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;

        break;
    }

    default:
        ASSERT(false, "not impl");
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

    VkResult result = VulkanWrapper::CreateBuffer(m_device.getDeviceInfo()._device, &bufferCreateInfo, VULKAN_ALLOCATOR, &m_buffer);
    if (result != VK_SUCCESS)
    {
        LOG_ERROR("VulkanBuffer::create vkCreateBuffer is failed. Error: %s", ErrorString(result).c_str());
        return false;
    }

#if VULKAN_DEBUG_MARKERS
    if (m_device.getVulkanDeviceCaps()._debugUtilsObjectNameEnabled)
    {
        VkDebugUtilsObjectNameInfoEXT debugUtilsObjectNameInfo = {};
        debugUtilsObjectNameInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
        debugUtilsObjectNameInfo.pNext = nullptr;
        debugUtilsObjectNameInfo.objectType = VK_OBJECT_TYPE_BUFFER;
        debugUtilsObjectNameInfo.objectHandle = reinterpret_cast<u64>(m_buffer);
        debugUtilsObjectNameInfo.pObjectName = m_debugName.c_str();

        VulkanWrapper::SetDebugUtilsObjectName(m_device.getDeviceInfo()._device, &debugUtilsObjectNameInfo);
    }
#endif //VULKAN_DEBUG_MARKERS

    m_memory = VulkanMemory::allocateBufferMemory(*m_memoryAllocator, m_buffer, memoryFlags);
    if (m_memory == VulkanMemory::s_invalidMemory)
    {
        VulkanBuffer::destroy();

        LOG_ERROR("VulkanBuffer::VulkanBuffer::create() is failed");
        return false;
    }
#if VULKAN_DEBUG
    m_memoryAllocator->linkVulkanObject(m_memory, this);
#endif //VULKAN_DEBUG

    if (!createViewBuffer())
    {
        VulkanBuffer::destroy();

        LOG_ERROR("VulkanBuffer::VulkanBuffer::create() is failed");
        return false;
    }

    return true;
}

void VulkanBuffer::destroy()
{
    ASSERT(!m_mapped, "mapped");

    //TODO
    //destroy buffer view

    if (m_buffer)
    {
        VulkanWrapper::DestroyBuffer(m_device.getDeviceInfo()._device, m_buffer, VULKAN_ALLOCATOR);
        m_buffer = VK_NULL_HANDLE;
    }

    if (m_memory != VulkanMemory::s_invalidMemory)
    {
#if VULKAN_DEBUG
        m_memoryAllocator->unlinkVulkanObject(m_memory, this);
#endif //VULKAN_DEBUG
        VulkanMemory::freeMemory(*m_memoryAllocator, m_memory);
    }
}

bool VulkanBuffer::upload(VulkanCommandBuffer* cmdBuffer, u32 offset, u64 size, const void* data)
{
    if (!m_buffer)
    {
        ASSERT(false, "nullptr");
        return false;
    }

    if (!size || !data)
    {
        return false;
    }

    if (m_size < size)
    {
        ASSERT(false, "different size in non dynamic data");
        return false;
    }

    if (VulkanBuffer::hasUsageFlag(BufferUsage::Buffer_Dynamic))
    {
        //Don"t care about usge status
        c8* dstData = reinterpret_cast<c8*>(VulkanBuffer::map());
        if (dstData)
        {
            memcpy(dstData + offset, data, size);
            VulkanBuffer::unmap();

            return true;
        }

        return false;
    }
    else
    {
        const u32 k_updateBuffer_MaxSize = 65'536;

        if (m_size <= k_updateBuffer_MaxSize)
        {
            ASSERT(!VulkanResource::isUsed(), "still submitted");
            cmdBuffer->cmdUpdateBuffer(this, offset, m_size, data);
        }
        else
        {
            VulkanBuffer* stagingBuffer = m_device.getStaginBufferManager()->createStagingBuffer(size);
            if (!stagingBuffer)
            {
                ASSERT(false, "staginBuffer is nullptr");
                return false;
            }

            void* stagingData = stagingBuffer->map();
            ASSERT(stagingData, "stagingData is nullptr");
            memcpy(stagingData, data, size);
            stagingBuffer->unmap();

            ASSERT(!VulkanResource::isUsed(), "still submitted");
            m_device.getStaginBufferManager()->destroyAfterUse(stagingBuffer);

            VkBufferCopy bufferCopy = {};
            bufferCopy.srcOffset = 0;
            bufferCopy.dstOffset = offset;
            bufferCopy.size = size;

            //TODO memory barrier
            cmdBuffer->cmdCopyBufferToBuffer(stagingBuffer, this, { bufferCopy });
            //TODO memory barrier
        }
    }

    return true;
}

bool VulkanBuffer::readback(VulkanCommandBuffer* cmdBuffer, u32 offset, u64 size, VulkanBuffer*& stagingBuffer)
{
    if (!m_buffer || !stagingBuffer)
    {
        ASSERT(false, "nullptr");
        return false;
    }

    if (m_size < size)
    {
        ASSERT(false, "different size in non dynamic data");
        return false;
    }

    if (VulkanBuffer::hasUsageFlag(BufferUsage::Buffer_Dynamic))
    {
        //Don"t care about usge status
        c8* srcData = reinterpret_cast<c8*>(VulkanBuffer::map());
        if (srcData)
        {
            c8* dstData = reinterpret_cast<c8*>(stagingBuffer->map());

            memcpy(srcData, dstData + offset, size);
            VulkanBuffer::unmap();

            return true;
        }

        return false;
    }
    else
    {
        VkBufferCopy bufferCopy = {};
        bufferCopy.srcOffset = offset;
        bufferCopy.dstOffset = 0;
        bufferCopy.size = size;

        //TODO memory barrier
        cmdBuffer->cmdCopyBufferToBuffer(this, stagingBuffer, { bufferCopy });
        //TODO memory barrier
    }

    return true;
}

void* VulkanBuffer::map()
{
    if (!m_buffer)
    {
        ASSERT(false, "nullptr");
        return nullptr;
    }

    ASSERT(!m_mapped, "already mappped");
    ASSERT(m_memory._mapped, "m_memory._mapped can't map");
    if (m_memory._flag & VK_MEMORY_PROPERTY_HOST_CACHED_BIT)
    {
        VkMappedMemoryRange mappedMemoryRange = {};
        mappedMemoryRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
        mappedMemoryRange.pNext = nullptr;
        mappedMemoryRange.memory = m_memory._memory;
        mappedMemoryRange.size = m_memory._size;
        mappedMemoryRange.offset = m_memory._offset;

        VkResult result = VulkanWrapper::InvalidateMappedMemoryRanges(m_device.getDeviceInfo()._device, 1, &mappedMemoryRange);
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

        VkResult result = VulkanWrapper::FlushMappedMemoryRanges(m_device.getDeviceInfo()._device, 1, &mappedMemoryRange);
        if (result != VK_SUCCESS)
        {
            ASSERT(false, "error");
        }
    }
    m_mapped = false;
}

bool VulkanBuffer::createViewBuffer()
{
    //TODO

    return true;
}

} //namespace vk
} //namespace renderer
} //namespace v3d
#endif //VULKAN_RENDER
