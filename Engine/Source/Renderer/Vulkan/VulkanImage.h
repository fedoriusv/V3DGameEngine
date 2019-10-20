#pragma once

#include "Common.h"
#include "Renderer/Image.h"
#include "Renderer/Formats.h"
#include "Renderer/TextureProperties.h"

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
    * VulkanImage final class. Vulkan Render side
    */
    class VulkanImage final : public Image, public VulkanResource
    {
    public:

        VulkanImage() = delete;

        VulkanImage(VulkanMemory::VulkanMemoryAllocator* memory, VkDevice device, VkImageType type, VkFormat format, VkExtent3D dimension, u32 layers, u32 mipsLevel, VkImageTiling tiling, TextureUsageFlags usage, const std::string& name = "") noexcept;
        VulkanImage(VulkanMemory::VulkanMemoryAllocator* memory, VkDevice device, VkFormat format, VkExtent3D dimension, VkSampleCountFlagBits samples, TextureUsageFlags usage, const std::string& name = "") noexcept;
        ~VulkanImage();

        bool create() override;
        void destroy() override;

        bool create(VkImage image);

        void clear(Context* context, const core::Vector4D& color) override;
        void clear(Context* context, f32 depth, u32 stencil) override;

        bool upload(Context* context, const core::Dimension3D& size, u32 layers, u32 mips, const void* data) override;
        bool upload(Context* context, const core::Dimension3D& offsets, const core::Dimension3D& size, u32 layers, const void* data) override;

        static VkFormat convertImageFormatToVkFormat(Format format);
        static Format convertVkImageFormatToFormat(VkFormat format);
        static VkImageType convertTextureTargetToVkImageType(TextureTarget target);
        static VkSampleCountFlagBits convertRenderTargetSamplesToVkSampleCount(TextureSamples samples);

        static VkImageSubresourceRange makeImageSubresourceRange(const VulkanImage* image, s32 layer = -1, s32 mip = -1);
        static VkImageSubresourceLayers makeImageSubresourceLayers(const VulkanImage* image, s32 layer = -1, s32 mip = 0);

        static VkImageAspectFlags getImageAspectFlags(VkFormat format);
        static bool isColorFormat(VkFormat format);
        static bool isDepthStencilFormat(VkFormat format);
        static bool isCompressedFormat(VkFormat format);
        static std::tuple<VkAccessFlags, VkAccessFlags> getAccessFlagsFromImageLayout(VkImageLayout oldLayout, VkImageLayout newLayout);

        static bool isAttachmentLayout(const VulkanImage* image, s32 layer = -1);
        static u32 calculateImageSize(const core::Dimension3D& size, u32 mipLevel, VkFormat format);

        VkImage               getHandle() const;
        VkImageAspectFlags    getImageAspectFlags() const;
        VkSampleCountFlagBits getSampleCount() const;
        VkImageView           getImageView(s32 layer = -1, VkImageAspectFlags aspect  = 0) const;
        VkFormat              getFormat() const;
        VkExtent3D            getSize() const;

        VkImageLayout         getLayout(s32 layer = -1, s32 mip = -1) const;
        VkImageLayout         setLayout(VkImageLayout layout, s32 layer = -1, s32 mip = -1);

        VulkanImage*          getResolveImage() const;

    private:

        enum ImageAspect : s32
        {
            ImageAspect_General = -1,
            ImageAspect_Color = 0,
            ImageAspect_Depth,
            ImageAspect_Stencil,
            ImageAspect_DepthStencil,

            ImageAspect_Count
        };

        static ImageAspect convertVkImageAspectFlags(VkImageAspectFlags aspect);
        static VkImageAspectFlags convertImageAspectFlagsToVk(ImageAspect aspect);
        static VkImageSubresourceRange makeImageSubresourceRangeWithAspect(const VulkanImage* image, s32 layer = -1, s32 mip = -1, ImageAspect aspect = ImageAspect::ImageAspect_General);

        bool createViewImage();

        bool internalUpload(Context* context, const core::Dimension3D& offsets, const core::Dimension3D& size, u32 layers, u32 mips, u64 dataSize, const void* data);

        VkDevice                    m_device;

        VkImageType                 m_type;
        VkFormat                    m_format;
        VkExtent3D                  m_dimension;
        u32                         m_mipsLevel;
        u32                         m_layersLevel;

        VkSampleCountFlagBits       m_samples;
        VkImageTiling               m_tiling;
        
        VkImageAspectFlags          m_aspectMask;

        VkImage                     m_image;

        VkImageView                 m_generalImageView[ImageAspect::ImageAspect_Count];
        std::vector<VkImageView>    m_imageView;


        std::vector<VkImageLayout>  m_layout;

        TextureUsageFlags           m_usage;

        VulkanImage*                m_resolveImage;

        VulkanMemory::VulkanAlloc   m_memory;
        VulkanMemory::VulkanMemoryAllocator* m_memoryAllocator;

        bool                        m_swapchainImage;

#if VULKAN_DEBUG_MARKERS
        std::string m_debugName;
#endif //VULKAN_DEBUG_MARKERS
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace vk
} //namespace renderer
} //namespace v3d
#endif //VULKAN_RENDER
