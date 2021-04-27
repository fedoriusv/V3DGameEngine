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

        explicit D3DImage(ID3D12Device* device, Format format, u32 width, u32 height, u32 arrays, u32 samples, TextureUsageFlags flags, const std::string& name = "") noexcept;
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

        template<typename VIEW_DESC_TYPE>
        const VIEW_DESC_TYPE& getView(u32 slice = k_generalLayer, u32 mip = k_allMipmapsLevels) const
        {
            u32 index = D3D12CalcSubresource((mip == k_allMipmapsLevels) ? 0 : mip, (slice == k_generalLayer) ? 0 : slice, 0, (mip == k_allMipmapsLevels) ? m_mipmaps : 1, (slice == k_generalLayer) ? m_arrays : 1) + 1;
            if (slice == k_generalLayer && mip == k_allMipmapsLevels)
            {
                index = D3D12CalcSubresource(0, 0, 0, m_mipmaps, m_arrays);
            }
           
            if constexpr (std::is_same<VIEW_DESC_TYPE, D3D12_SHADER_RESOURCE_VIEW_DESC>())
            {
                ASSERT(!(m_flags & D3D12_RESOURCE_FLAG_DENY_SHADER_RESOURCE), "must be shader resource flag support");
                ASSERT(std::get<0>(m_views).size() >= index, "range out");
                return std::get<0>(m_views)[index];
            }
            else if constexpr (std::is_same<VIEW_DESC_TYPE, D3D12_RENDER_TARGET_VIEW_DESC>())
            {
                ASSERT((m_flags & D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET), "must be shader render target flag support");
                ASSERT(std::get<1>(m_views).size() >= index, "range out");
                return std::get<1>(m_views)[index];
            }
            else if constexpr (std::is_same<VIEW_DESC_TYPE, D3D12_DEPTH_STENCIL_VIEW_DESC>())
            {
                ASSERT((m_flags & D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL), "must be shader depth-stencil flag support");
                ASSERT(std::get<2>(m_views).size() >= index, "range out");
                return std::get<2>(m_views)[index];
            }
        }


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

        static DXGI_FORMAT convertToTypelessFormat(DXGI_FORMAT format);
        static DXGI_FORMAT getCompatibilityFormat(DXGI_FORMAT format);

        bool internalUpdate(Context* context, const core::Dimension3D& offsets, const core::Dimension3D& size, u32 layers, u32 mips, const void* data);

        void createResourceView(DXGI_FORMAT shaderResourceFormat);

        ID3D12Device* const m_device;

        ID3D12Resource* m_resource;
        CD3DX12_CPU_DESCRIPTOR_HANDLE m_handle;
        D3D12_RESOURCE_STATES m_state;
        D3D12_RESOURCE_FLAGS m_flags;

        std::tuple<std::vector<D3D12_SHADER_RESOURCE_VIEW_DESC>, std::vector<D3D12_RENDER_TARGET_VIEW_DESC>, std::vector<D3D12_DEPTH_STENCIL_VIEW_DESC>> m_views;

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
