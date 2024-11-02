#include "VulkanResource.h"

#ifdef VULKAN_RENDER
#   include "VulkanWrapper.h"
#   include "VulkanCommandBuffer.h"
#   include "VulkanFence.h"

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
    std::scoped_lock lock(m_mutex);

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
    std::scoped_lock lock(m_mutex);

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
    std::scoped_lock lock(m_mutex);

    while (!m_deleterList.empty())
    {
        auto iter = m_deleterList.front();
        m_deleterList.pop();

        iter.second(iter.first);
    }
}

} //namespace vk
} //namespace renderer
} //namespace v3d
#endif //VULKAN_RENDER
