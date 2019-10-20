#pragma once

#include "Renderer/RenderPass.h"
#include "Renderer/TextureProperties.h"

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
    * VulkanRenderPass final class. Vulkan Render side
    */
    class VulkanRenderPass final : public RenderPass, public VulkanResource
    {
    public:

        struct VulkanAttachmentDescription
        {
            VkFormat              _format;
            VkSampleCountFlagBits _samples;
            VkAttachmentLoadOp    _loadOp;
            VkAttachmentStoreOp   _storeOp;
            VkAttachmentLoadOp    _stencilLoadOp;
            VkAttachmentStoreOp   _stensilStoreOp;

            VkImageLayout         _initialLayout;
            VkImageLayout         _finalLayout;

            bool                  _autoResolve;
            bool                  _swapchainImage;
        };

        static VkAttachmentLoadOp convertAttachLoadOpToVkAttachmentLoadOp(RenderTargetLoadOp loadOp);
        static VkAttachmentStoreOp convertAttachStoreOpToVkAttachmentStoreOp(RenderTargetStoreOp storeOp);
        static VkImageLayout convertTransitionStateToImageLayout(TransitionOp state);

        VulkanRenderPass(VkDevice device, const std::vector<VulkanAttachmentDescription>& desc);
        ~VulkanRenderPass();

        VkRenderPass getHandle() const;

        template<u32 status>
        VkImageLayout getAttachmentLayout(u32 index) const
        {
            ASSERT(index < m_layout.size(), "range out");
            static_assert(status < std::tuple_size<LayoutState>::value);

            return std::get<status>(m_layout[index]);
        }

        bool create() override;
        void destroy() override;

    private:

        VkDevice     m_device;
        VkRenderPass m_renderpass;

        using LayoutState = std::tuple<VkImageLayout, VkImageLayout>;
        std::vector<LayoutState> m_layout;
        std::vector<VulkanAttachmentDescription> m_descriptions;

#if VULKAN_DEBUG_MARKERS
        std::string m_debugName;
#endif //VULKAN_DEBUG_MARKERS
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace vk
} //namespace renderer
} //namespace v3d
#endif //VULKAN_RENDER
