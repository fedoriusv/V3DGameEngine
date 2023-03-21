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

    class D3DBuffer;
    class D3DHeapAllocator;

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief D3DConstanBufferManager final class. DirectX Render side
    */
    class D3DConstantBufferManager final
    {
    public:

        explicit D3DConstantBufferManager(ID3D12Device* device, D3DHeapAllocator* allocator = nullptr) noexcept;
        ~D3DConstantBufferManager();

        std::tuple<D3DBuffer*, u32> acquireConstanBuffer(u64 requestSize);

        void updateConstantBufferStatus();
        void destroyConstantBuffers();

    private:

        ID3D12Device* const m_device;
        D3DHeapAllocator* const m_allocator;

        std::queue<D3DBuffer*> m_freeConstantBuffers;
        std::list<D3DBuffer*> m_usedConstantBuffers;
        std::tuple<D3DBuffer*, u32> m_currentConstantBuffer;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace dx3d
} //namespace renderer
} //namespace v3d
#endif //D3D_RENDER
