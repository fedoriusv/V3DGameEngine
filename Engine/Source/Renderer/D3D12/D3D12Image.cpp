#include "D3D12Image.h"
#include "Utils/Logger.h"

#ifdef D3D_RENDER
#   include "D3D12GraphicContext.h"
#   include "D3D12CommandList.h"
#   include "D3D12CommandListManager.h"
#   include "D3D12Debug.h"

#   include "d3dx12.h"

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
        return DXGI_FORMAT_UNKNOWN;

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

D3DImage::D3DImage(ID3D12Device* device, Format format, u32 width, u32 height, u32 samples, TextureUsageFlags flags, const std::string& name) noexcept
    : Image()
    , m_device(device)
    , m_imageResource(nullptr)
    , m_state(D3D12_RESOURCE_STATE_COMMON)

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
}

D3DImage::D3DImage(ID3D12Device* device, D3D12_RESOURCE_DIMENSION dimension, Format format, const core::Dimension3D& size, u32 arrays, u32 mipmap, TextureUsageFlags flags, const std::string& name) noexcept
    : Image()
    , m_device(device)
    , m_imageResource(nullptr)
    , m_state(D3D12_RESOURCE_STATE_COMMON)

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
}


D3DImage::~D3DImage()
{
    LOG_DEBUG("D3DImage::~D3DImage destructor %llx", this);
    ASSERT(!m_imageResource, "not nullptr");
}

bool D3DImage::create()
{
    ASSERT(!m_imageResource, "image already created");

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
    textureDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

    HRESULT result = m_device->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT), D3D12_HEAP_FLAG_NONE, &textureDesc, D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(&m_imageResource));
    if (FAILED(result))
    {
        LOG_ERROR("D3DImage::create CreateCommittedResource is failed. Error %s", D3DDebug::stringError(result).c_str());
        return false;
    }

    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Format = textureDesc.Format;
    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
   
    switch (textureDesc.Dimension)
    {
    case D3D12_RESOURCE_DIMENSION_TEXTURE2D:
    {
        if (m_arrays == 1)
        {
            srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;

            srvDesc.Texture2D.MostDetailedMip = 0;
            srvDesc.Texture2D.MipLevels = m_mipmaps;
            srvDesc.Texture2D.PlaneSlice = 1;
            srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;
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


    //m_device->CreateShaderResourceView(m_imageResource, &srvDesc, m_srvHeap->GetCPUDescriptorHandleForHeapStart());

    return true;
}

bool D3DImage::create(ID3D12Resource* resource, CD3DX12_CPU_DESCRIPTOR_HANDLE& handle)
{
    ASSERT(resource, "nullptr");
    m_imageResource = resource;
#if D3D_DEBUG
    wchar_t wtext[20];
    mbstowcs(wtext, m_debugName.c_str(), m_debugName.size() + 1);
    m_imageResource->SetName(LPCWSTR(wtext));
#endif
    m_handle = handle;
    m_swapchain = true;

    return true;
}

void D3DImage::destroy()
{
    if (m_swapchain)
    {
        if (m_imageResource)
        {
            u32 res = m_imageResource->Release();
            m_imageResource = nullptr;
        }

        return;
    }

    SAFE_DELETE(m_imageResource);
}

void D3DImage::clear(Context* context, const core::Vector4D& color)
{
    D3DCommandList* commandlist = static_cast<D3DGraphicContext*>(context)->getCurrentCommandList();
    ASSERT(commandlist, "nullptr");

    const FLOAT dxClearColor[] = { color.x, color.y, color.z, color.w };
    const D3D12_RECT dxRect =
    {
        0,
        0,
        static_cast<LONG>(m_size.width),
        static_cast<LONG>(m_size.height)
    };

    static_cast<D3DGraphicsCommandList*>(commandlist)->clearRenderTarget(this, dxClearColor, { dxRect });
}

void D3DImage::clear(Context* context, f32 depth, u32 stencil)
{
    D3DCommandList* commandlist = static_cast<D3DGraphicContext*>(context)->getCurrentCommandList();
    ASSERT(commandlist, "nullptr");

    D3D12_CLEAR_FLAGS flags = D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL;
    const D3D12_RECT dxRect =
    {
        0,
        0,
        static_cast<LONG>(m_size.width),
        static_cast<LONG>(m_size.height)
    };

    static_cast<D3DGraphicsCommandList*>(commandlist)->clearRenderTarget(this, depth, stencil, flags, { dxRect });
}

bool D3DImage::upload(Context* context, const core::Dimension3D& size, u32 layers, u32 mips, const void* data)
{
    ASSERT(false, "not impl");
    return false;
}

bool D3DImage::upload(Context* context, const core::Dimension3D& offsets, const core::Dimension3D& size, u32 layers, const void* data)
{
    ASSERT(false, "not impl");
    return false;
}

const core::Dimension3D& D3DImage::getSize() const
{
    return m_size;
}

DXGI_FORMAT D3DImage::getFormat() const
{
    return m_format;
}

Format D3DImage::getOriginFormat() const
{
    return m_originFormat;
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

const CD3DX12_CPU_DESCRIPTOR_HANDLE& D3DImage::getDescriptorHandle() const
{
    return m_handle;
}

ID3D12Resource* D3DImage::getResource() const
{
    ASSERT(m_imageResource, "nullptr");
    return m_imageResource;
}

} //namespace dx3d
} //namespace renderer
} //namespace v3d12
#endif //D3D_RENDER
