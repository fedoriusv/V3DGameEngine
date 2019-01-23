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

    class FrameResource
    {
    public:

        enum Status
        {
            Status_Free,
            Status_InQueue,
            Status_Submitting,
            Status_Done
        };

        FrameResource() noexcept;
        ~FrameResource();


        void captureInsideFrame(u64 frame);
        bool isCaptured() const;


    private:

        Status           m_status;
        std::atomic<u64> m_frame;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace v3d
} //namespace renderer
} //namespace v3d
#endif //VULKAN_RENDER
