#pragma once

#include "Renderer/Render.h"
#include "Renderer/RenderTargetState.h"

#ifdef VULKAN_RENDER
#   include "VulkanWrapper.h"
#   include "VulkanResource.h"

namespace v3d
{
namespace renderer
{
namespace vk
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    class VulkanDevice;

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief VulkanRenderpass final class. Vulkan Render side
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

        explicit VulkanRenderPass(VulkanDevice* device, const RenderPassDesc& description, const std::string& name = "") noexcept;
        ~VulkanRenderPass();

        VkRenderPass getHandle() const;

        const VulkanAttachmentDescription& getAttachmentDescription(u32 index) const;
        u32 getCountAttachments() const;

        template<u32 status>
        VkImageLayout getAttachmentLayout(u32 index) const
        {
            ASSERT(index < m_layout.size(), "range out");
            static_assert(status <std::tuple_size<LayoutState>::value);

            return std::get<status>(m_layout[index]);
        }

        bool isDrawingToSwapchain() const;

        bool create();
        void destroy();

    private:

        bool createRenderpass();
        bool createRenderpass2();

        VulkanDevice&                               m_device;
        VkRenderPass                                m_renderpass;

        using LayoutState = std::tuple<VkImageLayout, VkImageLayout>;

        std::vector<LayoutState>                    m_layout;
        std::vector<VulkanAttachmentDescription>    m_descriptions;
        u32                                         m_viewsMask;

        bool                                        m_drawingToSwapchain;

#if VULKAN_DEBUG_MARKERS
        std::string m_debugName;
#endif //VULKAN_DEBUG_MARKERS
    };

    inline bool VulkanRenderPass::isDrawingToSwapchain() const
    {
        return m_drawingToSwapchain;
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

    inline u32 VulkanRenderPass::getCountAttachments() const
    {
        return static_cast<u32>(m_descriptions.size());
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief VulkanRenderpassManager class.
    * Multithreaded
    */
    class VulkanRenderpassManager final
    {
    public:

        explicit VulkanRenderpassManager(VulkanDevice* device) noexcept;
        ~VulkanRenderpassManager();

        [[nodiscard]] VulkanRenderPass* acquireRenderpass(const RenderPassDesc& description, const std::string& name = "");
        bool removeRenderPass(VulkanRenderPass* renderPass);

        void clear();

    private:

        VulkanRenderpassManager(const VulkanRenderpassManager&) = delete;
        VulkanRenderpassManager& operator=(const VulkanRenderpassManager&) = delete;

        VulkanDevice&        m_device;
        std::recursive_mutex m_mutex;

        std::unordered_map<DescInfo<RenderPassDesc>, VulkanRenderPass*, DescInfo<RenderPassDesc>::Hash, DescInfo<RenderPassDesc>::Compare> m_renderPassList;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace vk
} //namespace renderer
} //namespace v3d
#endif //VULKAN_RENDER
