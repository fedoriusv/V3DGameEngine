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

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * D3DConstanBufferManager final class. DirectX Render side
    */
    class D3DConstantBufferManager final
    {
    public:

        explicit D3DConstantBufferManager(ID3D12Device* device) noexcept;
        ~D3DConstantBufferManager();

        D3DBuffer* acquireConstanBuffer(u64 requestSize);

        void updateStatus();

    private:

        ID3D12Device* m_device;

        std::queue<D3DBuffer*> m_freeConstantBuffers;
        std::list<D3DBuffer*> m_usedConstantBuffers;

        static const u32 k_constantBufferSize = 1024U;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace dx3d
} //namespace renderer
} //namespace v3d
#endif //D3D_RENDER
