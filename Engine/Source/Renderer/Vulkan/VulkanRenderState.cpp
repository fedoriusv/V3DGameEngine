#include "VulkanRenderState.h"

#ifdef VULKAN_RENDER
#include "VulkanDeviceCaps.h"
#include "VulkanImage.h"
#include "VulkanBuffer.h"
#include "VulkanSampler.h"
#include "VulkanCommandBufferManager.h"
#include "VulkanCommandBuffer.h"

namespace v3d
{
namespace renderer
{
namespace vk
{

VulkanRenderState::VulkanRenderState() noexcept
    : _dirty(DirtyStateMask::DirtyState_All)
{
}

VulkanRenderState::~VulkanRenderState()
{
    for (u32 type = toEnumType(ShaderType::First); type <= (u32)toEnumType(ShaderType::Last); ++type)
    {
        if (_pushConstant[type]._data)
        {
            V3D_FREE(_pushConstant[type]._data, memory::MemoryLabel::MemoryRenderCore);
        }
    }
}

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
                return;
            }
        }
    }
    _imageBarriers.emplace(layout, std::make_tuple(texture, subresource));

    setDirty(DirtyStateMask::DirtyState_Barriers);
}

void VulkanRenderState::flushBarriers(VulkanCommandBuffer* cmdBuffer)
{
    for (auto& image : _imageBarriers)
    {
        if (cmdBuffer->getResourceStateTracker().getLayout(std::get<0>(image.second), std::get<1>(image.second)) == image.first)
        {
            //don"t add duplicates
            continue;
        }
        VulkanTransitionState::transitionImage(cmdBuffer, std::get<1>(image), std::get<0>(image));
    }
    _imageBarriers.clear();

    unsetDirty(DirtyStateMask::DirtyState_Barriers);
}

void VulkanRenderState::init(VulkanDevice* device)
{
    _clearValues.resize(k_maxColorAttachments + 1);
    memset(&_boundSetInfo[0], 0, sizeof(_boundSetInfo));
    memset(&_boundSets[0], 0, sizeof(_boundSets));

    u32 maxSize = device->getVulkanDeviceCaps().getPhysicalDeviceLimits().maxPushConstantsSize;
    for (u32 type = toEnumType(ShaderType::First); type <= (u32)toEnumType(ShaderType::Last); ++type)
    {
        _pushConstant[type]._size = maxSize;
        _pushConstant[type]._data = V3D_MALLOC(maxSize, memory::MemoryLabel::MemoryRenderCore);
    }
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
        _boundSetInfo[i] = {};
        _boundSets[i] = VK_NULL_HANDLE;
    }
    _descriptorSets.clear();
    _dynamicOffsets.clear();

#ifdef DEBUG
    for (u32 type = toEnumType(ShaderType::First); type <= (u32)toEnumType(ShaderType::Last); ++type)
    {
        memset(_pushConstant[type]._data, 0, sizeof(_pushConstant[type]._size));
    }
#endif

    _imageBarriers.clear();

    _dirty = DirtyStateMask::DirtyState_All;
}

} //namespace vk
} //namespace renderer
} //namespace v3d
#endif //VULKAN_RENDER
