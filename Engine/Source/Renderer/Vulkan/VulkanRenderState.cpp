#include "VulkanRenderState.h"

#ifdef VULKAN_RENDER
#include "VulkanDeviceCaps.h"
#include "VulkanImage.h"
#include "VulkanBuffer.h"
#include "VulkanSampler.h"
#include "VulkanCommandBufferManager.h"

namespace v3d
{
namespace renderer
{
namespace vk
{

void VulkanRenderState::addImageBarrier(VulkanImage* texture, const RenderTexture::Subresource& subresource, VkImageLayout layout)
{
    if (_imageBarriers.contains(layout))
    {
        //TODO: check
        auto found = _imageBarriers.equal_range(layout);
        for (auto iter = found.first; iter != found.second; ++iter)
        {
            if (std::get<0>(iter->second) == texture && std::get<1>(iter->second) == subresource)
            {
                if (std::get<0>(iter->second)->getLayout(subresource) == layout)
                {
                    //don"t add duplicates
                    return;
                }
            }
        }
    }
    _imageBarriers.emplace(layout, std::make_tuple(texture, subresource));

    setDirty(DiryStateMask::DiryState_ImageBarriers);
}

void VulkanRenderState::flushBarriers(VulkanCommandBuffer* cmdBuffer)
{
    for (auto& image : _imageBarriers)
    {
        VulkanTransitionState::transitionImage(cmdBuffer, std::get<1>(image), std::get<0>(image));
    }
    _imageBarriers.clear();

    _dirty &= ~DiryStateMask::DiryState_ImageBarriers;
}

void VulkanRenderState::bind(BindingType type, u32 set, u32 binding, VulkanBuffer* buffer, u64 offset, u64 range)
{
    ASSERT(buffer, "must be valid");
    BindingInfo& bindingInfo = _sets[set]._bindings[binding];
    bindingInfo._binding = binding;
    bindingInfo._arrayIndex = 0;
    bindingInfo._type = type;
    bindingInfo._info._bufferInfo = makeVkDescriptorBufferInfo(buffer, offset, range);

    _sets[set]._resource[binding] = buffer;
    _sets[set]._activeBindingsFlags |= 1 << binding;
    setDirty(DiryStateMask::DiryState_DescriptorSet + set);

    if (type == BindingType::DynamicUniform)
    {
        bindingInfo._info._bufferInfo.offset = 0;
        _dynamicOffsets.push_back(static_cast<u32>(offset));
    }
}

void VulkanRenderState::bind(BindingType type, u32 set, u32 binding, u32 arrayIndex, VulkanImage* image, const RenderTexture::Subresource& subresource, VulkanSampler* sampler)
{
    //collect resources
}

void VulkanRenderState::invalidate()
{
    _viewports = {};
    _scissors = {};
    _stencilMask = VK_STENCIL_FACE_FRONT_AND_BACK;
    _stencilRef = 0;

    _graphicPipeline = nullptr;
    _computePipeline = nullptr;

    _renderpass = nullptr;
    _framebuffer = nullptr;
    _renderArea = {};
    _clearValues = {};
    _insideRenderpass = false;

    for (u32 i = 0; i < k_maxDescriptorSetCount; ++i)
    {
        _sets[i] = {};
    }
    _descriptorSets.clear();
    _dynamicOffsets.clear();

    _imageBarriers.clear();

    _dirty = DiryStateMask::DiryState_All;
}

} //namespace vk
} //namespace renderer
} //namespace v3d
#endif //VULKAN_RENDER
