#pragma once

#include "Common.h"
#include "Renderer/Pipeline.h"

#ifdef VULKAN_RENDER
#   include "VulkanWrapper.h"
#   include "VulkanDeviceCaps.h"
#   include "VulkanDescriptorSet.h"

namespace v3d
{
namespace renderer
{
namespace vk
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    class VulkanResource;
    class VulkanImage;
    class VulkanBuffer;
    class VulkanSampler;
    class VulkanRenderPass;
    class VulkanFramebuffer;
    class VulkanCommandBuffer;
    class VulkanGraphicPipeline;
    class VulkanComputePipeline;

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    enum DiryStateMask
    {
        DiryState_Viewport = 1 << 0,
        DiryState_Scissors = 1 << 1,
        DiryState_StencilRef = 1 << 2,

        DiryState_Pipeline = 1 << 3,
        DiryState_RenderPass = 1 << 4,

        DiryState_ImageBarriers = 1 << 10,
        DiryState_BufferBarriers = 1 << 10,

        DiryState_DescriptorSet = 1 << 11,
        DiryState_DescriptorSetShift = 1 << (11 + k_maxDescriptorSetCount - 1),

        DiryState_All = 0xFFFFFFFFFFFFFFFF
    };

    typedef u64 DiryStateMaskFlags;

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief VulkanRenderState class. Vulkan Render side
    */
    struct VulkanRenderState final
    {
    public:

        VulkanRenderState() noexcept
            : _dirty(DiryStateMask::DiryState_All)
        {
        }

        VulkanRenderState& operator=(VulkanRenderState&& other) noexcept
        {
            _viewports = other._viewports;
            _scissors = other._scissors;
            _stencilMask = other._stencilMask;
            _stencilRef = other._stencilRef;

            _graphicPipeline = other._graphicPipeline;
            _computePipeline = other._computePipeline;

            _renderpass = other._renderpass;
            _framebuffer = other._framebuffer;
            _renderArea = other._renderArea;
            _clearValues = other._clearValues;
            _insideRenderpass = other._insideRenderpass;

            for (u32 i = 0; i < k_maxDescriptorSetCount; ++i)
            {
                _sets[i] = other._sets[i];
            }
            _descriptorSets = other._descriptorSets;
            _dynamicOffsets = other._dynamicOffsets;


            _imageBarriers = std::move(_imageBarriers);

            _dirty = other._dirty;

            return *this;
        }

        void addImageBarrier(VulkanImage* texture, const RenderTexture::Subresource& subresource, VkImageLayout layout);
        void flushBarriers(VulkanCommandBuffer* cmdBuffer);

        void bind(BindingType type, u32 set, u32 binding, VulkanBuffer* buffer, u64 offset, u64 range);
        void bind(BindingType type, u32 set, u32 binding, u32 arrayIndex, VulkanImage* image, const RenderTexture::Subresource& subresource, VulkanSampler* sampler);

        void invalidate();

        void setDirty(DiryStateMaskFlags mask);
        void unsetDirty(DiryStateMaskFlags mask);
        bool isDirty(DiryStateMaskFlags mask);

        VkViewport                                       _viewports = {};
        VkRect2D                                         _scissors = {};
        VkStencilFaceFlags                               _stencilMask = VK_STENCIL_FACE_FRONT_AND_BACK;
        u32                                              _stencilRef = 0;
                                                         
        VulkanGraphicPipeline*                           _graphicPipeline = nullptr;
        VulkanComputePipeline*                           _computePipeline = nullptr;

        VulkanRenderPass*                                _renderpass = nullptr;
        VulkanFramebuffer*                               _framebuffer = nullptr;
        VkRect2D                                         _renderArea = {};
        std::array<VkClearValue, k_maxColorAttachments>  _clearValues;
        bool                                             _insideRenderpass = false;

        SetInfo                                         _sets[k_maxDescriptorSetCount];
        std::vector<VkDescriptorSet>                    _descriptorSets;
        std::vector<u32>                                _dynamicOffsets;

    private:

        std::multimap<VkImageLayout, std::tuple<VulkanImage*, RenderTexture::Subresource>>  _imageBarriers;

        DiryStateMaskFlags _dirty;
    };

    inline void VulkanRenderState::setDirty(DiryStateMaskFlags mask)
    {
        _dirty |= mask;
    }

    inline void VulkanRenderState::unsetDirty(DiryStateMaskFlags mask)
    {
        _dirty &= ~mask;
    }

    inline bool VulkanRenderState::isDirty(DiryStateMaskFlags mask)
    {
        return _dirty & mask;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace vk
} //namespace renderer
} //namespace v3d
#endif //VULKAN_RENDER
