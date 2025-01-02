#pragma once

#include "Renderer/Render.h"
#include "Renderer/RenderTargetState.h"

#ifdef VULKAN_RENDER
#include "VulkanWrapper.h"
#include "VulkanResource.h"

namespace v3d
{
namespace renderer
{
namespace vk
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    class VulkanDevice;
    class VulkanRenderPass;

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief VulkanFramebuffer final class. Vulkan Render side
    */
    class VulkanFramebuffer final : public Framebuffer, public VulkanResource
    {
    public:

        explicit VulkanFramebuffer(VulkanDevice* device, const std::vector<TextureHandle>& images, const math::Dimension2D& size, const std::string& name = "") noexcept;
        ~VulkanFramebuffer();

        VkFramebuffer getHandle() const;

        const std::vector<TextureHandle>& getImages() const;
        const math::Dimension2D& getArea() const;

        bool create(const VulkanRenderPass* renderpass);
        void destroy();

    private:

        VulkanFramebuffer() = delete;
        VulkanFramebuffer(const VulkanFramebuffer&) = delete;

        VulkanDevice&               m_device;

        std::vector<TextureHandle>  m_images;
        std::vector<VkImageView>    m_imageViews;
        math::Dimension2D           m_size;

        VkFramebuffer               m_framebuffer;

#if VULKAN_DEBUG_MARKERS
        std::string m_debugName;
#endif //VULKAN_DEBUG_MARKERS
    };

    inline const std::vector<TextureHandle>& VulkanFramebuffer::getImages() const
    {
        return m_images;
    }

    inline const math::Dimension2D& VulkanFramebuffer::getArea() const
    {
        return m_size;
    }

    inline VkFramebuffer VulkanFramebuffer::getHandle() const
    {
        ASSERT(m_framebuffer != VK_NULL_HANDLE, "nullptr");
        return m_framebuffer;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief VulkanFramebufferManager class.
    * Multithreaded
    */
    class VulkanFramebufferManager final
    {
    public:

        struct VulkanFramebufferDesc final
        {
            VulkanFramebufferDesc() noexcept = default;
            VulkanFramebufferDesc(const VulkanFramebufferDesc& desc) noexcept = default;

            bool operator==(const VulkanFramebufferDesc& other) const
            {
                if (this == &other)
                {
                    return true;
                }

                return memcmp(this, &other, sizeof(VulkanFramebufferDesc)) == 0;
            }

            std::array<RenderTexture::ValueType, k_maxColorAttachments + 1> _renderTargetsIDs;
        };

        explicit VulkanFramebufferManager(VulkanDevice* device) noexcept;
        ~VulkanFramebufferManager();

        [[nodiscard]] std::tuple<VulkanFramebuffer*, bool> acquireFramebuffer(const VulkanRenderPass* renderpass, const FramebufferDesc& description, const std::string& name = "");
        bool removeFramebuffer(VulkanFramebuffer* framebuffer);
        void clear();

    private:

        VulkanFramebufferManager() = delete;
        VulkanFramebufferManager(const VulkanFramebufferManager&) = delete;

        VulkanDevice&   m_device;
        std::mutex      m_mutex;
        std::unordered_map<DescInfo<VulkanFramebufferDesc>, VulkanFramebuffer*, DescInfo<VulkanFramebufferDesc>::Hash, DescInfo<VulkanFramebufferDesc>::Compare> m_framebufferList;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace vk
} //namespace renderer
} //namespace v3d
#endif //VULKAN_RENDER
