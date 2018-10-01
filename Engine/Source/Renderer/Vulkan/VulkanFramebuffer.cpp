#include "VulkanFramebuffer.h"

#ifdef VULKAN_RENDER
namespace v3d
{
namespace renderer
{
namespace vk
{

VulkanFramebuffer::VulkanFramebuffer()
    : m_framebuffer(VK_NULL_HANDLE)
{
}

VulkanFramebuffer::~VulkanFramebuffer()
{
}

VkFramebuffer VulkanFramebuffer::getHandle() const
{
    return m_framebuffer;
}

bool VulkanFramebuffer::create()
{
    return false;
}

void VulkanFramebuffer::destroy()
{
}

} //namespace vk
} //namespace renderer
} //namespace v3d
#endif //VULKAN_RENDER
