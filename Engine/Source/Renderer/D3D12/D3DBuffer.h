#pragma once

#include "Common.h"
#include "Renderer/BufferProperties.h"
#include "Renderer/Buffer.h"

#ifdef D3D_RENDER
#include "D3DConfiguration.h"
#include "D3DRenderResource.h"

namespace v3d
{
namespace renderer
{
namespace dx3d
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * D3DBuffer final class. DirectX Render side
    */
    class D3DBuffer final : public Buffer, public D3DRenderResource
    {
    public:

        explicit D3DBuffer(ID3D12Device* device, Buffer::BufferType type, StreamBufferUsageFlags usageFlag, u64 size, const std::string& name = "") noexcept;
        ~D3DBuffer();

        D3DBuffer() = delete;
        D3DBuffer(const D3DBuffer&) = delete;

        bool create() override;
        void destroy() override;

        u64 getSize() const;

        bool upload(Context* context, u32 offset, u64 size, const void* data) override;

        const CD3DX12_CPU_DESCRIPTOR_HANDLE& getDescriptorHandle() const;
        ID3D12Resource* getResource() const;

        D3D12_GPU_VIRTUAL_ADDRESS getGPUAddress() const;

    private:

        ID3D12Device* m_device;

        ID3D12Resource* m_bufferResource;
        D3D12_VERTEX_BUFFER_VIEW m_bufferView;

        CD3DX12_CPU_DESCRIPTOR_HANDLE m_handle;

        BufferType m_type;
        u64 m_size;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace dx3d
} //namespace renderer
} //namespace v3d
#endif //D3D_RENDER
