#include "VulkanFramebuffer.h"

#include "Utils/Logger.h"

#ifdef VULKAN_RENDER
#include "VulkanDebug.h"
#include "VulkanImage.h"
#include "VulkanRenderpass.h"
#include "VulkanSwapchain.h"
#include "VulkanDevice.h"

namespace v3d
{
namespace renderer
{
namespace vk
{

VulkanFramebuffer::VulkanFramebuffer(VulkanDevice* device, const std::vector<TextureHandle>& images, const math::Dimension2D& size, const std::string& name) noexcept
    : m_device(*device)
    , m_images(images)
    , m_size(size)

    , m_framebuffer(VK_NULL_HANDLE)
{
#if VULKAN_DEBUG
    LOG_DEBUG("VulkanFramebuffer::VulkanFramebuffer constructor %llx", this);
#endif //VULKAN_DEBUG
#if VULKAN_DEBUG_MARKERS
    m_debugName = name.empty() ? "Framebuffer" : name;
    m_debugName.append(VulkanDebugUtils::k_addressPreffix);
    m_debugName.append(std::to_string(reinterpret_cast<const u64>(this)));
#endif //VULKAN_DEBUG_MARKERS
}

VulkanFramebuffer::~VulkanFramebuffer()
{
#if VULKAN_DEBUG
    LOG_DEBUG("VulkanFramebuffer::VulkanFramebuffer destructor %llx", this);
#endif //VULKAN_DEBUG
    ASSERT(!m_framebuffer, "framebuffer is not nullptr");
}

bool VulkanFramebuffer::create(const VulkanRenderPass* pass)
{
    ASSERT(!m_framebuffer, "framebuffer is not nullptr");
    ASSERT(m_device.getVulkanDeviceCaps().getPhysicalDeviceLimits().maxFramebufferWidth >= m_size._width &&
        m_device.getVulkanDeviceCaps().getPhysicalDeviceLimits().maxFramebufferHeight >= m_size._height, "maxFramebufferSize is over range");

    m_imageViews.reserve(pass->getCountAttachments());
    for (u32 attach = 0; attach < pass->getCountAttachments(); ++attach)
    {
        const VulkanImage* vkImage = static_cast<VulkanImage*>(objectFromHandle<RenderTexture>(m_images[attach]));
        const VulkanRenderPass::VulkanAttachmentDescription& desc = pass->getAttachmentDescription(attach);
        m_imageViews.push_back(vkImage->getImageView(VulkanImage::makeVulkanImageSubresource(vkImage, desc._layer, desc._mip)));
#if VULKAN_DEBUG
        LOG_DEBUG("VulkanFramebuffer::create Framebuffer area (width %u, height %u), image (width %u, height %u)", m_size.m_width, m_size.m_height, vkImage->getSize().width, vkImage->getSize().height);
#endif

        if (VulkanImage::isColorFormat(vkImage->getFormat()))
        {
            if (vkImage->getResolveImage())
            {
                ASSERT(vkImage->getSampleCount() > VK_SAMPLE_COUNT_1_BIT, "wrong sample count");
                const VulkanImage* vkResolveImage = vkImage->getResolveImage();
                m_imageViews.push_back(vkResolveImage->getImageView(VulkanImage::makeVulkanImageSubresource(vkImage, desc._layer, desc._mip)));
            }
        }
        else
        {
            if (m_device.getVulkanDeviceCaps()._supportDepthAutoResolve && vkImage->getResolveImage())
            {
                ASSERT(vkImage->getSampleCount() > VK_SAMPLE_COUNT_1_BIT, "wrong sample count");
                const VulkanImage* vkResolveImage = vkImage->getResolveImage();
                m_imageViews.push_back(vkResolveImage->getImageView(VulkanImage::makeVulkanImageSubresource(vkImage, desc._layer, desc._mip)));
            }
        }
    }

    VkFramebufferCreateInfo framebufferCreateInfo = {};
    framebufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebufferCreateInfo.pNext = nullptr;
    framebufferCreateInfo.flags = 0;
    framebufferCreateInfo.renderPass = pass->getHandle();
    framebufferCreateInfo.attachmentCount = static_cast<u32>(m_imageViews.size());
    framebufferCreateInfo.pAttachments = m_imageViews.data();
    framebufferCreateInfo.width = m_size._width;
    framebufferCreateInfo.height = m_size._height;
    framebufferCreateInfo.layers = 1; //TODO: fill count layers

    ASSERT(m_framebuffer == VK_NULL_HANDLE, "not empty");
    VkResult result = VulkanWrapper::CreateFramebuffer(m_device.getDeviceInfo()._device, &framebufferCreateInfo, VULKAN_ALLOCATOR, &m_framebuffer);
    if (result != VK_SUCCESS)
    {
        LOG_ERROR("VulkanFramebuffer::create vkCreateFramebuffer is failed. Error: %s", ErrorString(result).c_str());
        return false;
    }

#if VULKAN_DEBUG_MARKERS
    if (m_device.getVulkanDeviceCaps()._debugUtilsObjectNameEnabled)
    {
        VkDebugUtilsObjectNameInfoEXT debugUtilsObjectNameInfo = {};
        debugUtilsObjectNameInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
        debugUtilsObjectNameInfo.pNext = nullptr;
        debugUtilsObjectNameInfo.objectType = VK_OBJECT_TYPE_FRAMEBUFFER;
        debugUtilsObjectNameInfo.objectHandle = reinterpret_cast<u64>(m_framebuffer);
        debugUtilsObjectNameInfo.pObjectName = m_debugName.c_str();

        VulkanWrapper::SetDebugUtilsObjectName(m_device.getDeviceInfo()._device, &debugUtilsObjectNameInfo);
    }
#endif //VULKAN_DEBUG_MARKERS

#if VULKAN_DEBUG
    LOG_DEBUG("VulkanFramebuffer::create Framebuffer has been created %llx. Size (width %u, height %u) ", m_framebuffer, m_size.m_width, m_size.m_height);
#endif
    return true;
}

void VulkanFramebuffer::destroy()
{
    m_imageViews.clear();

    if (m_framebuffer)
    {
        VulkanWrapper::DestroyFramebuffer(m_device.getDeviceInfo()._device, m_framebuffer, VULKAN_ALLOCATOR);
        m_framebuffer = VK_NULL_HANDLE;
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////

VulkanFramebufferManager::VulkanFramebufferManager(VulkanDevice* device) noexcept
    : m_device(*device)
{
}

VulkanFramebufferManager::~VulkanFramebufferManager()
{
    VulkanFramebufferManager::clear();
}

std::tuple<VulkanFramebuffer*, bool> VulkanFramebufferManager::acquireFramebuffer(const VulkanRenderPass* renderpass, const FramebufferDesc& description, const std::string& name)
{
    std::vector<TextureHandle> images;
    images.reserve(renderpass->getCountAttachments());
    auto buildFramebufferDescription = [&](VulkanFramebufferDesc& desc) -> void
        {
            for (u32 index = 0; index < renderpass->getCountAttachments(); ++index)
            {
                TextureHandle image = VulkanImage::isColorFormat(renderpass->getAttachmentDescription(index)._format) ? description._images[index] : description._images.back();
                desc._renderTargetsIDs[index] = objectFromHandle<RenderTexture>(image)->ID();
                images.push_back(image);
            }
        };

    VulkanFramebufferDesc desc;
    buildFramebufferDescription(desc);


    std::lock_guard lock(m_mutex);

    auto found = m_framebufferList.find(desc);
    if (found != m_framebufferList.cend())
    {
        return std::make_tuple(found->second, false);
    }

    VulkanFramebuffer* framebuffer = V3D_NEW(VulkanFramebuffer, memory::MemoryLabel::MemoryRenderCore)(&m_device, images, description._renderArea, name);
    if (!framebuffer->create(renderpass))
    {
        framebuffer->destroy();

        ASSERT(false, "can't create framebuffer");
        return std::make_tuple(nullptr, false);
    }
    [[maybe_unused]] auto inserted = m_framebufferList.emplace(desc, framebuffer);
    ASSERT(inserted.second, "must be valid insertion");

    return std::make_tuple(framebuffer, true);
}

bool VulkanFramebufferManager::removeFramebuffer(VulkanFramebuffer* framebuffer)
{
    std::lock_guard lock(m_mutex);

    auto iter = m_framebufferList.begin();
    while (iter != m_framebufferList.end())
    {
        if (iter->second == framebuffer)
        {
            break;
        }
        ++iter;
    }
    ASSERT(iter != m_framebufferList.end(), "not found");

    VulkanFramebuffer* vkFramebuffer = iter->second;
    if (framebuffer->linked())
    {
        LOG_WARNING("FramebufferManager::removeFramebuffer framebuffer still linked, but reqested to delete");
        ASSERT(false, "framebuffer");
        return false;
    }
    m_framebufferList.erase(iter);

    framebuffer->destroy();
    V3D_DELETE(framebuffer, memory::MemoryLabel::MemoryRenderCore);

    return true;
}

void VulkanFramebufferManager::clear()
{
    std::lock_guard lock(m_mutex);

    for (auto& iter : m_framebufferList)
    {
        VulkanFramebuffer* framebuffer = iter.second;
        if (framebuffer->linked())
        {
            LOG_WARNING("FramebufferManager::clear framebuffer still linked, but reqested to delete");
            ASSERT(false, "framebuffer");
        }

        framebuffer->destroy();
        V3D_DELETE(framebuffer, memory::MemoryLabel::MemoryRenderCore);
    }
    m_framebufferList.clear();
}

} //namespace vk
} //namespace renderer
} //namespace v3d
#endif //VULKAN_RENDER
