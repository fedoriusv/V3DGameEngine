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

            bool                  _swapchainImage;
        };

        static VkAttachmentLoadOp convertAttachLoadOpToVkAttachmentLoadOp(RenderTargetLoadOp loadOp);
        static VkAttachmentStoreOp convertAttachStoreOpToVkAttachmentStoreOp(RenderTargetStoreOp storeOp);
        static VkImageLayout convertTransitionStateToImageLayout(TransitionOp state);

        VulkanRenderPass(VkDevice device, const std::vector<VulkanAttachmentDescription>& desc);
        ~VulkanRenderPass();

        VkRenderPass getHandle() const;

        bool create() override;
        void destroy() override;

    private:

        VkDevice     m_device;
        VkRenderPass m_renderpass;

        std::vector<VulkanAttachmentDescription> m_descriptions;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace vk
} //namespace renderer
} //namespace v3d
#endif //VULKAN_RENDER
