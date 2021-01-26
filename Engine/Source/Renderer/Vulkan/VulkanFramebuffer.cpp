#include "VulkanFramebuffer.h"
#include "VulkanDeviceCaps.h"
#include "VulkanRenderpass.h"
#include "VulkanImage.h"
#include "VulkanDebug.h"
#include "VulkanGraphicContext.h"
#include "VulkanSwapchain.h"


#include "Utils/Logger.h"

#ifdef VULKAN_RENDER
namespace v3d
{
namespace renderer
{
namespace vk
{

VulkanFramebuffer::VulkanFramebuffer(VkDevice device, const Context* context, const std::vector<Image*>& images, const core::Dimension2D& size, const std::string& name) noexcept
    : m_device(device)
    , m_context(context)
    , m_images(images)
    , m_size(size)

    , m_framebuffer(VK_NULL_HANDLE)
{
    LOG_DEBUG("VulkanFramebuffer::VulkanFramebuffer constructor %llx", this);
#if VULKAN_DEBUG_MARKERS
    m_debugName = name.empty() ? "Framebuffer" : name;
    m_debugName.append(VulkanDebugUtils::k_addressPreffix);
    m_debugName.append(std::to_string(reinterpret_cast<const u64>(this)));
#endif //VULKAN_DEBUG_MARKERS
}

VulkanFramebuffer::~VulkanFramebuffer()
{
    LOG_DEBUG("VulkanFramebuffer::VulkanFramebuffer destructor %llx", this);
    ASSERT(!m_framebuffer, "framebuffer is not nullptr");
}

VkFramebuffer VulkanFramebuffer::getHandle() const
{
    ASSERT(m_framebuffer != VK_NULL_HANDLE, "nullptr");
    return m_framebuffer;
}

const std::vector<Image*>& VulkanFramebuffer::getImages() const
{
    return m_images;
}

bool VulkanFramebuffer::create(const RenderPass* pass)
{
    ASSERT(!m_framebuffer, "framebuffer is not nullptr");
    ASSERT(VulkanDeviceCaps::getInstance()->getPhysicalDeviceLimits().maxFramebufferWidth >= m_size.width &&
        VulkanDeviceCaps::getInstance()->getPhysicalDeviceLimits().maxFramebufferHeight >= m_size.height, "maxFramebufferSize is over range");


    const VulkanRenderPass* vkPass = static_cast<const VulkanRenderPass*>(pass);
    m_imageViews.reserve(m_images.size());
    for (u32 attach = 0; attach < m_images.size(); ++attach)
    {
        const VulkanImage* vkImage = static_cast<const VulkanImage*>(m_images[attach]);
        m_imageViews.push_back(vkImage->getImageView(vkPass->getAttachmentDescription(attach)._layer));

        if (VulkanImage::isColorFormat(vkImage->getFormat()))
        {
            if (vkImage->getResolveImage())
            {
                ASSERT(vkImage->getSampleCount() > VK_SAMPLE_COUNT_1_BIT, "wrong sample count");
                const VulkanImage* vkResolveImage = vkImage->getResolveImage();
                m_imageViews.push_back(vkResolveImage->getImageView());
            }
        }
        else
        {
            if (VulkanDeviceCaps::getInstance()->supportDepthAutoResolve && vkImage->getResolveImage())
            {
                ASSERT(vkImage->getSampleCount() > VK_SAMPLE_COUNT_1_BIT, "wrong sample count");
                const VulkanImage* vkResolveImage = vkImage->getResolveImage();
                m_imageViews.push_back(vkResolveImage->getImageView());
            }
        }
    }

    VkFramebufferCreateInfo framebufferCreateInfo = {};
    framebufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebufferCreateInfo.pNext = nullptr;
    framebufferCreateInfo.flags = 0;
    framebufferCreateInfo.renderPass = vkPass->getHandle();
    framebufferCreateInfo.attachmentCount = static_cast<u32>(m_imageViews.size());
    framebufferCreateInfo.pAttachments = m_imageViews.data();
    framebufferCreateInfo.width = m_size.width;
    framebufferCreateInfo.height = m_size.height;
    framebufferCreateInfo.layers = 1;

    ASSERT(m_framebuffer == VK_NULL_HANDLE, "not empty");
    VkResult result = VulkanWrapper::CreateFramebuffer(m_device, &framebufferCreateInfo, VULKAN_ALLOCATOR, &m_framebuffer);
    if (result != VK_SUCCESS)
    {
        LOG_ERROR("VulkanFramebuffer::create vkCreateFramebuffer is failed. Error: %s", ErrorString(result).c_str());
        return false;
    }

#if VULKAN_DEBUG_MARKERS
    if (VulkanDeviceCaps::getInstance()->debugUtilsObjectNameEnabled)
    {
        VkDebugUtilsObjectNameInfoEXT debugUtilsObjectNameInfo = {};
        debugUtilsObjectNameInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
        debugUtilsObjectNameInfo.pNext = nullptr;
        debugUtilsObjectNameInfo.objectType = VK_OBJECT_TYPE_FRAMEBUFFER;
        debugUtilsObjectNameInfo.objectHandle = reinterpret_cast<u64>(m_framebuffer);
        debugUtilsObjectNameInfo.pObjectName = m_debugName.c_str();

        VulkanWrapper::SetDebugUtilsObjectName(m_device, &debugUtilsObjectNameInfo);
    }
#endif //VULKAN_DEBUG_MARKERS

#if VULKAN_DEBUG
    LOG_DEBUG("VulkanFramebuffer::create Framebuffer has been created %llx. Size (width %u, height %u) ", m_framebuffer, m_size.width, m_size.height);
#endif
    return true;
}

void VulkanFramebuffer::destroy()
{
    m_imageViews.clear();

    if (m_framebuffer)
    {
        VulkanWrapper::DestroyFramebuffer(m_device, m_framebuffer, VULKAN_ALLOCATOR);
        m_framebuffer = VK_NULL_HANDLE;
    }
}

} //namespace vk
} //namespace renderer
} //namespace v3d
#endif //VULKAN_RENDER
