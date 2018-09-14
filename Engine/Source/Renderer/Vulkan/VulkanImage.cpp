#include "VulkanImage.h"
#include "VulkanDebug.h"
#include "Utils/Logger.h"

#ifdef VULKAN_RENDER
namespace v3d
{
namespace renderer
{
namespace vk
{

VulkanImage::VulkanImage(VkDevice device, VkImageType type, VkFormat format, VkExtent3D dimension, u32 mipsLevel)
    : m_device(device)
    , m_type(type)
    , m_format(format)
    , m_dimension(dimension)
    , m_mipsLevel(mipsLevel)
    , m_layersLevel(1)

    , m_aspectMask(VK_IMAGE_ASPECT_COLOR_BIT)

    , m_samples(VK_SAMPLE_COUNT_1_BIT)
    , m_tiling(VK_IMAGE_TILING_OPTIMAL)

    , m_image(VK_NULL_HANDLE)
    , m_imageView(VK_NULL_HANDLE)
{
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

    imageCreateInfo.imageType = m_type;
    imageCreateInfo.format = m_format;
    imageCreateInfo.extent = m_dimension;
    imageCreateInfo.mipLevels = m_mipsLevel;
    imageCreateInfo.arrayLayers = m_layersLevel;

    imageCreateInfo.samples = m_samples;
    imageCreateInfo.tiling = m_tiling;
    imageCreateInfo.usage = 0; //TODO:

    imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    imageCreateInfo.queueFamilyIndexCount = 0;
    imageCreateInfo.pQueueFamilyIndices = nullptr;

    imageCreateInfo.initialLayout = (m_tiling == VK_IMAGE_TILING_OPTIMAL) ? VK_IMAGE_LAYOUT_UNDEFINED : VK_IMAGE_LAYOUT_PREINITIALIZED;

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

VkFormat VulkanImage::convertImageFormatToVkFormat(renderer::ImageFormat format)
{
    return VK_FORMAT_UNDEFINED;
}

VkImageType VulkanImage::convertTextureTargetToVkImageType(TextureTarget target)
{
    return VK_IMAGE_TYPE_2D;
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
    imageViewCreateInfo.subresourceRange.aspectMask = m_aspectMask; //TODO: detect 
    imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
    imageViewCreateInfo.subresourceRange.levelCount = m_mipsLevel;
    imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
    imageViewCreateInfo.subresourceRange.layerCount = m_layersLevel;

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
