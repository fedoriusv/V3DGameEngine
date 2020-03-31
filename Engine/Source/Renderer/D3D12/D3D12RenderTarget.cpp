#include "D3D12RenderTarget.h"

#ifdef D3D_RENDER

namespace v3d
{
namespace renderer
{
namespace dx3d
{

D3DRenderTarget::D3DRenderTarget()
{
}

D3DRenderTarget::~D3DRenderTarget()
{
}

bool D3DRenderTarget::create(const RenderPass* pass)
{
    return false;
}

void D3DRenderTarget::destroy()
{
}

} //namespace dx3d
} //namespace renderer
} //namespace v3d
#endif //D3D_RENDER