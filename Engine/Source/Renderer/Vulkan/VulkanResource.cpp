#include "VulkanResource.h"

#ifdef VULKAN_RENDER
#include "VulkanCommandBuffer.h"
namespace v3d
{
namespace renderer
{
namespace vk
{

VulkanResource::VulkanResource() noexcept
    : m_status(VulkanResource::Status::Status_Free)
    , m_counter(0)
    , m_frame(0)
{
}

VulkanResource::~VulkanResource()
{
    ASSERT(!isCaptured(), "still captured");
}

VulkanResource::Status VulkanResource::getStatus() const
{
    return m_status;
}

bool VulkanResource::isCaptured() const
{
    return m_status == VulkanResource::Status::Status_Captured;
}

bool VulkanResource::waitComplete(u64 time)
{
    if (isCaptured())
    {
        bool result = true;
        for (auto& cmdBuffer : m_cmdBuffers)
        {
            if (!cmdBuffer->waitComplete(time))
            {
                result = false;
            }
        }
        return result;
    }

    return true;
}

void VulkanResource::captureInsideCommandBuffer(VulkanCommandBuffer* buffer, u64 frame) const
{
    buffer->captureResource(const_cast<VulkanResource*>(this), frame);
}

} //namespace vk
} //namespace renderer
} //namespace v3d
#endif //VULKAN_RENDER
