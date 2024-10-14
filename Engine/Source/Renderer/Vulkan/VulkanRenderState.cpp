#include "VulkanRenderState.h"

#ifdef VULKAN_RENDER
#include "VulkanDeviceCaps.h"
#include "VulkanImage.h"
#include "VulkanCommandBufferManager.h"

namespace v3d
{
namespace renderer
{
namespace vk
{

void VulkanRenderState::addImageBarrier(const VulkanImage* texture, const RenderTexture::Subresource& subresource, VkImageLayout layout)
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

    setDirty(DiryMask::ImageBarrier);
}

void VulkanRenderState::flushBarriers(VulkanCommandBuffer* cmdBuffer)
{
    for (auto& image : _imageBarriers)
    {
        VulkanTransitionState::transitionImage(cmdBuffer, std::get<1>(image), std::get<0>(image));
    }
    _imageBarriers.clear();

    _dirty &= ~DiryMask::ImageBarrier;
}

void VulkanRenderState::invalidate()
{
}

} //namespace vk
} //namespace renderer
} //namespace v3d
#endif //VULKAN_RENDER
