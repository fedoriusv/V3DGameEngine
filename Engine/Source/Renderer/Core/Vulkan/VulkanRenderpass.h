#pragma once

#include "Renderer/TextureProperties.h"
#include "Renderer/Core/RenderPass.h"

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
    * @brief VulkanRenderPass final class. Vulkan Render side
    */
    class VulkanRenderPass final : public RenderPass, public VulkanResource
    {
    public:

        static VkAttachmentLoadOp convertAttachLoadOpToVkAttachmentLoadOp(RenderTargetLoadOp loadOp);
        static VkAttachmentStoreOp convertAttachStoreOpToVkAttachmentStoreOp(RenderTargetStoreOp storeOp);

        /**
        * @brief VulkanAttachmentDescription struct
        */
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

            u32                   _layer;
            u32                   _mip;

            bool                  _autoResolve;
            bool                  _swapchainImage;
        };

        explicit VulkanRenderPass(VkDevice device, const RenderPassDescription& description, [[maybe_unused]] const std::string& name = "") noexcept;
        ~VulkanRenderPass();

        VkRenderPass getHandle() const;

        const VulkanAttachmentDescription& getAttachmentDescription(u32 index) const;

        template<u32 status>
        VkImageLayout getAttachmentLayout(u32 index) const
        {
            ASSERT(index < m_layout.size(), "range out");
            static_assert(status <std::tuple_size<LayoutState>::value);

            return std::get<status>(m_layout[index]);
        }

        bool isDrawingToSwapchain() const;

        bool create() override;
        void destroy() override;

    private:

        bool createRenderpass();
        bool createRenderpass2();

        VkDevice     m_device;
        VkRenderPass m_renderpass;

        using LayoutState = std::tuple<VkImageLayout, VkImageLayout>;
        std::vector<LayoutState> m_layout;
        std::vector<VulkanAttachmentDescription> m_descriptions;

#if VULKAN_DEBUG_MARKERS
        std::string m_debugName;
#endif //VULKAN_DEBUG_MARKERS
    };

    inline bool VulkanRenderPass::isDrawingToSwapchain() const
    {
        return (VulkanRenderPass::getDescription()._countColorAttachments == 1) && (VulkanRenderPass::getDescription()._attachments[0]._backbuffer);
    }

    inline VkRenderPass VulkanRenderPass::getHandle() const
    {
        ASSERT(m_renderpass != VK_NULL_HANDLE, "nullptr");
        return m_renderpass;
    }

    inline const VulkanRenderPass::VulkanAttachmentDescription& VulkanRenderPass::getAttachmentDescription(u32 index) const
    {
        ASSERT(index < m_descriptions.size(), "range out");
        return m_descriptions[index];
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace vk
} //namespace renderer
} //namespace v3d
#endif //VULKAN_RENDER
