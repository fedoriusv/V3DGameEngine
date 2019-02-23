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
    class VulkanCommandBuffer;

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * VulkanResource class. Vulkan Render side
    */
    class VulkanResource
    {
    public:

        enum Status : u32
        {
            Status_Free,
            Status_Captured,
            Status_Done
        };

        VulkanResource() noexcept;
        virtual ~VulkanResource();

        Status getStatus() const;
        bool isCaptured() const;

        bool waitComplete(u64 time = 0);

        void captureInsideCommandBuffer(VulkanCommandBuffer* buffer, u64 frame) const;

    private:

        std::atomic<Status>     m_status;
        std::atomic<s64>        m_counter;
        std::atomic<u64>        m_frame;
        std::vector<VulkanCommandBuffer*> m_cmdBuffers;

        friend VulkanCommandBuffer;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace v3d
} //namespace renderer
} //namespace v3d
#endif //VULKAN_RENDER
