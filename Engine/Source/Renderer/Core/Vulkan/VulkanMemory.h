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

    class VulkanResource;

    /**
    * @brief VulkanMemory class. Render side.
    * Management of alloc/dealloc vulkan memory
    */
    class VulkanMemory final
    {
    public:

        static std::string memoryPropertyFlagToStringVK(VkMemoryPropertyFlagBits flag);

        enum MemoryProperty
        {
            dedicatedMemory = 0x01,
        };

        VulkanMemory() = delete;
        VulkanMemory(const VulkanMemory&) = delete;

        struct VulkanAllocation final
        {
            VkDeviceMemory          _memory;
            void*                   _mapped;
            void*                   _metadata;
            VkDeviceSize            _size;
            VkDeviceSize            _offset;
            VkMemoryPropertyFlags   _flag;
            u32                     _property;

            bool operator==(const VulkanAllocation& op)
            {
                return (memcmp(this, &op, sizeof(VulkanAllocation)) == 0);
            }

            bool operator!=(const VulkanAllocation& op)
            {
                return !(*this == op);
            }

        };

        /**
        * @brief VulkanMemoryAllocator class. Base allocator class
        */
        class VulkanMemoryAllocator
        {
        public:

            explicit VulkanMemoryAllocator(VkDevice device) noexcept;
            virtual ~VulkanMemoryAllocator();

#if VULKAN_DEBUG
            virtual void linkVulkanObject(const VulkanAllocation& allocation, const VulkanResource* object) {};
            virtual void unlinkVulkanObject(const VulkanAllocation& allocation, const VulkanResource* object) {};
#endif //VULKAN_DEBUG

        protected:

            VulkanMemoryAllocator() = delete;
            VulkanMemoryAllocator(const VulkanMemoryAllocator&) = delete;
            VulkanMemoryAllocator& operator=(const VulkanMemoryAllocator&) = delete;

            VkDevice m_device;

        private:

            virtual VulkanAllocation allocate(VkDeviceSize size, VkDeviceSize align, u32 memoryTypeIndex, const void* extensions = nullptr) = 0;
            virtual void deallocate(VulkanMemory::VulkanAllocation& memory) = 0;

            friend VulkanMemory;
        };

        static s32 findMemoryTypeIndex(const VkMemoryRequirements& memoryRequirements, VkMemoryPropertyFlags memoryPropertyFlags);
        static bool isSupportedMemoryType(VkMemoryPropertyFlags memoryPropertyFlags, bool isEqual);

        static VulkanMemory::VulkanAllocation s_invalidMemory;

        static VulkanAllocation allocateImageMemory(VulkanMemoryAllocator& allocator, VkImage image, VkMemoryPropertyFlags flags);
        static VulkanAllocation allocateBufferMemory(VulkanMemoryAllocator& allocator, VkBuffer buffer, VkMemoryPropertyFlags flags);

        static void freeMemory(VulkanMemoryAllocator& allocator, VulkanAllocation& memory);

    private:

        static VulkanMemoryAllocator* getSimpleMemoryAllocator(VkDevice device);

        static class SimpleVulkanMemoryAllocator* s_simpleVulkanMemoryAllocator;
        static std::recursive_mutex s_mutex;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief SimpleVulkanMemoryAllocator class. Simple allocation
    */
    class SimpleVulkanMemoryAllocator final : public VulkanMemory::VulkanMemoryAllocator
    {
    public:
        SimpleVulkanMemoryAllocator(VkDevice device) noexcept;
        ~SimpleVulkanMemoryAllocator();

    private:

        SimpleVulkanMemoryAllocator() = delete;
        SimpleVulkanMemoryAllocator(const SimpleVulkanMemoryAllocator&) = delete;

        VulkanMemory::VulkanAllocation allocate(VkDeviceSize size, VkDeviceSize align, u32 memoryTypeIndex, const void* extensions = nullptr) override;
        void deallocate(VulkanMemory::VulkanAllocation& memory) override;

#if VULKAN_DEBUG_MARKERS
        static u32 s_debugNameGenerator;
#endif //VULKAN_DEBUG_MARKERS
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief PoolVulkanMemoryAllocator class. Pool management allocation
    */
    class PoolVulkanMemoryAllocator final : public VulkanMemory::VulkanMemoryAllocator
    {
    public:

        PoolVulkanMemoryAllocator(VkDevice device, u64 allocationSize) noexcept;
        ~PoolVulkanMemoryAllocator();

#if VULKAN_DEBUG
        void linkVulkanObject(const VulkanMemory::VulkanAllocation& allocation, const VulkanResource* object) override;
        void unlinkVulkanObject(const VulkanMemory::VulkanAllocation& allocation, const VulkanResource* object) override;
#endif //VULKAN_DEBUG

    private:

        PoolVulkanMemoryAllocator() = delete;
        PoolVulkanMemoryAllocator(const PoolVulkanMemoryAllocator&) = delete;

        VulkanMemory::VulkanAllocation allocate(VkDeviceSize size, VkDeviceSize align, u32 memoryTypeIndex, const void* extensions = nullptr) override;
        void deallocate(VulkanMemory::VulkanAllocation& memory) override;

        struct MemoryChunck
        {
            VkDeviceSize _size;
            VkDeviceSize _offset;
        };

        struct Pool final
        {
            struct ChunckSizeSort
            {
                bool operator()(const MemoryChunck& l, const MemoryChunck& r) const
                {
                    if (l._size < r._size)
                    {
                        return true;
                    }
                    else if (l._size > r._size)
                    {
                        return false;
                    }

                    return l._offset < r._offset;
                }
            };

            struct ChunckOffsetSort
            {
                bool operator()(const MemoryChunck& l, const MemoryChunck& r) const
                {
                    return l._offset < r._offset;
                }
            };

            Pool() noexcept;
            ~Pool() = default;

            std::set<MemoryChunck, ChunckOffsetSort>    _chunks;
            std::set<MemoryChunck, ChunckSizeSort>      _freeChunks;
            VkDeviceMemory                              _memory;
            VkDeviceSize                                _size;
            VkDeviceSize                                _chunkSize;
            void*                                       _mapped;
            u32                                         _memoryTypeIndex;
#if VULKAN_DEBUG
            std::set<const VulkanResource*>            _objectList;
#endif //VULKAN_DEBUG
        };

        bool findAllocationFromPool(std::multimap<VkDeviceSize, Pool*>& heaps, VkDeviceSize size, VulkanMemory::VulkanAllocation& memory);
        Pool* createPool(VkDeviceSize poolSize, VkDeviceSize requestedSize, u32 memoryTypeIndex);


        VkDeviceSize m_allocationSize;
        std::multimap<VkDeviceSize, Pool*> m_heaps[VK_MAX_MEMORY_TYPES];

    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace vk
} //namespace renderer
} //namespace v3d
#endif //VULKAN_RENDER
