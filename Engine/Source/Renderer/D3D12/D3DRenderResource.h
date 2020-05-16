#pragma once

#include "Common.h"

#ifdef D3D_RENDER
#include "D3DConfiguration.h"

namespace v3d
{
namespace renderer
{
namespace dx3d
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    class D3DFence;
    class D3DCommandList;

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * D3DRenderResource final class. DirectX Render side
    */
    class D3DRenderResource
    {

    public:

        explicit D3DRenderResource() noexcept;
        ~D3DRenderResource();

        bool isUsed() const;

    private:

        bool attachFence(D3DFence* fence, u64 frame = 0);
        bool detachFence(D3DFence* fence);
        void releaseFences();
    private:

        friend D3DCommandList;

        std::set<D3DFence*> m_fences;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace dx3d
} //namespace renderer
} //namespace v3d
#endif //D3D_RENDER
