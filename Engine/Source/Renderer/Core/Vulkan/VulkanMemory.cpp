#include "VulkanMemory.h"

#include "Utils/Logger.h"

#ifdef VULKAN_RENDER
#include "VulkanDebug.h"
#include "VulkanContext.h"

namespace v3d
{
namespace renderer
{
namespace vk
{

VulkanMemory::VulkanAllocation VulkanMemory::s_invalidMemory = 
{
    VK_NULL_HANDLE,
    nullptr,
    nullptr,
    0,
    0,
    0,
    0
};

SimpleVulkanMemoryAllocator* VulkanMemory::s_simpleVulkanMemoryAllocator = nullptr;

std::recursive_mutex VulkanMemory::s_mutex;


s32 VulkanMemory::findMemoryTypeIndex(const VkMemoryRequirements& memoryRequirements, VkMemoryPropertyFlags memoryPropertyFlags)
{
    const VkPhysicalDeviceMemoryProperties& physicalDeviceMemoryProperties = VulkanDeviceCaps::getInstance()->getDeviceMemoryProperties();
    u32 memoryTypeBits = memoryRequirements.memoryTypeBits;
    for (u32 memoryTypeIndex = 0; memoryTypeIndex < physicalDeviceMemoryProperties.memoryTypeCount; ++memoryTypeIndex)
    {
        if ((memoryTypeBits & 1) == 1)
        {
            if ((physicalDeviceMemoryProperties.memoryTypes[memoryTypeIndex].propertyFlags & memoryPropertyFlags) == memoryPropertyFlags)
            {
                return memoryTypeIndex;
            }
        }

        memoryTypeBits >>= 1;
    }

    return -1;
}

bool VulkanMemory::isSupportedMemoryType(VkMemoryPropertyFlags memoryPropertyFlags, bool isEqual)
{
    const VkPhysicalDeviceMemoryProperties& physicalDeviceMemoryProperties = VulkanDeviceCaps::getInstance()->getDeviceMemoryProperties();
    for (u32 memoryTypeIndex = 0; memoryTypeIndex < physicalDeviceMemoryProperties.memoryTypeCount; ++memoryTypeIndex)
    {
        if (isEqual)
        {
            if (physicalDeviceMemoryProperties.memoryTypes[memoryTypeIndex].propertyFlags == memoryPropertyFlags)
            {
                return true;
            }
        }
        else
        {
            if ((physicalDeviceMemoryProperties.memoryTypes[memoryTypeIndex].propertyFlags & memoryPropertyFlags) == memoryPropertyFlags)
            {
                return true;
            }
        }
    }

    return false;
}

VulkanMemory::VulkanAllocation VulkanMemory::allocateImageMemory(VulkanMemoryAllocator& allocator, VkImage image, VkMemoryPropertyFlags flags)
{
    if (image == VK_NULL_HANDLE)
    {
        LOG_ERROR("VulkanMemory::allocateImageMemory: invalid image");
        ASSERT(false, "invalid image");

        return VulkanMemory::s_invalidMemory;
    }

    {
        std::lock_guard<std::recursive_mutex> lock(s_mutex);

        s32 memoryTypeIndex = -1;
        VulkanAllocation memory = {};
        if (VulkanDeviceCaps::getInstance()->supportDedicatedAllocation)
        {
            VkMemoryDedicatedRequirements memoryDedicatedRequirements = {};
            memoryDedicatedRequirements.sType = VK_STRUCTURE_TYPE_MEMORY_DEDICATED_REQUIREMENTS;
            memoryDedicatedRequirements.pNext = nullptr;

            VkMemoryRequirements2 memoryRequirements2 = {};
            memoryRequirements2.sType = VK_STRUCTURE_TYPE_MEMORY_REQUIREMENTS_2;
            memoryRequirements2.pNext = &memoryDedicatedRequirements;
            memoryRequirements2.memoryRequirements = {};

            VkImageMemoryRequirementsInfo2 imageMemoryRequirementsInfo2 = {};
            imageMemoryRequirementsInfo2.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_REQUIREMENTS_INFO_2;
            imageMemoryRequirementsInfo2.pNext = nullptr;
            imageMemoryRequirementsInfo2.image = image;
            VulkanWrapper::GetImageMemoryRequirements2(allocator.m_device, &imageMemoryRequirementsInfo2, &memoryRequirements2);

            memoryTypeIndex = VulkanMemory::findMemoryTypeIndex(memoryRequirements2.memoryRequirements, flags);
            if (memoryTypeIndex < 0)
            {
                LOG_ERROR("VulkanMemory::allocateImageMemory2: invalid memoryTypeIndex %d, memoryRequirements.memoryTypeBits %d, VkMemoryPropertyFlags %d", memoryTypeIndex, memoryRequirements2.memoryRequirements.memoryTypeBits, flags);
                ASSERT(false, "invalid memoryTypeIndex");

                return  VulkanMemory::s_invalidMemory;
            }
#if VULKAN_DEBUG
            LOG_DEBUG("VulkanMemory:GetImageMemoryRequirements2: memoryTypeIndex %d, memoryRequirements.memoryTypeBits %d, VkMemoryPropertyFlags %d", memoryTypeIndex, memoryRequirements2.memoryRequirements.memoryTypeBits, flags);
            LOG_DEBUG("VulkanMemory:GetImageMemoryRequirements2 prefersDedicatedAllocation || requiresDedicatedAllocation %u", (memoryDedicatedRequirements.prefersDedicatedAllocation || memoryDedicatedRequirements.requiresDedicatedAllocation));
#endif // VULKAN_DEBUG

            void* vkExtensions = nullptr;

            VkMemoryDedicatedAllocateInfo memoryDedicatedAllocateInfo = {};
            if (memoryDedicatedRequirements.prefersDedicatedAllocation || memoryDedicatedRequirements.requiresDedicatedAllocation)
            {
                memoryDedicatedAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_DEDICATED_ALLOCATE_INFO;
                memoryDedicatedAllocateInfo.pNext = nullptr;
                memoryDedicatedAllocateInfo.image = image;
                memoryDedicatedAllocateInfo.buffer = VK_NULL_HANDLE;
                vkExtensions = &memoryDedicatedAllocateInfo;

                memory = VulkanMemory::getSimpleMemoryAllocator(allocator.m_device)->allocate(memoryRequirements2.memoryRequirements.size, memoryRequirements2.memoryRequirements.alignment, memoryTypeIndex, vkExtensions);
                if (memory._memory == VK_NULL_HANDLE)
                {
                    return VulkanMemory::s_invalidMemory;
                }

                memory._property |= MemoryProperty::dedicatedMemory;
            }
            else
            {
                memory = allocator.allocate(memoryRequirements2.memoryRequirements.size, memoryRequirements2.memoryRequirements.alignment, memoryTypeIndex, nullptr);
                if (memory._memory == VK_NULL_HANDLE)
                {
                    return VulkanMemory::s_invalidMemory;
                }
            }
        }
        else
        {
            VkMemoryRequirements memoryRequirements = {};
            VulkanWrapper::GetImageMemoryRequirements(allocator.m_device, image, &memoryRequirements);

            s32 memoryTypeIndex = VulkanMemory::findMemoryTypeIndex(memoryRequirements, flags);
            if (memoryTypeIndex < 0)
            {
                LOG_ERROR("VulkanMemory::allocateImageMemory: invalid memoryTypeIndex %d, memoryRequirements.memoryTypeBits %d, VkMemoryPropertyFlags %d", memoryTypeIndex, memoryRequirements.memoryTypeBits, flags);
                ASSERT(false, "invalid memoryTypeIndex");

                return  VulkanMemory::s_invalidMemory;
            }
#if VULKAN_DEBUG
            LOG_DEBUG("VulkanMemory:GetImageMemoryRequirements: memoryTypeIndex %d, memoryRequirements.memoryTypeBits %d, VkMemoryPropertyFlags %d", memoryTypeIndex, memoryRequirements.memoryTypeBits, flags);
#endif // VULKAN_DEBUG

            memory = allocator.allocate(memoryRequirements.size, memoryRequirements.alignment, memoryTypeIndex, nullptr);
            if (memory._memory == VK_NULL_HANDLE)
            {
                return VulkanMemory::s_invalidMemory;
            }
        }

        VkResult result = VulkanWrapper::BindImageMemory(allocator.m_device, image, memory._memory, memory._offset);
        if (result != VK_SUCCESS)
        {
            LOG_ERROR("VulkanMemory::allocateImageMemory: vkBindImageMemory. Error %s", ErrorString(result).c_str());

            if (memory._property & MemoryProperty::dedicatedMemory)
            {
                VulkanMemory::getSimpleMemoryAllocator(allocator.m_device)->deallocate(memory);
            }
            else
            {
                allocator.deallocate(memory);
            }

            return VulkanMemory::s_invalidMemory;
        }

        return memory;
    }
}

VulkanMemory::VulkanAllocation VulkanMemory::allocateBufferMemory(VulkanMemoryAllocator& allocator, VkBuffer buffer, VkMemoryPropertyFlags flags)
{
    if (buffer == VK_NULL_HANDLE)
    {
        LOG_ERROR("VulkanMemory::allocateBufferMemory: invalid buffer");
        ASSERT(false, "invalid buffer");

        return VulkanMemory::s_invalidMemory;
    }

    {
        std::lock_guard<std::recursive_mutex> lock(s_mutex);

        VkMemoryRequirements memoryRequirements = {};
        VulkanWrapper::GetBufferMemoryRequirements(allocator.m_device, buffer, &memoryRequirements);

        s32 memoryTypeIndex = VulkanMemory::findMemoryTypeIndex(memoryRequirements, flags);
        if (memoryTypeIndex < 0)
        {
            LOG_ERROR("VulkanMemory::allocateBufferMemory: invalid memoryTypeIndex %d, memoryRequirements.memoryTypeBits %d, VkMemoryPropertyFlags %d", memoryTypeIndex, memoryRequirements.memoryTypeBits, flags);
            ASSERT(false, "invalid memoryTypeIndex");

            return VulkanMemory::s_invalidMemory;
        }
#if VULKAN_DEBUG
        LOG_DEBUG("VulkanMemory:GetBufferMemoryRequirements: memoryTypeIndex %d, memoryRequirements.memoryTypeBits %d, VkMemoryPropertyFlags %d", memoryTypeIndex, memoryRequirements.memoryTypeBits, flags);
#endif // VULKAN_DEBUG

        VulkanAllocation memory = allocator.allocate(memoryRequirements.size, memoryRequirements.alignment, memoryTypeIndex);
        if (memory._memory == VK_NULL_HANDLE)
        {
            return VulkanMemory::s_invalidMemory;
        }

        VkResult result = VulkanWrapper::BindBufferMemory(allocator.m_device, buffer, memory._memory, memory._offset);
        if (result != VK_SUCCESS)
        {
            LOG_ERROR("VulkanMemory::allocateBufferMemory: vkBindBufferMemory. Error %s", ErrorString(result).c_str());
            allocator.deallocate(memory);

            return  VulkanMemory::s_invalidMemory;
        }

        return memory;
    }
}

void VulkanMemory::freeMemory(VulkanMemoryAllocator& allocator, VulkanAllocation& memory)
{
    {
        std::lock_guard<std::recursive_mutex> lock(s_mutex);

        if (memory._property & MemoryProperty::dedicatedMemory)
        {
            VulkanMemory::getSimpleMemoryAllocator(allocator.m_device)->deallocate(memory);
        }
        else
        {
            allocator.deallocate(memory);
        }
    }
}

VulkanMemory::VulkanMemoryAllocator* VulkanMemory::getSimpleMemoryAllocator(VkDevice device)
{
    {
        std::lock_guard<std::recursive_mutex> lock(s_mutex);

        if (!s_simpleVulkanMemoryAllocator)
        {
            s_simpleVulkanMemoryAllocator = new SimpleVulkanMemoryAllocator(device);
        }
        return s_simpleVulkanMemoryAllocator;
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////

VulkanMemory::VulkanMemoryAllocator::VulkanMemoryAllocator(VkDevice device) noexcept
    : m_device(device)
{
}

VulkanMemory::VulkanMemoryAllocator::~VulkanMemoryAllocator()
{
}

/////////////////////////////////////////////////////////////////////////////////////////////////////

#if VULKAN_DEBUG_MARKERS
u32 SimpleVulkanMemoryAllocator::s_debugNameGenerator = 0;
#endif //VULKAN_DEBUG_MARKERS

SimpleVulkanMemoryAllocator::SimpleVulkanMemoryAllocator(VkDevice device) noexcept
    : VulkanMemoryAllocator(device)
{
}

SimpleVulkanMemoryAllocator::~SimpleVulkanMemoryAllocator()
{
}

VulkanMemory::VulkanAllocation SimpleVulkanMemoryAllocator::allocate(VkDeviceSize size, VkDeviceSize align, u32 memoryTypeIndex, const void* extensions)
{
    VkDeviceSize alignedSize = core::alignUp<VkDeviceSize>(size, align);

    VulkanMemory::VulkanAllocation memory = {};

    VkMemoryAllocateInfo memoryAllocateInfo = {};
    memoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    memoryAllocateInfo.pNext = extensions ? extensions : nullptr;
    memoryAllocateInfo.allocationSize = alignedSize;
    memoryAllocateInfo.memoryTypeIndex = memoryTypeIndex;

    VkResult result = VulkanWrapper::AllocateMemory(m_device, &memoryAllocateInfo, VULKAN_ALLOCATOR, &memory._memory);
    if (result != VK_SUCCESS)
    {
        LOG_ERROR("SimpleVulkanMemoryAllocator::allocate vkAllocateMemory is failed. Error: %s", ErrorString(result).c_str());
        return VulkanMemory::s_invalidMemory;
    }

    const VkPhysicalDeviceMemoryProperties& physicalDeviceMemoryProperties = VulkanDeviceCaps::getInstance()->getDeviceMemoryProperties();

    memory._size = alignedSize;
    memory._offset = 0;
    memory._mapped = nullptr;
    memory._flag = physicalDeviceMemoryProperties.memoryTypes[memoryTypeIndex].propertyFlags;

    if (memory._flag & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)
    {
        VkResult result = VulkanWrapper::MapMemory(m_device, memory._memory, 0, VK_WHOLE_SIZE, 0, &memory._mapped);
        if (result != VK_SUCCESS)
        {
            LOG_ERROR("SimpleVulkanMemoryAllocator::allocate: vkMapMemory. Error %s", ErrorString(result).c_str());
            SimpleVulkanMemoryAllocator::deallocate(memory);

            return VulkanMemory::s_invalidMemory;
        }
    }

#if VULKAN_DEBUG_MARKERS
    if (VulkanDeviceCaps::getInstance()->debugUtilsObjectNameEnabled)
    {
        std::string debugName = "SimpleAlloc_Memory_" + std::to_string(s_debugNameGenerator++);

        VkDebugUtilsObjectNameInfoEXT debugUtilsObjectNameInfo = {};
        debugUtilsObjectNameInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
        debugUtilsObjectNameInfo.pNext = nullptr;
        debugUtilsObjectNameInfo.objectType = VK_OBJECT_TYPE_DEVICE_MEMORY;
        debugUtilsObjectNameInfo.objectHandle = reinterpret_cast<u64>(memory._memory);
        debugUtilsObjectNameInfo.pObjectName = debugName.c_str();

        VulkanWrapper::SetDebugUtilsObjectName(m_device, &debugUtilsObjectNameInfo);
    }
#endif //VULKAN_DEBUG_MARKERS

    return memory;
}

void SimpleVulkanMemoryAllocator::deallocate(VulkanMemory::VulkanAllocation& memory)
{
    if (memory._mapped)
    {
        ASSERT(memory._memory, "nullptr");
        VulkanWrapper::UnmapMemory(m_device, memory._memory);
    }

    if (memory._memory != VK_NULL_HANDLE)
    {
        VulkanWrapper::FreeMemory(m_device, memory._memory, VULKAN_ALLOCATOR);
    }
    memory = VulkanMemory::s_invalidMemory;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////

PoolVulkanMemoryAllocator::PoolVulkanMemoryAllocator(VkDevice device, u64 allocationSize) noexcept
    : VulkanMemoryAllocator(device)
    , m_allocationSize(allocationSize)
{
}

PoolVulkanMemoryAllocator::~PoolVulkanMemoryAllocator()
{
    for (u32 i = 0; i < VK_MAX_MEMORY_TYPES; ++i)
    {
        ASSERT(m_heaps[i].empty(), "not empty");
        m_heaps[i].clear();
    }
}

PoolVulkanMemoryAllocator::Pool::Pool() noexcept
    : _memory(0)
    , _size(0)
    , _chunkSize(0)
    , _mapped(nullptr)
    , _memoryTypeIndex(-1)
{
}

VulkanMemory::VulkanAllocation PoolVulkanMemoryAllocator::allocate(VkDeviceSize size, VkDeviceSize align, u32 memoryTypeIndex, const void* extensions)
{
    VkDeviceSize alignedSize = core::alignUp<VkDeviceSize>(size, align);

    ASSERT(memoryTypeIndex < VK_MAX_MEMORY_TYPES, "out of range");
    auto& heaps = m_heaps[memoryTypeIndex];
    
    //find in heap
    if (VulkanMemory::VulkanAllocation memory = {}; PoolVulkanMemoryAllocator::findAllocationFromPool(heaps, alignedSize, memory))
    {
        return memory;
    }

    //create new pool
    ASSERT(m_allocationSize > alignedSize, "Too small size of pool. Set bigger");
    Pool* newPool = createPool(m_allocationSize, alignedSize, memoryTypeIndex);
    if (!newPool)
    {
        return VulkanMemory::s_invalidMemory;
    }

    MemoryChunck reqestedChunk = {};
    reqestedChunk._size = alignedSize;
    reqestedChunk._offset = 0;
    newPool->_chunks.insert(reqestedChunk);

    heaps.emplace(newPool->_chunkSize, newPool);

    const VkPhysicalDeviceMemoryProperties& physicalDeviceMemoryProperties = VulkanDeviceCaps::getInstance()->getDeviceMemoryProperties();
    VkMemoryPropertyFlags flag = physicalDeviceMemoryProperties.memoryTypes[memoryTypeIndex].propertyFlags;

    return {
        newPool->_memory,
        newPool->_mapped,
        newPool,
        alignedSize,
        0,
        flag,
    };
}

void PoolVulkanMemoryAllocator::deallocate(VulkanMemory::VulkanAllocation& memory)
{
    Pool* pool = reinterpret_cast<Pool*>(memory._metadata);
    ASSERT(pool, "pool nullptr");

    auto usedChunkIter = pool->_chunks.find({ memory._size, memory._offset });
    ASSERT(usedChunkIter != pool->_chunks.cend(), "not found");
    MemoryChunck usedChunk = const_cast<MemoryChunck&>(*usedChunkIter);

    MemoryChunck newFreeChunk = {};
    newFreeChunk._size = usedChunk._size;
    newFreeChunk._offset = usedChunk._offset;


    //left border
    if (usedChunkIter != pool->_chunks.begin())
    {
        auto chunkPrevIter = std::next(usedChunkIter, -1);
        auto freeChunkPrevIter = pool->_freeChunks.find(*chunkPrevIter);
        if (freeChunkPrevIter != pool->_freeChunks.end())
        {
            ASSERT(usedChunk._offset >= freeChunkPrevIter->_size, "wrong");
            if (usedChunk._offset - freeChunkPrevIter->_size == freeChunkPrevIter->_offset)
            {
                newFreeChunk._offset = freeChunkPrevIter->_offset;
                newFreeChunk._size += freeChunkPrevIter->_size;

                pool->_freeChunks.erase(freeChunkPrevIter);
                pool->_chunks.erase(chunkPrevIter);
            }
        }
    }

    auto chunkNextIter = std::next(usedChunkIter, 1);
    if (chunkNextIter != pool->_chunks.end())
    {
        auto freeChunkNextIter = pool->_freeChunks.find(*chunkNextIter);
        if (freeChunkNextIter != pool->_freeChunks.end())
        {
            ASSERT(freeChunkNextIter->_offset >= newFreeChunk._size, "wrong");
            if (freeChunkNextIter->_offset - newFreeChunk._size == newFreeChunk._offset)
            {
                newFreeChunk._offset = newFreeChunk._offset; //same
                newFreeChunk._size += freeChunkNextIter->_size;

                pool->_freeChunks.erase(freeChunkNextIter);
                pool->_chunks.erase(chunkNextIter);
            }
        }
    }

    pool->_chunks.erase(usedChunkIter);
    
    pool->_freeChunks.insert(newFreeChunk);
    pool->_chunks.insert(newFreeChunk);


    auto& heaps = m_heaps[pool->_memoryTypeIndex];
    auto poolIters = heaps.equal_range(pool->_chunkSize);

    auto poolIter = heaps.end();
    for (auto iter = poolIters.first; iter != poolIters.second; ++iter)
    {
        if (iter->second == pool)
        {
            poolIter = iter;
            break;
        }
    }
    ASSERT(poolIter != heaps.end(), "not found");


    if (pool->_size == newFreeChunk._size)
    {
        //empty pool
        ASSERT(pool->_chunks.size() == pool->_freeChunks.size(), "fail");
        auto& heaps = m_heaps[pool->_memoryTypeIndex];

        if (pool->_mapped)
        {
            ASSERT(pool->_memory, "nullptr");
            VulkanWrapper::UnmapMemory(m_device, pool->_memory);
        }

        if (pool->_memory != VK_NULL_HANDLE)
        {
            VulkanWrapper::FreeMemory(m_device, pool->_memory, VULKAN_ALLOCATOR);
        }

        delete pool;
        heaps.erase(poolIter);
    }
    else
    {
        VkDeviceSize maxChuckSize = pool->_freeChunks.empty() ? 0 : pool->_freeChunks.rbegin()->_size;
        if (maxChuckSize != pool->_chunkSize)
        {
            heaps.erase(poolIter);

            pool->_chunkSize = maxChuckSize;
            heaps.emplace(pool->_chunkSize, pool);
        }
    }

    memory = VulkanMemory::s_invalidMemory;
}

bool PoolVulkanMemoryAllocator::findAllocationFromPool(std::multimap<VkDeviceSize, Pool*>& heaps, VkDeviceSize size, VulkanMemory::VulkanAllocation& memory)
{
    auto heapIter = heaps.lower_bound(size);
    if (heapIter == heaps.cend())
    {
        return false;
    }
    Pool* pool = heapIter->second;

    auto chunkSizeIter = pool->_freeChunks.lower_bound({ size, 0 });
    if (chunkSizeIter == pool->_freeChunks.cend())
    {
        return false;
    }
    MemoryChunck chunkSize = const_cast<MemoryChunck&>(*chunkSizeIter);
    pool->_freeChunks.erase(chunkSizeIter);

    auto chunkOffsetIter = pool->_chunks.lower_bound(chunkSize);
    if (chunkOffsetIter == pool->_chunks.cend())
    {
        return false;
    }
    pool->_chunks.erase(chunkOffsetIter);


    ASSERT(chunkSize._size > size, "fail");
    VkDeviceSize allocatedSize = 0;
    VkDeviceSize allocatedOffset = 0;
    if (chunkSize._size - size >= VulkanDeviceCaps::getInstance()->getPhysicalDeviceLimits().minMemoryMapAlignment)
    {
        MemoryChunck reqestedChunkSize;
        reqestedChunkSize._size = size;
        reqestedChunkSize._offset = chunkSize._offset;
        pool->_chunks.insert(reqestedChunkSize);

        allocatedSize = reqestedChunkSize._size;
        allocatedOffset = reqestedChunkSize._offset;

        MemoryChunck freeChunkSize;
        freeChunkSize._size = chunkSize._size - size;
        freeChunkSize._offset = chunkSize._offset + size;
        pool->_chunks.insert(freeChunkSize);
        pool->_freeChunks.insert(freeChunkSize);
    }
    else
    {
        pool->_chunks.insert(chunkSize);

        allocatedSize = chunkSize._size;
        allocatedOffset = chunkSize._offset;
    }

    const VkPhysicalDeviceMemoryProperties& physicalDeviceMemoryProperties = VulkanDeviceCaps::getInstance()->getDeviceMemoryProperties();
    VkMemoryPropertyFlags flag = physicalDeviceMemoryProperties.memoryTypes[pool->_memoryTypeIndex].propertyFlags;

    memory._memory = pool->_memory;
    memory._mapped = pool->_mapped;
    memory._metadata = pool;
    memory._size = allocatedSize;
    memory._offset = allocatedOffset;
    memory._flag = flag;

    VkDeviceSize maxChuckSize = pool->_freeChunks.empty() ? 0 : pool->_freeChunks.rbegin()->_size;
    if (maxChuckSize != pool->_chunkSize)
    {
        heaps.erase(heapIter);

        pool->_chunkSize = maxChuckSize;
        heaps.emplace(pool->_chunkSize, pool);
    }

    return true;
}

PoolVulkanMemoryAllocator::Pool* PoolVulkanMemoryAllocator::createPool(VkDeviceSize poolSize, VkDeviceSize requestedSize, u32 memoryTypeIndex)
{
    VkMemoryAllocateInfo memoryAllocateInfo = {};
    memoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    memoryAllocateInfo.pNext = nullptr;
    memoryAllocateInfo.allocationSize = poolSize;
    memoryAllocateInfo.memoryTypeIndex = memoryTypeIndex;

    VkDeviceMemory deviceMemory;
    VkResult result = VulkanWrapper::AllocateMemory(m_device, &memoryAllocateInfo, VULKAN_ALLOCATOR, &deviceMemory);
    if (result != VK_SUCCESS)
    {
        LOG_ERROR("PoolVulkanMemoryAllocator::allocate vkAllocateMemory is failed. Error: %s", ErrorString(result).c_str());
        return nullptr;
    }

    MemoryChunck freeMemoryChunk;
    freeMemoryChunk._size = poolSize - requestedSize;
    freeMemoryChunk._offset = requestedSize;

    Pool* newPool = new Pool();
    newPool->_memory = deviceMemory;
    newPool->_size = poolSize;
    newPool->_chunkSize = poolSize - requestedSize;
    newPool->_mapped = nullptr;
    newPool->_memoryTypeIndex = memoryTypeIndex;

    newPool->_chunks.insert(freeMemoryChunk);
    newPool->_freeChunks.insert(freeMemoryChunk);

    const VkPhysicalDeviceMemoryProperties& physicalDeviceMemoryProperties = VulkanDeviceCaps::getInstance()->getDeviceMemoryProperties();
    VkMemoryPropertyFlags flag = physicalDeviceMemoryProperties.memoryTypes[memoryTypeIndex].propertyFlags;

    if (flag & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)
    {
        VkResult result = VulkanWrapper::MapMemory(m_device, newPool->_memory, 0, VK_WHOLE_SIZE, 0, &newPool->_mapped);
        if (result != VK_SUCCESS)
        {
            LOG_ERROR("PoolVulkanMemoryAllocator::allocate: vkMapMemory. Error %s", ErrorString(result).c_str());

            VulkanWrapper::FreeMemory(m_device, newPool->_memory, VULKAN_ALLOCATOR);
            delete newPool;

            return nullptr;
        }
    }

    return newPool;
}

#if VULKAN_DEBUG
void PoolVulkanMemoryAllocator::linkVulkanObject(const VulkanMemory::VulkanAllocation& allocation, const VulkanResource* object)
{
    if (allocation._metadata)
    {
        reinterpret_cast<Pool*>(allocation._metadata)->_objectList.insert(object);
    }
}

void PoolVulkanMemoryAllocator::unlinkVulkanObject(const VulkanMemory::VulkanAllocation& allocation, const VulkanResource* object)
{
    if (allocation._metadata)
    {
        reinterpret_cast<Pool*>(allocation._metadata)->_objectList.erase(object);
    }
}
#endif //VULKAN_DEBUG

} //namespace vk
} //namespace renderer
} //namespace v3d
#endif //VULKAN_RENDER