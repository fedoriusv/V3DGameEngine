#pragma once

#include "Renderer/RenderPass.h"
#include "Renderer/TextureProperties.h"

#ifdef VULKAN_RENDER
#include "VulkanWrapper.h"

namespace v3d
{
namespace renderer
{
namespace vk
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    class VulkanRenderPass final : public RenderPass
    {
    public:

        struct VulkanAttachmentDescription
        {
            VkFormat              _format;
            VkSampleCountFlagBits _samples;
            VkAttachmentLoadOp    _loadOp;
            VkAttachmentStoreOp   _storeOp;
        };

        static VkAttachmentLoadOp convertAttachLoadOpToVkAttachmentLoadOp(RenderTargetLoadOp loadOp);
        static VkAttachmentStoreOp convertAttachStoreOpToVkAttachmentStoreOp(RenderTargetStoreOp storeOp);

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
