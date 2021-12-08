#pragma once

#include "Common.h"
#include "Renderer/BufferProperties.h"
#include "Renderer/Buffer.h"

#ifdef D3D_RENDER
#include "D3DConfiguration.h"
#include "D3DResource.h"

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

        bool upload(Context* context, u32 offset, u64 size, const void* data) override;

        ID3D12Resource* getResource() const;
        D3D12_GPU_VIRTUAL_ADDRESS getGPUAddress() const;

        D3D12_RESOURCE_STATES getState() const;
        D3D12_RESOURCE_STATES setState(D3D12_RESOURCE_STATES state);

    private:

        ID3D12Device* m_device;
        D3DHeapAllocator* const m_allocator;

        ID3D12Resource* m_resource;

        CD3DX12_HEAP_PROPERTIES m_heapProperties;
        D3D12_RESOURCE_STATES m_state;
        BufferType m_type;
        u64 m_size;

#if D3D_DEBUG
        std::string m_debugName;
#endif
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace dx3d
} //namespace renderer
} //namespace v3d
#endif //D3D_RENDER
