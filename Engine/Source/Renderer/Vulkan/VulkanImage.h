#pragma once

#include "Common.h"
#include "Image.h"
#include "ImageFormats.h"
#include "Object/TextureEnums.h"

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

        VulkanImage(VkDevice device, VkImageType type, VkFormat format, VkExtent3D dimension, u32 mipsLevel, VkImageTiling tiling);
        VulkanImage(VkDevice device, VkFormat format, VkExtent3D dimension, VkSampleCountFlagBits samples);
        ~VulkanImage();

        bool create() override;
        void destroy() override;

        bool create(VkImage image);

        void clear(const Context* context, const core::Vector4D& color) override;
        void clear(const Context* context, f32 depth, u32 stencil) override;

        static VkFormat convertImageFormatToVkFormat(renderer::ImageFormat format);
        static VkImageType convertTextureTargetToVkImageType(TextureTarget target);
        static VkSampleCountFlagBits convertRenderTargetSamplesToVkSampleCount(RenderTargetSamples samples);

        static VkImageSubresourceRange makeImageSubresourceRange(const VulkanImage* image);

        static VkImageAspectFlags getImageAspectFlags(VkFormat format);
        static bool isColorFormat(VkFormat format);
        static bool isDepthStencilFormat(VkFormat format);
        static std::tuple<VkAccessFlags, VkAccessFlags> getAccessFlagsFromImageLayout(VkImageLayout oldLayout, VkImageLayout newLayout);

        VkImage             getHandle() const;
        VkImageAspectFlags  getImageAspectFlags() const;

        VkImageLayout       getLayout() const;
        void                setLayout(VkImageLayout layout);

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

        VkImageLayout           m_layout;
        VkImageUsageFlags       m_usage;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace vk
} //namespace renderer
} //namespace v3d
#endif //VULKAN_RENDER
