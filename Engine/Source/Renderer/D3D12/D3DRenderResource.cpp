#include "D3DRenderResource.h"

#ifdef D3D_RENDER
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

} //namespace dx3d
} //namespace renderer
} //namespace v3d
#endif //D3D_RENDER