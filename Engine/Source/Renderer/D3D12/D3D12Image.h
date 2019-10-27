#pragma once

#include "Common.h"
#include "Renderer/Image.h"
#include "Renderer/Formats.h"
#include "Renderer/TextureProperties.h"

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
    * D3DImage final class. DirectX Render side
    */
    class D3DImage : public Image
    {
    public:

        explicit D3DImage(DXGI_FORMAT format, u32 width, u32 height) noexcept;
        ~D3DImage();

        bool create() override;
        bool create(ID3D12Resource* resource, CD3DX12_CPU_DESCRIPTOR_HANDLE& handle);

        void destroy() override;

        void clear(Context* context, const core::Vector4D& color) override;
        void clear(Context* context, f32 depth, u32 stencil) override;

        bool upload(Context* context, const core::Dimension3D& size, u32 layers, u32 mips, const void* data) override;
        bool upload(Context* context, const core::Dimension3D& offsets, const core::Dimension3D& size, u32 layers, const void* data) override;

        const CD3DX12_CPU_DESCRIPTOR_HANDLE& getDescriptorHandle() const;
        ID3D12Resource* getResource() const;

        const core::Dimension3D& getSize() const;

        D3D12_RESOURCE_STATES getState() const;
        D3D12_RESOURCE_STATES setState(D3D12_RESOURCE_STATES state);

    private:

        ComPtr<ID3D12Resource> m_imageResource;
        CD3DX12_CPU_DESCRIPTOR_HANDLE m_handle;

        D3D12_RESOURCE_STATES m_state;

        core::Dimension3D m_size;
        DXGI_FORMAT m_format;

        bool m_swapchain;
    };

/////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace d3d12
} //namespace renderer
} //namespace v3d
#endif //D3D_RENDER
