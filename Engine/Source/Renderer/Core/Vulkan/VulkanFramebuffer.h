#pragma once

#include "Renderer/Core/Framebuffer.h"

#ifdef VULKAN_RENDER
#include "VulkanWrapper.h"
#include "VulkanResource.h"

namespace v3d
{
namespace renderer
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    class Context;

    /////////////////////////////////////////////////////////////////////////////////////////////////////
namespace vk
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * VulkanFramebuffer final class. Vulkan Render side
    */
    class VulkanFramebuffer final : public Framebuffer, public VulkanResource
    {
    public:

        VulkanFramebuffer(VkDevice device, const Context* context, const std::vector<Image*>& images, const core::Dimension2D& size, [[maybe_unused]] const std::string& name = "") noexcept;
        ~VulkanFramebuffer();

        VkFramebuffer getHandle() const;
        const std::vector<Image*>& getImages() const;

        const core::Dimension2D& getArea() const;

        bool create(const RenderPass* pass) override;
        void destroy() override;

    private:

        VkDevice                    m_device;
        [[maybe_unused]] const Context* const m_context;

        const std::vector<Image*>   m_images;
        std::vector<VkImageView>    m_imageViews;
        core::Dimension2D           m_size;

        VkFramebuffer               m_framebuffer;

#if VULKAN_DEBUG_MARKERS
        std::string m_debugName;
#endif //VULKAN_DEBUG_MARKERS
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace vk
} //namespace renderer
} //namespace v3d
#endif //VULKAN_RENDER
