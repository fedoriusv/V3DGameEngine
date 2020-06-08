#include "D3DImage.h"
#include "Utils/Logger.h"

#ifdef D3D_RENDER
#include "d3dx12.h"
#include "D3DDebug.h"
#include "D3DDeviceCaps.h"
#include "D3DResource.h"
#include "D3DCommandList.h"
#include "D3DCommandListManager.h"
#include "D3DGraphicContext.h"

namespace v3d
{
namespace renderer
{
namespace dx3d
{

DXGI_FORMAT D3DImage::convertImageFormatToD3DFormat(Format format)
{
    switch (format)
    {
    case v3d::renderer::Format_Undefined:
        return DXGI_FORMAT_UNKNOWN;

    case v3d::renderer::Format_R4G4_UNorm_Pack8:
        return DXGI_FORMAT_UNKNOWN;
    case v3d::renderer::Format_R4G4B4A4_UNorm_Pack16:
        return DXGI_FORMAT_UNKNOWN;
    case v3d::renderer::Format_B4G4R4A4_UNorm_Pack16:
        return DXGI_FORMAT_UNKNOWN;
    case v3d::renderer::Format_R5G6B5_UNorm_Pack16:
        return DXGI_FORMAT_UNKNOWN;
    case v3d::renderer::Format_B5G6R5_UNorm_Pack16:
        return DXGI_FORMAT_UNKNOWN;
    case v3d::renderer::Format_R5G5B5A1_UNorm_Pack16:
        return DXGI_FORMAT_UNKNOWN;
    case v3d::renderer::Format_B5G5R5A1_UNorm_Pack16:
        return DXGI_FORMAT_UNKNOWN;
    case v3d::renderer::Format_A1R5G5B5_UNorm_Pack16:
        return DXGI_FORMAT_UNKNOWN;

    case v3d::renderer::Format_R8_UNorm:
        return DXGI_FORMAT_R8_UNORM;
    case v3d::renderer::Format_R8_SNorm:
        return DXGI_FORMAT_R8_SNORM;
    case v3d::renderer::Format_R8_UScaled:
        return DXGI_FORMAT_UNKNOWN;
    case v3d::renderer::Format_R8_SScaled:
        return DXGI_FORMAT_UNKNOWN;
    case v3d::renderer::Format_R8_UInt:
        return DXGI_FORMAT_R8_UINT;
    case v3d::renderer::Format_R8_SInt:
        return DXGI_FORMAT_R8_SINT;
    case v3d::renderer::Format_R8_SRGB:
        return DXGI_FORMAT_UNKNOWN;

    case v3d::renderer::Format_R8G8_UNorm:
        return DXGI_FORMAT_R8G8_UNORM;
    case v3d::renderer::Format_R8G8_SNorm:
        return DXGI_FORMAT_R8G8_SNORM;
    case v3d::renderer::Format_R8G8_UScaled:
        return DXGI_FORMAT_UNKNOWN;
    case v3d::renderer::Format_R8G8_SScaled:
        return DXGI_FORMAT_UNKNOWN;
    case v3d::renderer::Format_R8G8_UInt:
        return DXGI_FORMAT_R8G8_UINT;
    case v3d::renderer::Format_R8G8_SInt:
        return DXGI_FORMAT_R8G8_SINT;
    case v3d::renderer::Format_R8G8_SRGB:
        return DXGI_FORMAT_UNKNOWN;

    case v3d::renderer::Format_R8G8B8_UNorm:
        return DXGI_FORMAT_UNKNOWN;
    case v3d::renderer::Format_R8G8B8_SNorm:
        return DXGI_FORMAT_UNKNOWN;
    case v3d::renderer::Format_R8G8B8_UScaled:
        return DXGI_FORMAT_UNKNOWN;
    case v3d::renderer::Format_R8G8B8_SScaled:
        return DXGI_FORMAT_UNKNOWN;
    case v3d::renderer::Format_R8G8B8_UInt:
        return DXGI_FORMAT_UNKNOWN;
    case v3d::renderer::Format_R8G8B8_SInt:
        return DXGI_FORMAT_UNKNOWN;
    case v3d::renderer::Format_R8G8B8_SRGB:
        return DXGI_FORMAT_UNKNOWN;
    case v3d::renderer::Format_B8G8R8_UNorm:
        return DXGI_FORMAT_UNKNOWN;
    case v3d::renderer::Format_B8G8R8_SNorm:
        return DXGI_FORMAT_UNKNOWN;
    case v3d::renderer::Format_B8G8R8_UScaled:
        return DXGI_FORMAT_UNKNOWN;
    case v3d::renderer::Format_B8G8R8_SScaled:
        return DXGI_FORMAT_UNKNOWN;
    case v3d::renderer::Format_B8G8R8_UInt:
        return DXGI_FORMAT_UNKNOWN;
    case v3d::renderer::Format_B8G8R8_SInt:
        return DXGI_FORMAT_UNKNOWN;
    case v3d::renderer::Format_B8G8R8_SRGB:
        return DXGI_FORMAT_UNKNOWN;

    case v3d::renderer::Format_R8G8B8A8_UNorm:
        return DXGI_FORMAT_R8G8B8A8_UNORM;
    case v3d::renderer::Format_R8G8B8A8_SNorm:
        return DXGI_FORMAT_R8G8B8A8_SNORM;
    case v3d::renderer::Format_R8G8B8A8_UScaled:
        return DXGI_FORMAT_UNKNOWN;
    case v3d::renderer::Format_R8G8B8A8_SScaled:
        return DXGI_FORMAT_UNKNOWN;
    case v3d::renderer::Format_R8G8B8A8_UInt:
        return DXGI_FORMAT_R8G8B8A8_UINT;
    case v3d::renderer::Format_R8G8B8A8_SInt:
        return DXGI_FORMAT_R8G8B8A8_SINT;
    case v3d::renderer::Format_R8G8B8A8_SRGB:
        return DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
    case v3d::renderer::Format_B8G8R8A8_UNorm:
        return DXGI_FORMAT_B8G8R8A8_UNORM;
    case v3d::renderer::Format_B8G8R8A8_SNorm:
        return DXGI_FORMAT_UNKNOWN;
    case v3d::renderer::Format_B8G8R8A8_UScaled:
        return DXGI_FORMAT_UNKNOWN;
    case v3d::renderer::Format_B8G8R8A8_SScaled:
        return DXGI_FORMAT_UNKNOWN;
    case v3d::renderer::Format_B8G8R8A8_UInt:
        return DXGI_FORMAT_UNKNOWN;
    case v3d::renderer::Format_B8G8R8A8_SInt:
        return DXGI_FORMAT_UNKNOWN;
    case v3d::renderer::Format_B8G8R8A8_SRGB:
        return DXGI_FORMAT_UNKNOWN;
    case v3d::renderer::Format_A8B8G8R8_UNorm_Pack32:
        return DXGI_FORMAT_UNKNOWN;
    case v3d::renderer::Format_A8B8G8R8_SNorm_Pack32:
        return DXGI_FORMAT_UNKNOWN;
    case v3d::renderer::Format_A8B8G8R8_UScaled_Pack32:
        return DXGI_FORMAT_UNKNOWN;
    case v3d::renderer::Format_A8B8G8R8_SScaled_Pack32:
        return DXGI_FORMAT_UNKNOWN;
    case v3d::renderer::Format_A8B8G8R8_UInt_Pack32:
        return DXGI_FORMAT_UNKNOWN;
    case v3d::renderer::Format_A8B8G8R8_SInt_Pack32:
        return DXGI_FORMAT_UNKNOWN;
    case v3d::renderer::Format_A8B8G8R8_SRGB_Pack32:
        return DXGI_FORMAT_UNKNOWN;

    case v3d::renderer::Format_A2R10G10B10_UNorm_Pack32:
        return DXGI_FORMAT_UNKNOWN;
    case v3d::renderer::Format_A2R10G10B10_SNorm_Pack32:
        return DXGI_FORMAT_UNKNOWN;
    case v3d::renderer::Format_A2R10G10B10_UScaled_Pack32:
        return DXGI_FORMAT_UNKNOWN;
    case v3d::renderer::Format_A2R10G10B10_SScaled_Pack32:
        return DXGI_FORMAT_UNKNOWN;
    case v3d::renderer::Format_A2R10G10B10_UInt_Pack32:
        return DXGI_FORMAT_UNKNOWN;
    case v3d::renderer::Format_A2R10G10B10_SInt_Pack32:
        return DXGI_FORMAT_UNKNOWN;
    case v3d::renderer::Format_A2B10G10R10_UNorm_Pack32:
        return DXGI_FORMAT_UNKNOWN;
    case v3d::renderer::Format_A2B10G10R10_SNorm_Pack32:
        return DXGI_FORMAT_UNKNOWN;
    case v3d::renderer::Format_A2B10G10R10_UScaled_Pack32:
        return DXGI_FORMAT_UNKNOWN;
    case v3d::renderer::Format_A2B10G10R10_SScaled_Pack32:
        return DXGI_FORMAT_UNKNOWN;
    case v3d::renderer::Format_A2B10G10R10_UInt_Pack32:
        return DXGI_FORMAT_UNKNOWN;
    case v3d::renderer::Format_A2B10G10R10_SInt_Pack32:
        return DXGI_FORMAT_UNKNOWN;

    case v3d::renderer::Format_R16_UNorm:
        return DXGI_FORMAT_R16_UNORM;
    case v3d::renderer::Format_R16_SNorm:
        return DXGI_FORMAT_R16_SNORM;
    case v3d::renderer::Format_R16_UScaled:
        return DXGI_FORMAT_UNKNOWN;
    case v3d::renderer::Format_R16_SScaled:
        return DXGI_FORMAT_UNKNOWN;
    case v3d::renderer::Format_R16_UInt:
        return DXGI_FORMAT_R16_UINT;
    case v3d::renderer::Format_R16_SInt:
        return DXGI_FORMAT_R16_SINT;
    case v3d::renderer::Format_R16_SFloat:
        return DXGI_FORMAT_R16_FLOAT;

    case v3d::renderer::Format_R16G16_UNorm:
        return DXGI_FORMAT_R16G16_UNORM;
    case v3d::renderer::Format_R16G16_SNorm:
        return DXGI_FORMAT_R16G16_SNORM;
    case v3d::renderer::Format_R16G16_UScaled:
        return DXGI_FORMAT_UNKNOWN;
    case v3d::renderer::Format_R16G16_SScaled:
        return DXGI_FORMAT_UNKNOWN;
    case v3d::renderer::Format_R16G16_UInt:
        return DXGI_FORMAT_R16G16_UINT;
    case v3d::renderer::Format_R16G16_SInt:
        return DXGI_FORMAT_R16G16_SINT;
    case v3d::renderer::Format_R16G16_SFloat:
        return DXGI_FORMAT_R16G16_FLOAT;

    case v3d::renderer::Format_R16G16B16_UNorm:
        return DXGI_FORMAT_UNKNOWN;
    case v3d::renderer::Format_R16G16B16_SNorm:
        return DXGI_FORMAT_UNKNOWN;
    case v3d::renderer::Format_R16G16B16_UScaled:
        return DXGI_FORMAT_UNKNOWN;
    case v3d::renderer::Format_R16G16B16_SScaled:
        return DXGI_FORMAT_UNKNOWN;
    case v3d::renderer::Format_R16G16B16_UInt:
        return DXGI_FORMAT_UNKNOWN;
    case v3d::renderer::Format_R16G16B16_SInt:
        return DXGI_FORMAT_UNKNOWN;
    case v3d::renderer::Format_R16G16B16_SFloat:
        return DXGI_FORMAT_UNKNOWN;

    case v3d::renderer::Format_R16G16B16A16_UNorm:
        return DXGI_FORMAT_R16G16B16A16_UNORM;
    case v3d::renderer::Format_R16G16B16A16_SNorm:
        return DXGI_FORMAT_R16G16B16A16_SNORM;
    case v3d::renderer::Format_R16G16B16A16_UScaled:
        return DXGI_FORMAT_UNKNOWN;
    case v3d::renderer::Format_R16G16B16A16_SScaled:
        return DXGI_FORMAT_UNKNOWN;
    case v3d::renderer::Format_R16G16B16A16_UInt:
        return DXGI_FORMAT_R16G16B16A16_UINT;
    case v3d::renderer::Format_R16G16B16A16_SInt:
        return DXGI_FORMAT_R16G16B16A16_SINT;
    case v3d::renderer::Format_R16G16B16A16_SFloat:
        return DXGI_FORMAT_R16G16B16A16_FLOAT;

    case v3d::renderer::Format_R32_UInt:
        return DXGI_FORMAT_R32_UINT;
    case v3d::renderer::Format_R32_SInt:
        return DXGI_FORMAT_R32_SINT;
    case v3d::renderer::Format_R32_SFloat:
        return DXGI_FORMAT_R32_FLOAT;

    case v3d::renderer::Format_R32G32_UInt:
        return DXGI_FORMAT_R32G32_UINT;
    case v3d::renderer::Format_R32G32_SInt:
        return DXGI_FORMAT_R32G32_SINT;
    case v3d::renderer::Format_R32G32_SFloat:
        return DXGI_FORMAT_R32G32_FLOAT;

    case v3d::renderer::Format_R32G32B32_UInt:
        return DXGI_FORMAT_R32G32B32_UINT;
    case v3d::renderer::Format_R32G32B32_SInt:
        return DXGI_FORMAT_R32G32B32_SINT;
    case v3d::renderer::Format_R32G32B32_SFloat:
        return DXGI_FORMAT_R32G32B32_FLOAT;

    case v3d::renderer::Format_R32G32B32A32_UInt:
        return DXGI_FORMAT_R32G32B32A32_UINT;
    case v3d::renderer::Format_R32G32B32A32_SInt:
        return DXGI_FORMAT_R32G32B32A32_SINT;
    case v3d::renderer::Format_R32G32B32A32_SFloat:
        return DXGI_FORMAT_R32G32B32A32_FLOAT;

    case v3d::renderer::Format_R64_UInt:
        return DXGI_FORMAT_UNKNOWN;
    case v3d::renderer::Format_R64_SInt:
        return DXGI_FORMAT_UNKNOWN;
    case v3d::renderer::Format_R64_SFloat:
        return DXGI_FORMAT_UNKNOWN;
    case v3d::renderer::Format_R64G64_UInt:
        return DXGI_FORMAT_UNKNOWN;
    case v3d::renderer::Format_R64G64_SInt:
        return DXGI_FORMAT_UNKNOWN;
    case v3d::renderer::Format_R64G64_SFloat:
        return DXGI_FORMAT_UNKNOWN;
    case v3d::renderer::Format_R64G64B64_UInt:
        return DXGI_FORMAT_UNKNOWN;
    case v3d::renderer::Format_R64G64B64_SInt:
        return DXGI_FORMAT_UNKNOWN;
    case v3d::renderer::Format_R64G64B64_SFloat:
        return DXGI_FORMAT_UNKNOWN;
    case v3d::renderer::Format_R64G64B64A64_UInt:
        return DXGI_FORMAT_UNKNOWN;
    case v3d::renderer::Format_R64G64B64A64_SInt:
        return DXGI_FORMAT_UNKNOWN;
    case v3d::renderer::Format_R64G64B64A64_SFloat:
        return DXGI_FORMAT_UNKNOWN;
    case v3d::renderer::Format_B10G11R11_UFloat_Pack32:
        return DXGI_FORMAT_UNKNOWN;
    case v3d::renderer::Format_E5B9G9R9_UFloat_Pack32:
        return DXGI_FORMAT_UNKNOWN;

    case v3d::renderer::Format_D16_UNorm:
        return DXGI_FORMAT_D16_UNORM;
    case v3d::renderer::Format_X8_D24_UNorm_Pack32:
        return DXGI_FORMAT_UNKNOWN;
    case v3d::renderer::Format_D32_SFloat:
        return DXGI_FORMAT_D32_FLOAT;
    case v3d::renderer::Format_S8_UInt:
        return DXGI_FORMAT_UNKNOWN;
    case v3d::renderer::Format_D16_UNorm_S8_UInt:
        return DXGI_FORMAT_UNKNOWN;
    case v3d::renderer::Format_D24_UNorm_S8_UInt:
        return DXGI_FORMAT_D24_UNORM_S8_UINT;
    case v3d::renderer::Format_D32_SFloat_S8_UInt:
        return DXGI_FORMAT_D32_FLOAT_S8X24_UINT;

    case v3d::renderer::Format_BC1_RGB_UNorm_Block:
        return DXGI_FORMAT_UNKNOWN;
    case v3d::renderer::Format_BC1_RGB_SRGB_Block:
        return DXGI_FORMAT_UNKNOWN;
    case v3d::renderer::Format_BC1_RGBA_UNorm_Block:
        return DXGI_FORMAT_BC1_UNORM;
    case v3d::renderer::Format_BC1_RGBA_SRGB_Block:
        return DXGI_FORMAT_BC1_UNORM_SRGB;
    case v3d::renderer::Format_BC2_UNorm_Block:
        return DXGI_FORMAT_BC2_UNORM;
    case v3d::renderer::Format_BC2_SRGB_Block:
        return DXGI_FORMAT_BC2_UNORM_SRGB;
    case v3d::renderer::Format_BC3_UNorm_Block:
        return DXGI_FORMAT_BC3_UNORM;
    case v3d::renderer::Format_BC3_SRGB_Block:
        return DXGI_FORMAT_BC3_UNORM_SRGB;
    case v3d::renderer::Format_BC4_UNorm_Block:
        return DXGI_FORMAT_BC4_UNORM;
    case v3d::renderer::Format_BC4_SNorm_Block:
        return DXGI_FORMAT_BC4_SNORM;
    case v3d::renderer::Format_BC5_UNorm_Block:
        return DXGI_FORMAT_BC5_UNORM;
    case v3d::renderer::Format_BC5_SNorm_Block:
        return DXGI_FORMAT_BC5_SNORM;
    case v3d::renderer::Format_BC6H_UFloat_Block:
        return DXGI_FORMAT_UNKNOWN;
    case v3d::renderer::Format_BC6H_SFloat_Block:
        return DXGI_FORMAT_UNKNOWN;
    case v3d::renderer::Format_BC7_UNorm_Block:
        return DXGI_FORMAT_BC7_UNORM;
    case v3d::renderer::Format_BC7_SRGB_Block:
        return DXGI_FORMAT_UNKNOWN;

    case v3d::renderer::Format_ETC2_R8G8B8_UNorm_Block:
        return DXGI_FORMAT_UNKNOWN;
    case v3d::renderer::Format_ETC2_R8G8B8_SRGB_Block:
        return DXGI_FORMAT_UNKNOWN;
    case v3d::renderer::Format_ETC2_R8G8B8A1_UNorm_Block:
        return DXGI_FORMAT_UNKNOWN;
    case v3d::renderer::Format_ETC2_R8G8B8A1_SRGB_Block:
        return DXGI_FORMAT_UNKNOWN;
    case v3d::renderer::Format_ETC2_R8G8B8A8_UNorm_Block:
        return DXGI_FORMAT_UNKNOWN;
    case v3d::renderer::Format_ETC2_R8G8B8A8_SRGB_Block:
        return DXGI_FORMAT_UNKNOWN;
    case v3d::renderer::Format_EAC_R11_UNorm_Block:
        return DXGI_FORMAT_UNKNOWN;
    case v3d::renderer::Format_EAC_R11_SNorm_Block:
        return DXGI_FORMAT_UNKNOWN;
    case v3d::renderer::Format_EAC_R11G11_UNorm_Block:
        return DXGI_FORMAT_UNKNOWN;
    case v3d::renderer::Format_EAC_R11G11_SNorm_Block:
        return DXGI_FORMAT_UNKNOWN;

    case v3d::renderer::Format_ASTC_4x4_UNorm_Block:
        return DXGI_FORMAT_UNKNOWN;
    case v3d::renderer::Format_ASTC_4x4_SRGB_Block:
        return DXGI_FORMAT_UNKNOWN;
    case v3d::renderer::Format_ASTC_5x4_UNorm_Block:
        return DXGI_FORMAT_UNKNOWN;
    case v3d::renderer::Format_ASTC_5x4_SRGB_Block:
        return DXGI_FORMAT_UNKNOWN;
    case v3d::renderer::Format_ASTC_5x5_UNorm_Block:
        return DXGI_FORMAT_UNKNOWN;
    case v3d::renderer::Format_ASTC_5x5_SRGB_Block:
        return DXGI_FORMAT_UNKNOWN;
    case v3d::renderer::Format_ASTC_6x5_UNorm_Block:
        return DXGI_FORMAT_UNKNOWN;
    case v3d::renderer::Format_ASTC_6x5_SRGB_Block:
        return DXGI_FORMAT_UNKNOWN;
    case v3d::renderer::Format_ASTC_6x6_UNorm_Block:
        return DXGI_FORMAT_UNKNOWN;
    case v3d::renderer::Format_ASTC_6x6_SRGB_Block:
        return DXGI_FORMAT_UNKNOWN;
    case v3d::renderer::Format_ASTC_8x5_UNorm_Block:
        return DXGI_FORMAT_UNKNOWN;
    case v3d::renderer::Format_ASTC_8x5_SRGB_Block:
        return DXGI_FORMAT_UNKNOWN;
    case v3d::renderer::Format_ASTC_8x6_UNorm_Block:
        return DXGI_FORMAT_UNKNOWN;
    case v3d::renderer::Format_ASTC_8x6_SRGB_Block:
        return DXGI_FORMAT_UNKNOWN;
    case v3d::renderer::Format_ASTC_8x8_UNorm_Block:
        return DXGI_FORMAT_UNKNOWN;
    case v3d::renderer::Format_ASTC_8x8_SRGB_Block:
        return DXGI_FORMAT_UNKNOWN;
    case v3d::renderer::Format_ASTC_10x5_UNorm_Block:
        return DXGI_FORMAT_UNKNOWN;
    case v3d::renderer::Format_ASTC_10x5_SRGB_Block:
        return DXGI_FORMAT_UNKNOWN;
    case v3d::renderer::Format_ASTC_10x6_UNorm_Block:
        return DXGI_FORMAT_UNKNOWN;
    case v3d::renderer::Format_ASTC_10x6_SRGB_Block:
        return DXGI_FORMAT_UNKNOWN;
    case v3d::renderer::Format_ASTC_10x8_UNorm_Block:
        return DXGI_FORMAT_UNKNOWN;
    case v3d::renderer::Format_ASTC_10x8_SRGB_Block:
        return DXGI_FORMAT_UNKNOWN;
    case v3d::renderer::Format_ASTC_10x10_UNorm_Block:
        return DXGI_FORMAT_UNKNOWN;
    case v3d::renderer::Format_ASTC_10x10_SRGB_Block:
        return DXGI_FORMAT_UNKNOWN;
    case v3d::renderer::Format_ASTC_12x10_UNorm_Block:
        return DXGI_FORMAT_UNKNOWN;
    case v3d::renderer::Format_ASTC_12x10_SRGB_Block:
        return DXGI_FORMAT_UNKNOWN;
    case v3d::renderer::Format_ASTC_12x12_UNorm_Block:
        return DXGI_FORMAT_UNKNOWN;
    case v3d::renderer::Format_ASTC_12x12_SRGB_Block:
        return DXGI_FORMAT_UNKNOWN;

    default:
        ASSERT(false, "unknown");
    }

    return DXGI_FORMAT_UNKNOWN;
}

D3D12_RESOURCE_DIMENSION D3DImage::convertImageTargetToD3DDimension(TextureTarget target)
{
    switch (target)
    {
    case TextureTarget::Texture1D:
    case TextureTarget::Texture1DArray:
        return D3D12_RESOURCE_DIMENSION_TEXTURE1D;

    case TextureTarget::Texture2D:
    case TextureTarget::Texture2DArray:
        return D3D12_RESOURCE_DIMENSION_TEXTURE2D;

    case TextureTarget::Texture3D:
    case TextureTarget::TextureCubeMap:
        return D3D12_RESOURCE_DIMENSION_TEXTURE3D;

    default:
        ASSERT(false, "unknown");
    }

    return D3D12_RESOURCE_DIMENSION_TEXTURE2D;
}

bool D3DImage::isDepthStencilFormat(DXGI_FORMAT format)
{
    switch (format)
    {
    case DXGI_FORMAT_D32_FLOAT_S8X24_UINT:
    case DXGI_FORMAT_D32_FLOAT:
    case DXGI_FORMAT_D24_UNORM_S8_UINT:
    case DXGI_FORMAT_D16_UNORM:
        return true;

    default:
        return false;
    }

    return false;
}

bool D3DImage::isColorFormat(DXGI_FORMAT format)
{
    return !D3DImage::isDepthStencilFormat(format);
}

bool D3DImage::isDepthFormatOnly(DXGI_FORMAT format)
{
    return format == DXGI_FORMAT_D32_FLOAT;
}

bool D3DImage::isStencilFormatOnly(DXGI_FORMAT format)
{
    return format == DXGI_FORMAT_D16_UNORM;
}

D3DImage::D3DImage(ID3D12Device* device, Format format, u32 width, u32 height, u32 samples, TextureUsageFlags flags, const std::string& name) noexcept
    : Image()
    , m_device(device)
    , m_resource(nullptr)
    , m_state(D3D12_RESOURCE_STATE_COMMON)
    , m_flags(D3D12_RESOURCE_FLAG_DENY_SHADER_RESOURCE)

    , m_dimension(D3D12_RESOURCE_DIMENSION_TEXTURE2D)
    , m_format(convertImageFormatToD3DFormat(format))

    , m_size({ width, height , 1})
    , m_arrays(1)
    , m_mipmaps(1)
    , m_samples(samples)

    , m_swapchain(false)

    , m_originFormat(format)
#if D3D_DEBUG
    , m_debugName(name)
#endif
{
    LOG_DEBUG("D3DImage::D3DImage constructor %llx", this);
    if (D3DImage::isDepthStencilFormat(m_format))
    {
        m_flags |= D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
    }
    else
    {
        m_flags |= D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
    }

    if (flags & TextureUsage::TextureUsage_Sampled)
    {
        m_flags &= ~D3D12_RESOURCE_FLAG_DENY_SHADER_RESOURCE;
    }

    memset(&m_view, 0, sizeof(D3D12_SHADER_RESOURCE_VIEW_DESC));
}

D3DImage::D3DImage(ID3D12Device* device, D3D12_RESOURCE_DIMENSION dimension, Format format, const core::Dimension3D& size, u32 arrays, u32 mipmap, TextureUsageFlags flags, const std::string& name) noexcept
    : Image()
    , m_device(device)
    , m_resource(nullptr)
    , m_state(D3D12_RESOURCE_STATE_COMMON)
    , m_flags(D3D12_RESOURCE_FLAG_NONE)

    , m_dimension(dimension)
    , m_format(convertImageFormatToD3DFormat(format))

    , m_size(size)
    , m_arrays(arrays)
    , m_mipmaps(mipmap)
    , m_samples(1U)

    , m_swapchain(false)

    , m_originFormat(format)
#if D3D_DEBUG
    , m_debugName(name)
#endif
{
    LOG_DEBUG("D3DImage::D3DImage constructor %llx", this);
    if (D3DImage::isDepthStencilFormat(m_format))
    {
        m_flags |= D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
    }
    else
    {
        if (flags & TextureUsage::TextureUsage_Attachment)
        {
            m_flags |= D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
        }
    }

    memset(&m_view, 0, sizeof(D3D12_SHADER_RESOURCE_VIEW_DESC));
}


D3DImage::~D3DImage()
{
    LOG_DEBUG("D3DImage::~D3DImage destructor %llx", this);
    ASSERT(!m_resource, "not nullptr");
}

bool D3DImage::create()
{
    ASSERT(!m_resource, "image already created");
    if ((m_flags & D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET) || (m_flags & D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL))
    {
        if (!D3DDeviceCaps::getInstance()->getImageFormatSupportInfo(m_originFormat, DeviceCaps::TilingType_Optimal)._supportAttachment)
        {
            LOG_ERROR("Format %d is not supported to attachmnet", m_originFormat);
            ASSERT(false, "not support");
            return false;
        }
    }

    if (!(m_flags & D3D12_RESOURCE_FLAG_DENY_SHADER_RESOURCE))
    {
        if (!D3DDeviceCaps::getInstance()->getImageFormatSupportInfo(m_originFormat, DeviceCaps::TilingType_Optimal)._supportSampled)
        {
            LOG_ERROR("Format %d is not supported to sampled image", m_originFormat);
            ASSERT(false, "not support");
            return false;
        }
    }

    D3D12_RESOURCE_DESC textureDesc = {};
    textureDesc.Dimension = m_dimension;
    textureDesc.Alignment = 0;
    textureDesc.Width = m_size.width;
    textureDesc.Height = m_size.height;
    textureDesc.DepthOrArraySize = m_size.depth * m_arrays;
    textureDesc.MipLevels = m_mipmaps;
    textureDesc.Format = m_format;
    textureDesc.SampleDesc = { 1, 0 };
    textureDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
    textureDesc.Flags = m_flags;

    HRESULT result = m_device->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT), D3D12_HEAP_FLAG_NONE, &textureDesc, m_state, nullptr, IID_PPV_ARGS(&m_resource));
    if (FAILED(result))
    {
        LOG_ERROR("D3DImage::create CreateCommittedResource is failed. Error %s", D3DDebug::stringError(result).c_str());
        return false;
    }
#if D3D_DEBUG
    wchar_t wtext[64];
    if (!m_debugName.empty())
    {
        mbstowcs(wtext, m_debugName.c_str(), m_debugName.size() + 1);
    }
    else
    {
        std::string debugName = "ImageResource: " + std::to_string(reinterpret_cast<const u64>(this));
        mbstowcs(wtext, debugName.c_str(), debugName.size() + 1);
    }
    m_resource->SetName(LPCWSTR(wtext));
#endif

    m_view.Format = textureDesc.Format;
    m_view.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
   
    switch (textureDesc.Dimension)
    {
    case D3D12_RESOURCE_DIMENSION_TEXTURE2D:
    {
        if (m_arrays == 1)
        {
            m_view.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;

            m_view.Texture2D.MostDetailedMip = 0;
            m_view.Texture2D.MipLevels = m_mipmaps;
            m_view.Texture2D.PlaneSlice = 0;
            m_view.Texture2D.ResourceMinLODClamp = 0.0f;
        }
        else
        {
            ASSERT(false, "not impl");
        }
        break;
    }

    case D3D12_RESOURCE_DIMENSION_TEXTURE1D:
    case D3D12_RESOURCE_DIMENSION_TEXTURE3D:
    default:
        ASSERT(false, "not impl");
    }

    return true;
}

bool D3DImage::create(ID3D12Resource* resource, CD3DX12_CPU_DESCRIPTOR_HANDLE& handle)
{
    ASSERT(resource, "nullptr");
    m_resource = resource;
#if D3D_DEBUG
    wchar_t wtext[64];
    if (!m_debugName.empty())
    {
        mbstowcs(wtext, m_debugName.c_str(), m_debugName.size() + 1);
    }
    else
    {
        std::string debugName = "SwapchainImageResource: " + std::to_string(reinterpret_cast<const u64>(this));
        mbstowcs(wtext, debugName.c_str(), debugName.size() + 1);
    }
    m_resource->SetName(LPCWSTR(wtext));
#endif
    m_handle = handle;
    m_swapchain = true;

    return true;
}

void D3DImage::destroy()
{
    if (m_swapchain)
    {
        if (m_resource)
        {
            //TODO
            u32 res = m_resource->Release();
            m_resource = nullptr;
        }

        return;
    }

    SAFE_DELETE(m_resource);
}

void D3DImage::clear(Context* context, const core::Vector4D& color)
{
    D3DGraphicsCommandList* commandlist = (D3DGraphicsCommandList*)static_cast<D3DGraphicContext*>(context)->getOrAcquireCurrentCommandList();
    ASSERT(commandlist, "nullptr");

    const FLOAT dxClearColor[] = { color.x, color.y, color.z, color.w };
    const D3D12_RECT dxRect =
    {
        0,
        0,
        static_cast<LONG>(m_size.width),
        static_cast<LONG>(m_size.height)
    };

    D3D12_RESOURCE_STATES oldState = m_state;
    commandlist->transition(this, D3D12_RESOURCE_STATE_RENDER_TARGET);

    if (m_swapchain)
    {
        commandlist->clearRenderTarget(m_handle, dxClearColor, { dxRect });
    }
    else
    {
        ASSERT(false, "no impl");
    }

    commandlist->transition(this, oldState);
}

void D3DImage::clear(Context* context, f32 depth, u32 stencil)
{
    D3DGraphicsCommandList* commandlist = (D3DGraphicsCommandList*)static_cast<D3DGraphicContext*>(context)->getOrAcquireCurrentCommandList();
    ASSERT(commandlist, "nullptr");

    D3D12_CLEAR_FLAGS flags = D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL;
    const D3D12_RECT dxRect =
    {
        0,
        0,
        static_cast<LONG>(m_size.width),
        static_cast<LONG>(m_size.height)
    };

    D3D12_RESOURCE_STATES oldState = m_state;
    commandlist->transition(this, D3D12_RESOURCE_STATE_RENDER_TARGET);

    if (m_swapchain)
    {
        commandlist->clearRenderTarget(m_handle, depth, stencil, flags, { dxRect });
    }
    else
    {
        ASSERT(false, "no impl");
    }

    commandlist->transition(this, oldState);
}

bool D3DImage::upload(Context* context, const core::Dimension3D& size, u32 slices, u32 mips, const void* data)
{
    return D3DImage::internalUpdate(context, core::Dimension3D(0, 0, 0), size, slices, mips, data);
}

bool D3DImage::upload(Context* context, const core::Dimension3D& offsets, const core::Dimension3D& size, u32 slices, const void* data)
{
    return D3DImage::internalUpdate(context, offsets, size, slices, 1, data);
}

bool D3DImage::internalUpdate(Context* context, const core::Dimension3D& offsets, const core::Dimension3D& size, u32 slices, u32 mips, const void* data)
{
    D3DGraphicContext* dxContext = static_cast<D3DGraphicContext*>(context);
    ASSERT(dxContext, "nullptr");

    D3DGraphicsCommandList* commandlist = static_cast<D3DGraphicsCommandList*>(dxContext->getOrAcquireCurrentCommandList(D3DCommandList::Type::Direct));
    ASSERT(commandlist, "nullptr");

    const UINT64 uploadBufferSize = GetRequiredIntermediateSize(m_resource, 0, 1);

    ID3D12Resource* uploadResource = nullptr;
    {
        HRESULT result = m_device->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD), D3D12_HEAP_FLAG_NONE, &CD3DX12_RESOURCE_DESC::Buffer(uploadBufferSize), D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&uploadResource));
        if (FAILED(result))
        {
            LOG_ERROR("D3DImage::upload CreateCommittedResource is failed. Error %s", D3DDebug::stringError(result).c_str());
            SAFE_DELETE(uploadResource);

            return false;
        }
#if D3D_DEBUG
        wchar_t wtext[64];
        std::string debugName = "UploadBufferResource: " + std::to_string(reinterpret_cast<const u64>(this));
        mbstowcs(wtext, debugName.c_str(), debugName.size() + 1);
        uploadResource->SetName(LPCWSTR(wtext));
#endif
    }

    auto calculateMipSize = [](const core::Dimension3D& size) -> core::Dimension3D
    {
        core::Dimension3D mipSize;
        mipSize.width = std::max(size.width / 2, 1U);
        mipSize.height = std::max(size.height / 2, 1U);
        mipSize.depth = std::max(size.depth / 2, 1U);

        return mipSize;
    };

    auto calculateSubresourceRowPitch = [](const core::Dimension3D& size, u32 mipLevel, Format format) -> u64
    {
        const u32 width = std::max(size.width >> mipLevel, 1U);
        if (ImageFormat::isFormatCompressed(format))
        {
            const core::Dimension2D& blockDim = ImageFormat::getBlockDimension(format);
            const u32 widthSize = (width + blockDim.width - 1) / blockDim.width;

            return u64(widthSize * ImageFormat::getFormatBlockSize(format));
        }

        return u64(width * ImageFormat::getFormatBlockSize(format));
    };

    auto calculateSubresourceSlicePitch = [](u64 rowPitch, const core::Dimension3D& size, u32 mipLevel, Format format) -> u64
    {
        u32 height = std::max(size.height >> mipLevel, 1U);
        if (ImageFormat::isFormatCompressed(format))
        {
            const core::Dimension2D& blockDim = ImageFormat::getBlockDimension(format);
            const u32 heightSize = (height + blockDim.height - 1) / blockDim.height;

            return rowPitch * u64(heightSize * ImageFormat::getFormatBlockSize(format) * size.depth);
        }
        u32 blockArea = ImageFormat::getBlockDimension(format).getArea();
        f32 sizePerPixel = (f32)ImageFormat::getFormatBlockSize(format) / (f32)blockArea;

        return rowPitch * u64(height * ImageFormat::getFormatBlockSize(format) * size.depth);
    };

    std::vector<D3D12_SUBRESOURCE_DATA> subresource;
    subresource.reserve(slices * mips);

    ASSERT(offsets.width == 0 && offsets.height == 0 && offsets.depth == 0, "not impl");
    for (u32 slice = 0; slice < slices; ++slice)
    {
        core::Dimension3D mipsSize = size;
        for (u32 mip = 0; mip < mips; ++mip)
        {
            UINT offset = D3D12CalcSubresource(mip, 0, slice, mips, slices);
            //TODO
            D3D12_SUBRESOURCE_DATA textureData = {};
            textureData.pData = reinterpret_cast<const u8*>(data) + offset;
            textureData.RowPitch = calculateSubresourceRowPitch(size, mip, m_originFormat);
            textureData.SlicePitch = calculateSubresourceSlicePitch(textureData.RowPitch, size, mip, m_originFormat);

            subresource.push_back(textureData);

            mipsSize = calculateMipSize(size);
        }
    }

    D3D12_RESOURCE_STATES oldState = getState();
    commandlist->transition(this, D3D12_RESOURCE_STATE_COPY_DEST);

    UpdateSubresources(commandlist->getHandle(), m_resource, uploadResource, 0, 0, static_cast<UINT>(subresource.size()), subresource.data());

    UploadResource* upload = new UploadResource(uploadResource);
    commandlist->setUsed(upload, 0);

    if (!(m_flags & D3D12_RESOURCE_FLAG_DENY_SHADER_RESOURCE))
    {
        commandlist->transition(this, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
    }
    else
    {
        commandlist->transition(this, oldState);
    }

    bool result = true;
    if (D3DDeviceCaps::getInstance()->immediateSubmitUpload)
    {
        commandlist->close();
        result = dxContext->getCommandListManager()->execute(commandlist);
        ASSERT(result, "fail");
    }

    dxContext->getResourceDeleter().requestToDelete(upload, [upload]() -> void
        {
            delete upload;
        });

    return result;
}

const core::Dimension3D& D3DImage::getSize() const
{
    return m_size;
}

D3D12_RESOURCE_DIMENSION D3DImage::getDimension() const
{
    return m_dimension;
}

DXGI_FORMAT D3DImage::getFormat() const
{
    return m_format;
}

Format D3DImage::getOriginFormat() const
{
    return m_originFormat;
}

u32 D3DImage::getCountSamples() const
{
    return m_samples;
}

D3D12_RESOURCE_STATES D3DImage::getState() const
{
    return m_state;
}

D3D12_RESOURCE_STATES D3DImage::setState(D3D12_RESOURCE_STATES state)
{
    D3D12_RESOURCE_STATES oldState = std::exchange(m_state, state);
    return oldState;
}

ID3D12Resource* D3DImage::getResource() const
{
    ASSERT(m_resource, "nullptr");
    return m_resource;
}

const D3D12_SHADER_RESOURCE_VIEW_DESC& D3DImage::getView() const
{
    return m_view;
}

} //namespace dx3d
} //namespace renderer
} //namespace v3d
#endif //D3D_RENDER
