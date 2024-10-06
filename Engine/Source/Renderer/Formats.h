#pragma once

#include "Common.h"

namespace v3d
{
namespace renderer
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief Format enum
    */
    enum Format : u32
    {
        Format_Undefined = 0,

        Format_R4G4_UNorm_Pack8 = 1,
        Format_R4G4B4A4_UNorm_Pack16 = 2,
        Format_B4G4R4A4_UNorm_Pack16 = 3,
        Format_R5G6B5_UNorm_Pack16 = 4,
        Format_B5G6R5_UNorm_Pack16 = 5,
        Format_R5G5B5A1_UNorm_Pack16 = 6,
        Format_B5G5R5A1_UNorm_Pack16 = 7,
        Format_A1R5G5B5_UNorm_Pack16 = 8,

        Format_R8_UNorm = 9,
        Format_R8_SNorm = 10,
        Format_R8_UScaled = 11,
        Format_R8_SScaled = 12,
        Format_R8_UInt = 13,
        Format_R8_SInt = 14,
        Format_R8_SRGB = 15,

        Format_R8G8_UNorm = 16,
        Format_R8G8_SNorm = 17,
        Format_R8G8_UScaled = 18,
        Format_R8G8_SScaled = 19,
        Format_R8G8_UInt = 20,
        Format_R8G8_SInt = 21,
        Format_R8G8_SRGB = 22,

        Format_R8G8B8_UNorm = 23,
        Format_R8G8B8_SNorm = 24,
        Format_R8G8B8_UScaled = 25,
        Format_R8G8B8_SScaled = 26,
        Format_R8G8B8_UInt = 27,
        Format_R8G8B8_SInt = 28,
        Format_R8G8B8_SRGB = 29,
        Format_B8G8R8_UNorm = 30,
        Format_B8G8R8_SNorm = 31,
        Format_B8G8R8_UScaled = 32,
        Format_B8G8R8_SScaled = 33,
        Format_B8G8R8_UInt = 34,
        Format_B8G8R8_SInt = 35,
        Format_B8G8R8_SRGB = 36,

        Format_R8G8B8A8_UNorm = 37,
        Format_R8G8B8A8_SNorm = 38,
        Format_R8G8B8A8_UScaled = 39,
        Format_R8G8B8A8_SScaled = 40,
        Format_R8G8B8A8_UInt = 41,
        Format_R8G8B8A8_SInt = 42,
        Format_R8G8B8A8_SRGB = 43,
        Format_B8G8R8A8_UNorm = 44,
        Format_B8G8R8A8_SNorm = 45,
        Format_B8G8R8A8_UScaled = 46,
        Format_B8G8R8A8_SScaled = 47,
        Format_B8G8R8A8_UInt = 48,
        Format_B8G8R8A8_SInt = 49,
        Format_B8G8R8A8_SRGB = 50,

        Format_A8B8G8R8_UNorm_Pack32 = 51,
        Format_A8B8G8R8_SNorm_Pack32 = 52,
        Format_A8B8G8R8_UScaled_Pack32 = 53,
        Format_A8B8G8R8_SScaled_Pack32 = 54,
        Format_A8B8G8R8_UInt_Pack32 = 55,
        Format_A8B8G8R8_SInt_Pack32 = 56,
        Format_A8B8G8R8_SRGB_Pack32 = 57,

        Format_A2R10G10B10_UNorm_Pack32 = 58,
        Format_A2R10G10B10_SNorm_Pack32 = 59,
        Format_A2R10G10B10_UScaled_Pack32 = 60,
        Format_A2R10G10B10_SScaled_Pack32 = 61,
        Format_A2R10G10B10_UInt_Pack32 = 62,
        Format_A2R10G10B10_SInt_Pack32 = 63,
        Format_A2B10G10R10_UNorm_Pack32 = 64,
        Format_A2B10G10R10_SNorm_Pack32 = 65,
        Format_A2B10G10R10_UScaled_Pack32 = 66,
        Format_A2B10G10R10_SScaled_Pack32 = 67,
        Format_A2B10G10R10_UInt_Pack32 = 68,
        Format_A2B10G10R10_SInt_Pack32 = 69,

        Format_R16_UNorm = 70,
        Format_R16_SNorm = 71,
        Format_R16_UScaled = 72,
        Format_R16_SScaled = 73,
        Format_R16_UInt = 74,
        Format_R16_SInt = 75,
        Format_R16_SFloat = 76,

        Format_R16G16_UNorm = 77,
        Format_R16G16_SNorm = 78,
        Format_R16G16_UScaled = 79,
        Format_R16G16_SScaled = 80,
        Format_R16G16_UInt = 81,
        Format_R16G16_SInt = 82,
        Format_R16G16_SFloat = 83,

        Format_R16G16B16_UNorm = 84,
        Format_R16G16B16_SNorm = 85,
        Format_R16G16B16_UScaled = 86,
        Format_R16G16B16_SScaled = 87,
        Format_R16G16B16_UInt = 88,
        Format_R16G16B16_SInt = 89,
        Format_R16G16B16_SFloat = 90,

        Format_R16G16B16A16_UNorm = 91,
        Format_R16G16B16A16_SNorm = 92,
        Format_R16G16B16A16_UScaled = 93,
        Format_R16G16B16A16_SScaled = 94,
        Format_R16G16B16A16_UInt = 95,
        Format_R16G16B16A16_SInt = 96,
        Format_R16G16B16A16_SFloat = 97,

        Format_R32_UInt = 98,
        Format_R32_SInt = 99,
        Format_R32_SFloat = 100,
        Format_R32G32_UInt = 101,
        Format_R32G32_SInt = 102,
        Format_R32G32_SFloat = 103,
        Format_R32G32B32_UInt = 104,
        Format_R32G32B32_SInt = 105,
        Format_R32G32B32_SFloat = 106,
        Format_R32G32B32A32_UInt = 107,
        Format_R32G32B32A32_SInt = 108,
        Format_R32G32B32A32_SFloat = 109,

        Format_R64_UInt = 110,
        Format_R64_SInt = 111,
        Format_R64_SFloat = 112,
        Format_R64G64_UInt = 113,
        Format_R64G64_SInt = 114,
        Format_R64G64_SFloat = 115,
        Format_R64G64B64_UInt = 116,
        Format_R64G64B64_SInt = 117,
        Format_R64G64B64_SFloat = 118,
        Format_R64G64B64A64_UInt = 119,
        Format_R64G64B64A64_SInt = 120,
        Format_R64G64B64A64_SFloat = 121,
        Format_B10G11R11_UFloat_Pack32 = 122,
        Format_E5B9G9R9_UFloat_Pack32 = 123,

        Format_D16_UNorm = 124,
        Format_X8_D24_UNorm_Pack32 = 125,
        Format_D32_SFloat = 126,
        Format_S8_UInt = 127,
        Format_D16_UNorm_S8_UInt = 128,
        Format_D24_UNorm_S8_UInt = 129,
        Format_D32_SFloat_S8_UInt = 130,

        Format_BC1_RGB_UNorm_Block = 131,   //Format_RGB_DXT1_Unorm_Block8
        Format_BC1_RGB_SRGB_Block = 132,    //Format_RGB_DXT1_SRGB_Block8
        Format_BC1_RGBA_UNorm_Block = 133,  //Format_RGBA_DXT1_Unorm_Block8
        Format_BC1_RGBA_SRGB_Block = 134,   //Format_RGBA_DXT1_SRGA_Block8
        Format_BC2_UNorm_Block = 135,       //Format_RGB_DXT3_Unorm_Block16
        Format_BC2_SRGB_Block = 136,        //Format_RGB_DXT3_SRGB_Block16
        Format_BC3_UNorm_Block = 137,       //Format_RGB_DXT5_Unorm_Block16
        Format_BC3_SRGB_Block = 138,        //Format_RGB_DXT5_SRGB_Block16
        Format_BC4_UNorm_Block = 139,
        Format_BC4_SNorm_Block = 140,
        Format_BC5_UNorm_Block = 141,
        Format_BC5_SNorm_Block = 142,
        Format_BC6H_UFloat_Block = 143,
        Format_BC6H_SFloat_Block = 144,
        Format_BC7_UNorm_Block = 145,
        Format_BC7_SRGB_Block = 146,

        Format_ETC2_R8G8B8_UNorm_Block = 147,
        Format_ETC2_R8G8B8_SRGB_Block = 148,
        Format_ETC2_R8G8B8A1_UNorm_Block = 149,
        Format_ETC2_R8G8B8A1_SRGB_Block = 150,
        Format_ETC2_R8G8B8A8_UNorm_Block = 151,
        Format_ETC2_R8G8B8A8_SRGB_Block = 152,
        Format_EAC_R11_UNorm_Block = 153,
        Format_EAC_R11_SNorm_Block = 154,
        Format_EAC_R11G11_UNorm_Block = 155,
        Format_EAC_R11G11_SNorm_Block = 156,

        Format_ASTC_4x4_UNorm_Block = 157,
        Format_ASTC_4x4_SRGB_Block = 158,
        Format_ASTC_5x4_UNorm_Block = 159,
        Format_ASTC_5x4_SRGB_Block = 160,
        Format_ASTC_5x5_UNorm_Block = 161,
        Format_ASTC_5x5_SRGB_Block = 162,
        Format_ASTC_6x5_UNorm_Block = 163,
        Format_ASTC_6x5_SRGB_Block = 164,
        Format_ASTC_6x6_UNorm_Block = 165,
        Format_ASTC_6x6_SRGB_Block = 166,
        Format_ASTC_8x5_UNorm_Block = 167,
        Format_ASTC_8x5_SRGB_Block = 168,
        Format_ASTC_8x6_UNorm_Block = 169,
        Format_ASTC_8x6_SRGB_Block = 170,
        Format_ASTC_8x8_UNorm_Block = 171,
        Format_ASTC_8x8_SRGB_Block = 172,
        Format_ASTC_10x5_UNorm_Block = 173,
        Format_ASTC_10x5_SRGB_Block = 174,
        Format_ASTC_10x6_UNorm_Block = 175,
        Format_ASTC_10x6_SRGB_Block = 176,
        Format_ASTC_10x8_UNorm_Block = 177,
        Format_ASTC_10x8_SRGB_Block = 178,
        Format_ASTC_10x10_UNorm_Block = 179,
        Format_ASTC_10x10_SRGB_Block = 180,
        Format_ASTC_12x10_UNorm_Block = 181,
        Format_ASTC_12x10_SRGB_Block = 182,
        Format_ASTC_12x12_UNorm_Block = 183,
        Format_ASTC_12x12_SRGB_Block = 184,

        /*Format_G8B8G8R8_422_UNorm = 1000156000,
        Format_B8G8R8G8_422_UNorm = 1000156001,
        Format_G8_B8_R8_3PLANE_420_UNorm = 1000156002,
        Format_G8_B8R8_2PLANE_420_UNorm = 1000156003,
        Format_G8_B8_R8_3PLANE_422_UNorm = 1000156004,
        Format_G8_B8R8_2PLANE_422_UNorm = 1000156005,
        Format_G8_B8_R8_3PLANE_444_UNorm = 1000156006,
        Format_R10X6_UNorm_Pack16 = 1000156007,
        Format_R10X6G10X6_UNorm_2PACK16 = 1000156008,
        Format_R10X6G10X6B10X6A10X6_UNorm_4PACK16 = 1000156009,
        Format_G10X6B10X6G10X6R10X6_422_UNorm_4PACK16 = 1000156010,
        Format_B10X6G10X6R10X6G10X6_422_UNorm_4PACK16 = 1000156011,
        Format_G10X6_B10X6_R10X6_3PLANE_420_UNorm_3PACK16 = 1000156012,
        Format_G10X6_B10X6R10X6_2PLANE_420_UNorm_3PACK16 = 1000156013,
        Format_G10X6_B10X6_R10X6_3PLANE_422_UNorm_3PACK16 = 1000156014,
        Format_G10X6_B10X6R10X6_2PLANE_422_UNorm_3PACK16 = 1000156015,
        Format_G10X6_B10X6_R10X6_3PLANE_444_UNorm_3PACK16 = 1000156016,
        Format_R12X4_UNorm_Pack16 = 1000156017,
        Format_R12X4G12X4_UNorm_2PACK16 = 1000156018,
        Format_R12X4G12X4B12X4A12X4_UNorm_4PACK16 = 1000156019,
        Format_G12X4B12X4G12X4R12X4_422_UNorm_4PACK16 = 1000156020,
        Format_B12X4G12X4R12X4G12X4_422_UNorm_4PACK16 = 1000156021,
        Format_G12X4_B12X4_R12X4_3PLANE_420_UNorm_3PACK16 = 1000156022,
        Format_G12X4_B12X4R12X4_2PLANE_420_UNorm_3PACK16 = 1000156023,
        Format_G12X4_B12X4_R12X4_3PLANE_422_UNorm_3PACK16 = 1000156024,
        Format_G12X4_B12X4R12X4_2PLANE_422_UNorm_3PACK16 = 1000156025,
        Format_G12X4_B12X4_R12X4_3PLANE_444_UNorm_3PACK16 = 1000156026,
        Format_G16B16G16R16_422_UNorm = 1000156027,
        Format_B16G16R16G16_422_UNorm = 1000156028,
        Format_G16_B16_R16_3PLANE_420_UNorm = 1000156029,
        Format_G16_B16R16_2PLANE_420_UNorm = 1000156030,
        Format_G16_B16_R16_3PLANE_422_UNorm = 1000156031,
        Format_G16_B16R16_2PLANE_422_UNorm = 1000156032,
        Format_G16_B16_R16_3PLANE_444_UNorm = 1000156033,
        Format_PVRTC1_2BPP_UNorm_Block_IMG = 1000054000,
        Format_PVRTC1_4BPP_UNorm_Block_IMG = 1000054001,
        Format_PVRTC2_2BPP_UNorm_Block_IMG = 1000054002,
        Format_PVRTC2_4BPP_UNorm_Block_IMG = 1000054003,
        Format_PVRTC1_2BPP_SRGB_Block_IMG = 1000054004,
        Format_PVRTC1_4BPP_SRGB_Block_IMG = 1000054005,
        Format_PVRTC2_2BPP_SRGB_Block_IMG = 1000054006,
        Format_PVRTC2_4BPP_SRGB_Block_IMG = 1000054007,*/

        Format_Count
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief ImageFormat class
    */
    class ImageFormat final
    {
    public:

        /**
        * @brief Format functions.
        * @param Format format
        * @return count bytes per block
        */
        [[nodiscard]] static u32 getFormatBlockSize(Format format);

        /**
        * @brief Format functions.
        * @param Format format
        * @return count components
        */
        [[nodiscard]] static u32 getFormatCountComponent(Format format);

        /**
        * @brief Format functions.
        * @param Format format
        * @return true if is compressed format
        */
        [[nodiscard]] static bool isFormatCompressed(Format format);

        /**
        * @brief Format functions.
        * @param Format format
        * @return width, heigt inside compression format
        */
        [[nodiscard]] static math::Dimension2D getBlockDimension(Format format);

        /**
        * @brief calculateImageSize function.
        * @param const math::Dimension3D& size
        * @param u32 mipLevel
        * @param Format format
        * @return mip level szie of image in bytes
        */
        [[nodiscard]] static u64 calculateImageMipSize(const math::Dimension3D& size, u32 mipLevel, Format format);

        /**
        * @brief calculateImageSize function.
        * @param const math::Dimension3D& size
        * @param u32 mips
        * @param u32 layers
        * @param Format format
        * @return image size with mips in bytes
        */
        [[nodiscard]] static u64 calculateImageSize(const math::Dimension3D& size, u32 mips, u32 layers, Format format);

        /**
        * @brief calculateMipmapCount function.
        * @param const math::Dimension3D& size
        * @return count mipmap levels
        */
        [[nodiscard]] static u32 calculateMipmapCount(const math::Dimension3D& size);
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace renderer
} //namespace v3d
