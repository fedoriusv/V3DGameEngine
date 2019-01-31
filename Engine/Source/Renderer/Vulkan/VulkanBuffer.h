#pragma once

#include "Common.h"
#include "Renderer/Buffer.h"

#ifdef VULKAN_RENDER
#include "VulkanWrapper.h"
#include "VulkanMemory.h"
#include "VulkanResource.h"

namespace v3d
{
namespace renderer
{
namespace vk
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * VulkanBuffer class. Vulkan Render side
    */
    class VulkanBuffer final : public Buffer, public VulkanResource
    {
    public:

        VulkanBuffer(VulkanMemory* memory, VkDevice device, Buffer::BufferType type, u16 usageFlag, u64 size);
        ~VulkanBuffer();

        bool create() override;
        void destroy() override;

        bool upload(Context* context, u32 offset, u64 size, void* data) override;

        VkBuffer getHandle() const;

        void* map();
        void unmap();

    private:

        bool recreate();

        VkDevice m_device;

        VulkanMemory::VulkanAlloc m_memory;
        VulkanMemory* m_memoryManager;

        u32 m_usageFlags;
        BufferType m_type;

        u64 m_size;
        bool m_mapped;

        VkBuffer m_buffer;

        static VulkanMemory::VulkanMemoryAllocator* s_memoryAllocator;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace vk
} //namespace renderer
} //namespace v3d
#endif //VULKAN_RENDER
