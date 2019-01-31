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
    * VulkanStaginBuffer final class. Vulkan Render side
    */
    class VulkanStaginBuffer final
    {
    public:

        VulkanStaginBuffer(VulkanMemory::VulkanMemoryAllocator* memory, VkDevice device, u64 size, StreamBufferUsageFlags usageFlag) noexcept;
        ~VulkanStaginBuffer();

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
    * VulkanStaginBufferManager final class. Vulkan Render side
    */
    class VulkanStaginBufferManager final
    {
    public:

        explicit VulkanStaginBufferManager(VkDevice device) noexcept;
        ~VulkanStaginBufferManager();

        VulkanStaginBuffer* createStagingBuffer(u64 size, u16 usageFlag) const;

        void destroyAfterUse(VulkanStaginBuffer* buffer);

        void destroyStagingBuffers();

        VkDevice m_device;

        std::recursive_mutex m_mutex;
        std::vector<VulkanStaginBuffer*> m_stagingBuffers;

        VulkanMemory::VulkanMemoryAllocator* m_memoryManager;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace vk
} //namespace renderer
} //namespace v3d
#endif //VULKAN_RENDER
