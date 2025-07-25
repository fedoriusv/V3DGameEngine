#pragma once

#include "Common.h"
#include "Renderer/Buffer.h"

#ifdef VULKAN_RENDER
# include "VulkanWrapper.h"
# include "VulkanMemory.h"
# include "VulkanResource.h"

namespace v3d
{
namespace renderer
{
namespace vk
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    class VulkanDevice;

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief VulkanBuffer final class. Vulkan Render side
    */
    class VulkanBuffer final : public RenderBuffer, public VulkanResource
    {
    public:

        explicit VulkanBuffer(VulkanDevice* device, VulkanMemory::VulkanMemoryAllocator* alloc, RenderBuffer::Type type, u64 size, BufferUsageFlags usage, const std::string& name = "") noexcept;
        ~VulkanBuffer();

        bool create() override;
        void destroy() override;

        bool hasUsageFlag(BufferUsage usage) const override;

        bool upload(VulkanCommandBuffer* cmdBuffer, u32 offset, u64 size, const void* data);
        bool readback(VulkanCommandBuffer* cmdBuffer, u32 offset,u64 size, VulkanBuffer*& stagingBuffer);

        void clear(VulkanCommandBuffer* cmdBuffer, u32 value);

        VkBuffer getHandle() const;
        u64 getSize() const;

        void* map(u32 offset = ~1, u32 size = ~1) override;
        void unmap(u32 offset = ~1, u32 size = ~1) override;

#if DEBUG_OBJECT_MEMORY
        static std::set<VulkanBuffer*> s_objects;
#endif //DEBUG_OBJECT_MEMORY

    private:

        VulkanBuffer() = delete;
        VulkanBuffer(const VulkanBuffer&) = delete;

        bool createViewBuffer();

        VulkanDevice&                        m_device;
        VulkanMemory::VulkanMemoryAllocator* m_memoryAllocator;

        VkBuffer                             m_buffer;
        VulkanMemory::VulkanAllocation       m_memory;

        RenderBuffer::Type                   m_type;
        u64                                  m_size;
        BufferUsageFlags                     m_usageFlags;

        bool                                 m_mapped;
        
#if VULKAN_DEBUG_MARKERS
        std::string                          m_debugName;
#endif //VULKAN_DEBUG_MARKERS
    };

    inline VkBuffer VulkanBuffer::getHandle() const
    {
        ASSERT(m_buffer != VK_NULL_HANDLE, "nullptr");
        return m_buffer;
    }

    inline u64 VulkanBuffer::getSize() const
    {
        return m_size;
    }

    inline bool VulkanBuffer::hasUsageFlag(BufferUsage usage) const
    {
        return m_usageFlags & usage;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace vk
} //namespace renderer
} //namespace v3d
#endif //VULKAN_RENDER
