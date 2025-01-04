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
            m_delayedList.emplace(resource, deleter);
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
        auto result = m_states.emplace(image, image->m_layout);
        ASSERT(result.second, "must be valid");
        iter = result.first;
    }

    auto& layouts = iter->second;
    u32 index = 1 + (resource._baseLayer * image->m_mipLevels + resource._baseMip);
    ASSERT(index < layouts.size(), "out of range");
    return layouts[index];
}

VkImageLayout VulkanResourceStateTracker::setLayout(VulkanImage* image, VkImageLayout newlayout, const RenderTexture::Subresource& resource)
{
    auto iter = m_states.find(image);
    if (iter == m_states.end())
    {
        auto result = m_states.emplace(image, image->m_layout);
        ASSERT(result.second, "must be valid");
        iter = result.first;
    }

    auto& layouts = iter->second;
    VkImageLayout oldLayout = layouts.front();
    for (u32 layerIndex = 0; layerIndex < resource._layers; ++layerIndex)
    {
        for (u32 mipIndex = 0; mipIndex < resource._mips; ++mipIndex)
        {
            u32 index = 1 + ((resource._baseLayer + layerIndex) * image->m_mipLevels + (resource._baseMip + mipIndex));
            ASSERT(index < layouts.size(), "out of range");
            [[maybe_unused]] VkImageLayout layout = std::exchange(layouts[index], newlayout);
        }
    }

    layouts.front() = newlayout; //General layout. Need do for every layer and mip
    return oldLayout;
}

void VulkanResourceStateTracker::finalizeGlobalState()
{
    for (auto& [image, layouts] : m_states)
    {
        image->m_layout = layouts;
    }
    m_states.clear();
}

} //namespace vk
} //namespace renderer
} //namespace v3d
#endif //VULKAN_RENDER
