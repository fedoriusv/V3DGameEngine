#include "VulkanFramebuffer.h"
#include "VulkanDebug.h"
#include "Utils/Logger.h"

#ifdef VULKAN_RENDER
#include "VulkanWrapper.h"

namespace v3d
{
namespace renderer
{
namespace vk
{

VulkanFramebuffer::VulkanFramebuffer(VkDevice device, VkRenderPass pass, const std::vector<VkImageView>& images, const core::Dimension2D& size)
    : m_device(device)
    , m_renderpass(pass)
    , m_images(images) //std::move
    , m_size(size)

    , m_framebuffer(VK_NULL_HANDLE)
{
    LOG_DEBUG("VulkanFramebuffer::VulkanFramebuffer constructor %llx", this);
}

VulkanFramebuffer::~VulkanFramebuffer()
{
    LOG_DEBUG("VulkanFramebuffer::VulkanFramebuffer destructor %llx", this);
    ASSERT(!m_framebuffer, "framebuffer is not nullptr");
}

VkFramebuffer VulkanFramebuffer::getHandle() const
{
    return m_framebuffer;
}

bool VulkanFramebuffer::create()
{
    VkFramebufferCreateInfo framebufferCreateInfo = {};
    framebufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebufferCreateInfo.pNext = nullptr;
    framebufferCreateInfo.flags = 0;
    framebufferCreateInfo.renderPass = m_renderpass;
    framebufferCreateInfo.attachmentCount = m_images.size();
    framebufferCreateInfo.pAttachments = m_images.data();
    framebufferCreateInfo.width = m_size.width;
    framebufferCreateInfo.height = m_size.height;
    framebufferCreateInfo.layers = 1;

    VkResult result = VulkanWrapper::CreateFramebuffer(m_device, &framebufferCreateInfo, VULKAN_ALLOCATOR, &m_framebuffer);
    if (result != VK_SUCCESS)
    {
        LOG_DEBUG("VulkanFramebuffer::create vkCreateFramebuffer is failed. Error: %s", ErrorString(result).c_str());
        return false;
    }

    return false;
}

void VulkanFramebuffer::destroy()
{
    VulkanWrapper::DestroyFramebuffer(m_device, m_framebuffer, VULKAN_ALLOCATOR);
    m_framebuffer = nullptr;
}

} //namespace vk
} //namespace renderer
} //namespace v3d
#endif //VULKAN_RENDER
