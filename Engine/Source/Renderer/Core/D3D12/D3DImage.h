#pragma once

#include "Common.h"
#include "Renderer/Formats.h"
#include "Renderer/TextureProperties.h"
#include "Renderer/Core/Image.h"

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
    * @brief D3DImage final class. DirectX Render side
    */
    class D3DImage final : public Image, public D3DResource
    {
    public:

        explicit D3DImage(ID3D12Device* device, Format format, u32 width, u32 height, u32 arrays, u32 samples, TextureUsageFlags flags, [[maybe_unused]] const std::string& name = "") noexcept;
        explicit D3DImage(ID3D12Device* device, D3D12_RESOURCE_DIMENSION dimension, Format format, const math::Dimension3D& size, u32 arrays, u32 mipmap, TextureUsageFlags flags, [[maybe_unused]] const std::string& name = "") noexcept;
        ~D3DImage();

        bool create() override;
        bool create(ID3D12Resource* resource, D3D12_CPU_DESCRIPTOR_HANDLE& handle);

        void destroy() override;

        void clear(Context* context, const math::Vector4D& color) override;
        void clear(Context* context, f32 depth, u32 stencil) override;

        bool upload(Context* context, const math::Dimension3D& size, u32 slices, u32 mips, const void* data) override;
        bool upload(Context* context, const math::Dimension3D& offsets, const math::Dimension3D& size, u32 slices, const void* data) override;

        D3DImage* getResolveImage() const;
        ID3D12Resource* getResource() const;

        template<typename VIEW_DESC_TYPE>
        const VIEW_DESC_TYPE& getView(const Image::Subresource& subresource) const
        {
            if constexpr (std::is_same<VIEW_DESC_TYPE, D3D12_SHADER_RESOURCE_VIEW_DESC>())
            {
                ASSERT(!(m_flags & D3D12_RESOURCE_FLAG_DENY_SHADER_RESOURCE), "must be shader resource flag support");
                auto found = std::get<0>(m_views).find(subresource);
                ASSERT(found != std::get<0>(m_views).cend(), "not found");
                return found->second;
            }
            else if constexpr (std::is_same<VIEW_DESC_TYPE, D3D12_UNORDERED_ACCESS_VIEW_DESC>())
            {
                ASSERT(m_flags & D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS, "must be UAV flag support");
                auto found = std::get<1>(m_views).find(subresource);
                ASSERT(found != std::get<1>(m_views).cend(), "not found");
                return found->second;
            }
            else if constexpr (std::is_same<VIEW_DESC_TYPE, D3D12_RENDER_TARGET_VIEW_DESC>())
            {
                ASSERT(m_flags & D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET, "must be render target flag support");
                auto found = std::get<2>(m_views).find(subresource);
                ASSERT(found != std::get<2>(m_views).cend(), "not found");
                return found->second;
            }
            else if constexpr (std::is_same<VIEW_DESC_TYPE, D3D12_DEPTH_STENCIL_VIEW_DESC>())
            {
                ASSERT(m_flags & D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL, "must be depth-stencil flag support");
                auto found = std::get<3>(m_views).find(subresource);
                ASSERT(found != std::get<3>(m_views).cend(), "not found");
                return found->second;
            }

        }

        const math::Dimension3D& getSize() const;
        u32 getCountSamples() const;
        DXGI_FORMAT getFormat() const;
        Format getOriginFormat() const;

        D3D12_RESOURCE_STATES getState() const;
        D3D12_RESOURCE_STATES setState(D3D12_RESOURCE_STATES state);
        D3D12_RESOURCE_STATES getState(const Image::Subresource& subresource) const;
        D3D12_RESOURCE_STATES setState(const Image::Subresource& subresource, D3D12_RESOURCE_STATES state);

        static DXGI_FORMAT convertImageFormatToD3DFormat(Format format);
        static D3D12_RESOURCE_DIMENSION convertImageTargetToD3DDimension(TextureTarget target);

        static bool isDepthStencilFormat(DXGI_FORMAT format);
        static bool isColorFormat(DXGI_FORMAT format);
        static bool isDepthFormatOnly(DXGI_FORMAT format);
        static bool isStencilFormatOnly(DXGI_FORMAT format);

        static const Image::Subresource makeD3DImageSubresource(const D3DImage* image, u32 slice = k_generalLayer, u32 mips = k_allMipmapsLevels);

        static DXGI_FORMAT getResolveCompatibilityFormat(DXGI_FORMAT format);
        static DXGI_FORMAT getSampledCompatibilityFormat(DXGI_FORMAT format);

#if DEBUG_OBJECT_MEMORY
        static std::set<D3DImage*> s_objects;
#endif //DEBUG_OBJECT_MEMORY

    private:

        static DXGI_FORMAT convertToTypelessFormat(DXGI_FORMAT format);

        D3D12_RESOURCE_DIMENSION getDimension() const;

        void createResourceView(DXGI_FORMAT shaderResourceFormat);
        bool internalUpdate(Context* context, const math::Dimension3D& offsets, const math::Dimension3D& size, u32 layers, u32 mips, const void* data);

        ID3D12Device* const m_device;

        ID3D12Resource* m_resource;
        D3D12_CPU_DESCRIPTOR_HANDLE m_handle;
        std::vector<D3D12_RESOURCE_STATES> m_state;
        D3D12_RESOURCE_FLAGS m_flags;

        struct ViewKey
        {
            ViewKey() noexcept;
            ViewKey(const Image::Subresource& desc) noexcept;

            Image::Subresource _desc;
            u32 _hash;

            struct Hash
            {
                u32 operator()(const ViewKey& desc) const;
            };

            struct Compare
            {
                bool operator()(const ViewKey& op1, const ViewKey& op2) const;
            };
        };

        std::tuple<
            std::unordered_map<ViewKey, D3D12_SHADER_RESOURCE_VIEW_DESC, ViewKey::Hash, ViewKey::Compare>,
            std::unordered_map<ViewKey, D3D12_UNORDERED_ACCESS_VIEW_DESC, ViewKey::Hash, ViewKey::Compare>,
            std::unordered_map<ViewKey, D3D12_RENDER_TARGET_VIEW_DESC, ViewKey::Hash, ViewKey::Compare>,
            std::unordered_map<ViewKey, D3D12_DEPTH_STENCIL_VIEW_DESC, ViewKey::Hash, ViewKey::Compare>> m_views;

        D3D12_RESOURCE_DIMENSION m_dimension;
        DXGI_FORMAT m_format;
        math::Dimension3D m_size;
        u32 m_mipmaps;
        u32 m_arrays;
        u32 m_samples;
        D3DImage* m_resolveImage;

        bool m_swapchain;

        Format m_originFormat;

#if D3D_DEBUG_MARKERS
        std::string m_debugName;
#endif //D3D_DEBUG_MARKERS
    };

    inline D3DImage* D3DImage::getResolveImage() const
    {
        return m_resolveImage;
    }

    inline ID3D12Resource* D3DImage::getResource() const
    {
        ASSERT(m_resource, "nullptr");
        return m_resource;
    }

    inline const math::Dimension3D& D3DImage::getSize() const
    {
        return m_size;
    }

    inline D3D12_RESOURCE_DIMENSION D3DImage::getDimension() const
    {
        return m_dimension;
    }

    inline DXGI_FORMAT D3DImage::getFormat() const
    {
        return m_format;
    }

    inline Format D3DImage::getOriginFormat() const
    {
        return m_originFormat;
    }

    inline u32 D3DImage::getCountSamples() const
    {
        return m_samples;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace dx3d
} //namespace renderer
} //namespace v3d
#endif //D3D_RENDER
