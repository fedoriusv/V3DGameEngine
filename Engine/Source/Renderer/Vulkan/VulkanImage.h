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
    class VulkanSwapchain;
    class VulkanStateTracker;

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
        static VkImageSubresourceRange makeImageSubresourceRange(const RenderTexture::Subresource& resource, VkImageAspectFlags aspectMask);

        static std::string imageFormatStringVK(VkFormat format);
        static std::string imageTypeStringVK(VkImageType format);

        static VkComponentMapping getComponentMapping(VkFormat format);
        static VkImageAspectFlags getImageAspectFlags(VkFormat format);
        static bool isColorFormat(VkFormat format);
        static bool isDepthStencilFormat(VkFormat format);
        static bool isDepthFormat(VkFormat format);
        static bool isStencilFormat(VkFormat format);
        static bool isCompressedFormat(VkFormat format);
        static bool isASTCFormat(VkFormat format);
        static bool isSRGBFormat(VkFormat format);

        static VulkanSwapchain* getSwapchainFromImage(const VulkanImage* image);

        explicit VulkanImage(VulkanDevice* device, VulkanMemory::VulkanMemoryAllocator* alloc, VkImageType type, VkFormat format, VkExtent3D dimension, u32 layers, u32 mipsLevel, VkImageTiling tiling, TextureUsageFlags usage, const std::string& name = "") noexcept;
        explicit VulkanImage(VulkanDevice* device, VulkanMemory::VulkanMemoryAllocator* alloc, VkFormat format, VkExtent3D dimension, VkSampleCountFlagBits samples, u32 layers, TextureUsageFlags usage, const std::string& name = "") noexcept;
        ~VulkanImage();

        bool create() override;
        bool create(VkImage image, VulkanSwapchain* swapchain);

        void destroy() override;

        bool hasUsageFlag(TextureUsage usage) const override;

        void clear(VulkanCommandBuffer* cmdBuffer, const color::Color& color);
        void clear(VulkanCommandBuffer* cmdBuffer, f32 depth, u32 stencil);

        bool upload(VulkanCommandBuffer* cmdBuffer, u32 size, const void* data);
        bool upload(VulkanCommandBuffer* cmdBuffer, const math::Dimension3D& size, u32 layers, u32 mips, const void* data);
        bool upload(VulkanCommandBuffer* cmdBuffer, const math::Dimension3D& offsets, const math::Dimension3D& size, u32 layers, const void* data);

        bool generateMipmaps(VulkanCommandBuffer* cmdBuffer, u32 layer);

        VkImage               getHandle() const;
        VkImageAspectFlags    getImageAspectFlags() const;
        VkSampleCountFlagBits getSampleCount() const;
        VkImageView           getImageView(const RenderTexture::Subresource& resource, VkImageAspectFlags aspects = 0) const;
        VkFormat              getFormat() const;
        VkExtent3D            getSize() const;
        u32                   getArrayLayers() const;

        VulkanImage*          getResolveImage() const;

#if DEBUG_OBJECT_MEMORY
        static std::set<VulkanImage*> s_objects;
#endif //DEBUG_OBJECT_MEMORY

    private:

        friend VulkanResourceStateTracker;

        VulkanImage() = delete;
        VulkanImage(const VulkanImage&) = delete;

        bool createViewImage();
        VkImageView createViewImage(VkImageSubresourceRange imageSubresourceRange, const std::string& name = "") const;
        bool internalUpload(VulkanCommandBuffer* cmdBuffer, const math::Dimension3D& offsets, const math::Dimension3D& size, u32 layers, u32 mips, u64 dataSize, const void* data);

        VkImageLayout         getGlobalLayout(const RenderTexture::Subresource& resource) const;
        VkImageLayout         setGlobalLayout(VkImageLayout layout, const RenderTexture::Subresource& resource);

        VulkanDevice&                           m_device;
        VulkanMemory::VulkanMemoryAllocator*    m_memoryAllocator;

        VkImageType                             m_type;
        VkFormat                                m_format;
        VkExtent3D                              m_dimension;
        u32                                     m_mipLevels;
        u32                                     m_arrayLayers;
        VkSampleCountFlagBits                   m_samples;
        VkImageTiling                           m_tiling;
        VkImageAspectFlags                      m_aspectMask;
        TextureUsageFlags                       m_usage;

        VkImage                                 m_image;
        VulkanImage*                            m_resolveImage;
        VulkanMemory::VulkanAllocation          m_memory;
        VulkanSwapchain*                        m_relatedSwapchain;

        mutable std::unordered_map<DescInfo<VkImageSubresourceRange>, VkImageView, DescInfo<VkImageSubresourceRange>::Hash, DescInfo<VkImageSubresourceRange>::Compare> m_imageViews;
        std::vector<VkImageLayout>              m_globalLayout;

#if VULKAN_DEBUG_MARKERS
        std::string                             m_debugName;
#endif //VULKAN_DEBUG_MARKERS
    };

    inline RenderTexture::Subresource VulkanImage::makeVulkanImageSubresource(const VulkanImage* image, u32 layer, u32 mip)
    {
        ASSERT(image, "nullptr");
        RenderTexture::Subresource resource = { layer, 1, mip, 1 };

        if (layer == k_generalLayer)
        {
            resource._baseLayer = 0;
            resource._layers = image->m_arrayLayers;
        }

        if (mip == k_allMipmapsLevels)
        {
            resource._baseMip = 0;
            resource._mips = image->m_mipLevels;
        }

        return resource;
    }

    inline VkImageSubresourceRange VulkanImage::makeImageSubresourceRange(const RenderTexture::Subresource& resource, VkImageAspectFlags aspectMask)
    {
        VkImageSubresourceRange imageSubresourceRange = {};
        imageSubresourceRange.aspectMask = aspectMask;
        imageSubresourceRange.baseArrayLayer = resource._baseLayer;
        imageSubresourceRange.layerCount = resource._layers;
        imageSubresourceRange.baseMipLevel = resource._baseMip;
        imageSubresourceRange.levelCount = resource._mips;

        return imageSubresourceRange;
    }

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

    inline u32 VulkanImage::getArrayLayers() const
    {
        return m_arrayLayers;
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

    inline bool VulkanImage::hasUsageFlag(TextureUsage usage) const
    {
        return m_usage & usage;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace vk
} //namespace renderer
} //namespace v3d
#endif //VULKAN_RENDER
