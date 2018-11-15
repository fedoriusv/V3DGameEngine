#pragma once

#include "Framebuffer.h"

#ifdef VULKAN_RENDER
#include "VulkanWrapper.h"

namespace v3d
{
namespace renderer
{
namespace vk
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    class VulkanFramebuffer final : public Framebuffer
    {
    public:

        VulkanFramebuffer(VkDevice device, const std::vector<Image*>& images, const core::Dimension2D& size);
        ~VulkanFramebuffer();

        VkFramebuffer getHandle() const;

        bool create(const RenderPass* pass) override;
        void destroy() override;

    private:

        VkDevice      m_device;

        const std::vector<Image*>   m_images;
        std::vector<VkImageView>    m_imageViews;
        core::Dimension2D m_size;

        VkFramebuffer m_framebuffer;

    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace vk
} //namespace renderer
} //namespace v3d
#endif //VULKAN_RENDER
