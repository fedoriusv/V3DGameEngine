#include "D3DResource.h"

#ifdef D3D_RENDER
#include "D3DFence.h"

namespace v3d
{
namespace renderer
{
namespace dx3d
{

D3DResource::D3DResource() noexcept
{
}

D3DResource::~D3DResource()
{
    ASSERT(!isUsed(), "still is used");
}

bool D3DResource::attachFence(D3DFence* fence, u64 frame)
{
    auto iter = m_fences.insert(fence);
    if (iter.second)
    {
        return true;
    }

    return false;
}

bool D3DResource::detachFence(D3DFence* fence)
{
    auto iter = m_fences.find(fence);
    if (iter != m_fences.end())
    {
        m_fences.erase(iter);
        return true;
    }

    return false;
}

void D3DResource::releaseFences()
{
    m_fences.clear();
}

bool D3DResource::isUsed() const
{
    return !m_fences.empty();
}

void D3DResource::waitToComplete()
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

void D3DResourceDeleter::requestToDelete(D3DResource* resource, const std::function<void(void)>& deleter)
{
    m_delayedList.emplace(resource, deleter);
}

void D3DResourceDeleter::update(bool wait)
{
    ASSERT(m_deleterList.empty(), "should be empty");
    std::queue<std::pair<D3DResource*, std::function<void(void)>>> delayedList;
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