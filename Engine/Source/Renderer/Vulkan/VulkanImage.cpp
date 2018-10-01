#include "VulkanImage.h"
#include "VulkanGraphicContext.h"
#include "VulkanCommandBufferManager.h"
#include "VulkanDebug.h"

#include "Utils/Logger.h"

#ifdef VULKAN_RENDER
namespace v3d
{
namespace renderer
{
namespace vk
{

std::tuple<VkAccessFlags, VkAccessFlags> VulkanImage::getAccessFlagsFromImageLayout(VkImageLayout oldLayout, VkImageLayout newLayout)
{
    VkAccessFlags srcFlag = 0;
    VkAccessFlags dstFlag = 0;

    switch (oldLayout)
    {
    case VK_IMAGE_LAYOUT_UNDEFINED:
        srcFlag = 0;
        break;

    case VK_IMAGE_LAYOUT_PREINITIALIZED:
        srcFlag = VK_ACCESS_HOST_WRITE_BIT;
        break;

    case VK_IMAGE_LAYOUT_GENERAL:
        srcFlag |= VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;
        srcFlag |= VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        srcFlag |= VK_ACCESS_TRANSFER_WRITE_BIT;
        srcFlag |= VK_ACCESS_TRANSFER_READ_BIT;
        srcFlag |= VK_ACCESS_SHADER_READ_BIT;
        srcFlag |= VK_ACCESS_SHADER_WRITE_BIT;
        break;

    case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
        srcFlag |= VK_ACCESS_TRANSFER_READ_BIT;
        srcFlag |= VK_ACCESS_TRANSFER_WRITE_BIT;
        break;

    case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
        srcFlag |= VK_ACCESS_TRANSFER_READ_BIT;
        srcFlag |= VK_ACCESS_TRANSFER_WRITE_BIT;
        break;

    case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
        srcFlag = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        break;

    case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
        srcFlag = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
        break;

    case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
        srcFlag = VK_ACCESS_SHADER_READ_BIT;
        break;

    case VK_IMAGE_LAYOUT_PRESENT_SRC_KHR:
        dstFlag = VK_ACCESS_MEMORY_READ_BIT;
        break;
    }

    switch (newLayout)
    {
    case VK_IMAGE_LAYOUT_GENERAL:
        dstFlag |= VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;
        dstFlag |= VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        dstFlag |= VK_ACCESS_TRANSFER_WRITE_BIT;
        dstFlag |= VK_ACCESS_TRANSFER_READ_BIT;
        dstFlag |= VK_ACCESS_SHADER_READ_BIT;
        dstFlag |= VK_ACCESS_SHADER_WRITE_BIT;
        break;

    case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
        dstFlag |= VK_ACCESS_TRANSFER_WRITE_BIT;
        dstFlag |= VK_ACCESS_TRANSFER_READ_BIT;
        break;

    case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
        dstFlag |= VK_ACCESS_TRANSFER_WRITE_BIT;
        dstFlag |= VK_ACCESS_TRANSFER_READ_BIT;
        break;

    case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
        dstFlag = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        break;

    case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
        dstFlag = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
        break;

    case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
        dstFlag = VK_ACCESS_SHADER_READ_BIT;
        break;

    case VK_IMAGE_LAYOUT_PRESENT_SRC_KHR:
        dstFlag = VK_ACCESS_MEMORY_READ_BIT;
        break;
    }

    return { srcFlag, dstFlag };
}

VulkanImage::VulkanImage(VkDevice device, VkImageType type, VkFormat format, VkExtent3D dimension, u32 mipsLevel, VkImageTiling tiling)
    : m_device(device)
    , m_type(type)
    , m_format(format)
    , m_dimension(dimension)
    , m_mipsLevel(mipsLevel)
    , m_layersLevel(1)

    , m_aspectMask(VulkanImage::getImageAspectFlags(format))

    , m_samples(VK_SAMPLE_COUNT_1_BIT)
    , m_tiling(VK_IMAGE_TILING_OPTIMAL)

    , m_image(VK_NULL_HANDLE)
    , m_imageView(VK_NULL_HANDLE)

    , m_layout((m_tiling == VK_IMAGE_TILING_OPTIMAL) ? VK_IMAGE_LAYOUT_UNDEFINED : VK_IMAGE_LAYOUT_PREINITIALIZED)
    , m_usage(VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT)
{
    LOG_DEBUG("VulkanImage::VulkanImage constructor %llx", this);
}

VulkanImage::VulkanImage(VkDevice device, VkFormat format, VkExtent3D dimension, VkSampleCountFlagBits samples)
    : m_device(device)
    , m_type(VK_IMAGE_TYPE_2D)
    , m_format(format)
    , m_dimension(dimension)
    , m_mipsLevel(1)
    , m_layersLevel(1)

    , m_aspectMask(VulkanImage::getImageAspectFlags(format))

    , m_samples(samples)
    , m_tiling(VK_IMAGE_TILING_OPTIMAL)

    , m_image(VK_NULL_HANDLE)
    , m_imageView(VK_NULL_HANDLE)

    , m_layout(VK_IMAGE_LAYOUT_UNDEFINED)
    , m_usage(VK_IMAGE_USAGE_TRANSFER_SRC_BIT)
{
    if (VulkanImage::isColorFormat(format))
    {
        m_usage |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    }
    else
    {
        m_usage |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    }

    LOG_DEBUG("VulkanImage::VulkanImage constructor %llx", this);
}

VulkanImage::~VulkanImage()
{
    LOG_DEBUG("VulkanImage::VulkanImage destructor %llx", this);

    ASSERT(!m_imageView, "m_imageView is not nullptr");
    ASSERT(!m_image, "image not nullptr");
}

bool VulkanImage::create()
{
    ASSERT(!m_image, "image already created");

    VkImageCreateInfo imageCreateInfo = {};
    imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageCreateInfo.pNext = nullptr;
    imageCreateInfo.flags = 0;
    if (m_layersLevel == 6U)
    {
        imageCreateInfo.flags |= VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
    }
    if (m_format == VK_FORMAT_UNDEFINED)
    {
        imageCreateInfo.flags |= VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT;
    }

    imageCreateInfo.imageType = m_type;
    imageCreateInfo.format = m_format;
    imageCreateInfo.extent = m_dimension;
    imageCreateInfo.mipLevels = m_mipsLevel;
    imageCreateInfo.arrayLayers = m_layersLevel;

    imageCreateInfo.samples = m_samples;
    imageCreateInfo.tiling = m_tiling;
    imageCreateInfo.usage = m_usage;

    imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    imageCreateInfo.queueFamilyIndexCount = 0;
    imageCreateInfo.pQueueFamilyIndices = nullptr;

    imageCreateInfo.initialLayout = m_layout;

    VkResult result = VulkanWrapper::CreateImage(m_device, &imageCreateInfo, VULKAN_ALLOCATOR, &m_image);
    if (result != VK_SUCCESS)
    {
        LOG_DEBUG("VulkanImage::VulkanImage vkCreateImage is failed. Error: %s", ErrorString(result).c_str());
        return false;
    }

    if (!createViewImage())
    {
        VulkanImage::destroy();

        LOG_ERROR("VulkanImage::VulkanImage::create(img) is failed");
        return false;
    }

    return true;
}

bool VulkanImage::create(VkImage image)
{
    ASSERT(image, "image is nullptr");
    ASSERT(!m_image, "m_image already exist");
    m_image = image;

    if (!createViewImage())
    {
        LOG_ERROR("VulkanImage::VulkanImage::create(img) is failed");
        return false;
    }

    return true;
}

void VulkanImage::clear(const Context * context, const core::Vector4D & color)
{
    LOG_DEBUG("VulkanGraphicContext::clearColor [%f, %f, %f, %f]", color[0], color[1], color[2], color[3]);
    VkClearColorValue clearColorValue = { color[0], color[1], color[2], color[3] };

    const VulkanGraphicContext* vulkanContext = static_cast<const VulkanGraphicContext*>(context);
    VulkanCommandBuffer* commandBuffer = vulkanContext->getCurrentBuffer(VulkanCommandBufferManager::CommandTargetType::CmdDrawBuffer);
    ASSERT(commandBuffer, "commandBuffer is nullptr");

    VkImageLayout layout = m_layout;
    vulkanContext->transferImageLayout(this,VK_PIPELINE_STAGE_TRANSFER_BIT | VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    commandBuffer->cmdClearImage(this, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, &clearColorValue);
    vulkanContext->transferImageLayout(this,VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT | VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, layout);
}

void VulkanImage::clear(const Context * context, f32 depth, u32 stencil)
{
    LOG_DEBUG("VulkanGraphicContext::clearDepthStencil [%f, %u]", depth, stencil);
    VkClearDepthStencilValue clearDepthStencilValue = { depth, stencil };

    const VulkanGraphicContext* vulkanContext = static_cast<const VulkanGraphicContext*>(context);
    VulkanCommandBuffer* commandBuffer = vulkanContext->getCurrentBuffer(VulkanCommandBufferManager::CommandTargetType::CmdDrawBuffer);
    ASSERT(commandBuffer, "commandBuffer is nullptr");

    VkImageLayout layout = m_layout;
    vulkanContext->transferImageLayout(this, VK_PIPELINE_STAGE_TRANSFER_BIT | VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    commandBuffer->cmdClearImage(this, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, &clearDepthStencilValue);
    vulkanContext->transferImageLayout(this, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT | VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, layout);
}

VkFormat VulkanImage::convertImageFormatToVkFormat(renderer::ImageFormat format)
{
    return VK_FORMAT_UNDEFINED;
}

VkImageType VulkanImage::convertTextureTargetToVkImageType(TextureTarget target)
{
    return VK_IMAGE_TYPE_2D;
}

VkSampleCountFlagBits VulkanImage::convertRenderTargetSamplesToVkSampleCount(TextureSamples samples)
{
    switch (samples)
    {
    case TextureSamples::SampleCount_x1:
        return VK_SAMPLE_COUNT_1_BIT;

    case TextureSamples::SampleCount_x2:
        return VK_SAMPLE_COUNT_2_BIT;

    case TextureSamples::SampleCount_x4:
        return VK_SAMPLE_COUNT_4_BIT;

    case TextureSamples::SampleCount_x8:
        return VK_SAMPLE_COUNT_8_BIT;

    case TextureSamples::SampleCount_x16:
        return VK_SAMPLE_COUNT_16_BIT;

    case TextureSamples::SampleCount_x32:
        return VK_SAMPLE_COUNT_32_BIT;

    case TextureSamples::SampleCount_x64:
        return VK_SAMPLE_COUNT_64_BIT;

    default:
        ASSERT(false, "cant convert");
    }

    return VK_SAMPLE_COUNT_1_BIT;
}

VkImageSubresourceRange VulkanImage::makeImageSubresourceRange(const VulkanImage * image)
{
    VkImageSubresourceRange imageSubresourceRange = {};
    imageSubresourceRange.aspectMask = image->m_aspectMask;
    imageSubresourceRange.baseMipLevel = 0;
    imageSubresourceRange.levelCount = image->m_mipsLevel;
    imageSubresourceRange.baseArrayLayer = 0;
    imageSubresourceRange.layerCount = image->m_layersLevel;

    return imageSubresourceRange;
}

VkImageAspectFlags VulkanImage::getImageAspectFlags(VkFormat format)
{
    switch (format)
    {
    case VK_FORMAT_D16_UNORM:
    case VK_FORMAT_X8_D24_UNORM_PACK32:
    case VK_FORMAT_D32_SFLOAT:
        return VK_IMAGE_ASPECT_DEPTH_BIT;

    case VK_FORMAT_S8_UINT:
        return VK_IMAGE_ASPECT_STENCIL_BIT;

    case VK_FORMAT_D16_UNORM_S8_UINT:
    case VK_FORMAT_D24_UNORM_S8_UINT:
    case VK_FORMAT_D32_SFLOAT_S8_UINT:
        return VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;

    default:
        return VK_IMAGE_ASPECT_COLOR_BIT;
    }
}

bool VulkanImage::isColorFormat(VkFormat format)
{
    return !isDepthStencilFormat(format);
}

bool VulkanImage::isDepthStencilFormat(VkFormat format)
{
    switch (format)
    {
    case VK_FORMAT_D16_UNORM:
    case VK_FORMAT_X8_D24_UNORM_PACK32:
    case VK_FORMAT_D32_SFLOAT:

    case VK_FORMAT_S8_UINT:

    case VK_FORMAT_D16_UNORM_S8_UINT:
    case VK_FORMAT_D24_UNORM_S8_UINT:
    case VK_FORMAT_D32_SFLOAT_S8_UINT:
        return true;

    default:
        return false;
    }
}

VkImage VulkanImage::getHandle() const
{
    return m_image;
}

VkImageAspectFlags VulkanImage::getImageAspectFlags() const
{
    return m_aspectMask;
}

VkImageLayout VulkanImage::getLayout() const
{
    return m_layout;
}

void VulkanImage::setLayout(VkImageLayout layout)
{
    m_layout = layout;
}

void VulkanImage::destroy()
{
    if (m_imageView)
    {
        VulkanWrapper::DestroyImageView(m_device, m_imageView, VULKAN_ALLOCATOR);
        m_imageView = VK_NULL_HANDLE;
    }

    if (m_image)
    {
        VulkanWrapper::DestroyImage(m_device, m_image, VULKAN_ALLOCATOR);
        m_image = VK_NULL_HANDLE;
    }
}

bool VulkanImage::createViewImage()
{
    auto convertImageTypeToImageViewType = [](VkImageType type, bool cube = false, bool array = false) -> VkImageViewType
    {
        switch (type)
        {
        case VK_IMAGE_TYPE_1D:

            if (array)
            {
                return VK_IMAGE_VIEW_TYPE_1D_ARRAY;
            }
            return VK_IMAGE_VIEW_TYPE_1D;

        case VK_IMAGE_TYPE_2D:

            if (cube)
            {
                return VK_IMAGE_VIEW_TYPE_CUBE;
            }
            else if (array)
            {
                return VK_IMAGE_VIEW_TYPE_2D_ARRAY;
            }
            return VK_IMAGE_VIEW_TYPE_2D;

        case VK_IMAGE_TYPE_3D:
            return VK_IMAGE_VIEW_TYPE_3D;
        }

        return VK_IMAGE_VIEW_TYPE_2D;
    };

    VkImageViewCreateInfo imageViewCreateInfo = {};
    imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    imageViewCreateInfo.pNext = nullptr;
    imageViewCreateInfo.flags = 0;
    imageViewCreateInfo.image = m_image;
    imageViewCreateInfo.viewType = convertImageTypeToImageViewType(m_type, false, m_layersLevel > 1);
    imageViewCreateInfo.format = m_format;
    imageViewCreateInfo.components = { VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A };
    imageViewCreateInfo.subresourceRange = VulkanImage::makeImageSubresourceRange(this);

    VkResult result = VulkanWrapper::CreateImageView(m_device, &imageViewCreateInfo, VULKAN_ALLOCATOR, &m_imageView);
    if (result != VK_SUCCESS)
    {
        LOG_DEBUG("VulkanImage::createViewImage vkCreateImageView is failed. Error: %s", ErrorString(result).c_str());
        return false;
    }

    return true;
}

bool VulkanImage::createSampler()
{
    return false;
}

} //namespace vk
} //namespace renderer
} //namespace v3d
#endif //VULKAN_RENDER
