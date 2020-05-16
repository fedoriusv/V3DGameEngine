#pragma once

#include "Common.h"

#ifdef D3D_RENDER
#include "D3DConfiguration.h"
#include "D3DCommandList.h"
#include "D3DPipelineState.h"

namespace v3d
{
namespace renderer
{
namespace dx3d
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    class D3DFence;

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * D3DCommandListManager final class. DirectX Render side
    */
    class D3DCommandListManager final
    {
    public:

        explicit D3DCommandListManager(ID3D12Device* device, ID3D12CommandQueue* commandQueue, u32 swapImageCount) noexcept;
        ~D3DCommandListManager();

        D3DGraphicsCommandList* acquireCommandList(D3DCommandList::Type type);

        void waitAndClear();

        bool execute(D3DCommandList* cmdList, bool wait = false);

        bool update(bool updateAll = false);
        bool wait(bool waitAll = false);

        bool sync(u32 index, bool wait = false);

    private:

        ID3D12Device* const m_device;
        ID3D12CommandQueue* m_commandQueue;

        ID3D12CommandAllocator* m_commandAllocator[D3DCommandList::Type::CountTypes];

        std::queue<D3DCommandList*> m_freeCommandList[D3DCommandList::Type::CountTypes];

        u32 m_currentIndex;
        std::vector<std::list<D3DCommandList*>> m_usedCommandList;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace dx3d
} //namespace renderer
} //namespace v3d
#endif //D3D_RENDER
