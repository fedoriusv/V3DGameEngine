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

        VulkanImage(VulkanMemory::VulkanMemoryAllocator* memory, VkDevice device, VkImageType type, VkFormat format, VkExtent3D dimension, u32 mipsLevel, VkImageTiling tiling, TextureUsageFlags usage);
        VulkanImage(VulkanMemory::VulkanMemoryAllocator* memory, VkDevice device, VkFormat format, VkExtent3D dimension, VkSampleCountFlagBits samples, TextureUsageFlags usage);
        ~VulkanImage();

        bool create() override;
        void destroy() override;

        bool create(VkImage image);

        void clear(Context* context, const core::Vector4D& color) override;
        void clear(Context* context, f32 depth, u32 stencil) override;

        bool upload(Context* context, const core::Dimension3D& offsets, const core::Dimension3D& size, u32 mips, const void* data) override;

        static VkFormat convertImageFormatToVkFormat(Format format);
        static Format convertVkImageFormatToFormat(VkFormat format);
        static VkImageType convertTextureTargetToVkImageType(TextureTarget target);
        static VkSampleCountFlagBits convertRenderTargetSamplesToVkSampleCount(TextureSamples samples);

        static VkImageSubresourceRange makeImageSubresourceRange(const VulkanImage* image);

        static VkImageAspectFlags getImageAspectFlags(VkFormat format);
        static bool isColorFormat(VkFormat format);
        static bool isDepthStencilFormat(VkFormat format);
        static bool isCompressedFormat(VkFormat format);
        static std::tuple<VkAccessFlags, VkAccessFlags> getAccessFlagsFromImageLayout(VkImageLayout oldLayout, VkImageLayout newLayout);

        VkImage             getHandle() const;
        VkImageAspectFlags  getImageAspectFlags() const;
        VkImageView         getImageView() const;
        VkFormat            getFormat() const;

        VkImageLayout       getLayout() const;
        VkImageLayout       setLayout(VkImageLayout layout);

    private:

        bool createViewImage();
        bool createSampler();

        VkDevice                    m_device;

        VkImageType                 m_type;
        VkFormat                    m_format;
        VkExtent3D                  m_dimension;
        u32                         m_mipsLevel;
        u32                         m_layersLevel;

        VkSampleCountFlagBits       m_samples;
        VkImageTiling               m_tiling;

        VkImage                     m_image;
        VkImageView                 m_imageView;
        VkImageAspectFlags          m_aspectMask;

        VkImageLayout               m_layout;

        TextureUsageFlags           m_usage;

        VulkanImage*                m_resolveImage; //?

        VulkanMemory::VulkanAlloc   m_memory;
        VulkanMemory::VulkanMemoryAllocator* m_memoryAllocator;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace vk
} //namespace renderer
} //namespace v3d
#endif //VULKAN_RENDER
