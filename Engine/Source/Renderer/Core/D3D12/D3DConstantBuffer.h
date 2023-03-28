#pragma once

#include "Common.h"

#ifdef D3D_RENDER
#include "D3DConfiguration.h"
#include "D3DBuffer.h"

namespace v3d
{
namespace renderer
{
namespace dx3d
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    class D3DBuffer;
    class D3DHeapAllocator;

    /**
    * @brief D3DConstantBuffer final class. DirectX Render side
    */
    class D3DConstantBuffer final
    {
    public:

        D3DConstantBuffer(ID3D12Device* device, u32 size, D3DHeapAllocator* allocator) noexcept;
        ~D3DConstantBuffer();

        u32 getSize() const;
        bool isUsed() const;
        D3DBuffer* getBuffer() const;

        bool create();
        void destroy();

        void update(u32 offset, u32 size, const void* data);

    private:

        D3DBuffer* m_buffer;
        void* m_mappedData;
    };

    inline u32 D3DConstantBuffer::getSize() const
    {
        ASSERT(m_buffer, "nullptr");
        return m_buffer->getSize();
    }

    inline D3DBuffer* D3DConstantBuffer::getBuffer() const
    {
        ASSERT(m_buffer, "nullptr");
        return m_buffer;
    }

    inline bool D3DConstantBuffer::isUsed() const
    {
        ASSERT(m_buffer, "nullptr");
        return m_buffer->isUsed();
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief D3DConstanBufferManager final class. DirectX Render side
    */
    class D3DConstantBufferManager final
    {
    public:

        explicit D3DConstantBufferManager(ID3D12Device* device, D3DHeapAllocator* allocator = nullptr) noexcept;
        ~D3DConstantBufferManager();

        std::tuple<D3DConstantBuffer*, u32> acquireConstanBuffer(u64 requestSize);

        void updateConstantBufferStatus();
        void destroyConstantBuffers();

    private:

        ID3D12Device* const m_device;
        D3DHeapAllocator* const m_allocator;

        std::queue<D3DConstantBuffer*> m_freeConstantBuffers;
        std::list<D3DConstantBuffer*> m_usedConstantBuffers;
        std::tuple<D3DConstantBuffer*, u32> m_currentConstantBuffer;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace dx3d
} //namespace renderer
} //namespace v3d
#endif //D3D_RENDER
