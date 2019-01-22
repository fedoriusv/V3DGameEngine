#pragma once

#include "Common.h"
#include "Utils/NonCopyable.h"

#ifdef VULKAN_RENDER
#include "VulkanWrapper.h"

namespace v3d
{
namespace renderer
{
namespace vk
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    class VulkanMemory final : utils::NonCopyable
    {
    public:

        struct VulkanAlloc
        {
            VkDeviceMemory          _memory;
            VkMemoryPropertyFlags   _flag;
            VkDeviceSize            _size;
            VkDeviceSize            _offset;
            void*                   _mapped;

            bool operator==(const VulkanAlloc& op)
            {
                return _memory == op._memory && _flag == op._flag &&
                    _size == op._size && _offset == op._offset &&
                    _mapped == op._mapped;
            }

        };

        class VulkanMemoryAllocator
        {
        public:

            VulkanMemoryAllocator() {};
            virtual ~VulkanMemoryAllocator() {};

        private:

            virtual VulkanAlloc allocate(VkDevice device, VkDeviceSize size, u32 memoryTypeIndex) = 0;
            virtual void deallocate(VkDevice device, VulkanMemory::VulkanAlloc& memory) = 0;

            friend VulkanMemory;
        };

        static s32 findMemoryTypeIndex(const VkMemoryRequirements& memoryRequirements, VkMemoryPropertyFlags memoryPropertyFlags);


        static VulkanMemory::VulkanAlloc s_invalidMemory;

        explicit VulkanMemory(VkDevice device);
        ~VulkanMemory();

        VulkanAlloc allocateImageMemory(VulkanMemoryAllocator& allocator, VkImage image, VkMemoryPropertyFlags flags);
        VulkanAlloc allocateBufferMemory(VulkanMemoryAllocator& allocator, VkBuffer buffer, VkMemoryPropertyFlags flags);

        void freeMemory(VulkanMemoryAllocator& allocator, VulkanAlloc& memory);

    private:

        VkDevice m_device;

        std::recursive_mutex m_mutex;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    class SimpleVulkanMemoryAllocator final : public VulkanMemory::VulkanMemoryAllocator
    {
    public:
        SimpleVulkanMemoryAllocator();
        ~SimpleVulkanMemoryAllocator();

    private:
        VulkanMemory::VulkanAlloc allocate(VkDevice device, VkDeviceSize size, u32 memoryTypeIndex) override;
        void deallocate(VkDevice device, VulkanMemory::VulkanAlloc& memory) override;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    class PoolVulkanMemoryAllocator final : public VulkanMemory::VulkanMemoryAllocator
    {
    public:
        PoolVulkanMemoryAllocator(u64 initializeMemSize, u64 blockSize, u64 minAllocSize, u64 hugeAllocSize);
        ~PoolVulkanMemoryAllocator();

    private:
        VulkanMemory::VulkanAlloc allocate(VkDevice device, VkDeviceSize size, u32 memoryTypeIndex) override;
        void deallocate(VkDevice device, VulkanMemory::VulkanAlloc& memory) override;

        const u64 m_initializeMemSize;
        const u64 m_blockSize;
        const u64 m_minAllocSize;
        const u64 m_hugeAllocSize;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace vk
} //namespace renderer
} //namespace v3d
#endif //VULKAN_RENDER
