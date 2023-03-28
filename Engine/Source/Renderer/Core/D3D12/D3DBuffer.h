#pragma once

#include "Common.h"
#include "Renderer/BufferProperties.h"
#include "Renderer/Core/Buffer.h"

#ifdef D3D_RENDER
#include "D3DConfiguration.h"
#include "D3DResource.h"
#include "D3DMemoryHeap.h"

namespace v3d
{
namespace renderer
{
namespace dx3d
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    class D3DHeapAllocator;

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief D3DBuffer final class. DirectX Render side
    */
    class D3DBuffer final : public Buffer, public D3DResource
    {
    public:

        explicit D3DBuffer(ID3D12Device* device, Buffer::BufferType type, StreamBufferUsageFlags usageFlag, u64 size, const std::string& name = "", D3DHeapAllocator* allocator = nullptr) noexcept;
        ~D3DBuffer();

        D3DBuffer() = delete;
        D3DBuffer(const D3DBuffer&) = delete;

        bool create() override;
        void destroy() override;

        u64 getSize() const;

        bool write(Context* context, u32 offset, u64 size, const void* data) override;
        bool read(Context* context, u32 offset, u64 size, const std::function<void(u32, void*)>& readback) override;

        void* map(u32 offset, u64 size, bool reading = false);
        void unmap(u32 offset, u64 size);

        ID3D12Resource* getResource() const;
        D3D12_GPU_VIRTUAL_ADDRESS getGPUAddress() const;

        D3D12_RESOURCE_STATES getState() const;
        D3D12_RESOURCE_STATES setState(D3D12_RESOURCE_STATES state);

#if DEBUG_OBJECT_MEMORY
        static std::set<D3DBuffer*> s_objects;
#endif //DEBUG_OBJECT_MEMORY

    private:

        ID3D12Device* m_device;
        D3DHeapAllocator* const m_allocator;

        ID3D12Resource* m_resource;

        CD3DX12_HEAP_PROPERTIES m_heapProperties;
        D3DMemoryHeap m_memoryHeap;

        D3D12_RESOURCE_STATES m_state;
        BufferType m_type;
        u64 m_size;

        bool m_isMapped;
        bool m_isReading;
#if D3D_DEBUG_MARKERS
        std::string m_debugName;
#endif
    };

    inline u64 D3DBuffer::getSize() const
    {
        return m_size;
    }

    inline ID3D12Resource* D3DBuffer::getResource() const
    {
        ASSERT(m_resource, "nullptr");
        return m_resource;
    }

    inline D3D12_GPU_VIRTUAL_ADDRESS D3DBuffer::getGPUAddress() const
    {
        ASSERT(m_resource, "nullptr");
        return m_resource->GetGPUVirtualAddress();
    }

    inline D3D12_RESOURCE_STATES D3DBuffer::getState() const
    {
        return m_state;
    }

    inline D3D12_RESOURCE_STATES D3DBuffer::setState(D3D12_RESOURCE_STATES state)
    {
        return std::exchange(m_state, state);
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace dx3d
} //namespace renderer
} //namespace v3d
#endif //D3D_RENDER
