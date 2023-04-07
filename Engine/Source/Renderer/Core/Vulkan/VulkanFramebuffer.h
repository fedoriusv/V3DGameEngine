#pragma once

#include "Renderer/Core/Framebuffer.h"

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

    /**
    * @brief VulkanFramebuffer final class. Vulkan Render side
    */
    class VulkanFramebuffer final : public Framebuffer, public VulkanResource
    {
    public:

        explicit VulkanFramebuffer(VkDevice device, const std::vector<Image*>& images, const math::Dimension2D& size, [[maybe_unused]] const std::string& name = "") noexcept;
        ~VulkanFramebuffer();

        VkFramebuffer getHandle() const;
        const std::vector<Image*>& getImages() const;

        const math::Dimension2D& getArea() const;

        bool create(const RenderPass* pass) override;
        void destroy() override;

    private:

        VulkanFramebuffer() = delete;
        VulkanFramebuffer(const VulkanFramebuffer&) = delete;

        VkDevice                    m_device;

        const std::vector<Image*>   m_images;
        std::vector<VkImageView>    m_imageViews;
        math::Dimension2D           m_size;

        VkFramebuffer               m_framebuffer;

#if VULKAN_DEBUG_MARKERS
        std::string m_debugName;
#endif //VULKAN_DEBUG_MARKERS
    };

    inline const std::vector<Image*>& VulkanFramebuffer::getImages() const
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

} //namespace vk
} //namespace renderer
} //namespace v3d
#endif //VULKAN_RENDER
