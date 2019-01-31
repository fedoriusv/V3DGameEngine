#pragma once

#include "Common.h"
#include "Utils/Singleton.h"

#ifdef VULKAN_RENDER
#include "VulkanWrapper.h"
#include "VulkanBuffer.h"

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

        VulkanStaginBuffer(VulkanMemory* memory, VkDevice device, u64 size, u16 usageFlag) noexcept;
        ~VulkanStaginBuffer();

        bool create();
        void destroy();

        void* map();
        void unmap();

        VulkanBuffer* getBuffer() const;

    private:

        VkDevice        m_device;
        VulkanBuffer*   m_buffer;
        VulkanMemory*   m_memoryManager;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * VulkanStaginBufferManager final class. Vulkan Render side
    */
    class VulkanStaginBufferManager final
    {
    public:

        VulkanStaginBufferManager(VkDevice device);
        ~VulkanStaginBufferManager();

        VulkanStaginBuffer* createStagingBuffer(u64 size, u16 usageFlag) const;

        void destroyAfterUse(VulkanStaginBuffer* buffer);

        void destroyStagingBuffers();

        VkDevice m_device;

        std::recursive_mutex m_mutex;
        std::vector<VulkanStaginBuffer*> m_stagingBuffers;

        class VulkanMemory* m_memoryManager;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace vk
} //namespace renderer
} //namespace v3d
#endif //VULKAN_RENDER
