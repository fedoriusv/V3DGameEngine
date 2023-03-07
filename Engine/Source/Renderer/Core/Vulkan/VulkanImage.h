#pragma once

#include "Common.h"
#include "Renderer/Formats.h"
#include "Renderer/TextureProperties.h"
#include "Renderer/Core/Image.h"

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

    /**
    * @brief VulkanImage final class. Vulkan Render side
    */
    class VulkanImage final : public Image, public VulkanResource
    {
    public:

        static VkFormat convertImageFormatToVkFormat(Format format);
        static Format convertVkImageFormatToFormat(VkFormat format);
        static VkImageType convertTextureTargetToVkImageType(TextureTarget target);
        static VkSampleCountFlagBits convertRenderTargetSamplesToVkSampleCount(TextureSamples samples);

        static Image::Subresource makeVulkanImageSubresource(const VulkanImage* image, u32 layer = k_generalLayer, u32 mip = k_allMipmapsLevels);
        static VkImageSubresourceRange makeImageSubresourceRange(const VulkanImage* image, const Image::Subresource& resource);

        static std::string imageFormatStringVK(VkFormat format);
        static std::string imageTypeStringVK(VkImageType format);

        static VkImageAspectFlags getImageAspectFlags(VkFormat format);
        static bool isColorFormat(VkFormat format);
        static bool isDepthStencilFormat(VkFormat format);
        static bool isCompressedFormat(VkFormat format);
        static bool isASTCFormat(VkFormat format);
        static bool isSRGBFormat(VkFormat format);

        VulkanImage(VulkanMemory::VulkanMemoryAllocator* memory, VkDevice device, VkImageType type, VkFormat format, VkExtent3D dimension, u32 layers, u32 mipsLevel, VkImageTiling tiling, TextureUsageFlags usage, [[maybe_unused]] const std::string& name = "") noexcept;
        VulkanImage(VulkanMemory::VulkanMemoryAllocator* memory, VkDevice device, VkFormat format, VkExtent3D dimension, VkSampleCountFlagBits samples, u32 layers, TextureUsageFlags usage, [[maybe_unused]] const std::string& name = "") noexcept;
        ~VulkanImage();

        bool create() override;
        void destroy() override;

        bool create(VkImage image);

        void clear(Context* context, const core::Vector4D& color) override;
        void clear(Context* context, f32 depth, u32 stencil) override;

        bool upload(Context* context, const core::Dimension3D& size, u32 layers, u32 mips, const void* data) override;
        bool upload(Context* context, const core::Dimension3D& offsets, const core::Dimension3D& size, u32 layers, const void* data) override;

        bool generateMipmaps(Context* context, u32 layer);

        VkImage               getHandle() const;
        VkImageAspectFlags    getImageAspectFlags() const;
        VkSampleCountFlagBits getSampleCount() const;
        VkImageView           getImageView(const Image::Subresource& resource, VkImageAspectFlags aspects = 0) const;
        VkFormat              getFormat() const;
        VkExtent3D            getSize() const;

        VkImageLayout         getLayout(const Image::Subresource& resource) const;
        VkImageLayout         setLayout(VkImageLayout layout, const Image::Subresource& resource);

        VulkanImage*          getResolveImage() const;

        bool                  isSwapchain() const;

#if DEBUG_OBJECT_MEMORY
        static std::set<VulkanImage*> s_objects;
#endif //DEBUG_OBJECT_MEMORY

    private:

        VulkanImage() = delete;
        VulkanImage(const VulkanImage&) = delete;

        static VkImageSubresourceRange makeImageSubresourceRangeWithAspect(const VulkanImage* image, const Image::Subresource& resource, VkImageAspectFlags aspect);

        bool createViewImage();
        bool internalUpload(Context* context, const core::Dimension3D& offsets, const core::Dimension3D& size, u32 layers, u32 mips, u64 dataSize, const void* data);

        bool isPresentTextureUsageFlag(TextureUsageFlags flag) const;

        VkDevice                    m_device;

        VkImageType                 m_type;
        VkFormat                    m_format;
        VkExtent3D                  m_dimension;
        u32                         m_mipLevels;
        u32                         m_layerLevels;

        VkSampleCountFlagBits       m_samples;
        VkImageTiling               m_tiling;
        VkImageAspectFlags          m_aspectMask;

        VkImage                     m_image;

        struct ImageViewKey
        {
            ImageViewKey() noexcept;
            explicit ImageViewKey(VkImageSubresourceRange& desc) noexcept;

            VkImageSubresourceRange _desc;
            u32 _hash;

            struct Hash
            {
                u32 operator()(const ImageViewKey& desc) const;
            };

            struct Compare
            {
                bool operator()(const ImageViewKey& op1, const ImageViewKey& op2) const;
            };
        };

        std::unordered_map<ImageViewKey, VkImageView, ImageViewKey::Hash, ImageViewKey::Compare> m_imageViews;

        std::vector<VkImageLayout>  m_layout;

        TextureUsageFlags m_usage;

        VulkanImage* m_resolveImage;

        VulkanMemory::VulkanAllocation       m_memory;
        VulkanMemory::VulkanMemoryAllocator* m_memoryAllocator;

        bool m_swapchainImage;

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

    inline bool VulkanImage::isSwapchain() const
    {
        return m_swapchainImage;
    }

    inline VkImage VulkanImage::getHandle() const
    {
        ASSERT(m_image != VK_NULL_HANDLE, "nullptr");
        return m_image;
    }

    inline VkImageLayout VulkanImage::getLayout(const Image::Subresource& resource) const
    {
        u32 index = 1 + (resource._baseLayer * m_mipLevels + resource._baseMip);
        ASSERT(index < m_layout.size(), "out of range");
        return m_layout[index];
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace vk
} //namespace renderer
} //namespace v3d
#endif //VULKAN_RENDER
