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

    unsetDirty(DiryStateMask::DiryState_ImageBarriers);
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
    std::fill(_clearValues.begin(), _clearValues.end(), VkClearValue{});
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
