#pragma once

#include "Common.h"

#ifdef VULKAN_RENDER
#include "VulkanWrapper.h"
#include "VulkanBuffer.h"
#include "VulkanMemory.h"

namespace v3d
{
namespace renderer
{
namespace vk
{
    class VulkanMemory;

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * VulkanStagingBuffer final class. Vulkan Render side
    */
    class VulkanStagingBuffer final
    {
    public:

        VulkanStagingBuffer(const VulkanStagingBuffer&) = delete;

        VulkanStagingBuffer(VulkanMemory::VulkanMemoryAllocator* memory, VkDevice device, u64 size, StreamBufferUsageFlags usageFlag) noexcept;
        ~VulkanStagingBuffer();

        bool create();
        void destroy();

        void* map();
        void unmap();

        VulkanBuffer* getBuffer() const;

    private:

        VulkanBuffer*   m_buffer;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * VulkanStagingBufferManager final class. Vulkan Render side
    */
    class VulkanStagingBufferManager final
    {
    public:

        VulkanStagingBufferManager(const VulkanStagingBufferManager&) = delete;

        explicit VulkanStagingBufferManager(VkDevice device) noexcept;
        ~VulkanStagingBufferManager();

        VulkanStagingBuffer* createStagingBuffer(u64 size, u16 usageFlag) const;

        void destroyAfterUse(VulkanStagingBuffer* buffer);

        void destroyStagingBuffers();

        VkDevice m_device;

        std::recursive_mutex m_mutex;
        std::vector<VulkanStagingBuffer*> m_stagingBuffers;

        VulkanMemory::VulkanMemoryAllocator* m_memoryManager;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace vk
} //namespace renderer
} //namespace v3d
#endif //VULKAN_RENDER
