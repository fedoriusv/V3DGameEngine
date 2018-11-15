#include "VulkanMemory.h"
#include "VulkanDebug.h"
#include "VulkanDeviceCaps.h"
#include "VulkanGraphicContext.h"

#include "Utils/Logger.h"

#ifdef VULKAN_RENDER
#include "VulkanWrapper.h"

namespace v3d
{
namespace renderer
{
namespace vk
{

VulkanMemory::VulkanAlloc VulkanMemory::s_invalidMemory = 
{
    VK_NULL_HANDLE,
    0,
    0,
    nullptr
};

s32 VulkanMemory::findMemoryTypeIndex(const VkMemoryRequirements& memoryRequirements, VkMemoryPropertyFlags memoryPropertyFlags)
{
    const VkPhysicalDeviceMemoryProperties& physicalDeviceMemoryProperties = VulkanDeviceCaps::getInstance()->getDeviceMemoryProperties();
    u32 memoryTypeBits = memoryRequirements.memoryTypeBits;
    for (u32 memoryTypeIndex = 0; memoryTypeIndex < VK_MAX_MEMORY_TYPES; ++memoryTypeIndex)
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

VulkanMemory::VulkanMemory(VkDevice device)
    : m_device(device)
{
}

VulkanMemory::~VulkanMemory()
{
}

VulkanMemory::VulkanAlloc VulkanMemory::allocateImageMemory(VulkanMemoryAllocator& allocator, VkImage image, VkMemoryPropertyFlags flags)
{
    if (image == VK_NULL_HANDLE)
    {
        LOG_ERROR("VulkanMemory::allocateImageMemory: invalid image");
        ASSERT(false, "invalid image");

        return VulkanMemory::s_invalidMemory;
    }

    {
        std::lock_guard<std::recursive_mutex> lock(m_mutex);

        VkMemoryRequirements memoryRequirements = {};
        VulkanWrapper::GetImageMemoryRequirements(m_device, image, &memoryRequirements);

        s32 memoryTypeIndex = VulkanMemory::findMemoryTypeIndex(memoryRequirements, flags);
        if (memoryTypeIndex < 0)
        {
            LOG_ERROR("VulkanMemory::allocateImageMemory: invalid memoryTypeIndex");
            ASSERT(false, "invalid memoryTypeIndex");

            return  VulkanMemory::s_invalidMemory;
        }

        VulkanAlloc memory = allocator.allocate(m_device, memoryRequirements.size, memoryTypeIndex);
        if (memory._memory == VK_NULL_HANDLE)
        {
            return VulkanMemory::s_invalidMemory;
        }

        if (flags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)
        {
            VkResult result = VulkanWrapper::MapMemory(m_device, memory._memory, 0, VK_WHOLE_SIZE, 0, &memory._mapped);
            if (result != VK_SUCCESS)
            {
                LOG_ERROR("VulkanMemory::allocateImageMemory: vkMapMemory. Error %s", ErrorString(result).c_str());
                allocator.deallocate(m_device, memory);

                return VulkanMemory::s_invalidMemory;
            }
        }

        VkResult result = VulkanWrapper::BindImageMemory(m_device, image, memory._memory, memory._offset);
        if (result != VK_SUCCESS)
        {
            LOG_ERROR("VulkanMemory::allocateImageMemory: vkBindImageMemory. Error %s", ErrorString(result).c_str());
            allocator.deallocate(m_device, memory);

            return VulkanMemory::s_invalidMemory;;
        }

        return memory;
    }
}

VulkanMemory::VulkanAlloc VulkanMemory::allocateBufferMemory(VulkanMemoryAllocator& allocator, VkBuffer buffer, VkMemoryPropertyFlags flags)
{
    if (buffer == VK_NULL_HANDLE)
    {
        LOG_ERROR("VulkanMemory::allocateBufferMemory: invalid buffer");
        ASSERT(false, "invalid buffer");

        return VulkanMemory::s_invalidMemory;
    }

    {
        std::lock_guard<std::recursive_mutex> lock(m_mutex);

        VkMemoryRequirements memoryRequirements = {};
        VulkanWrapper::GetBufferMemoryRequirements(m_device, buffer, &memoryRequirements);

        s32 memoryTypeIndex = VulkanMemory::findMemoryTypeIndex(memoryRequirements, flags);
        if (memoryTypeIndex < 0)
        {
            LOG_ERROR("VulkanMemory::allocateBufferMemory: invalid memoryTypeIndex");
            ASSERT(false, "invalid memoryTypeIndex");

            return VulkanMemory::s_invalidMemory;
        }

        VulkanAlloc memory = allocator.allocate(m_device, memoryRequirements.size, memoryTypeIndex);
        if (memory._memory == VK_NULL_HANDLE)
        {
            return VulkanMemory::s_invalidMemory;
        }

        if (flags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)
        {
            VkResult result = VulkanWrapper::MapMemory(m_device, memory._memory, 0, VK_WHOLE_SIZE, 0, &memory._mapped);
            if (result != VK_SUCCESS)
            {
                LOG_ERROR("VulkanMemory::allocateBufferMemory: vkMapMemory. Error %s", ErrorString(result).c_str());
                allocator.deallocate(m_device, memory);

                return VulkanMemory::s_invalidMemory;
            }
        }

        VkResult result = VulkanWrapper::BindBufferMemory(m_device, buffer, memory._memory, memory._offset);
        if (result != VK_SUCCESS)
        {
            LOG_ERROR("VulkanMemory::allocateBufferMemory: vkBindBufferMemory. Error %s", ErrorString(result).c_str());
            allocator.deallocate(m_device, memory);

            return  VulkanMemory::s_invalidMemory;;
        }

        return memory;
    }
}

void VulkanMemory::freeMemory(VulkanMemoryAllocator& allocator, VulkanAlloc& memory)
{
    {
        std::lock_guard<std::recursive_mutex> lock(m_mutex);

        allocator.deallocate(m_device, memory);
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////

SimpleVulkanMemoryAllocator::SimpleVulkanMemoryAllocator()
{
}

SimpleVulkanMemoryAllocator::~SimpleVulkanMemoryAllocator()
{
}

VulkanMemory::VulkanAlloc SimpleVulkanMemoryAllocator::allocate(VkDevice device, VkDeviceSize size, u32 memoryTypeIndex)
{
    VulkanMemory::VulkanAlloc memory = {};

    VkMemoryAllocateInfo memoryAllocateInfo = {};
    memoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    memoryAllocateInfo.pNext = nullptr; //TODO:
    memoryAllocateInfo.allocationSize = size;
    memoryAllocateInfo.memoryTypeIndex = memoryTypeIndex;

    VkResult result = VulkanWrapper::AllocateMemory(device, &memoryAllocateInfo, VULKAN_ALLOCATOR, &memory._memory);
    if (result != VK_SUCCESS)
    {
        LOG_ERROR("SimpleVulkanMemoryAllocator::allocate vkAllocateMemory is failed. Error: %s", ErrorString(result).c_str());
        return VulkanMemory::s_invalidMemory;
    }

    memory._size = size;
    memory._offset = 0;
    memory._mapped = nullptr;

    return memory;
}

void SimpleVulkanMemoryAllocator::deallocate(VkDevice device, VulkanMemory::VulkanAlloc& memory)
{
    if (memory._mapped)
    {
        VulkanWrapper::UnmapMemory(device, memory._memory);
    }

    if (memory._memory != VK_NULL_HANDLE)
    {
        VulkanWrapper::FreeMemory(device, memory._memory, VULKAN_ALLOCATOR);
    }
    memory = VulkanMemory::s_invalidMemory;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////

PoolVulkanMemoryAllocator::PoolVulkanMemoryAllocator(u64 initializeMemSize, u64 blockSize, u64 minAllocSize, u64 hugeAllocSize)
    : m_initializeMemSize(initializeMemSize)
    , m_blockSize(blockSize)
    , m_minAllocSize(minAllocSize)
    , m_hugeAllocSize(hugeAllocSize)
{
}

PoolVulkanMemoryAllocator::~PoolVulkanMemoryAllocator()
{
}

VulkanMemory::VulkanAlloc PoolVulkanMemoryAllocator::allocate(VkDevice device, VkDeviceSize size, u32 memoryTypeIndex)
{
    //TODO:
    ASSERT(false, "not implemented");
    return VulkanMemory::s_invalidMemory;
}

void PoolVulkanMemoryAllocator::deallocate(VkDevice device, VulkanMemory::VulkanAlloc & memory)
{
    ASSERT(false, "not implemented");
    //TODO:
}

/////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace vk
} //namespace renderer
} //namespace v3d
#endif //VULKAN_RENDER
