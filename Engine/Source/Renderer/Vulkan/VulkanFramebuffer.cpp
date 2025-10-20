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

VulkanFramebuffer::VulkanFramebuffer(VulkanDevice* device, const std::vector<std::tuple<VulkanImage*, RenderTexture::Subresource>>& images, const VkRect2D& renderArea, const std::string& name) noexcept
    : m_device(*device)
    , m_renderArea(renderArea)
    , m_images(images)
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
    ASSERT(m_device.getVulkanDeviceCaps().getPhysicalDeviceLimits().maxFramebufferWidth >= m_renderArea.extent.width &&
        m_device.getVulkanDeviceCaps().getPhysicalDeviceLimits().maxFramebufferHeight >= m_renderArea.extent.height, "maxFramebufferSize is over range");

    std::vector<VkImageView> imageViews;
    u32 layers = 1;
    imageViews.reserve(pass->getCountAttachments());
    for (u32 attach = 0; attach < pass->getCountAttachments(); ++attach)
    {
        auto& [vkImage, subresource] = m_images[attach];
        layers = std::max(layers, subresource._layers);
        imageViews.push_back(vkImage->getImageView(subresource));
#if VULKAN_DEBUG
        LOG_DEBUG("VulkanFramebuffer::create Framebuffer area (width %u, height %u), image (width %u, height %u)", m_size.m_width, m_size.m_height, vkImage->getSize().width, vkImage->getSize().height);
#endif

        if (VulkanImage::isColorFormat(vkImage->getFormat()))
        {
            if (vkImage->getResolveImage())
            {
                ASSERT(vkImage->getSampleCount() > VK_SAMPLE_COUNT_1_BIT, "wrong sample count");
                const VulkanImage* vkResolveImage = vkImage->getResolveImage();
                imageViews.push_back(vkResolveImage->getImageView(subresource));
            }
        }
        else
        {
            if (m_device.getVulkanDeviceCaps()._supportDepthAutoResolve && vkImage->getResolveImage())
            {
                ASSERT(vkImage->getSampleCount() > VK_SAMPLE_COUNT_1_BIT, "wrong sample count");
                const VulkanImage* vkResolveImage = vkImage->getResolveImage();
                imageViews.push_back(vkResolveImage->getImageView(subresource));
            }
        }
    }

    VkFramebufferCreateInfo framebufferCreateInfo = {};
    framebufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebufferCreateInfo.pNext = nullptr;
    framebufferCreateInfo.flags = 0;
    framebufferCreateInfo.renderPass = pass->getHandle();
    framebufferCreateInfo.attachmentCount = static_cast<u32>(imageViews.size());
    framebufferCreateInfo.pAttachments = imageViews.data();
    framebufferCreateInfo.width = std::max(m_renderArea.extent.width - m_renderArea.offset.x, 1U);
    framebufferCreateInfo.height = std::max(m_renderArea.extent.height - m_renderArea.offset.x, 1U);
    framebufferCreateInfo.layers = layers;

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
    m_images.clear();
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

std::tuple<VulkanFramebuffer*, bool> VulkanFramebufferManager::acquireFramebuffer(const VulkanRenderPass* renderpass, const FramebufferDesc& framebufferDesc, const std::string& name)
{
    std::vector<std::tuple<VulkanImage*, RenderTexture::Subresource>> images;
    images.reserve(renderpass->getCountAttachments());
    auto buildFramebufferDescription = [&](VulkanFramebufferDesc& desc) -> void
        {
            for (u32 index = 0; index < renderpass->getCountAttachments(); ++index)
            {
                Texture* texture = VulkanImage::isColorFormat(renderpass->getAttachmentDescription(index)._format) ? framebufferDesc._imageViews[index]._texture : framebufferDesc._imageViews.back()._texture;
                VulkanImage* vkImage = nullptr;
                const RenderTexture::Subresource& subresource = VulkanImage::isColorFormat(renderpass->getAttachmentDescription(index)._format) ? framebufferDesc._imageViews[index]._subresource : framebufferDesc._imageViews.back()._subresource;
                if (texture->hasUsageFlag(TextureUsage::TextureUsage_Backbuffer))
                {
                    VulkanSwapchain* swapchain = static_cast<VulkanSwapchain*>(objectFromHandle<Swapchain>(texture->getTextureHandle()));
                    vkImage = swapchain->getCurrentSwapchainImage();
                }
                else
                {
                    vkImage = static_cast<VulkanImage*>(objectFromHandle<RenderTexture>(texture->getTextureHandle()));
                }

                desc._renderTargets[index] = std::make_tuple(vkImage->ID(), subresource);
                images.emplace_back(vkImage, subresource);
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

    VkRect2D renderArea{ { 0, 0 }, { framebufferDesc._renderArea._width, framebufferDesc._renderArea._height } };
    VulkanFramebuffer* framebuffer = V3D_NEW(VulkanFramebuffer, memory::MemoryLabel::MemoryRenderCore)(&m_device, images, renderArea, name);
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
