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
        DiryState_Viewport = 0,
        DiryState_Scissors = 1,
        DiryState_StencilRef = 2,

        DiryState_Pipeline = 3,
        DiryState_RenderPass = 4,

        DiryState_ImageBarriers = 10,
        DiryState_BufferBarriers = 10,

        DiryState_DescriptorSet = 11,
        DiryState_DescriptorSetShift = (11 + k_maxDescriptorSetCount - 1),

        DiryState_All = 0x0/*0xFFFFFFFFFFFFFFFF*/
    };

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

            _clearValues.resize(k_maxColorAttachments + 1);
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
            std::swap(_clearValues, other._clearValues);
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

        void bind(BindingType type, u32 set, u32 binding, VulkanBuffer* buffer, u32 offset, u32 range);
        void bind(BindingType type, u32 set, u32 binding, u32 arrayIndex, VulkanImage* image, const RenderTexture::Subresource& subresource);
        void bind(BindingType type, u32 set, u32 binding, VulkanSampler* sampler);

        void invalidate();

        void setDirty(DiryStateMask mask);
        void unsetDirty(DiryStateMask mask);
        bool isDirty(DiryStateMask mask);

        VkViewport                                       _viewports = {};
        VkRect2D                                         _scissors = {};
        VkStencilFaceFlags                               _stencilMask = VK_STENCIL_FACE_FRONT_AND_BACK;
        u32                                              _stencilRef = 0;
                                                         
        VulkanGraphicPipeline*                           _graphicPipeline = nullptr;
        VulkanComputePipeline*                           _computePipeline = nullptr;

        VulkanRenderPass*                                _renderpass = nullptr;
        VulkanFramebuffer*                               _framebuffer = nullptr;
        VkRect2D                                         _renderArea = {};
        std::vector<VkClearValue>                        _clearValues;
        bool                                             _insideRenderpass = false;

        SetInfo                                         _sets[k_maxDescriptorSetCount];
        std::vector<VkDescriptorSet>                    _descriptorSets;
        std::vector<u32>                                _dynamicOffsets;

    private:

        std::multimap<VkImageLayout, std::tuple<VulkanImage*, RenderTexture::Subresource>>  _imageBarriers;

        u64 _dirty;
    };

    inline void VulkanRenderState::setDirty(DiryStateMask mask)
    {
        _dirty |= (1 << mask);
    }

    inline void VulkanRenderState::unsetDirty(DiryStateMask mask)
    {
        _dirty &= ~(1 << mask);
    }

    inline bool VulkanRenderState::isDirty(DiryStateMask mask)
    {
        return _dirty & (1 << mask);
    }

    inline void VulkanRenderState::bind(BindingType type, u32 set, u32 binding, VulkanBuffer* buffer, u32 offset, u32 range)
    {
        ASSERT(buffer, "must be valid");
        ASSERT(type == BindingType::Uniform || type == BindingType::DynamicUniform, "wrong type");
        BindingInfo& bindingInfo = _sets[set]._bindings[binding];
        bindingInfo._binding = binding;
        bindingInfo._arrayIndex = 0;
        bindingInfo._type = type;
        bindingInfo._info._bufferInfo = makeVkDescriptorBufferInfo(buffer, static_cast<u64>(offset), static_cast<u64>(range));

        _sets[set]._resource[binding] = buffer;
        _sets[set]._activeBindingsFlags |= 1 << binding;
        setDirty(DiryStateMask(DiryState_DescriptorSet + set));

        if (type == BindingType::DynamicUniform)
        {
            bindingInfo._info._bufferInfo.offset = 0;
            _dynamicOffsets.push_back(offset);
        }
    }

    inline void VulkanRenderState::bind(BindingType type, u32 set, u32 binding, u32 arrayIndex, VulkanImage* image, const RenderTexture::Subresource& subresource)
    {
        ASSERT(type == BindingType::Texture || type == BindingType::RWTexture, "wrong type");
        VkImageLayout layout = (type == BindingType::RWTexture) ? VK_IMAGE_LAYOUT_GENERAL : VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        BindingInfo& bindingInfo = _sets[set]._bindings[binding];
        bindingInfo._binding = binding;
        bindingInfo._arrayIndex = arrayIndex;
        bindingInfo._type = type;
        bindingInfo._info._imageInfo = makeVkDescriptorImageInfo(image, nullptr, layout, subresource);

        _sets[set]._resource[binding] = image;
        _sets[set]._activeBindingsFlags |= 1 << binding;
        setDirty(DiryStateMask(DiryState_DescriptorSet + set));

        addImageBarrier(image, subresource, layout);
    }

    inline void VulkanRenderState::bind(BindingType type, u32 set, u32 binding, VulkanSampler* sampler)
    {
        ASSERT(type == BindingType::Sampler, "wrong type");
        BindingInfo& bindingInfo = _sets[set]._bindings[binding];
        bindingInfo._binding = binding;
        bindingInfo._arrayIndex = 0;
        bindingInfo._type = type;
        bindingInfo._info._imageInfo = makeVkDescriptorImageInfo(nullptr, sampler, VK_IMAGE_LAYOUT_UNDEFINED, {});

        _sets[set]._resource[binding] = sampler;
        _sets[set]._activeBindingsFlags |= 1 << binding;
        setDirty(DiryStateMask(DiryState_DescriptorSet + set));
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace vk
} //namespace renderer
} //namespace v3d
#endif //VULKAN_RENDER
