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
    class VulkanCommandBuffer;

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief VulkanFramebuffer final class. Vulkan Render side
    */
    class VulkanFramebuffer final : public Framebuffer, public VulkanResource
    {
    public:

        explicit VulkanFramebuffer(VulkanDevice* device, const std::vector<std::tuple<VulkanImage*, RenderTexture::Subresource>>& images, const VkRect2D& renderArea, const std::string& name = "") noexcept;
        ~VulkanFramebuffer();

        VkFramebuffer getHandle() const;

        const std::tuple<VulkanImage*, RenderTexture::Subresource>& getImage(u32 index) const;
        const VkRect2D& getRenderArea() const;
        u32 getCountImage() const;

        bool create(const VulkanRenderPass* renderpass);
        void destroy();

    private:

        VulkanFramebuffer() = delete;
        VulkanFramebuffer(const VulkanFramebuffer&) = delete;

        VulkanDevice&                                                     m_device;

        std::vector<std::tuple<VulkanImage*, RenderTexture::Subresource>> m_images;
        VkRect2D                                                          m_renderArea;
        VkFramebuffer                                                     m_framebuffer;

#if VULKAN_DEBUG_MARKERS
        std::string m_debugName;
#endif //VULKAN_DEBUG_MARKERS

        friend VulkanCommandBuffer;
    };

    inline const std::tuple<VulkanImage*, RenderTexture::Subresource>& VulkanFramebuffer::getImage(u32 index) const
    {
        ASSERT(index < m_images.size(), "out of range");
        return m_images[index];
    }

    inline const VkRect2D& VulkanFramebuffer::getRenderArea() const
    {
        return m_renderArea;
    }

    inline u32 VulkanFramebuffer::getCountImage() const
    {
        return static_cast<u32>(m_images.size());
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

            std::array<std::tuple<RenderTexture::ValueType, RenderTexture::Subresource>, k_maxColorAttachments + 1> _renderTargets;
        };

        explicit VulkanFramebufferManager(VulkanDevice* device) noexcept;
        ~VulkanFramebufferManager();

        [[nodiscard]] std::tuple<VulkanFramebuffer*, bool> acquireFramebuffer(const VulkanRenderPass* renderpass, const FramebufferDesc& framebufferDesc, const std::string& name = "");
        bool removeFramebuffer(VulkanFramebuffer* framebuffer);
        void clear();

    private:

        VulkanFramebufferManager() = delete;
        VulkanFramebufferManager(const VulkanFramebufferManager&) = delete;

        VulkanDevice&   m_device;
        utils::Spinlock m_mutex;
        std::unordered_map<DescInfo<VulkanFramebufferDesc>, VulkanFramebuffer*, DescInfo<VulkanFramebufferDesc>::Hash, DescInfo<VulkanFramebufferDesc>::Compare> m_framebufferList;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace vk
} //namespace renderer
} //namespace v3d
#endif //VULKAN_RENDER
