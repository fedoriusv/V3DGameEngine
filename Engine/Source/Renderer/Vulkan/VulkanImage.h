#pragma once

#include "Common.h"
#include "Image.h"

#ifdef VULKAN_RENDER
#include "VulkanWrapper.h"

namespace v3d
{
namespace renderer
{
namespace vk
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    class VulkanImage : public Image
    {
    public:

        VulkanImage(VkDevice device);
        ~VulkanImage();

    private:

        VkDevice    m_device;
        VkImage     m_image;
        VkImageView m_imageView;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace vk
} //namespace renderer
} //namespace v3d
#endif //VULKAN_RENDER
