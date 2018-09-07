#include "VulkanImage.h"
#include "Utils/Logger.h"

#ifdef VULKAN_RENDER
namespace v3d
{
namespace renderer
{
namespace vk
{

VulkanImage::VulkanImage(VkDevice device)
    : m_device(device)
    , m_image(VK_NULL_HANDLE)
    , m_imageView(VK_NULL_HANDLE)
{
    LOG_DEBUG("VulkanTexture::VulkanTexture constructor %llx", this);
}

VulkanImage::~VulkanImage()
{
    LOG_DEBUG("VulkanTexture::VulkanTexture destructor %llx", this);

    ASSERT(!m_imageView, "m_imageView is not nullptr");
    ASSERT(!m_image, "image not nullptr");
}

} //namespace vk
} //namespace renderer
} //namespace v3d
#endif //VULKAN_RENDER
