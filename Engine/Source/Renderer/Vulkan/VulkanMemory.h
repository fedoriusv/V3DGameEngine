#pragma once

#include "Common.h"

#ifdef VULKAN_RENDER
#include "VulkanWrapper.h"

namespace v3d
{
namespace renderer
{
namespace vk
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    class VulkanMemory final
    {
    public:

        VulkanMemory() = delete;
        VulkanMemory(const VulkanMemory&) = delete;

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

            bool operator!=(const VulkanAlloc& op)
            {
                return !(*this == op);
            }

        };

        class VulkanMemoryAllocator
        {
        public:

            VulkanMemoryAllocator() = delete;
            VulkanMemoryAllocator(const VulkanMemoryAllocator&) = delete;
            VulkanMemoryAllocator& operator=(const VulkanMemoryAllocator&) = delete;

            VulkanMemoryAllocator(VkDevice device);
            virtual ~VulkanMemoryAllocator();

        protected:

            VkDevice m_device;

        private:

            virtual VulkanAlloc allocate(VkDeviceSize size, u32 memoryTypeIndex) = 0;
            virtual void deallocate(VulkanMemory::VulkanAlloc& memory) = 0;

            friend VulkanMemory;
        };

        static s32 findMemoryTypeIndex(const VkMemoryRequirements& memoryRequirements, VkMemoryPropertyFlags memoryPropertyFlags);
        static bool isSupportedMemoryType(VkMemoryPropertyFlags memoryPropertyFlags, bool isEqual);

        static VulkanMemory::VulkanAlloc s_invalidMemory;

        static VulkanAlloc allocateImageMemory(VulkanMemoryAllocator& allocator, VkImage image, VkMemoryPropertyFlags flags);
        static VulkanAlloc allocateBufferMemory(VulkanMemoryAllocator& allocator, VkBuffer buffer, VkMemoryPropertyFlags flags);

        static void freeMemory(VulkanMemoryAllocator& allocator, VulkanAlloc& memory);

    private:

        static std::recursive_mutex s_mutex;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    class SimpleVulkanMemoryAllocator final : public VulkanMemory::VulkanMemoryAllocator
    {
    public:
        SimpleVulkanMemoryAllocator(VkDevice device);
        ~SimpleVulkanMemoryAllocator();

    private:
        VulkanMemory::VulkanAlloc allocate(VkDeviceSize size, u32 memoryTypeIndex) override;
        void deallocate(VulkanMemory::VulkanAlloc& memory) override;

#if VULKAN_DEBUG_MARKERS
        static u32 s_debugNameGenerator;
#endif //VULKAN_DEBUG_MARKERS
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    class PoolVulkanMemoryAllocator final : public VulkanMemory::VulkanMemoryAllocator
    {
    public:
        PoolVulkanMemoryAllocator(VkDevice device, u64 initializeMemSize, u64 blockSize, u64 minAllocSize, u64 hugeAllocSize);
        ~PoolVulkanMemoryAllocator();

    private:
        VulkanMemory::VulkanAlloc allocate(VkDeviceSize size, u32 memoryTypeIndex) override;
        void deallocate(VulkanMemory::VulkanAlloc& memory) override;

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
