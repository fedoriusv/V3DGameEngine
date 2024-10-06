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
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    class VulkanDevice;
    class VulkanMemory;

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    *  @brief VulkanStagingBuffer final class. Vulkan Render side
    */
    class VulkanStagingBuffer final
    {
    public:

        explicit VulkanStagingBuffer(VulkanDevice* device, VulkanMemory::VulkanMemoryAllocator* memory, u64 size, BufferUsageFlags usageFlag) noexcept;
        ~VulkanStagingBuffer();

        bool create();
        void destroy();

        void* map();
        void unmap();

        VulkanBuffer* getBuffer() const;

    private:

        VulkanStagingBuffer(const VulkanStagingBuffer&) = delete;
        VulkanStagingBuffer& operator=(const VulkanStagingBuffer&) = delete;

        VulkanBuffer* m_buffer;
    };

    inline bool VulkanStagingBuffer::create()
    {
        ASSERT(m_buffer, "nullptr");
        return m_buffer->create();
    }

    inline void VulkanStagingBuffer::destroy()
    {
        ASSERT(m_buffer, "nullptr");
        m_buffer->destroy();
    }

    inline void* VulkanStagingBuffer::map()
    {
        ASSERT(m_buffer, "nullptr");
        return m_buffer->map();
    }

    inline void VulkanStagingBuffer::unmap()
    {
        ASSERT(m_buffer, "nullptr");
        m_buffer->unmap();
    }

    inline VulkanBuffer* VulkanStagingBuffer::getBuffer() const
    {
        ASSERT(m_buffer, "nullptr");
        return m_buffer;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    *  @brief VulkanStagingBufferManager final class. Vulkan Render side
    */
    class VulkanStagingBufferManager final
    {
    public:

        explicit VulkanStagingBufferManager(VulkanDevice* device) noexcept;
        ~VulkanStagingBufferManager();

        VulkanStagingBuffer* createStagingBuffer(u64 size, BufferUsageFlags usageFlag) const;

        void destroyAfterUse(VulkanStagingBuffer* buffer);
        void destroyStagingBuffers();

    private:

        VulkanStagingBufferManager(const VulkanStagingBufferManager&) = delete;
        VulkanStagingBufferManager& operator=(const VulkanStagingBufferManager&) = delete;

        VulkanDevice& m_device;

        std::vector<VulkanStagingBuffer*> m_stagingBuffers;
        VulkanMemory::VulkanMemoryAllocator* m_memoryManager;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace vk
} //namespace renderer
} //namespace v3d
#endif //VULKAN_RENDER
