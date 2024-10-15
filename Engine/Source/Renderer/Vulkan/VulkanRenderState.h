#pragma once

#include "Common.h"
#include "Renderer/Pipeline.h"

#ifdef VULKAN_RENDER
#   include "VulkanWrapper.h"
#   include "VulkanDeviceCaps.h"

namespace v3d
{
namespace renderer
{
namespace vk
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    class VulkanImage;
    class VulkanRenderPass;
    class VulkanFramebuffer;
    class VulkanCommandBuffer;

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief VulkanRenderState class. Vulkan Render side
    */
    struct VulkanRenderState final
    {
    public:

        VulkanRenderState() noexcept
            : _dirty(DiryMask::All)
        {
        }

        VulkanRenderState& operator=(VulkanRenderState&& other) noexcept
        {
            _viewports = other._viewports;
            _scissors = other._scissors;
            _stencilRef = other._stencilRef;

            _renderpass = other._renderpass;
            _framebuffer = other._framebuffer;
            _insideRenderpass = other._insideRenderpass;

            _imageBarriers = std::move(_imageBarriers);

            _dirty = other._dirty;

            return *this;
        }

        enum DiryMask
        {
            Viewport = 1 << 0,
            Scissors = 1 << 1,
            StencilRef = 1 << 2,

            Pipeline = 1 << 3,
            RenderPass = 1 << 4,
            Framebuffer = 1 << 5,

            ImageBarrier = 1 << 10,

            All = 0xFFFFFFFF
        };

        void addImageBarrier(const VulkanImage* texture, const RenderTexture::Subresource& subresource, VkImageLayout layout);
        void flushBarriers(VulkanCommandBuffer* cmdBuffer);

        void invalidate();

        void setDirty(DiryMask mask);

        VkViewport _viewports = {};
        VkRect2D   _scissors = {};
        u32        _stencilRef = 0;

        RenderPipeline*     _pipeline = nullptr;
        VulkanRenderPass*   _renderpass = nullptr;
        VulkanFramebuffer*  _framebuffer = nullptr;
        bool _insideRenderpass = false;

    private:

        std::multimap<VkImageLayout, std::tuple<const VulkanImage*, RenderTexture::Subresource>> _imageBarriers;

        u64        _dirty;
    };

    inline void VulkanRenderState::setDirty(VulkanRenderState::DiryMask mask)
    {
        _dirty |= mask;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace vk
} //namespace renderer
} //namespace v3d
#endif //VULKAN_RENDER
