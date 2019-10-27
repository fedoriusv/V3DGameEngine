#pragma once

#include "Common.h"

#ifdef D3D_RENDER
#   include "D3D12Configuration.h"
#   include "D3D12CommandList.h"
#   include "D3D12PipelineState.h"

namespace v3d
{
namespace renderer
{
namespace d3d12
{
    class D3DFence;

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * D3DCommandListManager final class. DirectX Render side
    */
    class D3DCommandListManager final
    {
    public:

        D3DCommandListManager(ID3D12Device* device, ID3D12CommandQueue* commandQueue) noexcept;
        ~D3DCommandListManager();

        D3DGraphicsCommandList* acquireCommandList(D3DCommandList::Type type);

        void clear();

        void update();
        bool execute(D3DCommandList* cmdList, bool wait = false);

        bool wait();

    private:

        ComPtr<ID3D12Device> m_device;

        ComPtr<ID3D12CommandQueue> m_commandQueue;
        D3DFence* m_fence;

        std::queue<D3DCommandList*> m_freeCommandList[D3DCommandList::Type::CountTypes];
        std::list<D3DCommandList*> m_usedCommandList;

        //std::map<Pipeline*, D3DCommandList*> m_commandList;

    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace d3d12
} //namespace renderer
} //namespace v3d


#endif //D3D_RENDER
