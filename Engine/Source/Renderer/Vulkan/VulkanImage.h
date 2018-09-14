#pragma once

#include "Common.h"
#include "Object/Texture.h"
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

        VulkanImage(VkDevice device, VkImageType type, VkFormat format, VkExtent3D dimension, u32 mipsLevel);
        ~VulkanImage();

        bool create() override;
        void destroy() override;

        bool create(VkImage image);

        static VkFormat convertImageFormatToVkFormat(renderer::ImageFormat format);
        static VkImageType convertTextureTargetToVkImageType(TextureTarget target);

    private:

        bool createViewImage();
        bool createSampler();

        VkDevice                m_device;

        VkImageType             m_type;
        VkFormat                m_format;
        VkExtent3D              m_dimension;
        u32                     m_mipsLevel;
        u32                     m_layersLevel;

        VkSampleCountFlagBits   m_samples;
        VkImageTiling           m_tiling;

        VkImage                 m_image;
        VkImageView             m_imageView;
        VkImageAspectFlags      m_aspectMask;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace vk
} //namespace renderer
} //namespace v3d
#endif //VULKAN_RENDER
