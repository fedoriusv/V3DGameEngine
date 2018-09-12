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

        VulkanImage(VkDevice device, VkImageType type, VkFormat format, VkExtent3D dimension, u32 mipLevels);
        ~VulkanImage();

        bool create() override;
        void destroy() override;

        bool create(VkImage image);

    private:

        bool createViewImage();
        bool createSampler();

        VkDevice                m_device;

        VkImageType             m_type;
        VkFormat                m_format;
        VkExtent3D              m_dimension;
        u32                     m_mipLevels;

        VkSampleCountFlagBits   m_samples;
        VkImageTiling           m_tiling;

        VkImage                 m_image;
        VkImageView             m_imageView;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace vk
} //namespace renderer
} //namespace v3d
#endif //VULKAN_RENDER
