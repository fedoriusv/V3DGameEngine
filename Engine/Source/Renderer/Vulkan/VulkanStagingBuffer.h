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
    * @brief VulkanStagingBufferManager final class. Vulkan Render side.
    * Multithreaded
    */
    class VulkanStagingBufferManager final
    {
    public:

        explicit VulkanStagingBufferManager(VulkanDevice* device) noexcept;
        ~VulkanStagingBufferManager();

        [[nodiscard]] VulkanBuffer* createStagingBuffer(u64 size) const;

        void destroyAfterUse(VulkanBuffer* buffer);
        void destroyStagingBuffers();

    private:

        VulkanStagingBufferManager(const VulkanStagingBufferManager&) = delete;
        VulkanStagingBufferManager& operator=(const VulkanStagingBufferManager&) = delete;

        VulkanDevice&                           m_device;
        VulkanMemory::VulkanMemoryAllocator*    m_memoryManager;

        std::vector<VulkanBuffer*>              m_stagingBuffers;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace vk
} //namespace renderer
} //namespace v3d
#endif //VULKAN_RENDER
