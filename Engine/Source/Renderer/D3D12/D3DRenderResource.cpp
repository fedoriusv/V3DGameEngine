#include "D3DRenderResource.h"

#ifdef D3D_RENDER
#include "D3DFence.h"

namespace v3d
{
namespace renderer
{
namespace dx3d
{

D3DRenderResource::D3DRenderResource() noexcept
{
}

D3DRenderResource::~D3DRenderResource()
{
    ASSERT(!isUsed(), "still is used");
}

bool D3DRenderResource::attachFence(D3DFence* fence, u64 frame)
{
    auto iter = m_fences.insert(fence);
    if (iter.second)
    {
        return true;
    }

    return false;
}

bool D3DRenderResource::detachFence(D3DFence* fence)
{
    auto iter = m_fences.find(fence);
    if (iter != m_fences.end())
    {
        m_fences.erase(iter);
        return true;
    }

    return false;
}

void D3DRenderResource::releaseFences()
{
    m_fences.clear();
}

bool D3DRenderResource::isUsed() const
{
    return !m_fences.empty();
}

void D3DRenderResource::waitToComplete()
{
    for (auto fence : m_fences)
    {
        fence->wait();
    }

    releaseFences();
}


D3DResourceDeleter::~D3DResourceDeleter()
{
    ASSERT(m_delayedList.empty(), "should be empty");
    ASSERT(m_deleterList.empty(), "should be empty");
}

void D3DResourceDeleter::requestToDelete(D3DRenderResource* resource, const std::function<void(void)>& deleter)
{
    m_delayedList.emplace(resource, deleter);
}

void D3DResourceDeleter::update(bool wait)
{
    ASSERT(m_deleterList.empty(), "should be empty");
    std::queue<std::pair<D3DRenderResource*, std::function<void(void)>>> delayedList;
    while (!m_delayedList.empty())
    {
        auto iter = m_delayedList.front();
        m_delayedList.pop();

        if (iter.first->isUsed())
        {
            if (wait)
            {
                iter.first->waitToComplete();
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
    m_delayedList.swap(delayedList);
#if DEBUG
    if (wait)
    {
        ASSERT(m_delayedList.empty(), "should be empty");
    }
#endif //DEBUG

    garbageCollect();
}

void D3DResourceDeleter::garbageCollect()
{
    while (!m_deleterList.empty())
    {
        auto iter = m_deleterList.front();
        m_deleterList.pop();

        iter.second();
    }
}

} //namespace dx3d
} //namespace renderer
} //namespace v3d
#endif //D3D_RENDER