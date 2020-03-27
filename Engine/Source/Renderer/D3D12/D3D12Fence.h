#pragma once

#include "Common.h"

#ifdef D3D_RENDER
#   include "D3D12Configuration.h"

namespace v3d
{
namespace renderer
{
namespace dx3d
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
        bool signal(ID3D12CommandQueue* queue);
        bool wait(u64 value);
        bool wait();

        void reset(u64 value = 0);

        bool completed(u64 value);
        bool completed();

        u64 incrementValue();
        u64 getValue() const;
        u64 getFenceValue() const;

        ID3D12Fence* getHandle() const;

    private:

        ID3D12Device* m_device;

        HANDLE m_fenceEvent;
        ID3D12Fence* m_fence;

        std::atomic<u64> m_value;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace v3d12
} //namespace renderer
} //namespace v3d
#endif //D3D_RENDER
