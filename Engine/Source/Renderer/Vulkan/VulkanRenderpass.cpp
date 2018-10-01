#include "VulkanRenderpass.h"

#ifdef VULKAN_RENDER
namespace v3d
{
namespace renderer
{
namespace vk
{

VulkanRenderPass::VulkanRenderPass()
    : m_renderpass(VK_NULL_HANDLE)
{
}

VulkanRenderPass::~VulkanRenderPass()
{
}

VkRenderPass VulkanRenderPass::getHandle() const
{
    return m_renderpass;
}

bool VulkanRenderPass::create()
{
    return false;
}

void VulkanRenderPass::destroy()
{
}

} //namespace vk
} //namespace renderer
} //namespace v3d
#endif //VULKAN_RENDER
