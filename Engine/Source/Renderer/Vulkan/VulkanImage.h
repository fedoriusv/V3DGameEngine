#pragma once

#include "Common.h"
#include "Renderer/Formats.h"
#include "Renderer/Texture.h"

#ifdef VULKAN_RENDER
#include "VulkanWrapper.h"
#include "VulkanMemory.h"
#include "VulkanResource.h"

namespace v3d
{
namespace renderer
{
namespace vk
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////
    
    class VulkanDevice;
    class VulkanCmdList;

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief VulkanImage final class. Vulkan Render side
    */
    class VulkanImage final : public RenderTexture, public VulkanResource
    {
    public:

        static VkFormat convertImageFormatToVkFormat(Format format);
        static Format convertVkImageFormatToFormat(VkFormat format);
        static VkImageType convertTextureTargetToVkImageType(TextureTarget target);
        static VkSampleCountFlagBits convertRenderTargetSamplesToVkSampleCount(TextureSamples samples);

        static RenderTexture::Subresource makeVulkanImageSubresource(const VulkanImage* image, u32 layer = k_generalLayer, u32 mip = k_allMipmapsLevels);
        static VkImageSubresourceRange makeImageSubresourceRange(const VulkanImage* image, const RenderTexture::Subresource& resource);

        static std::string imageFormatStringVK(VkFormat format);
        static std::string imageTypeStringVK(VkImageType format);

        static VkImageAspectFlags getImageAspectFlags(VkFormat format);
        static bool isColorFormat(VkFormat format);
        static bool isDepthStencilFormat(VkFormat format);
        static bool isCompressedFormat(VkFormat format);
        static bool isASTCFormat(VkFormat format);
        static bool isSRGBFormat(VkFormat format);

        explicit VulkanImage(VulkanDevice* device, VkImageType type, VkFormat format, VkExtent3D dimension, u32 layers, u32 mipsLevel, VkImageTiling tiling, TextureUsageFlags usage, const std::string& name = "") noexcept;
        explicit VulkanImage(VulkanDevice* device, VkFormat format, VkExtent3D dimension, VkSampleCountFlagBits samples, u32 layers, TextureUsageFlags usage, const std::string& name = "") noexcept;
        ~VulkanImage();

        bool create() override;
        bool create(VkImage image);

        void destroy() override;

        bool hasUsageFlag(TextureUsage usage) const override;

        void clear(VulkanCmdList* cmdList, const renderer::Color& color);
        void clear(VulkanCmdList* cmdList, f32 depth, u32 stencil);

        bool upload(VulkanCmdList* cmdList, const math::Dimension3D& size, u32 layers, u32 mips, const void* data);
        bool upload(VulkanCmdList* cmdList, const math::Dimension3D& offsets, const math::Dimension3D& size, u32 layers, const void* data);

        bool generateMipmaps(VulkanCmdList* cmdList, u32 layer);

        VkImage               getHandle() const;
        VkImageAspectFlags    getImageAspectFlags() const;
        VkSampleCountFlagBits getSampleCount() const;
        VkImageView           getImageView(const RenderTexture::Subresource& resource, VkImageAspectFlags aspects = 0) const;
        VkFormat              getFormat() const;
        VkExtent3D            getSize() const;

        VkImageLayout         getLayout(const RenderTexture::Subresource& resource) const;
        VkImageLayout         setLayout(VkImageLayout layout, const RenderTexture::Subresource& resource);

        VulkanImage*          getResolveImage() const;

#if DEBUG_OBJECT_MEMORY
        static std::set<VulkanImage*> s_objects;
#endif //DEBUG_OBJECT_MEMORY

    private:

        VulkanImage() = delete;
        VulkanImage(const VulkanImage&) = delete;

        static VkImageSubresourceRange makeImageSubresourceRangeWithAspect(const VulkanImage* image, const RenderTexture::Subresource& resource, VkImageAspectFlags aspect);

        bool createViewImage();
        bool internalUpload(VulkanCmdList* cmdList, const math::Dimension3D& offsets, const math::Dimension3D& size, u32 layers, u32 mips, u64 dataSize, const void* data);

        VulkanDevice&           m_device;

        VkImageType             m_type;
        VkFormat                m_format;
        VkExtent3D              m_dimension;
        u32                     m_mipLevels;
        u32                     m_layerLevels;
        VkSampleCountFlagBits   m_samples;
        VkImageTiling           m_tiling;
        VkImageAspectFlags      m_aspectMask;
        TextureUsageFlags       m_usage;

        VkImage                 m_image;
        VulkanImage*            m_resolveImage;
        VulkanMemory::VulkanAllocation m_memory;

        std::unordered_map<DescInfo<VkImageSubresourceRange>, VkImageView, DescInfo<VkImageSubresourceRange>::Hash, DescInfo<VkImageSubresourceRange>::Compare> m_imageViews;
        std::vector<VkImageLayout> m_layout;

#if VULKAN_DEBUG_MARKERS
        std::string m_debugName;
#endif //VULKAN_DEBUG_MARKERS
    };

    inline VkImageAspectFlags VulkanImage::getImageAspectFlags() const
    {
        return m_aspectMask;
    }

    inline VkSampleCountFlagBits VulkanImage::getSampleCount() const
    {
        return m_samples;
    }

    inline VkFormat VulkanImage::getFormat() const
    {
        return m_format;
    }

    inline VkExtent3D VulkanImage::getSize() const
    {
        return m_dimension;
    }

    inline VulkanImage* VulkanImage::getResolveImage() const
    {
        return m_resolveImage;
    }

    inline VkImage VulkanImage::getHandle() const
    {
        ASSERT(m_image != VK_NULL_HANDLE, "nullptr");
        return m_image;
    }

    inline VkImageLayout VulkanImage::getLayout(const RenderTexture::Subresource& resource) const
    {
        u32 index = 1 + (resource._baseLayer * m_mipLevels + resource._baseMip);
        ASSERT(index < m_layout.size(), "out of range");
        return m_layout[index];
    }

    inline bool VulkanImage::hasUsageFlag(TextureUsage usage) const
    {
        return m_usage & usage;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace vk
} //namespace renderer
} //namespace v3d
#endif //VULKAN_RENDER
