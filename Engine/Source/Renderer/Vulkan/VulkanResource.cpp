#include "VulkanResource.h"

#ifdef VULKAN_RENDER
#   include "VulkanWrapper.h"
#   include "VulkanCommandBuffer.h"
#   include "VulkanFence.h"
#   include "VulkanImage.h"

namespace v3d
{
namespace renderer
{
namespace vk
{

VulkanResource::VulkanResource() noexcept
    : m_fanceInfo()
#if VULKAN_DEBUG
    , m_refCount(0)
#endif
{
}

VulkanResource::~VulkanResource()
{
    ASSERT(!isUsed(), "still used");
}

VulkanResourceDeleter::~VulkanResourceDeleter()
{
    ASSERT(m_delayedList.empty(), "should be empty");
    ASSERT(m_deleterList.empty(), "should be empty");
}


void VulkanResourceDeleter::addResourceToDelete(VulkanResource* resource, const std::function<void(VulkanResource* resource)>& deleter, bool forceDelete)
{
    std::lock_guard lock(m_mutex);

    if (forceDelete) [[unlikely]]
    {
        deleter(resource);
    }
    else
    {
        if (resource->isUsed())
        {
            [[maybe_unused]] auto inserted = m_delayedList.emplace(resource, deleter);
            ASSERT(inserted.second, "already in the list");
        }
        else
        {
            deleter(resource);
        }
    }
}

void VulkanResourceDeleter::updateResourceDeleter(bool forceDelete)
{
    std::lock_guard lock(m_mutex);

    ASSERT(m_deleterList.empty(), "should be empty");
    std::queue<std::pair<VulkanResource*, std::function<void(VulkanResource* resource)>>> delayedList;
    while(!m_delayedList.empty())
    {
        auto iter = m_delayedList.front();
        m_delayedList.pop();

        if (!iter.first->isUsed() || forceDelete)
        {
            m_deleterList.push(iter);
        }
        else
        {
            delayedList.push(iter);
        }
    }

    ASSERT(m_delayedList.empty(), "should be empty");
    m_delayedList.swap(delayedList);

    resourceGarbageCollect();
}

void VulkanResourceDeleter::resourceGarbageCollect()
{
    std::lock_guard lock(m_mutex);

    while (!m_deleterList.empty())
    {
        auto iter = m_deleterList.front();
        m_deleterList.pop();

        iter.second(iter.first);
    }
}


VkImageLayout VulkanResourceStateTracker::getLayout(VulkanImage* image, const RenderTexture::Subresource& resource) const
{
    auto iter = m_states.find(image);
    if (iter == m_states.end())
    {
        auto result = m_states.emplace(image, std::make_tuple(image->m_globalLayout, image->m_globalLayout));
        ASSERT(result.second, "must be valid");
        iter = result.first;
    }

    auto& currentLayouts = std::get<1>(iter->second);
    u32 index = 1 + (resource._baseLayer * image->m_mipLevels + resource._baseMip);
    ASSERT(index < currentLayouts.size(), "out of range");
    return currentLayouts[index];
}

VkImageLayout VulkanResourceStateTracker::setLayout(VulkanImage* image, VkImageLayout newlayout, const RenderTexture::Subresource& resource)
{
    auto iter = m_states.find(image);
    if (iter == m_states.end())
    {
        //auto result = m_states.emplace(image, std::make_tuple(image->m_globalLayout, image->m_globalLayout));
        auto result = m_states.emplace(image, std::make_tuple(newlayout, newlayout));
        ASSERT(result.second, "must be valid");
        iter = result.first;
    }

    auto& currentLayouts = std::get<1>(iter->second);
    VkImageLayout oldLayout = currentLayouts.front();
    for (u32 layerIndex = 0; layerIndex < resource._layers; ++layerIndex)
    {
        for (u32 mipIndex = 0; mipIndex < resource._mips; ++mipIndex)
        {
            u32 index = 1 + ((resource._baseLayer + layerIndex) * image->m_mipLevels + (resource._baseMip + mipIndex));
            ASSERT(index < currentLayouts.size(), "out of range");
            [[maybe_unused]] VkImageLayout layout = std::exchange(currentLayouts[index], newlayout);
        }
    }

    currentLayouts.front() = newlayout; //General layout. Need do for every layer and mip
    return oldLayout;
}

void VulkanResourceStateTracker::finalizeGlobalState()
{
    for (auto& [image, layouts] : m_states)
    {
        image->m_globalLayout = std::get<1>(layouts);
    }
    m_states.clear();
}

void VulkanResourceStateTracker::prepareGlobalState(const std::function<VulkanCommandBuffer*()>& cmdBufferGetter)
{
    for (auto& [image, layouts] : m_states)
    {
        auto& beginLayouts = std::get<0>(layouts);
        auto& finalLayouts = std::get<1>(layouts);
        ASSERT(beginLayouts.size() == finalLayouts.size() && beginLayouts.size() == image->m_globalLayout.size(), "must has the same size");

        if (image->m_globalLayout != beginLayouts)
        {
            //general layout
            {
                RenderTexture::Subresource resource{ 0, image->getArrayLayers(), 0, image->m_mipLevels };
                VkImageLayout oldLayout = image->m_globalLayout[0];
                VkImageLayout newLayout = beginLayouts[0];
                if (oldLayout != newLayout)
                {
                    if (newLayout != VK_IMAGE_LAYOUT_UNDEFINED) //skip unknown
                    {
                        VulkanCommandBuffer* cmdBuffer = cmdBufferGetter();
                        cmdBuffer->cmdPipelineBarrier(image, resource, oldLayout, newLayout);
                    }

                    image->setGlobalLayout(newLayout, resource);
                }
            }

            for (u32 layerIndex = 0; layerIndex < image->getArrayLayers(); ++layerIndex)
            {
                for (u32 mipIndex = 0; mipIndex < image->m_mipLevels; ++mipIndex)
                {
                    RenderTexture::Subresource resource{ layerIndex, 1, mipIndex, 1 };
                    u32 index = 1 + (resource._baseLayer * image->m_mipLevels + resource._baseMip);
                    ASSERT(index < image->m_globalLayout.size(), "out of range");

                    VkImageLayout oldLayout = image->m_globalLayout[index];
                    VkImageLayout newLayout = beginLayouts[index];
                    if (oldLayout != newLayout)
                    {
                        if (newLayout != VK_IMAGE_LAYOUT_UNDEFINED) //skip unknown
                        {
                            VulkanCommandBuffer* cmdBuffer = cmdBufferGetter();
                            cmdBuffer->cmdPipelineBarrier(image, resource, oldLayout, newLayout);
                        }

                        image->setGlobalLayout(newLayout, resource);
                    }
                }
            }
        }
    }
}

} //namespace vk
} //namespace renderer
} //namespace v3d
#endif //VULKAN_RENDER
