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

std::string ImageFormatStringDX(DXGI_FORMAT format)
{
    switch (format)
    {
#define STR(r) case DXGI_##r: return #r
        STR(FORMAT_UNKNOWN);
        STR(FORMAT_R32G32B32A32_TYPELESS);
        STR(FORMAT_R32G32B32A32_FLOAT);
        STR(FORMAT_R32G32B32A32_UINT);
        STR(FORMAT_R32G32B32A32_SINT);
        STR(FORMAT_R32G32B32_TYPELESS);
        STR(FORMAT_R32G32B32_FLOAT);
        STR(FORMAT_R32G32B32_UINT);
        STR(FORMAT_R32G32B32_SINT);
        STR(FORMAT_R16G16B16A16_TYPELESS);
        STR(FORMAT_R16G16B16A16_FLOAT);
        STR(FORMAT_R16G16B16A16_UNORM);
        STR(FORMAT_R16G16B16A16_UINT);
        STR(FORMAT_R16G16B16A16_SNORM);
        STR(FORMAT_R16G16B16A16_SINT);
        STR(FORMAT_R32G32_TYPELESS);
        STR(FORMAT_R32G32_FLOAT);
        STR(FORMAT_R32G32_UINT);
        STR(FORMAT_R32G32_SINT);
        STR(FORMAT_R32G8X24_TYPELESS);
        STR(FORMAT_D32_FLOAT_S8X24_UINT);
        STR(FORMAT_R32_FLOAT_X8X24_TYPELESS);
        STR(FORMAT_X32_TYPELESS_G8X24_UINT);
        STR(FORMAT_R10G10B10A2_TYPELESS);
        STR(FORMAT_R10G10B10A2_UNORM);
        STR(FORMAT_R10G10B10A2_UINT);
        STR(FORMAT_R11G11B10_FLOAT);
        STR(FORMAT_R8G8B8A8_TYPELESS);
        STR(FORMAT_R8G8B8A8_UNORM);
        STR(FORMAT_R8G8B8A8_UNORM_SRGB);
        STR(FORMAT_R8G8B8A8_UINT);
        STR(FORMAT_R8G8B8A8_SNORM);
        STR(FORMAT_R8G8B8A8_SINT);
        STR(FORMAT_R16G16_TYPELESS);
        STR(FORMAT_R16G16_FLOAT);
        STR(FORMAT_R16G16_UNORM);
        STR(FORMAT_R16G16_UINT);
        STR(FORMAT_R16G16_SNORM);
        STR(FORMAT_R16G16_SINT);
        STR(FORMAT_R32_TYPELESS);
        STR(FORMAT_D32_FLOAT);
        STR(FORMAT_R32_FLOAT);
        STR(FORMAT_R32_UINT);
        STR(FORMAT_R32_SINT);
        STR(FORMAT_R24G8_TYPELESS);
        STR(FORMAT_D24_UNORM_S8_UINT);
        STR(FORMAT_R24_UNORM_X8_TYPELESS);
        STR(FORMAT_X24_TYPELESS_G8_UINT);
        STR(FORMAT_R8G8_TYPELESS);
        STR(FORMAT_R8G8_UNORM);
        STR(FORMAT_R8G8_UINT);
        STR(FORMAT_R8G8_SNORM);
        STR(FORMAT_R8G8_SINT);
        STR(FORMAT_R16_TYPELESS);
        STR(FORMAT_R16_FLOAT);
        STR(FORMAT_D16_UNORM);
        STR(FORMAT_R16_UNORM);
        STR(FORMAT_R16_UINT);
        STR(FORMAT_R16_SNORM);
        STR(FORMAT_R16_SINT);
        STR(FORMAT_R8_TYPELESS);
        STR(FORMAT_R8_UNORM);
        STR(FORMAT_R8_UINT);
        STR(FORMAT_R8_SNORM);
        STR(FORMAT_R8_SINT);
        STR(FORMAT_A8_UNORM);
        STR(FORMAT_R1_UNORM);
        STR(FORMAT_R9G9B9E5_SHAREDEXP);
        STR(FORMAT_R8G8_B8G8_UNORM);
        STR(FORMAT_G8R8_G8B8_UNORM);
        STR(FORMAT_BC1_TYPELESS);
        STR(FORMAT_BC1_UNORM);
        STR(FORMAT_BC1_UNORM_SRGB);
        STR(FORMAT_BC2_TYPELESS);
        STR(FORMAT_BC2_UNORM);
        STR(FORMAT_BC2_UNORM_SRGB);
        STR(FORMAT_BC3_TYPELESS);
        STR(FORMAT_BC3_UNORM);
        STR(FORMAT_BC3_UNORM_SRGB);
        STR(FORMAT_BC4_TYPELESS);
        STR(FORMAT_BC4_UNORM);
        STR(FORMAT_BC4_SNORM);
        STR(FORMAT_BC5_TYPELESS);
        STR(FORMAT_BC5_UNORM);
        STR(FORMAT_BC5_SNORM);
        STR(FORMAT_B5G6R5_UNORM);
        STR(FORMAT_B5G5R5A1_UNORM);
        STR(FORMAT_B8G8R8A8_UNORM);
        STR(FORMAT_B8G8R8X8_UNORM);
        STR(FORMAT_R10G10B10_XR_BIAS_A2_UNORM);
        STR(FORMAT_B8G8R8A8_TYPELESS);
        STR(FORMAT_B8G8R8A8_UNORM_SRGB);
        STR(FORMAT_B8G8R8X8_TYPELESS);
        STR(FORMAT_B8G8R8X8_UNORM_SRGB);
        STR(FORMAT_BC6H_TYPELESS);
        STR(FORMAT_BC6H_UF16);
        STR(FORMAT_BC6H_SF16);
        STR(FORMAT_BC7_TYPELESS);
        STR(FORMAT_BC7_UNORM);
        STR(FORMAT_BC7_UNORM_SRGB);
        STR(FORMAT_AYUV);
        STR(FORMAT_Y410);
        STR(FORMAT_Y416);
        STR(FORMAT_NV12);
        STR(FORMAT_P010);
        STR(FORMAT_P016);
        STR(FORMAT_420_OPAQUE);
        STR(FORMAT_YUY2);
        STR(FORMAT_Y210);
        STR(FORMAT_Y216);
        STR(FORMAT_NV11);
        STR(FORMAT_AI44);
        STR(FORMAT_IA44);
        STR(FORMAT_P8);
        STR(FORMAT_A8P8);
        STR(FORMAT_B4G4R4A4_UNORM);
        STR(FORMAT_P208);
        STR(FORMAT_V208);
        STR(FORMAT_V408);
#undef STR
        default:
            ASSERT(false, "not found");
    }

    return "UNKNOWN_FORMAT";
}

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
        return DXGI_FORMAT_B5G6R5_UNORM;
    case v3d::renderer::Format_B5G6R5_UNorm_Pack16:
        return DXGI_FORMAT_UNKNOWN;
    case v3d::renderer::Format_R5G5B5A1_UNorm_Pack16:
        return DXGI_FORMAT_UNKNOWN;
    case v3d::renderer::Format_B5G5R5A1_UNorm_Pack16:
        return DXGI_FORMAT_UNKNOWN;
    case v3d::renderer::Format_A1R5G5B5_UNorm_Pack16:
        return DXGI_FORMAT_B5G5R5A1_UNORM;

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
        return DXGI_FORMAT_BC6H_UF16;
    case v3d::renderer::Format_BC6H_SFloat_Block:
        return DXGI_FORMAT_BC6H_SF16;
    case v3d::renderer::Format_BC7_UNorm_Block:
        return DXGI_FORMAT_BC7_UNORM;
    case v3d::renderer::Format_BC7_SRGB_Block:
        return DXGI_FORMAT_BC7_UNORM_SRGB;

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

DXGI_FORMAT D3DImage::convertToTypelessFormat(DXGI_FORMAT format)
{
    switch (format)
    {
    case DXGI_FORMAT_UNKNOWN:
        return DXGI_FORMAT_UNKNOWN;

    case DXGI_FORMAT_R32G32B32A32_TYPELESS:
    case DXGI_FORMAT_R32G32B32A32_FLOAT:
    case DXGI_FORMAT_R32G32B32A32_UINT:
    case DXGI_FORMAT_R32G32B32A32_SINT:
        return DXGI_FORMAT_R32G32B32A32_TYPELESS;

    case DXGI_FORMAT_R32G32B32_TYPELESS:
    case DXGI_FORMAT_R32G32B32_FLOAT:
    case DXGI_FORMAT_R32G32B32_UINT:
    case DXGI_FORMAT_R32G32B32_SINT:
        return DXGI_FORMAT_R32G32B32_TYPELESS;

    case DXGI_FORMAT_R16G16B16A16_TYPELESS:
    case DXGI_FORMAT_R16G16B16A16_FLOAT:
    case DXGI_FORMAT_R16G16B16A16_UNORM:
    case DXGI_FORMAT_R16G16B16A16_UINT:
    case DXGI_FORMAT_R16G16B16A16_SNORM:
    case DXGI_FORMAT_R16G16B16A16_SINT:
        return DXGI_FORMAT_R16G16B16A16_TYPELESS;

    case DXGI_FORMAT_R32G32_TYPELESS:
    case DXGI_FORMAT_R32G32_FLOAT:
    case DXGI_FORMAT_R32G32_UINT:
    case DXGI_FORMAT_R32G32_SINT:
        return DXGI_FORMAT_R32G32_TYPELESS;

    case DXGI_FORMAT_R32G8X24_TYPELESS:
    case DXGI_FORMAT_D32_FLOAT_S8X24_UINT:
        return DXGI_FORMAT_R32G8X24_TYPELESS;

    case DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS:
    case DXGI_FORMAT_X32_TYPELESS_G8X24_UINT:
        return DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS;

    case DXGI_FORMAT_R10G10B10A2_TYPELESS:
    case DXGI_FORMAT_R10G10B10A2_UNORM:
    case DXGI_FORMAT_R10G10B10A2_UINT:
    case DXGI_FORMAT_R11G11B10_FLOAT:
        return DXGI_FORMAT_R10G10B10A2_TYPELESS;

    case DXGI_FORMAT_R8G8B8A8_TYPELESS:
    case DXGI_FORMAT_R8G8B8A8_UNORM:
    case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
    case DXGI_FORMAT_R8G8B8A8_UINT:
    case DXGI_FORMAT_R8G8B8A8_SNORM:
    case DXGI_FORMAT_R8G8B8A8_SINT:
        return DXGI_FORMAT_R8G8B8A8_TYPELESS;

    case DXGI_FORMAT_R16G16_TYPELESS:
    case DXGI_FORMAT_R16G16_FLOAT:
    case DXGI_FORMAT_R16G16_UNORM:
    case DXGI_FORMAT_R16G16_UINT:
    case DXGI_FORMAT_R16G16_SNORM:
    case DXGI_FORMAT_R16G16_SINT:
        return DXGI_FORMAT_R16G16_TYPELESS;

    case DXGI_FORMAT_R32_TYPELESS:
    case DXGI_FORMAT_D32_FLOAT:
    case DXGI_FORMAT_R32_FLOAT:
    case DXGI_FORMAT_R32_UINT:
    case DXGI_FORMAT_R32_SINT:
        return DXGI_FORMAT_R32_TYPELESS;

    case DXGI_FORMAT_R24G8_TYPELESS:
    case DXGI_FORMAT_D24_UNORM_S8_UINT:
        return DXGI_FORMAT_R24G8_TYPELESS;

    case DXGI_FORMAT_R24_UNORM_X8_TYPELESS:
    case DXGI_FORMAT_X24_TYPELESS_G8_UINT:
        return DXGI_FORMAT_R24_UNORM_X8_TYPELESS;

    case DXGI_FORMAT_R8G8_TYPELESS:
    case DXGI_FORMAT_R8G8_UNORM:
    case DXGI_FORMAT_R8G8_UINT:
    case DXGI_FORMAT_R8G8_SNORM:
    case DXGI_FORMAT_R8G8_SINT:
        return DXGI_FORMAT_R8G8_TYPELESS;

    case DXGI_FORMAT_R16_TYPELESS:
    case DXGI_FORMAT_R16_FLOAT:
    case DXGI_FORMAT_D16_UNORM:
    case DXGI_FORMAT_R16_UNORM:
    case DXGI_FORMAT_R16_UINT:
    case DXGI_FORMAT_R16_SNORM:
    case DXGI_FORMAT_R16_SINT:
        return DXGI_FORMAT_R16_TYPELESS;

    case DXGI_FORMAT_R8_TYPELESS:
    case DXGI_FORMAT_R8_UNORM:
    case DXGI_FORMAT_R8_UINT:
    case DXGI_FORMAT_R8_SNORM:
    case DXGI_FORMAT_R8_SINT:
    case DXGI_FORMAT_A8_UNORM:
    case DXGI_FORMAT_R1_UNORM:
    case DXGI_FORMAT_R9G9B9E5_SHAREDEXP:
    case DXGI_FORMAT_R8G8_B8G8_UNORM:
    case DXGI_FORMAT_G8R8_G8B8_UNORM:
        return DXGI_FORMAT_R8_TYPELESS;

    case DXGI_FORMAT_BC1_TYPELESS:
    case DXGI_FORMAT_BC1_UNORM:
    case DXGI_FORMAT_BC1_UNORM_SRGB:
        return DXGI_FORMAT_BC1_TYPELESS;

    case DXGI_FORMAT_BC2_TYPELESS:
    case DXGI_FORMAT_BC2_UNORM:
    case DXGI_FORMAT_BC2_UNORM_SRGB:
        return DXGI_FORMAT_BC2_TYPELESS;

    case DXGI_FORMAT_BC3_TYPELESS:
    case DXGI_FORMAT_BC3_UNORM:
    case DXGI_FORMAT_BC3_UNORM_SRGB:
        return DXGI_FORMAT_BC3_TYPELESS;

    case DXGI_FORMAT_BC4_TYPELESS:
    case DXGI_FORMAT_BC4_UNORM:
    case DXGI_FORMAT_BC4_SNORM:
        return DXGI_FORMAT_BC4_TYPELESS;

    case DXGI_FORMAT_BC5_TYPELESS:
    case DXGI_FORMAT_BC5_UNORM:
    case DXGI_FORMAT_BC5_SNORM:
    case DXGI_FORMAT_B5G6R5_UNORM:
    case DXGI_FORMAT_B5G5R5A1_UNORM:
    case DXGI_FORMAT_B8G8R8A8_UNORM:
    case DXGI_FORMAT_B8G8R8X8_UNORM:
    case DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM:
        return DXGI_FORMAT_BC5_TYPELESS;

    case DXGI_FORMAT_B8G8R8A8_TYPELESS:
    case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB:
        return DXGI_FORMAT_B8G8R8A8_TYPELESS;

    case DXGI_FORMAT_B8G8R8X8_TYPELESS:
    case DXGI_FORMAT_B8G8R8X8_UNORM_SRGB:
        return DXGI_FORMAT_B8G8R8X8_TYPELESS;

    case DXGI_FORMAT_BC6H_TYPELESS:
    case DXGI_FORMAT_BC6H_UF16:
    case DXGI_FORMAT_BC6H_SF16:
        return DXGI_FORMAT_BC6H_TYPELESS;

    case DXGI_FORMAT_BC7_TYPELESS:
    case DXGI_FORMAT_BC7_UNORM:
    case DXGI_FORMAT_BC7_UNORM_SRGB:
    case DXGI_FORMAT_AYUV:
    case DXGI_FORMAT_Y410:
    case DXGI_FORMAT_Y416:
    case DXGI_FORMAT_NV12:
    case DXGI_FORMAT_P010:
    case DXGI_FORMAT_P016:
    case DXGI_FORMAT_420_OPAQUE:
    case DXGI_FORMAT_YUY2:
    case DXGI_FORMAT_Y210:
    case DXGI_FORMAT_Y216:
    case DXGI_FORMAT_NV11:
    case DXGI_FORMAT_AI44:
    case DXGI_FORMAT_IA44:
    case DXGI_FORMAT_P8:
    case DXGI_FORMAT_A8P8:
    case DXGI_FORMAT_B4G4R4A4_UNORM:
    case DXGI_FORMAT_P208:
    case DXGI_FORMAT_V208:
    case DXGI_FORMAT_V408:
        return DXGI_FORMAT_BC7_TYPELESS;

    default:
        ASSERT(false, "format unknown");
    }

    return DXGI_FORMAT_UNKNOWN;
}

DXGI_FORMAT D3DImage::getCompatibilityFormat(DXGI_FORMAT format)
{
    switch (format)
    {
    case DXGI_FORMAT_D32_FLOAT:
        return DXGI_FORMAT_R32_FLOAT;

    default:
        ASSERT(false, "format not found");
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
    return format == DXGI_FORMAT_D32_FLOAT || format == DXGI_FORMAT_D16_UNORM;
}

bool D3DImage::isStencilFormatOnly(DXGI_FORMAT format)
{
    ASSERT(false, "unsupported");
    return false;
}

const Image::Subresource D3DImage::makeD3DImageSubresource(const D3DImage* image, u32 slice, u32 mips)
{
    ASSERT(image, "nullpltr");
    Image::Subresource subresource = {};

    if (slice == k_generalLayer)
    {
        subresource._baseLayer = 0;
        subresource._layers = image->m_arrays;
    }
    else
    {
        subresource._baseLayer = slice;
        subresource._layers = 1;
    }

    if (mips == k_allMipmapsLevels)
    {
        subresource._baseMip = 0;
        subresource._mips = image->m_mipmaps;
    }
    else
    {
        subresource._baseMip = mips;
        subresource._mips = 1;
    }

    return subresource;
}


D3DImage::D3DImage(ID3D12Device* device, Format format, u32 width, u32 height, u32 arrays, u32 samples, TextureUsageFlags flags, const std::string& name) noexcept
    : Image()
    , m_device(device)
    , m_resource(nullptr)
    , m_flags(D3D12_RESOURCE_FLAG_DENY_SHADER_RESOURCE)

    , m_dimension(D3D12_RESOURCE_DIMENSION_TEXTURE2D)
    , m_format(convertImageFormatToD3DFormat(format))

    , m_size({ width, height , 1})
    , m_arrays(arrays)
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

    if (flags & TextureUsage::TextureUsage_Storage)
    {
        m_flags |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
    }

    if (flags & TextureUsage::TextureUsage_GenerateMipmaps)
    {
        ASSERT(flags & TextureUsage::TextureUsage_Attachment, "must be attachment");
        m_mipmaps = ImageFormat::calculateMipmapCount({ m_size.width, m_size.height, m_size.depth });
    }

    m_state.resize(m_arrays * m_mipmaps + 1, D3D12_RESOURCE_STATE_COMMON);
}

D3DImage::D3DImage(ID3D12Device* device, D3D12_RESOURCE_DIMENSION dimension, Format format, const core::Dimension3D& size, u32 arrays, u32 mipmap, TextureUsageFlags flags, const std::string& name) noexcept
    : Image()
    , m_device(device)
    , m_resource(nullptr)
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

    if (flags & TextureUsage::TextureUsage_Storage)
    {
        m_flags |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
    }

    m_state.resize(m_arrays * m_mipmaps + 1, D3D12_RESOURCE_STATE_COMMON);
}


D3DImage::~D3DImage()
{
    LOG_DEBUG("D3DImage::~D3DImage destructor %llx", this);
    ASSERT(!m_resource, "not nullptr");
}

bool D3DImage::create()
{
    ASSERT(!m_resource, "image already created");

    D3D12_CLEAR_VALUE* optimizedClearValue = nullptr;
    D3D12_CLEAR_VALUE defaultClearValue = {};
    if ((m_flags & D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET) || (m_flags & D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL))
    {
        if (!D3DDeviceCaps::getInstance()->getImageFormatSupportInfo(m_originFormat, DeviceCaps::TilingType_Optimal)._supportAttachment)
        {
            LOG_ERROR("Format %s is not supported to attachment", ImageFormatStringDX(m_format).c_str());
            ASSERT(false, "not support");
            return false;
        }

        defaultClearValue.Format = m_format;
        if (isColorFormat(m_format))
        {
            defaultClearValue.Color[0] = 0.f;
            defaultClearValue.Color[1] = 0.f;
            defaultClearValue.Color[2] = 0.f;
            defaultClearValue.Color[3] = 0.f;
        }
        else
        {
            defaultClearValue.DepthStencil.Depth = 1.f;
            defaultClearValue.DepthStencil.Stencil = 0U;
        }
        optimizedClearValue = &defaultClearValue;
    }

    DXGI_FORMAT sampledFormat = m_format;
    if (!(m_flags & D3D12_RESOURCE_FLAG_DENY_SHADER_RESOURCE))
    {
        if (!D3DDeviceCaps::getInstance()->getImageFormatSupportInfo(m_originFormat, DeviceCaps::TilingType_Optimal)._supportSampled)
        {
            sampledFormat = getCompatibilityFormat(m_format);
            if (sampledFormat == DXGI_FORMAT_UNKNOWN)
            {
                ASSERT(false, "can't find supportd format");
                return false;
            }

            LOG_WARNING("Format %s is not supported sampled flag, replace to %s", ImageFormatStringDX(m_format).c_str(), ImageFormatStringDX(sampledFormat).c_str());
        }
    }

    if (!D3DDeviceCaps::getInstance()->getImageFormatSupportInfo(m_originFormat, DeviceCaps::TilingType_Optimal)._supportMip)
    {
        LOG_WARNING("Format %s is not supported mip levels", ImageFormatStringDX(m_format).c_str());
        m_mipmaps = 1;
    }

    if (m_flags & D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS)
    {
        if (!D3DDeviceCaps::getInstance()->getImageFormatSupportInfo(m_originFormat, DeviceCaps::TilingType_Optimal)._supportStorage)
        {
            LOG_ERROR("Format %s is not supported UAV", ImageFormatStringDX(m_format).c_str());
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
    textureDesc.Format = D3DImage::convertToTypelessFormat(m_format);
    textureDesc.SampleDesc = { 1, 0 };
    textureDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
    textureDesc.Flags = m_flags;

#if D3D_DEBUG
    LOG_DEBUG("CreateCommittedResource: Image [Size %u : %u : %u]; flags %u; format %s", textureDesc.Width, textureDesc.Height, textureDesc.DepthOrArraySize, textureDesc.Flags, ImageFormatStringDX(textureDesc.Format).c_str());
#endif
    HRESULT result = m_device->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT), D3D12_HEAP_FLAG_NONE, &textureDesc, m_state.front(), optimizedClearValue, IID_PPV_ARGS(&m_resource));
    if (FAILED(result))
    {
        LOG_ERROR("D3DImage::create CreateCommittedResource is failed. Error %s", D3DDebug::stringError(result).c_str());
        return false;
    }

    createResourceView(sampledFormat);

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

    return true;
}

void D3DImage::createResourceView(DXGI_FORMAT shaderResourceFormat)
{
    if (!(m_flags & D3D12_RESOURCE_FLAG_DENY_SHADER_RESOURCE))
    {
        auto viewShaderSubresource = [this](DXGI_FORMAT shaderResourceFormat, u32 baseMip, u32 mips, u32 firstSlice, u32 sliceArray) -> D3D12_SHADER_RESOURCE_VIEW_DESC
        {
            D3D12_SHADER_RESOURCE_VIEW_DESC view = {};

            view.Format = shaderResourceFormat;
            view.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

            switch (m_dimension)
            {
            case D3D12_RESOURCE_DIMENSION_TEXTURE2D:
            {
                if (m_arrays == 1U)
                {
                    view.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;

                    view.Texture2D.MostDetailedMip = baseMip;
                    view.Texture2D.MipLevels = mips;
                    view.Texture2D.PlaneSlice = 0;
                    view.Texture2D.ResourceMinLODClamp = 0.0f;
                }
                else if (m_arrays > 1U)
                {
                    if (m_arrays == 6U)
                    {
                        view.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;

                        view.TextureCube.MostDetailedMip = baseMip;
                        view.TextureCube.MipLevels = mips;
                        view.TextureCube.ResourceMinLODClamp = 0.0f;
                    }
                    else
                    {
                        view.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DARRAY;

                        view.Texture2DArray.MostDetailedMip = baseMip;
                        view.Texture2DArray.MipLevels = mips;
                        view.Texture2DArray.FirstArraySlice = firstSlice;
                        view.Texture2DArray.ArraySize = sliceArray;
                        view.Texture2DArray.PlaneSlice = 0;
                        view.Texture2DArray.ResourceMinLODClamp = 0.0f;
                    }
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

            return view;
        };

        auto& views = std::get<0>(m_views);
        if (m_arrays > 1)
        {
            D3D12_SHADER_RESOURCE_VIEW_DESC view = viewShaderSubresource(shaderResourceFormat, 0, m_mipmaps, 0, m_arrays);
            [[maybe_unused]] auto inserted = views.emplace(Image::makeImageSubresource(0, m_arrays, 0, m_mipmaps), view);
            ASSERT(inserted.second, "already inserted");
        }

        for (u32 slice = 0; slice < m_arrays; ++slice)
        {
            if (m_mipmaps > 1)
            {
                D3D12_SHADER_RESOURCE_VIEW_DESC view = viewShaderSubresource(shaderResourceFormat, 0, m_mipmaps, slice, 1);
                [[maybe_unused]] auto inserted = views.emplace(Image::makeImageSubresource(slice, 1, 0, m_mipmaps), view);
                ASSERT(inserted.second, "already inserted");
            }

            for (u32 mip = 0; mip < m_mipmaps; ++mip)
            {
                D3D12_SHADER_RESOURCE_VIEW_DESC view = viewShaderSubresource(shaderResourceFormat, mip, 1, slice, 1);
                [[maybe_unused]] auto inserted = views.emplace(Image::makeImageSubresource(slice, 1, mip, 1), view);
                ASSERT(inserted.second, "already inserted");
            }
        }
    }

    if (m_flags & D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS)
    {
        auto viewUAVSubresource = [this](DXGI_FORMAT shaderResourceFormat, u32 mip, u32 firstSlice, u32 sliceArrays) -> D3D12_UNORDERED_ACCESS_VIEW_DESC
        {
            D3D12_UNORDERED_ACCESS_VIEW_DESC view = {};

            view.Format = shaderResourceFormat;

            switch (m_dimension)
            {
            case D3D12_RESOURCE_DIMENSION_TEXTURE2D:
            {
                if (m_arrays == 1U)
                {
                    view.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;

                    view.Texture2D.MipSlice = mip;
                    view.Texture2D.PlaneSlice = 0;
                }
                else if (m_arrays > 1U)
                {
                    view.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2DARRAY;

                    view.Texture2DArray.FirstArraySlice = firstSlice;
                    view.Texture2DArray.ArraySize = sliceArrays;
                    view.Texture2DArray.MipSlice = mip;
                    view.Texture2DArray.PlaneSlice = 0;
                }
                else
                {
                    ASSERT(false, "wrong");
                }
                break;
            }

            case D3D12_RESOURCE_DIMENSION_TEXTURE1D:
            case D3D12_RESOURCE_DIMENSION_TEXTURE3D:
            default:
                ASSERT(false, "not impl");
            }

            return view;
        };

        auto& views = std::get<1>(m_views);
        for (u32 slice = 0; slice < m_arrays; ++slice)
        {
            for (u32 mip = 0; mip < m_mipmaps; ++mip)
            {
                D3D12_UNORDERED_ACCESS_VIEW_DESC view = viewUAVSubresource(shaderResourceFormat, mip, slice, 1);
                [[maybe_unused]] auto inserted = views.emplace(Image::makeImageSubresource(slice, 1, mip, 1), view);
                ASSERT(inserted.second, "already inserted");
            }
        }
    }

    if (m_flags & D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET)
    {
        auto viewRenderTargetSubresource = [this](u32 mip, u32 firstSlice, u32 sliceArrays)-> D3D12_RENDER_TARGET_VIEW_DESC
        {
            D3D12_RENDER_TARGET_VIEW_DESC view = {};

            view.Format = m_format;
            ASSERT(m_dimension == D3D12_RESOURCE_DIMENSION_TEXTURE2D, "wrong dimension");
            if (m_arrays == 1U)
            {
                if (m_samples == 1U)
                {
                    view.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
                    view.Texture2D.MipSlice = mip;
                    view.Texture2D.PlaneSlice = 0;
                }
                else
                {
                    view.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DMS;
                    view.Texture2DMS = {};
                }
            }
            else
            {
                if (m_samples == 1U)
                {
                    view.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DARRAY;
                    view.Texture2DArray.MipSlice = mip;
                    view.Texture2DArray.FirstArraySlice = firstSlice;
                    view.Texture2DArray.ArraySize = sliceArrays;
                    view.Texture2DArray.PlaneSlice = 0;
                }
                else
                {
                    view.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DMSARRAY;
                    view.Texture2DMSArray.FirstArraySlice = firstSlice;
                    view.Texture2DMSArray.ArraySize = sliceArrays;
                }
            }

            return view;
        };

        auto& views = std::get<2>(m_views);
        for (u32 slice = 0; slice < m_arrays; ++slice)
        {
            D3D12_RENDER_TARGET_VIEW_DESC view = viewRenderTargetSubresource(0, slice, 1);
            [[maybe_unused]] auto inserted = views.emplace(Image::makeImageSubresource(slice, 1, 0, 1), view);
            ASSERT(inserted.second, "already inserted");
        }
    }

    if (m_flags & D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL)
    {
        auto viewDepthStencilSubresource = [this](u32 mip, u32 firstSlice, u32 sliceArrays)-> D3D12_DEPTH_STENCIL_VIEW_DESC
        {
            D3D12_DEPTH_STENCIL_VIEW_DESC view = {};

            view.Format = m_format;
            view.Flags = D3D12_DSV_FLAG_NONE;
            ASSERT(m_dimension == D3D12_RESOURCE_DIMENSION_TEXTURE2D, "wrong dimension");

            if (m_arrays == 1U)
            {
                if (m_samples == 1U)
                {
                    view.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
                    view.Texture2D.MipSlice = mip;
                }
                else
                {
                    view.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2DMS;
                    view.Texture2DMS = {};
                }
            }
            else
            {
                if (m_samples == 1U)
                {
                    view.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2DARRAY;
                    view.Texture2DArray.MipSlice = mip;
                    view.Texture2DArray.FirstArraySlice = firstSlice;
                    view.Texture2DArray.ArraySize = sliceArrays;
                }
                else
                {
                    view.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2DMSARRAY;
                    view.Texture2DMSArray.FirstArraySlice = firstSlice;
                    view.Texture2DMSArray.ArraySize = sliceArrays;
                }
            }

            return view;
        };

        auto& views = std::get<3>(m_views);
        for (u32 slice = 0; slice < m_arrays; ++slice)
        {
            D3D12_DEPTH_STENCIL_VIEW_DESC view = viewDepthStencilSubresource(0, slice, 1);
            [[maybe_unused]] auto inserted = views.emplace(Image::makeImageSubresource(slice, 1, 0, 1), view);
            ASSERT(inserted.second, "already inserted");
        }
    }
}

bool D3DImage::create(ID3D12Resource* resource, CD3DX12_CPU_DESCRIPTOR_HANDLE& handle)
{
    ASSERT(resource, "nullptr");
    m_resource = resource;
    D3DImage::createResourceView(m_format);

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

    D3D12_RESOURCE_STATES oldState = m_state.front();
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

    D3D12_RESOURCE_STATES oldState = m_state.front();
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

    ASSERT(m_resource, "nullptr");
    UINT64 uploadBufferSize = 0;
    UINT64 subResourceCount = slices * mips;
    std::vector<D3D12_SUBRESOURCE_DATA> subResources(subResourceCount);
    std::vector<D3D12_PLACED_SUBRESOURCE_FOOTPRINT> subResourceFootPrints(subResourceCount);
    std::vector<UINT> subResourcesNumRows(subResourceCount);
    std::vector<UINT64> subResourcesNumRowsSize(subResourceCount);
    m_device->GetCopyableFootprints(&m_resource->GetDesc(), 0, static_cast<UINT>(subResources.size()), 0, subResourceFootPrints.data(), subResourcesNumRows.data(), subResourcesNumRowsSize.data(), &uploadBufferSize);

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

    auto calculateMipmapSize = [](const core::Dimension3D& size, u32 mipLevel, Format format) -> u64
    {
        const u32 width = std::max(size.width >> mipLevel, 1U);
        const u32 height = std::max(size.height >> mipLevel, 1U);
        if (ImageFormat::isFormatCompressed(format))
        {
            const core::Dimension2D& blockDim = ImageFormat::getBlockDimension(format);
            const u32 widthSize = (width + blockDim.width - 1) / blockDim.width;
            const u32 heightSize = (width + blockDim.height - 1) / blockDim.height;

            return u64(widthSize * heightSize * ImageFormat::getFormatBlockSize(format));
        }

        return u64(width * height * ImageFormat::getFormatBlockSize(format));
    };

    ASSERT(offsets.width == 0 && offsets.height == 0 && offsets.depth == 0, "not impl");
    for (u32 slice = 0; slice < slices; ++slice)
    {
        UINT64 dataOffset = 0;
        for (u32 mip = 0; mip < mips; ++mip)
        {
            UINT index = D3D12CalcSubresource(mip, slice, 0, mips, slices);

            const u32 width = std::max(m_size.width >> mip, 1U);

            D3D12_SUBRESOURCE_DATA& textureData = subResources[index];
            textureData.pData = reinterpret_cast<const BYTE*>(data) + dataOffset;
            textureData.RowPitch = calculateSubresourceRowPitch(m_size, mip, m_originFormat);
            textureData.SlicePitch = textureData.RowPitch * subResourceFootPrints[index].Footprint.Height;

            dataOffset += calculateMipmapSize(m_size, mip, m_originFormat);
        }
    }

    D3D12_RESOURCE_STATES oldState = getState();
    commandlist->transition(this, D3D12_RESOURCE_STATE_COPY_DEST);

    UpdateSubresources(commandlist->getHandle(), m_resource, uploadResource, 0, static_cast<UINT>(subResources.size()), uploadBufferSize, subResourceFootPrints.data(), subResourcesNumRows.data(), subResourcesNumRowsSize.data(), subResources.data());

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
    return m_state.front();
}

D3D12_RESOURCE_STATES D3DImage::setState(D3D12_RESOURCE_STATES state)
{
    D3D12_RESOURCE_STATES oldState = std::exchange(m_state.front(), state);
    std::fill(m_state.begin(), m_state.end(), state);

    return oldState;
}

D3D12_RESOURCE_STATES D3DImage::getState(const Image::Subresource& subresource) const
{
    ASSERT(subresource._mips == 1 && subresource._layers == 1, "must be 1");
    UINT index = D3D12CalcSubresource(subresource._baseMip, subresource._baseLayer, 0, subresource._mips, subresource._layers) + 1;
    ASSERT(index < m_state.size(), "range out");
    return m_state[index];
}

D3D12_RESOURCE_STATES D3DImage::setState(const Image::Subresource& subresource, D3D12_RESOURCE_STATES state)
{
    ASSERT(subresource._mips == 1 && subresource._layers == 1, "must be 1");
    UINT index = D3D12CalcSubresource(subresource._baseMip, subresource._baseLayer, 0, subresource._mips, subresource._layers) + 1;
    D3D12_RESOURCE_STATES oldState = std::exchange(m_state[index], state);
    m_state.front() = state;

    return oldState;
}

ID3D12Resource* D3DImage::getResource() const
{
    ASSERT(m_resource, "nullptr");
    return m_resource;
}

D3DImage::ViewKey::ViewKey() noexcept
    : _desc({})
    , _hash(0)
{
}

D3DImage::ViewKey::ViewKey(const Image::Subresource& desc) noexcept
    : _desc(desc)
    , _hash(crc32c::Crc32c(reinterpret_cast<const char*>(&desc), sizeof(Image::Subresource)))
{
}

u32 D3DImage::ViewKey::Hash::operator()(const ViewKey& desc) const
{
    ASSERT(desc._hash != 0, "empty hash");
    return desc._hash;
}

bool D3DImage::ViewKey::Compare::operator()(const ViewKey& op1, const ViewKey& op2) const
{
    return memcmp(&op1._desc, &op2._desc, sizeof(Image::Subresource)) == 0;
}

} //namespace dx3d
} //namespace renderer
} //namespace v3d
#endif //D3D_RENDER
