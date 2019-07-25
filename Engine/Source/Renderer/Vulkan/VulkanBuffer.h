#pragma once

#include "Common.h"
#include "Renderer/BufferProperties.h"
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

        VulkanBuffer() = delete;
        VulkanBuffer(const VulkanBuffer&) = delete;

        VulkanBuffer(VulkanMemory::VulkanMemoryAllocator* memory, VkDevice device, Buffer::BufferType type, StreamBufferUsageFlags usageFlag, u64 size, const std::string& name = "") noexcept;
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
        VulkanMemory::VulkanMemoryAllocator* m_memoryAllocator;

        StreamBufferUsageFlags m_usageFlags;
        BufferType m_type;

        u64 m_size;
        VkBuffer m_buffer;

        bool m_mapped;
        
#if VULKAN_DEBUG_MARKERS
        std::string m_debugName;
        static u32 s_debugNameGenerator;
#endif //VULKAN_DEBUG_MARKERS
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace vk
} //namespace renderer
} //namespace v3d
#endif //VULKAN_RENDER
