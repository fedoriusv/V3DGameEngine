#include "VulkanResource.h"

#ifdef VULKAN_RENDER
#include "VulkanWrapper.h"
//#include "VulkanCommandBuffer.h"

namespace v3d
{
namespace renderer
{
namespace vk
{

VulkanResource::VulkanResource() noexcept
    : m_status(VulkanResource::Status::Status_Free)
    , m_counter(0)
    , m_frame(0)
{
}

VulkanResource::~VulkanResource()
{
    ASSERT(!isCaptured(), "still captured");
}

bool VulkanResource::waitCompletion(u64 time)
{
    //TODO might be problem with mt
    if (isCaptured())
    {
        bool result = true;
        for (auto& cmdBuffer : m_cmdBuffers)
        {
            //if (!cmdBuffer->waitComplete(time))
            //{
            //    result = false;
            //}
        }
        return result;
    }

    return true;
}

void VulkanResource::captureInsideCommandBuffer(VulkanCommandBuffer* buffer, u64 frame) const
{
    //buffer->captureResource(const_cast<VulkanResource*>(this), frame);
}


VulkanResourceDeleter::~VulkanResourceDeleter()
{
    ASSERT(m_delayedList.empty(), "should be empty");
    ASSERT(m_deleterList.empty(), "should be empty");
}

void VulkanResourceDeleter::addResourceToDelete(VulkanResource* resource, const std::function<void(VulkanResource* resource)>& deleter, bool forceDelete)
{
    if (forceDelete)
    {
        //resource->waitComplete();
        deleter(resource);
    }
    else
    {
        if (resource->isCaptured())
        {
            m_delayedList.emplace(resource, deleter);
        }
        else
        {
            deleter(resource);
        }
    }
}

void VulkanResourceDeleter::updateResourceDeleter(bool wait)
{
    ASSERT(m_deleterList.empty(), "should be empty");
    std::queue<std::pair<VulkanResource*, std::function<void(VulkanResource* resource)>>> delayedList;
    while(!m_delayedList.empty())
    {
        auto iter = m_delayedList.front();
        m_delayedList.pop();

        if (iter.first->isCaptured())
        {
            if (wait)
            {
                //iter.first->waitComplete();
                m_deleterList.push(iter);
            }
            else
            {
                delayedList.push(iter);
            }
        }
        else
        {
            m_deleterList.push(iter);
        }
    }

    ASSERT(m_delayedList.empty(), "should be empty");
    m_delayedList.swap(std::move(delayedList));
#if DEBUG
    if (wait)
    {
        ASSERT(m_delayedList.empty(), "should be empty");
    }
#endif //DEBUG

    resourceGarbageCollect();
}

void VulkanResourceDeleter::resourceGarbageCollect()
{
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
