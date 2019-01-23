#include "VulkanFrameResource.h"

#ifdef VULKAN_RENDER

namespace v3d
{
namespace renderer
{
namespace vk
{

FrameResource::FrameResource() noexcept
    : m_status(Status::Status_Free)
{
}

FrameResource::~FrameResource()
{
}

void FrameResource::captureInsideFrame(u64 frame)
{
    m_frame = frame;
    m_status = Status::Status_InQueue;
}

bool FrameResource::isCaptured() const
{
    return (m_status == Status_InQueue) || (m_status == Status_Submitting);
}

} //namespace vk
} //namespace renderer
} //namespace v3d
#endif //VULKAN_RENDER
