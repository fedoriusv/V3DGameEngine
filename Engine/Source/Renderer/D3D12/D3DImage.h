#pragma once

#include "Common.h"
#include "Renderer/Image.h"
#include "Renderer/Formats.h"
#include "Renderer/TextureProperties.h"

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

    /**
    * D3DImage final class. DirectX Render side
    */
    class D3DImage final : public Image, public D3DResource
    {
    public:

        explicit D3DImage(ID3D12Device* device, Format format, u32 width, u32 height, u32 samples, TextureUsageFlags flags, const std::string& name = "") noexcept;
        explicit D3DImage(ID3D12Device* device, D3D12_RESOURCE_DIMENSION dimension, Format format, const core::Dimension3D& size, u32 arrays, u32 mipmap, TextureUsageFlags flags, const std::string& name = "") noexcept;
        ~D3DImage();

        bool create() override;
        bool create(ID3D12Resource* resource, CD3DX12_CPU_DESCRIPTOR_HANDLE& handle);

        void destroy() override;

        void clear(Context* context, const core::Vector4D& color) override;
        void clear(Context* context, f32 depth, u32 stencil) override;

        bool upload(Context* context, const core::Dimension3D& size, u32 slices, u32 mips, const void* data) override;
        bool upload(Context* context, const core::Dimension3D& offsets, const core::Dimension3D& size, u32 slices, const void* data) override;

        ID3D12Resource* getResource() const;
        const D3D12_SHADER_RESOURCE_VIEW_DESC& getView() const;

        const core::Dimension3D& getSize() const;
        D3D12_RESOURCE_DIMENSION getDimension() const;
        DXGI_FORMAT getFormat() const;
        Format getOriginFormat() const;
        u32 getCountSamples() const;

        D3D12_RESOURCE_STATES getState() const;
        D3D12_RESOURCE_STATES setState(D3D12_RESOURCE_STATES state);

        static DXGI_FORMAT convertImageFormatToD3DFormat(Format format);
        static D3D12_RESOURCE_DIMENSION convertImageTargetToD3DDimension(TextureTarget target);

        static bool isDepthStencilFormat(DXGI_FORMAT format);
        static bool isColorFormat(DXGI_FORMAT format);
        static bool isDepthFormatOnly(DXGI_FORMAT format);
        static bool isStencilFormatOnly(DXGI_FORMAT format);

    private:

        bool internalUpdate(Context* context, const core::Dimension3D& offsets, const core::Dimension3D& size, u32 layers, u32 mips, const void* data);

        ID3D12Device* const m_device;

        ID3D12Resource* m_resource;
        CD3DX12_CPU_DESCRIPTOR_HANDLE m_handle;
        D3D12_RESOURCE_STATES m_state;
        D3D12_RESOURCE_FLAGS m_flags;

        D3D12_SHADER_RESOURCE_VIEW_DESC m_view;

        D3D12_RESOURCE_DIMENSION m_dimension;
        DXGI_FORMAT m_format;
        core::Dimension3D m_size;
        u32 m_mipmaps;
        u32 m_arrays;

        u32 m_samples;

        bool m_swapchain;

        Format m_originFormat;

#if D3D_DEBUG
        std::string m_debugName;
#endif
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace dx3d
} //namespace renderer
} //namespace v3d
#endif //D3D_RENDER
