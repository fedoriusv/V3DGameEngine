#pragma once

#include "Common.h"

#ifdef D3D_RENDER
#   include "D3D12Configuration.h"

namespace v3d
{
namespace renderer
{
namespace d3d12
{

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * D3DFence final class. DirectX Render side
    */
    class D3DFence final
    {
    public:

        explicit D3DFence(ID3D12Device* device, u64 value = 0) noexcept;
        ~D3DFence();

        D3DFence() = delete;
        D3DFence(const D3DFence&) = delete;

        bool signal(u64 value);
        bool wait(u64 value);

        void reset(u64 value = 0);

        bool completed(u64 value);

        ID3D12Fence* getHandle() const;

    private:

        ID3D12Device* m_device;

        HANDLE m_fenceEvent;
        ID3D12Fence* m_fence;

        u64 m_value;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace vk
} //namespace renderer
} //namespace v3d12
#endif //D3D_RENDER
