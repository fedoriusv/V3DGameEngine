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
    class VulkanBuffer final : public renderer::RenderBuffer, public VulkanResource
    {
    public:

        explicit VulkanBuffer(VulkanDevice* device, VulkanMemory::VulkanMemoryAllocator* alloc, RenderBuffer::Type type, BufferUsageFlags usageFlag, u64 size, const std::string& name = "") noexcept;
        ~VulkanBuffer();

        bool create() override;
        void destroy() override;

        bool hasUsageFlag(BufferUsage usage) const override;

        //bool write(Context* context, u32 offset, u64 size, const void* data) override;
        //bool read(Context* context, u32 offset, u64 size, const std::function<void(u32, void*)>& readback) override;

        VkBuffer getHandle() const;

        void* map();
        void unmap();

#if DEBUG_OBJECT_MEMORY
        static std::set<VulkanBuffer*> s_objects;
#endif //DEBUG_OBJECT_MEMORY

    private:

        VulkanBuffer() = delete;
        VulkanBuffer(const VulkanBuffer&) = delete;

        VulkanDevice&                           m_device;
        VulkanMemory::VulkanAllocation          m_memory;
        VulkanMemory::VulkanMemoryAllocator*    m_memoryAllocator;

        BufferUsageFlags m_usageFlags;
        RenderBuffer::Type m_type;

        u64 m_size;
        VkBuffer m_buffer;

        bool m_mapped;
        
#if VULKAN_DEBUG_MARKERS
        std::string m_debugName;
#endif //VULKAN_DEBUG_MARKERS
    };

    inline VkBuffer VulkanBuffer::getHandle() const
    {
        ASSERT(m_buffer != VK_NULL_HANDLE, "nullptr");
        return m_buffer;
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
