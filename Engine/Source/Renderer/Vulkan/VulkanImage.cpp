#include "VulkanImage.h"

#include "Utils/Logger.h"
#include "Utils/CRC32.h"

#ifdef VULKAN_RENDER
#   include "VulkanDebug.h"
#   include "VulkanDevice.h"
#   include "VulkanCommandBufferManager.h"
#   include "VulkanStagingBuffer.h"
#   include "VulkanCommandBuffer.h"

namespace v3d
{
namespace renderer
{
namespace vk
{

std::string VulkanImage::imageTypeStringVK(VkImageType format)
{
    switch (format)
    {
#define STR(r) case VK_##r: return #r
        STR(IMAGE_TYPE_1D);
        STR(IMAGE_TYPE_2D);
        STR(IMAGE_TYPE_3D);
#undef STR
        default:
            ASSERT(false, "not found");
    }

    return "UNKNOWN_TYPE";
}

std::string VulkanImage::imageFormatStringVK(VkFormat format)
{
    switch (format)
    {
#define STR(r) case VK_##r: return #r
        STR(FORMAT_UNDEFINED);
        STR(FORMAT_R4G4_UNORM_PACK8);
        STR(FORMAT_R4G4B4A4_UNORM_PACK16);
        STR(FORMAT_B4G4R4A4_UNORM_PACK16);
        STR(FORMAT_R5G6B5_UNORM_PACK16);
        STR(FORMAT_B5G6R5_UNORM_PACK16);
        STR(FORMAT_R5G5B5A1_UNORM_PACK16);
        STR(FORMAT_B5G5R5A1_UNORM_PACK16);
        STR(FORMAT_A1R5G5B5_UNORM_PACK16);
        STR(FORMAT_R8_UNORM);
        STR(FORMAT_R8_SNORM);
        STR(FORMAT_R8_USCALED);
        STR(FORMAT_R8_SSCALED);
        STR(FORMAT_R8_UINT);
        STR(FORMAT_R8_SINT);
        STR(FORMAT_R8_SRGB);
        STR(FORMAT_R8G8_UNORM);
        STR(FORMAT_R8G8_SNORM);
        STR(FORMAT_R8G8_USCALED);
        STR(FORMAT_R8G8_SSCALED);
        STR(FORMAT_R8G8_UINT);
        STR(FORMAT_R8G8_SINT);
        STR(FORMAT_R8G8_SRGB);
        STR(FORMAT_R8G8B8_UNORM);
        STR(FORMAT_R8G8B8_SNORM);
        STR(FORMAT_R8G8B8_USCALED);
        STR(FORMAT_R8G8B8_SSCALED);
        STR(FORMAT_R8G8B8_UINT);
        STR(FORMAT_R8G8B8_SINT);
        STR(FORMAT_R8G8B8_SRGB);
        STR(FORMAT_B8G8R8_UNORM);
        STR(FORMAT_B8G8R8_SNORM);
        STR(FORMAT_B8G8R8_USCALED);
        STR(FORMAT_B8G8R8_SSCALED);
        STR(FORMAT_B8G8R8_UINT);
        STR(FORMAT_B8G8R8_SINT);
        STR(FORMAT_B8G8R8_SRGB);
        STR(FORMAT_R8G8B8A8_UNORM);
        STR(FORMAT_R8G8B8A8_SNORM);
        STR(FORMAT_R8G8B8A8_USCALED);
        STR(FORMAT_R8G8B8A8_SSCALED);
        STR(FORMAT_R8G8B8A8_UINT);
        STR(FORMAT_R8G8B8A8_SINT);
        STR(FORMAT_R8G8B8A8_SRGB);
        STR(FORMAT_B8G8R8A8_UNORM);
        STR(FORMAT_B8G8R8A8_SNORM);
        STR(FORMAT_B8G8R8A8_USCALED);
        STR(FORMAT_B8G8R8A8_SSCALED);
        STR(FORMAT_B8G8R8A8_UINT);
        STR(FORMAT_B8G8R8A8_SINT);
        STR(FORMAT_B8G8R8A8_SRGB);
        STR(FORMAT_A8B8G8R8_UNORM_PACK32);
        STR(FORMAT_A8B8G8R8_SNORM_PACK32);
        STR(FORMAT_A8B8G8R8_USCALED_PACK32);
        STR(FORMAT_A8B8G8R8_SSCALED_PACK32);
        STR(FORMAT_A8B8G8R8_UINT_PACK32);
        STR(FORMAT_A8B8G8R8_SINT_PACK32);
        STR(FORMAT_A8B8G8R8_SRGB_PACK32);
        STR(FORMAT_A2R10G10B10_UNORM_PACK32);
        STR(FORMAT_A2R10G10B10_SNORM_PACK32);
        STR(FORMAT_A2R10G10B10_USCALED_PACK32);
        STR(FORMAT_A2R10G10B10_SSCALED_PACK32);
        STR(FORMAT_A2R10G10B10_UINT_PACK32);
        STR(FORMAT_A2R10G10B10_SINT_PACK32);
        STR(FORMAT_A2B10G10R10_UNORM_PACK32);
        STR(FORMAT_A2B10G10R10_SNORM_PACK32);
        STR(FORMAT_A2B10G10R10_USCALED_PACK32);
        STR(FORMAT_A2B10G10R10_SSCALED_PACK32);
        STR(FORMAT_A2B10G10R10_UINT_PACK32);
        STR(FORMAT_A2B10G10R10_SINT_PACK32);
        STR(FORMAT_R16_UNORM);
        STR(FORMAT_R16_SNORM);
        STR(FORMAT_R16_USCALED);
        STR(FORMAT_R16_SSCALED);
        STR(FORMAT_R16_UINT);
        STR(FORMAT_R16_SINT);
        STR(FORMAT_R16_SFLOAT);
        STR(FORMAT_R16G16_UNORM);
        STR(FORMAT_R16G16_SNORM);
        STR(FORMAT_R16G16_USCALED);
        STR(FORMAT_R16G16_SSCALED);
        STR(FORMAT_R16G16_UINT);
        STR(FORMAT_R16G16_SINT);
        STR(FORMAT_R16G16_SFLOAT);
        STR(FORMAT_R16G16B16_UNORM);
        STR(FORMAT_R16G16B16_SNORM);
        STR(FORMAT_R16G16B16_USCALED);
        STR(FORMAT_R16G16B16_SSCALED);
        STR(FORMAT_R16G16B16_UINT);
        STR(FORMAT_R16G16B16_SINT);
        STR(FORMAT_R16G16B16_SFLOAT);
        STR(FORMAT_R16G16B16A16_UNORM);
        STR(FORMAT_R16G16B16A16_SNORM);
        STR(FORMAT_R16G16B16A16_USCALED);
        STR(FORMAT_R16G16B16A16_SSCALED);
        STR(FORMAT_R16G16B16A16_UINT);
        STR(FORMAT_R16G16B16A16_SINT);
        STR(FORMAT_R16G16B16A16_SFLOAT);
        STR(FORMAT_R32_UINT);
        STR(FORMAT_R32_SINT);
        STR(FORMAT_R32_SFLOAT);
        STR(FORMAT_R32G32_UINT);
        STR(FORMAT_R32G32_SINT);
        STR(FORMAT_R32G32_SFLOAT);
        STR(FORMAT_R32G32B32_UINT);
        STR(FORMAT_R32G32B32_SINT);
        STR(FORMAT_R32G32B32_SFLOAT);
        STR(FORMAT_R32G32B32A32_UINT);
        STR(FORMAT_R32G32B32A32_SINT);
        STR(FORMAT_R32G32B32A32_SFLOAT);
        STR(FORMAT_R64_UINT);
        STR(FORMAT_R64_SINT);
        STR(FORMAT_R64_SFLOAT);
        STR(FORMAT_R64G64_UINT);
        STR(FORMAT_R64G64_SINT);
        STR(FORMAT_R64G64_SFLOAT);
        STR(FORMAT_R64G64B64_UINT);
        STR(FORMAT_R64G64B64_SINT);
        STR(FORMAT_R64G64B64_SFLOAT);
        STR(FORMAT_R64G64B64A64_UINT);
        STR(FORMAT_R64G64B64A64_SINT);
        STR(FORMAT_R64G64B64A64_SFLOAT);
        STR(FORMAT_B10G11R11_UFLOAT_PACK32);
        STR(FORMAT_E5B9G9R9_UFLOAT_PACK32);
        STR(FORMAT_D16_UNORM);
        STR(FORMAT_X8_D24_UNORM_PACK32);
        STR(FORMAT_D32_SFLOAT);
        STR(FORMAT_S8_UINT);
        STR(FORMAT_D16_UNORM_S8_UINT);
        STR(FORMAT_D24_UNORM_S8_UINT);
        STR(FORMAT_D32_SFLOAT_S8_UINT);
        STR(FORMAT_BC1_RGB_UNORM_BLOCK);
        STR(FORMAT_BC1_RGB_SRGB_BLOCK);
        STR(FORMAT_BC1_RGBA_UNORM_BLOCK);
        STR(FORMAT_BC1_RGBA_SRGB_BLOCK);
        STR(FORMAT_BC2_UNORM_BLOCK);
        STR(FORMAT_BC2_SRGB_BLOCK);
        STR(FORMAT_BC3_UNORM_BLOCK);
        STR(FORMAT_BC3_SRGB_BLOCK);
        STR(FORMAT_BC4_UNORM_BLOCK);
        STR(FORMAT_BC4_SNORM_BLOCK);
        STR(FORMAT_BC5_UNORM_BLOCK);
        STR(FORMAT_BC5_SNORM_BLOCK);
        STR(FORMAT_BC6H_UFLOAT_BLOCK);
        STR(FORMAT_BC6H_SFLOAT_BLOCK);
        STR(FORMAT_BC7_UNORM_BLOCK);
        STR(FORMAT_BC7_SRGB_BLOCK);
        STR(FORMAT_ETC2_R8G8B8_UNORM_BLOCK);
        STR(FORMAT_ETC2_R8G8B8_SRGB_BLOCK);
        STR(FORMAT_ETC2_R8G8B8A1_UNORM_BLOCK);
        STR(FORMAT_ETC2_R8G8B8A1_SRGB_BLOCK);
        STR(FORMAT_ETC2_R8G8B8A8_UNORM_BLOCK);
        STR(FORMAT_ETC2_R8G8B8A8_SRGB_BLOCK);
        STR(FORMAT_EAC_R11_UNORM_BLOCK);
        STR(FORMAT_EAC_R11_SNORM_BLOCK);
        STR(FORMAT_EAC_R11G11_UNORM_BLOCK);
        STR(FORMAT_EAC_R11G11_SNORM_BLOCK);
        STR(FORMAT_ASTC_4x4_UNORM_BLOCK);
        STR(FORMAT_ASTC_4x4_SRGB_BLOCK);
        STR(FORMAT_ASTC_5x4_UNORM_BLOCK);
        STR(FORMAT_ASTC_5x4_SRGB_BLOCK);
        STR(FORMAT_ASTC_5x5_UNORM_BLOCK);
        STR(FORMAT_ASTC_5x5_SRGB_BLOCK);
        STR(FORMAT_ASTC_6x5_UNORM_BLOCK);
        STR(FORMAT_ASTC_6x5_SRGB_BLOCK);
        STR(FORMAT_ASTC_6x6_UNORM_BLOCK);
        STR(FORMAT_ASTC_6x6_SRGB_BLOCK);
        STR(FORMAT_ASTC_8x5_UNORM_BLOCK);
        STR(FORMAT_ASTC_8x5_SRGB_BLOCK);
        STR(FORMAT_ASTC_8x6_UNORM_BLOCK);
        STR(FORMAT_ASTC_8x6_SRGB_BLOCK);
        STR(FORMAT_ASTC_8x8_UNORM_BLOCK);
        STR(FORMAT_ASTC_8x8_SRGB_BLOCK);
        STR(FORMAT_ASTC_10x5_UNORM_BLOCK);
        STR(FORMAT_ASTC_10x5_SRGB_BLOCK);
        STR(FORMAT_ASTC_10x6_UNORM_BLOCK);
        STR(FORMAT_ASTC_10x6_SRGB_BLOCK);
        STR(FORMAT_ASTC_10x8_UNORM_BLOCK);
        STR(FORMAT_ASTC_10x8_SRGB_BLOCK);
        STR(FORMAT_ASTC_10x10_UNORM_BLOCK);
        STR(FORMAT_ASTC_10x10_SRGB_BLOCK);
        STR(FORMAT_ASTC_12x10_UNORM_BLOCK);
        STR(FORMAT_ASTC_12x10_SRGB_BLOCK);
        STR(FORMAT_ASTC_12x12_UNORM_BLOCK);
        STR(FORMAT_ASTC_12x12_SRGB_BLOCK);
#undef STR
    default:
        ASSERT(false, "not found");
    }

    return "UNKNOWN_FORMAT";
}

VkFormat VulkanImage::convertImageFormatToVkFormat(renderer::Format format)
{
    switch (format)
    {
    case v3d::renderer::Format_Undefined:
        return VK_FORMAT_UNDEFINED;

    case v3d::renderer::Format_R4G4_UNorm_Pack8:
         return VK_FORMAT_R4G4_UNORM_PACK8;
    case v3d::renderer::Format_R4G4B4A4_UNorm_Pack16:
         return VK_FORMAT_R4G4B4A4_UNORM_PACK16;
    case v3d::renderer::Format_B4G4R4A4_UNorm_Pack16:
         return VK_FORMAT_B4G4R4A4_UNORM_PACK16;
    case v3d::renderer::Format_R5G6B5_UNorm_Pack16:
         return VK_FORMAT_R5G6B5_UNORM_PACK16;
    case v3d::renderer::Format_B5G6R5_UNorm_Pack16:
         return VK_FORMAT_B5G6R5_UNORM_PACK16;
    case v3d::renderer::Format_R5G5B5A1_UNorm_Pack16:
         return VK_FORMAT_R5G5B5A1_UNORM_PACK16;
    case v3d::renderer::Format_B5G5R5A1_UNorm_Pack16:
         return VK_FORMAT_B5G5R5A1_UNORM_PACK16;
    case v3d::renderer::Format_A1R5G5B5_UNorm_Pack16:
         return VK_FORMAT_A1R5G5B5_UNORM_PACK16;

    case v3d::renderer::Format_R8_UNorm:
         return VK_FORMAT_R8_UNORM;
    case v3d::renderer::Format_R8_SNorm:
         return VK_FORMAT_R8_SNORM;
    case v3d::renderer::Format_R8_UScaled:
         return VK_FORMAT_R8_USCALED;
    case v3d::renderer::Format_R8_SScaled:
         return VK_FORMAT_R8_SSCALED;
    case v3d::renderer::Format_R8_UInt:
         return VK_FORMAT_R8_UINT;
    case v3d::renderer::Format_R8_SInt:
         return VK_FORMAT_R8_SINT;
    case v3d::renderer::Format_R8_SRGB:
         return VK_FORMAT_R8_SRGB;

    case v3d::renderer::Format_R8G8_UNorm:
         return VK_FORMAT_R8G8_UNORM;
    case v3d::renderer::Format_R8G8_SNorm:
         return VK_FORMAT_R8G8_SNORM;
    case v3d::renderer::Format_R8G8_UScaled:
         return VK_FORMAT_R8G8_USCALED;
    case v3d::renderer::Format_R8G8_SScaled:
         return VK_FORMAT_R8G8_SSCALED;
    case v3d::renderer::Format_R8G8_UInt:
         return VK_FORMAT_R8G8_UINT;
    case v3d::renderer::Format_R8G8_SInt:
         return VK_FORMAT_R8G8_SINT;
    case v3d::renderer::Format_R8G8_SRGB:
         return VK_FORMAT_R8G8_SRGB;

    case v3d::renderer::Format_R8G8B8_UNorm:
         return VK_FORMAT_R8G8B8_UNORM;
    case v3d::renderer::Format_R8G8B8_SNorm:
         return VK_FORMAT_R8G8B8_SNORM;
    case v3d::renderer::Format_R8G8B8_UScaled:
         return VK_FORMAT_R8G8B8_USCALED;
    case v3d::renderer::Format_R8G8B8_SScaled:
         return VK_FORMAT_R8G8B8_SSCALED;
    case v3d::renderer::Format_R8G8B8_UInt:
         return VK_FORMAT_R8G8B8_UINT;
    case v3d::renderer::Format_R8G8B8_SInt:
         return VK_FORMAT_R8G8B8_SINT;
    case v3d::renderer::Format_R8G8B8_SRGB:
         return VK_FORMAT_R8G8B8_SRGB;
    case v3d::renderer::Format_B8G8R8_UNorm:
         return VK_FORMAT_B8G8R8_UNORM;
    case v3d::renderer::Format_B8G8R8_SNorm:
         return VK_FORMAT_B8G8R8_SNORM;
    case v3d::renderer::Format_B8G8R8_UScaled:
         return VK_FORMAT_B8G8R8_USCALED;
    case v3d::renderer::Format_B8G8R8_SScaled:
         return VK_FORMAT_B8G8R8_SSCALED;
    case v3d::renderer::Format_B8G8R8_UInt:
         return VK_FORMAT_B8G8R8_UINT;
    case v3d::renderer::Format_B8G8R8_SInt:
         return VK_FORMAT_B8G8R8_SINT;
    case v3d::renderer::Format_B8G8R8_SRGB:
         return VK_FORMAT_B8G8R8_SRGB;

    case v3d::renderer::Format_R8G8B8A8_UNorm:
         return VK_FORMAT_R8G8B8A8_UNORM;
    case v3d::renderer::Format_R8G8B8A8_SNorm:
         return VK_FORMAT_R8G8B8A8_SNORM;
    case v3d::renderer::Format_R8G8B8A8_UScaled:
         return VK_FORMAT_R8G8B8A8_USCALED;
    case v3d::renderer::Format_R8G8B8A8_SScaled:
         return VK_FORMAT_R8G8B8A8_SSCALED;
    case v3d::renderer::Format_R8G8B8A8_UInt:
         return VK_FORMAT_R8G8B8A8_UINT;
    case v3d::renderer::Format_R8G8B8A8_SInt:
         return VK_FORMAT_R8G8B8A8_SINT;
    case v3d::renderer::Format_R8G8B8A8_SRGB:
         return VK_FORMAT_R8G8B8A8_SRGB;
    case v3d::renderer::Format_B8G8R8A8_UNorm:
         return VK_FORMAT_B8G8R8A8_UNORM;
    case v3d::renderer::Format_B8G8R8A8_SNorm:
         return VK_FORMAT_B8G8R8A8_SNORM;
    case v3d::renderer::Format_B8G8R8A8_UScaled:
         return VK_FORMAT_B8G8R8A8_USCALED;
    case v3d::renderer::Format_B8G8R8A8_SScaled:
         return VK_FORMAT_B8G8R8A8_SSCALED;
    case v3d::renderer::Format_B8G8R8A8_UInt:
         return VK_FORMAT_B8G8R8A8_UINT;
    case v3d::renderer::Format_B8G8R8A8_SInt:
         return VK_FORMAT_B8G8R8A8_SINT;
    case v3d::renderer::Format_B8G8R8A8_SRGB:
         return VK_FORMAT_B8G8R8A8_SRGB;
    case v3d::renderer::Format_A8B8G8R8_UNorm_Pack32:
         return VK_FORMAT_A8B8G8R8_UNORM_PACK32;
    case v3d::renderer::Format_A8B8G8R8_SNorm_Pack32:
         return VK_FORMAT_A8B8G8R8_SNORM_PACK32;
    case v3d::renderer::Format_A8B8G8R8_UScaled_Pack32:
         return VK_FORMAT_A8B8G8R8_USCALED_PACK32;
    case v3d::renderer::Format_A8B8G8R8_SScaled_Pack32:
         return VK_FORMAT_A8B8G8R8_SSCALED_PACK32;
    case v3d::renderer::Format_A8B8G8R8_UInt_Pack32:
         return VK_FORMAT_A8B8G8R8_UINT_PACK32;
    case v3d::renderer::Format_A8B8G8R8_SInt_Pack32:
         return VK_FORMAT_A8B8G8R8_SINT_PACK32;
    case v3d::renderer::Format_A8B8G8R8_SRGB_Pack32:
         return VK_FORMAT_A8B8G8R8_SRGB_PACK32;

    case v3d::renderer::Format_A2R10G10B10_UNorm_Pack32:
         return VK_FORMAT_A2R10G10B10_UNORM_PACK32;
    case v3d::renderer::Format_A2R10G10B10_SNorm_Pack32:
         return VK_FORMAT_A2R10G10B10_SNORM_PACK32;
    case v3d::renderer::Format_A2R10G10B10_UScaled_Pack32:
         return VK_FORMAT_A2R10G10B10_USCALED_PACK32;
    case v3d::renderer::Format_A2R10G10B10_SScaled_Pack32:
         return VK_FORMAT_A2R10G10B10_SSCALED_PACK32;
    case v3d::renderer::Format_A2R10G10B10_UInt_Pack32:
         return VK_FORMAT_A2R10G10B10_UINT_PACK32;
    case v3d::renderer::Format_A2R10G10B10_SInt_Pack32:
         return VK_FORMAT_A2R10G10B10_SINT_PACK32;
    case v3d::renderer::Format_A2B10G10R10_UNorm_Pack32:
         return VK_FORMAT_A2B10G10R10_UNORM_PACK32;
    case v3d::renderer::Format_A2B10G10R10_SNorm_Pack32:
         return VK_FORMAT_A2B10G10R10_SNORM_PACK32;
    case v3d::renderer::Format_A2B10G10R10_UScaled_Pack32:
         return VK_FORMAT_A2B10G10R10_USCALED_PACK32;
    case v3d::renderer::Format_A2B10G10R10_SScaled_Pack32:
         return VK_FORMAT_A2B10G10R10_SSCALED_PACK32;
    case v3d::renderer::Format_A2B10G10R10_UInt_Pack32:
         return VK_FORMAT_A2B10G10R10_UINT_PACK32;
    case v3d::renderer::Format_A2B10G10R10_SInt_Pack32:
         return VK_FORMAT_A2B10G10R10_SINT_PACK32;

    case v3d::renderer::Format_R16_UNorm:
         return VK_FORMAT_R16_UNORM;
    case v3d::renderer::Format_R16_SNorm:
         return VK_FORMAT_R16_SNORM;
    case v3d::renderer::Format_R16_UScaled:
         return VK_FORMAT_R16_USCALED;
    case v3d::renderer::Format_R16_SScaled:
         return VK_FORMAT_R16_SSCALED;
    case v3d::renderer::Format_R16_UInt:
         return VK_FORMAT_R16_UINT;
    case v3d::renderer::Format_R16_SInt:
         return VK_FORMAT_R16_SINT;
    case v3d::renderer::Format_R16_SFloat:
         return VK_FORMAT_R16_SFLOAT;

    case v3d::renderer::Format_R16G16_UNorm:
         return VK_FORMAT_R16G16_UNORM;
    case v3d::renderer::Format_R16G16_SNorm:
         return VK_FORMAT_R16G16_SNORM;
    case v3d::renderer::Format_R16G16_UScaled:
         return VK_FORMAT_R16G16_USCALED;
    case v3d::renderer::Format_R16G16_SScaled:
         return VK_FORMAT_R16G16_SSCALED;
    case v3d::renderer::Format_R16G16_UInt:
         return VK_FORMAT_R16G16_UINT;
    case v3d::renderer::Format_R16G16_SInt:
         return VK_FORMAT_R16G16_SINT;
    case v3d::renderer::Format_R16G16_SFloat:
         return VK_FORMAT_R16G16_SFLOAT;

    case v3d::renderer::Format_R16G16B16_UNorm:
         return VK_FORMAT_R16G16B16_UNORM;
    case v3d::renderer::Format_R16G16B16_SNorm:
         return VK_FORMAT_R16G16B16_SNORM;
    case v3d::renderer::Format_R16G16B16_UScaled:
         return VK_FORMAT_R16G16B16_USCALED;
    case v3d::renderer::Format_R16G16B16_SScaled:
         return VK_FORMAT_R16G16B16_SSCALED;
    case v3d::renderer::Format_R16G16B16_UInt:
         return VK_FORMAT_R16G16B16_UINT;
    case v3d::renderer::Format_R16G16B16_SInt:
         return VK_FORMAT_R16G16B16_SINT;
    case v3d::renderer::Format_R16G16B16_SFloat:
         return VK_FORMAT_R16G16B16_SFLOAT;

    case v3d::renderer::Format_R16G16B16A16_UNorm:
         return VK_FORMAT_R16G16B16A16_UNORM;
    case v3d::renderer::Format_R16G16B16A16_SNorm:
         return VK_FORMAT_R16G16B16A16_SNORM;
    case v3d::renderer::Format_R16G16B16A16_UScaled:
         return VK_FORMAT_R16G16B16A16_USCALED;
    case v3d::renderer::Format_R16G16B16A16_SScaled:
         return VK_FORMAT_R16G16B16A16_SSCALED;
    case v3d::renderer::Format_R16G16B16A16_UInt:
         return VK_FORMAT_R16G16B16A16_UINT;
    case v3d::renderer::Format_R16G16B16A16_SInt:
         return VK_FORMAT_R16G16B16A16_SINT;
    case v3d::renderer::Format_R16G16B16A16_SFloat:
         return VK_FORMAT_R16G16B16A16_SFLOAT;

    case v3d::renderer::Format_R32_UInt:
         return VK_FORMAT_R32_UINT;
    case v3d::renderer::Format_R32_SInt:
         return VK_FORMAT_R32_SINT;
    case v3d::renderer::Format_R32_SFloat:
         return VK_FORMAT_R32_SFLOAT;

    case v3d::renderer::Format_R32G32_UInt:
         return VK_FORMAT_R32G32_UINT;
    case v3d::renderer::Format_R32G32_SInt:
         return VK_FORMAT_R32G32_SINT;
    case v3d::renderer::Format_R32G32_SFloat:
         return VK_FORMAT_R32G32_SFLOAT;

    case v3d::renderer::Format_R32G32B32_UInt:
         return VK_FORMAT_R32G32B32_UINT;
    case v3d::renderer::Format_R32G32B32_SInt:
         return VK_FORMAT_R32G32B32_SINT;
    case v3d::renderer::Format_R32G32B32_SFloat:
         return VK_FORMAT_R32G32B32_SFLOAT;

    case v3d::renderer::Format_R32G32B32A32_UInt:
         return VK_FORMAT_R32G32B32A32_UINT;
    case v3d::renderer::Format_R32G32B32A32_SInt:
         return VK_FORMAT_R32G32B32A32_SINT;
    case v3d::renderer::Format_R32G32B32A32_SFloat:
         return VK_FORMAT_R32G32B32A32_SFLOAT;

    case v3d::renderer::Format_R64_UInt:
         return VK_FORMAT_R64_UINT;
    case v3d::renderer::Format_R64_SInt:
         return VK_FORMAT_R64_SINT;
    case v3d::renderer::Format_R64_SFloat:
         return VK_FORMAT_R64_SFLOAT;
    case v3d::renderer::Format_R64G64_UInt:
         return VK_FORMAT_R64G64_UINT;
    case v3d::renderer::Format_R64G64_SInt:
         return VK_FORMAT_R64G64_SINT;
    case v3d::renderer::Format_R64G64_SFloat:
         return VK_FORMAT_R64G64_SFLOAT;
    case v3d::renderer::Format_R64G64B64_UInt:
         return VK_FORMAT_R64G64B64_UINT;
    case v3d::renderer::Format_R64G64B64_SInt:
         return VK_FORMAT_R64G64B64_SINT;
    case v3d::renderer::Format_R64G64B64_SFloat:
         return VK_FORMAT_R64G64B64_SFLOAT;
    case v3d::renderer::Format_R64G64B64A64_UInt:
         return VK_FORMAT_R64G64B64A64_UINT;
    case v3d::renderer::Format_R64G64B64A64_SInt:
         return VK_FORMAT_R64G64B64A64_SINT;
    case v3d::renderer::Format_R64G64B64A64_SFloat:
         return VK_FORMAT_R64G64B64A64_SFLOAT;
    case v3d::renderer::Format_B10G11R11_UFloat_Pack32:
         return VK_FORMAT_B10G11R11_UFLOAT_PACK32;
    case v3d::renderer::Format_E5B9G9R9_UFloat_Pack32:
         return VK_FORMAT_E5B9G9R9_UFLOAT_PACK32;

    case v3d::renderer::Format_D16_UNorm:
         return VK_FORMAT_D16_UNORM;
    case v3d::renderer::Format_X8_D24_UNorm_Pack32:
         return VK_FORMAT_X8_D24_UNORM_PACK32;
    case v3d::renderer::Format_D32_SFloat:
         return VK_FORMAT_D32_SFLOAT;
    case v3d::renderer::Format_S8_UInt:
         return VK_FORMAT_S8_UINT;
    case v3d::renderer::Format_D16_UNorm_S8_UInt:
         return VK_FORMAT_D16_UNORM_S8_UINT;
    case v3d::renderer::Format_D24_UNorm_S8_UInt:
         return VK_FORMAT_D24_UNORM_S8_UINT;
    case v3d::renderer::Format_D32_SFloat_S8_UInt:
         return VK_FORMAT_D32_SFLOAT_S8_UINT;

    case v3d::renderer::Format_BC1_RGB_UNorm_Block:
         return VK_FORMAT_BC1_RGB_UNORM_BLOCK;
    case v3d::renderer::Format_BC1_RGB_SRGB_Block:
         return VK_FORMAT_BC1_RGB_SRGB_BLOCK;
    case v3d::renderer::Format_BC1_RGBA_UNorm_Block:
         return VK_FORMAT_BC1_RGBA_UNORM_BLOCK;
    case v3d::renderer::Format_BC1_RGBA_SRGB_Block:
         return VK_FORMAT_BC1_RGBA_SRGB_BLOCK;
    case v3d::renderer::Format_BC2_UNorm_Block:
         return VK_FORMAT_BC2_UNORM_BLOCK;
    case v3d::renderer::Format_BC2_SRGB_Block:
         return VK_FORMAT_BC2_SRGB_BLOCK;
    case v3d::renderer::Format_BC3_UNorm_Block:
         return VK_FORMAT_BC3_UNORM_BLOCK;
    case v3d::renderer::Format_BC3_SRGB_Block:
         return VK_FORMAT_BC3_SRGB_BLOCK;
    case v3d::renderer::Format_BC4_UNorm_Block:
         return VK_FORMAT_BC4_UNORM_BLOCK;
    case v3d::renderer::Format_BC4_SNorm_Block:
         return VK_FORMAT_BC4_SNORM_BLOCK;
    case v3d::renderer::Format_BC5_UNorm_Block:
         return VK_FORMAT_BC5_UNORM_BLOCK;
    case v3d::renderer::Format_BC5_SNorm_Block:
         return VK_FORMAT_BC5_SNORM_BLOCK;
    case v3d::renderer::Format_BC6H_UFloat_Block:
         return VK_FORMAT_BC6H_UFLOAT_BLOCK;
    case v3d::renderer::Format_BC6H_SFloat_Block:
         return VK_FORMAT_BC6H_SFLOAT_BLOCK;
    case v3d::renderer::Format_BC7_UNorm_Block:
         return VK_FORMAT_BC7_UNORM_BLOCK;
    case v3d::renderer::Format_BC7_SRGB_Block:
         return VK_FORMAT_BC7_SRGB_BLOCK;

    case v3d::renderer::Format_ETC2_R8G8B8_UNorm_Block:
         return VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK;
    case v3d::renderer::Format_ETC2_R8G8B8_SRGB_Block:
         return VK_FORMAT_ETC2_R8G8B8_SRGB_BLOCK;
    case v3d::renderer::Format_ETC2_R8G8B8A1_UNorm_Block:
         return VK_FORMAT_ETC2_R8G8B8A1_UNORM_BLOCK;
    case v3d::renderer::Format_ETC2_R8G8B8A1_SRGB_Block:
         return VK_FORMAT_ETC2_R8G8B8A1_SRGB_BLOCK;
    case v3d::renderer::Format_ETC2_R8G8B8A8_UNorm_Block:
         return VK_FORMAT_ETC2_R8G8B8A8_UNORM_BLOCK;
    case v3d::renderer::Format_ETC2_R8G8B8A8_SRGB_Block:
         return VK_FORMAT_ETC2_R8G8B8A8_SRGB_BLOCK;
    case v3d::renderer::Format_EAC_R11_UNorm_Block:
         return VK_FORMAT_EAC_R11_UNORM_BLOCK;
    case v3d::renderer::Format_EAC_R11_SNorm_Block:
         return VK_FORMAT_EAC_R11_SNORM_BLOCK;
    case v3d::renderer::Format_EAC_R11G11_UNorm_Block:
         return VK_FORMAT_EAC_R11G11_UNORM_BLOCK;
    case v3d::renderer::Format_EAC_R11G11_SNorm_Block:
         return VK_FORMAT_EAC_R11G11_SNORM_BLOCK;

    case v3d::renderer::Format_ASTC_4x4_UNorm_Block:
         return VK_FORMAT_ASTC_4x4_UNORM_BLOCK;
    case v3d::renderer::Format_ASTC_4x4_SRGB_Block:
         return VK_FORMAT_ASTC_4x4_SRGB_BLOCK;
    case v3d::renderer::Format_ASTC_5x4_UNorm_Block:
         return VK_FORMAT_ASTC_5x4_UNORM_BLOCK;
    case v3d::renderer::Format_ASTC_5x4_SRGB_Block:
         return VK_FORMAT_ASTC_5x4_SRGB_BLOCK;
    case v3d::renderer::Format_ASTC_5x5_UNorm_Block:
         return VK_FORMAT_ASTC_5x5_UNORM_BLOCK;
    case v3d::renderer::Format_ASTC_5x5_SRGB_Block:
         return VK_FORMAT_ASTC_5x5_SRGB_BLOCK;
    case v3d::renderer::Format_ASTC_6x5_UNorm_Block:
         return VK_FORMAT_ASTC_6x5_UNORM_BLOCK;
    case v3d::renderer::Format_ASTC_6x5_SRGB_Block:
         return VK_FORMAT_ASTC_6x5_SRGB_BLOCK;
    case v3d::renderer::Format_ASTC_6x6_UNorm_Block:
         return VK_FORMAT_ASTC_6x6_UNORM_BLOCK;
    case v3d::renderer::Format_ASTC_6x6_SRGB_Block:
         return VK_FORMAT_ASTC_6x6_SRGB_BLOCK;
    case v3d::renderer::Format_ASTC_8x5_UNorm_Block:
         return VK_FORMAT_ASTC_8x5_UNORM_BLOCK;
    case v3d::renderer::Format_ASTC_8x5_SRGB_Block:
         return VK_FORMAT_ASTC_8x5_SRGB_BLOCK;
    case v3d::renderer::Format_ASTC_8x6_UNorm_Block:
         return VK_FORMAT_ASTC_8x6_UNORM_BLOCK;
    case v3d::renderer::Format_ASTC_8x6_SRGB_Block:
         return VK_FORMAT_ASTC_8x6_SRGB_BLOCK;
    case v3d::renderer::Format_ASTC_8x8_UNorm_Block:
         return VK_FORMAT_ASTC_8x8_UNORM_BLOCK;
    case v3d::renderer::Format_ASTC_8x8_SRGB_Block:
         return VK_FORMAT_ASTC_8x8_SRGB_BLOCK;
    case v3d::renderer::Format_ASTC_10x5_UNorm_Block:
         return VK_FORMAT_ASTC_10x5_UNORM_BLOCK;
    case v3d::renderer::Format_ASTC_10x5_SRGB_Block:
         return VK_FORMAT_ASTC_10x5_SRGB_BLOCK;
    case v3d::renderer::Format_ASTC_10x6_UNorm_Block:
         return VK_FORMAT_ASTC_10x6_UNORM_BLOCK;
    case v3d::renderer::Format_ASTC_10x6_SRGB_Block:
         return VK_FORMAT_ASTC_10x6_SRGB_BLOCK;
    case v3d::renderer::Format_ASTC_10x8_UNorm_Block:
         return VK_FORMAT_ASTC_10x8_UNORM_BLOCK;
    case v3d::renderer::Format_ASTC_10x8_SRGB_Block:
         return VK_FORMAT_ASTC_10x8_SRGB_BLOCK;
    case v3d::renderer::Format_ASTC_10x10_UNorm_Block:
         return VK_FORMAT_ASTC_10x10_UNORM_BLOCK;
    case v3d::renderer::Format_ASTC_10x10_SRGB_Block:
         return VK_FORMAT_ASTC_10x10_SRGB_BLOCK;
    case v3d::renderer::Format_ASTC_12x10_UNorm_Block:
         return VK_FORMAT_ASTC_12x10_UNORM_BLOCK;
    case v3d::renderer::Format_ASTC_12x10_SRGB_Block:
         return VK_FORMAT_ASTC_12x10_SRGB_BLOCK;
    case v3d::renderer::Format_ASTC_12x12_UNorm_Block:
         return VK_FORMAT_ASTC_12x12_UNORM_BLOCK;
    case v3d::renderer::Format_ASTC_12x12_SRGB_Block:
         return VK_FORMAT_ASTC_12x12_SRGB_BLOCK;

    default:
        ASSERT(false, "unknown");
    }

    return VK_FORMAT_UNDEFINED;
}

Format VulkanImage::convertVkImageFormatToFormat(VkFormat format)
{
    return Format(format);
}

VkImageType VulkanImage::convertTextureTargetToVkImageType(TextureTarget target)
{
    switch (target)
    {
    case TextureTarget::Texture1D:
    case TextureTarget::Texture1DArray:
        return VK_IMAGE_TYPE_1D;

    case TextureTarget::Texture2D:
    case TextureTarget::Texture2DArray:
    case TextureTarget::TextureCubeMap:
        return VK_IMAGE_TYPE_2D;

    case TextureTarget::Texture3D:
        return VK_IMAGE_TYPE_3D;

    default:
        ASSERT(false, "unknown");
    }

    return VK_IMAGE_TYPE_2D;
}

VkSampleCountFlagBits VulkanImage::convertRenderTargetSamplesToVkSampleCount(TextureSamples samples)
{
    switch (samples)
    {
    case TextureSamples::TextureSamples_x1:
        return VK_SAMPLE_COUNT_1_BIT;

    case TextureSamples::TextureSamples_x2:
        return VK_SAMPLE_COUNT_2_BIT;

    case TextureSamples::TextureSamples_x4:
        return VK_SAMPLE_COUNT_4_BIT;

    case TextureSamples::TextureSamples_x8:
        return VK_SAMPLE_COUNT_8_BIT;

    case TextureSamples::TextureSamples_x16:
        return VK_SAMPLE_COUNT_16_BIT;

    case TextureSamples::TextureSamples_x32:
        return VK_SAMPLE_COUNT_32_BIT;

    case TextureSamples::TextureSamples_x64:
        return VK_SAMPLE_COUNT_64_BIT;

    default:
        ASSERT(false, "cant convert");
    }

    return VK_SAMPLE_COUNT_1_BIT;
}

VkImageAspectFlags VulkanImage::getImageAspectFlags(VkFormat format)
{
    switch (format)
    {
    case VK_FORMAT_D16_UNORM:
    case VK_FORMAT_X8_D24_UNORM_PACK32:
    case VK_FORMAT_D32_SFLOAT:
        return VK_IMAGE_ASPECT_DEPTH_BIT;

    case VK_FORMAT_S8_UINT:
        return VK_IMAGE_ASPECT_STENCIL_BIT;

    case VK_FORMAT_D16_UNORM_S8_UINT:
    case VK_FORMAT_D24_UNORM_S8_UINT:
    case VK_FORMAT_D32_SFLOAT_S8_UINT:
        return VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;

    default:
        return VK_IMAGE_ASPECT_COLOR_BIT;
    }
}

bool VulkanImage::isColorFormat(VkFormat format)
{
    return !isDepthStencilFormat(format);
}

bool VulkanImage::isDepthStencilFormat(VkFormat format)
{
    switch (format)
    {
    case VK_FORMAT_D16_UNORM:
    case VK_FORMAT_X8_D24_UNORM_PACK32:
    case VK_FORMAT_D32_SFLOAT:

    case VK_FORMAT_S8_UINT:

    case VK_FORMAT_D16_UNORM_S8_UINT:
    case VK_FORMAT_D24_UNORM_S8_UINT:
    case VK_FORMAT_D32_SFLOAT_S8_UINT:
        return true;

    default:
        return false;
    }
}

bool VulkanImage::isCompressedFormat(VkFormat format)
{
    switch (format)
    {
    case VK_FORMAT_BC1_RGB_UNORM_BLOCK:
    case VK_FORMAT_BC1_RGB_SRGB_BLOCK:
    case VK_FORMAT_BC1_RGBA_UNORM_BLOCK:
    case VK_FORMAT_BC1_RGBA_SRGB_BLOCK:
    case VK_FORMAT_BC2_UNORM_BLOCK:
    case VK_FORMAT_BC2_SRGB_BLOCK:
    case VK_FORMAT_BC3_UNORM_BLOCK:
    case VK_FORMAT_BC3_SRGB_BLOCK:
    case VK_FORMAT_BC4_UNORM_BLOCK:
    case VK_FORMAT_BC4_SNORM_BLOCK:
    case VK_FORMAT_BC5_UNORM_BLOCK:
    case VK_FORMAT_BC5_SNORM_BLOCK:
    case VK_FORMAT_BC6H_UFLOAT_BLOCK:
    case VK_FORMAT_BC6H_SFLOAT_BLOCK:
    case VK_FORMAT_BC7_UNORM_BLOCK:
    case VK_FORMAT_BC7_SRGB_BLOCK:

    case VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK:
    case VK_FORMAT_ETC2_R8G8B8_SRGB_BLOCK:
    case VK_FORMAT_ETC2_R8G8B8A1_UNORM_BLOCK:
    case VK_FORMAT_ETC2_R8G8B8A1_SRGB_BLOCK:
    case VK_FORMAT_ETC2_R8G8B8A8_UNORM_BLOCK:
    case VK_FORMAT_ETC2_R8G8B8A8_SRGB_BLOCK:
    case VK_FORMAT_EAC_R11_UNORM_BLOCK:
    case VK_FORMAT_EAC_R11_SNORM_BLOCK:
    case VK_FORMAT_EAC_R11G11_UNORM_BLOCK:
    case VK_FORMAT_EAC_R11G11_SNORM_BLOCK:

    case VK_FORMAT_ASTC_4x4_UNORM_BLOCK:
    case VK_FORMAT_ASTC_4x4_SRGB_BLOCK:
    case VK_FORMAT_ASTC_5x4_UNORM_BLOCK:
    case VK_FORMAT_ASTC_5x4_SRGB_BLOCK:
    case VK_FORMAT_ASTC_5x5_UNORM_BLOCK:
    case VK_FORMAT_ASTC_5x5_SRGB_BLOCK:
    case VK_FORMAT_ASTC_6x5_UNORM_BLOCK:
    case VK_FORMAT_ASTC_6x5_SRGB_BLOCK:
    case VK_FORMAT_ASTC_6x6_UNORM_BLOCK:
    case VK_FORMAT_ASTC_6x6_SRGB_BLOCK:
    case VK_FORMAT_ASTC_8x5_UNORM_BLOCK:
    case VK_FORMAT_ASTC_8x5_SRGB_BLOCK:
    case VK_FORMAT_ASTC_8x6_UNORM_BLOCK:
    case VK_FORMAT_ASTC_8x6_SRGB_BLOCK:
    case VK_FORMAT_ASTC_8x8_UNORM_BLOCK:
    case VK_FORMAT_ASTC_8x8_SRGB_BLOCK:
    case VK_FORMAT_ASTC_10x5_UNORM_BLOCK:
    case VK_FORMAT_ASTC_10x5_SRGB_BLOCK:
    case VK_FORMAT_ASTC_10x6_UNORM_BLOCK:
    case VK_FORMAT_ASTC_10x6_SRGB_BLOCK:
    case VK_FORMAT_ASTC_10x8_UNORM_BLOCK:
    case VK_FORMAT_ASTC_10x8_SRGB_BLOCK:
    case VK_FORMAT_ASTC_10x10_UNORM_BLOCK:
    case VK_FORMAT_ASTC_10x10_SRGB_BLOCK:
    case VK_FORMAT_ASTC_12x10_UNORM_BLOCK:
    case VK_FORMAT_ASTC_12x10_SRGB_BLOCK:
    case VK_FORMAT_ASTC_12x12_UNORM_BLOCK:
    case VK_FORMAT_ASTC_12x12_SRGB_BLOCK:
        return true;

    default:
        return false;
    }
}

bool VulkanImage::isASTCFormat(VkFormat format)
{
    switch (format)
    {
    case VK_FORMAT_ASTC_4x4_UNORM_BLOCK:
    case VK_FORMAT_ASTC_4x4_SRGB_BLOCK:
    case VK_FORMAT_ASTC_5x4_UNORM_BLOCK:
    case VK_FORMAT_ASTC_5x4_SRGB_BLOCK:
    case VK_FORMAT_ASTC_5x5_UNORM_BLOCK:
    case VK_FORMAT_ASTC_5x5_SRGB_BLOCK:
    case VK_FORMAT_ASTC_6x5_UNORM_BLOCK:
    case VK_FORMAT_ASTC_6x5_SRGB_BLOCK:
    case VK_FORMAT_ASTC_6x6_UNORM_BLOCK:
    case VK_FORMAT_ASTC_6x6_SRGB_BLOCK:
    case VK_FORMAT_ASTC_8x5_UNORM_BLOCK:
    case VK_FORMAT_ASTC_8x5_SRGB_BLOCK:
    case VK_FORMAT_ASTC_8x6_UNORM_BLOCK:
    case VK_FORMAT_ASTC_8x6_SRGB_BLOCK:
    case VK_FORMAT_ASTC_8x8_UNORM_BLOCK:
    case VK_FORMAT_ASTC_8x8_SRGB_BLOCK:
    case VK_FORMAT_ASTC_10x5_UNORM_BLOCK:
    case VK_FORMAT_ASTC_10x5_SRGB_BLOCK:
    case VK_FORMAT_ASTC_10x6_UNORM_BLOCK:
    case VK_FORMAT_ASTC_10x6_SRGB_BLOCK:
    case VK_FORMAT_ASTC_10x8_UNORM_BLOCK:
    case VK_FORMAT_ASTC_10x8_SRGB_BLOCK:
    case VK_FORMAT_ASTC_10x10_UNORM_BLOCK:
    case VK_FORMAT_ASTC_10x10_SRGB_BLOCK:
    case VK_FORMAT_ASTC_12x10_UNORM_BLOCK:
    case VK_FORMAT_ASTC_12x10_SRGB_BLOCK:
    case VK_FORMAT_ASTC_12x12_UNORM_BLOCK:
    case VK_FORMAT_ASTC_12x12_SRGB_BLOCK:
        return true;

    default:
        return false;
    }

}

bool VulkanImage::isSRGBFormat(VkFormat format)
{
    switch (format)
    {
    case VK_FORMAT_R8_SRGB:
    case VK_FORMAT_R8G8_SRGB:
    case VK_FORMAT_R8G8B8_SRGB:
    case VK_FORMAT_B8G8R8_SRGB:
    case VK_FORMAT_R8G8B8A8_SRGB:
    case VK_FORMAT_B8G8R8A8_SRGB:
    case VK_FORMAT_A8B8G8R8_SRGB_PACK32:
    case VK_FORMAT_BC1_RGB_SRGB_BLOCK:
    case VK_FORMAT_BC1_RGBA_SRGB_BLOCK:
    case VK_FORMAT_BC2_SRGB_BLOCK:
    case VK_FORMAT_BC3_SRGB_BLOCK:
    case VK_FORMAT_BC7_SRGB_BLOCK:
    case VK_FORMAT_ETC2_R8G8B8_SRGB_BLOCK:
    case VK_FORMAT_ETC2_R8G8B8A1_SRGB_BLOCK:
    case VK_FORMAT_ETC2_R8G8B8A8_SRGB_BLOCK:
    case VK_FORMAT_ASTC_4x4_SRGB_BLOCK:
    case VK_FORMAT_ASTC_5x4_SRGB_BLOCK:
    case VK_FORMAT_ASTC_5x5_SRGB_BLOCK:
    case VK_FORMAT_ASTC_6x5_SRGB_BLOCK:
    case VK_FORMAT_ASTC_6x6_SRGB_BLOCK:
    case VK_FORMAT_ASTC_8x5_SRGB_BLOCK:
    case VK_FORMAT_ASTC_8x6_SRGB_BLOCK:
    case VK_FORMAT_ASTC_8x8_SRGB_BLOCK:
    case VK_FORMAT_ASTC_10x5_SRGB_BLOCK:
    case VK_FORMAT_ASTC_10x6_SRGB_BLOCK:
    case VK_FORMAT_ASTC_10x8_SRGB_BLOCK:
    case VK_FORMAT_ASTC_10x10_SRGB_BLOCK:
    case VK_FORMAT_ASTC_12x10_SRGB_BLOCK:
    case VK_FORMAT_ASTC_12x12_SRGB_BLOCK:
        return true;

    default:
        return false;
    }

    return false;
}

VulkanSwapchain* VulkanImage::getSwapchainFromImage(const VulkanImage* image)
{
    ASSERT(image->hasUsageFlag(TextureUsage::TextureUsage_Backbuffer), "must be swapchain");
    return image->m_relatedSwapchain;
}

#if DEBUG_OBJECT_MEMORY
std::set<VulkanImage*> VulkanImage::s_objects;
#endif //DEBUG_OBJECT_MEMORY

VulkanImage::VulkanImage(VulkanDevice* device, VulkanMemory::VulkanMemoryAllocator* alloc, VkImageType type, VkFormat format, VkExtent3D dimension, u32 layers, u32 mipsLevel, VkImageTiling tiling, TextureUsageFlags usage, const std::string& name) noexcept
    : m_device(*device)
    , m_memoryAllocator(alloc)
    , m_type(type)
    , m_format(format)
    , m_dimension(dimension)
    , m_mipLevels(mipsLevel)
    , m_arrayLayers(layers)
    , m_samples(VK_SAMPLE_COUNT_1_BIT)
    , m_tiling(VK_IMAGE_TILING_OPTIMAL)
    , m_aspectMask(VulkanImage::getImageAspectFlags(format))
    , m_usage(usage)

    , m_image(VK_NULL_HANDLE)
    , m_resolveImage(nullptr)
    , m_memory(VulkanMemory::s_invalidMemory)
    , m_relatedSwapchain(nullptr)
{
#if VULKAN_DEBUG
    LOG_DEBUG("VulkanImage::VulkanImage constructor %llx", this);
#endif //VULKAN_DEBUG
#if VULKAN_DEBUG_MARKERS
    m_debugName = name.empty() ? "Image" : name;
    m_debugName.append(VulkanDebugUtils::k_addressPreffix);
    m_debugName.append(std::to_string(reinterpret_cast<const u64>(this)));
#endif //VULKAN_DEBUG_MARKERS

#if DEBUG_OBJECT_MEMORY
    s_objects.insert(this);
#endif //DEBUG_OBJECT_MEMORY

    m_globalLayout.resize(1 + static_cast<u64>(m_arrayLayers) * static_cast<u64>(m_mipLevels), (m_tiling == VK_IMAGE_TILING_OPTIMAL) ? VK_IMAGE_LAYOUT_UNDEFINED : VK_IMAGE_LAYOUT_PREINITIALIZED);
}

VulkanImage::VulkanImage(VulkanDevice* device, VulkanMemory::VulkanMemoryAllocator* alloc, VkFormat format, VkExtent3D dimension, VkSampleCountFlagBits samples, u32 layers, TextureUsageFlags usage, const std::string& name) noexcept
    : m_device(*device)
    , m_memoryAllocator(alloc)
    , m_type(VK_IMAGE_TYPE_2D)
    , m_format(format)
    , m_dimension(dimension)
    , m_mipLevels(1)
    , m_arrayLayers(layers)
    , m_samples(samples)
    , m_tiling(VK_IMAGE_TILING_OPTIMAL)
    , m_aspectMask(VulkanImage::getImageAspectFlags(format))
    , m_usage(usage)

    , m_image(VK_NULL_HANDLE)
    , m_resolveImage(nullptr)
    , m_memory(VulkanMemory::s_invalidMemory)
    , m_relatedSwapchain(nullptr)
{
#if VULKAN_DEBUG
    LOG_DEBUG("VulkanImage::VulkanImage constructor %llx", this);
#endif //VULKAN_DEBUG
    if (hasUsageFlag(TextureUsage::TextureUsage_Resolve))
    {
        m_usage = usage & ~TextureUsage::TextureUsage_GenerateMipmaps;
        if (VulkanImage::isColorFormat(m_format))
        {
            ASSERT(samples > VkSampleCountFlagBits::VK_SAMPLE_COUNT_1_BIT, "wrong sample count");
            m_resolveImage = V3D_NEW(VulkanImage, memory::MemoryLabel::MemoryRenderCore)(device, alloc, format, dimension, VkSampleCountFlagBits::VK_SAMPLE_COUNT_1_BIT, layers, usage & ~TextureUsage::TextureUsage_Resolve);
        }
        else
        {
            if (m_device.getVulkanDeviceCaps()._supportDepthAutoResolve)
            {
                ASSERT(samples > VkSampleCountFlagBits::VK_SAMPLE_COUNT_1_BIT, "wrong sample count");
                m_resolveImage = V3D_NEW(VulkanImage, memory::MemoryLabel::MemoryRenderCore)(device, alloc, format, dimension, VkSampleCountFlagBits::VK_SAMPLE_COUNT_1_BIT, layers, usage & ~TextureUsage::TextureUsage_Resolve);
            }
            else
            {
                ASSERT(false, "resolve for depth not supported");
            }
        }
    }

    if (hasUsageFlag(TextureUsage::TextureUsage_GenerateMipmaps))
    {
        ASSERT(hasUsageFlag(TextureUsage::TextureUsage_Attachment), "must be attachment");
        m_mipLevels = ImageFormat::calculateMipmapCount({ m_dimension.width, m_dimension.height, m_dimension.depth });
    }

#if VULKAN_DEBUG_MARKERS
    m_debugName = name.empty() ? "Image" : name;
    m_debugName.append(VulkanDebugUtils::k_addressPreffix);
    m_debugName.append(std::to_string(reinterpret_cast<const u64>(this)));
#endif //VULKAN_DEBUG_MARKERS

#if DEBUG_OBJECT_MEMORY
    s_objects.insert(this);
#endif //DEBUG_OBJECT_MEMORY

    m_globalLayout.resize(1 + static_cast<u64>(m_arrayLayers) * static_cast<u64>(m_mipLevels), VK_IMAGE_LAYOUT_UNDEFINED);
}

VulkanImage::~VulkanImage()
{
#if VULKAN_DEBUG
    LOG_DEBUG("VulkanImage::VulkanImage destructor %llx", this);
#endif //VULKAN_DEBUG
#if DEBUG_OBJECT_MEMORY
    s_objects.erase(this);
#endif //DEBUG_OBJECT_MEMORY

    ASSERT(!m_resolveImage, "m_resolveImage nullptr");
    ASSERT(m_imageViews.empty(), "m_imageViews is not empty");

    ASSERT(!m_image, "image not nullptr");
}

bool VulkanImage::create()
{
    ASSERT(!m_image, "image already created");

    const DeviceCaps::ImageFormatSupport& supportFormatInfo = m_device.getVulkanDeviceCaps().getImageFormatSupportInfo(VulkanImage::convertVkImageFormatToFormat(m_format),
        (m_tiling == VK_IMAGE_TILING_OPTIMAL) ? DeviceCaps::TilingType_Optimal : DeviceCaps::TilingType_Linear);

    bool unsupport = false;
    VkImageUsageFlags imageUsage = 0;
    VkImageCreateFlags imageFlags = 0;

    if (hasUsageFlag(TextureUsage::TextureUsage_Sampled))
    {
        unsupport = false;// !supportFormatInfo._supportSampled;
        if (!supportFormatInfo._supportSampled)
        {
            LOG_ERROR("VulkanImage::create, supportFormatInfo._supportSampled format %s is not supported", VulkanImage::imageFormatStringVK(m_format).c_str());
            ASSERT(supportFormatInfo._supportSampled, "format is not supported");
        }
        imageUsage |= VK_IMAGE_USAGE_SAMPLED_BIT;
    }

    if (hasUsageFlag(TextureUsage::TextureUsage_Attachment))
    {
        unsupport |= !supportFormatInfo._supportAttachment;
        if (!supportFormatInfo._supportAttachment)
        {
            LOG_ERROR("VulkanImage::create, supportFormatInfo._supportAttachment format %s is not supported", VulkanImage::imageFormatStringVK(m_format).c_str());
            ASSERT(supportFormatInfo._supportAttachment, "format is not supported");
        }

        if (VulkanImage::isColorFormat(m_format))
        {
            imageUsage |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        }
        else
        {
            ASSERT(VulkanImage::isDepthStencilFormat(m_format), "wrong format");
            imageUsage |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
        }

        if (m_usage == TextureUsage::TextureUsage_Attachment)
        {
            imageUsage |= VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT;
        }

        if (hasUsageFlag(TextureUsage::TextureUsage_GenerateMipmaps))
        {
            imageUsage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
        }
    }

    if (hasUsageFlag(TextureUsage::TextureUsage_Write))
    {
        imageUsage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    }

    if (hasUsageFlag(TextureUsage::TextureUsage_Read))
    {
        imageUsage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    }

    if (hasUsageFlag(TextureUsage::TextureUsage_Storage))
    {
        unsupport |= !supportFormatInfo._supportStorage;
        if (!supportFormatInfo._supportStorage)
        {
            LOG_ERROR("VulkanImage::create, supportFormatInfo._supportStorage format %s is not supported", VulkanImage::imageFormatStringVK(m_format).c_str());
            ASSERT(supportFormatInfo._supportStorage, "format is not supported");
        }
        imageUsage |= VK_IMAGE_USAGE_STORAGE_BIT;
    }

    if (m_arrayLayers == 6U)
    {
        imageFlags |= VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
    }

    if (m_format == VK_FORMAT_UNDEFINED)
    {
        imageFlags |= VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT;
    }
    //imageFlags |= VK_IMAGE_CREATE_PROTECTED_BIT;

    if (unsupport)
    {
        LOG_ERROR("VulkanImage::create, can't create image format unsupported");
        return false;
    }

    VkImageCreateInfo imageCreateInfo = {};
    imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageCreateInfo.pNext = nullptr;
    imageCreateInfo.flags = imageFlags;
    imageCreateInfo.imageType = m_type;
    imageCreateInfo.format = m_format;
    imageCreateInfo.extent = m_dimension;
    imageCreateInfo.mipLevels = m_mipLevels;
    imageCreateInfo.arrayLayers = m_arrayLayers;
    imageCreateInfo.samples = m_samples;
    imageCreateInfo.tiling = m_tiling;
    imageCreateInfo.usage = imageUsage;
    imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    imageCreateInfo.queueFamilyIndexCount = 0;
    imageCreateInfo.pQueueFamilyIndices = nullptr;
    imageCreateInfo.initialLayout = m_globalLayout.front();

#if VULKAN_DEBUG
    LOG_DEBUG("vkCreateImage: [Type %s][Size %u : %u : %u [%u]]; flags %u; usage %u; format %s", 
        VulkanImage::imageTypeStringVK(m_type).c_str(), m_dimension.width, m_dimension.height, m_dimension.depth, m_arrayLayers, imageFlags, imageUsage, VulkanImage::imageFormatStringVK(m_format).c_str());
#endif
    VkResult result = VulkanWrapper::CreateImage(m_device.getDeviceInfo()._device, &imageCreateInfo, VULKAN_ALLOCATOR, &m_image);
    if (result != VK_SUCCESS)
    {
        LOG_ERROR("VulkanFramebuffer::create vkCreateImage is failed. Error: %s", ErrorString(result).c_str());
        return false;
    }

#if VULKAN_DEBUG_MARKERS
    if (m_device.getVulkanDeviceCaps()._debugUtilsObjectNameEnabled)
    {
        VkDebugUtilsObjectNameInfoEXT debugUtilsObjectNameInfo = {};
        debugUtilsObjectNameInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
        debugUtilsObjectNameInfo.pNext = nullptr;
        debugUtilsObjectNameInfo.objectType = VK_OBJECT_TYPE_IMAGE;
        debugUtilsObjectNameInfo.objectHandle = reinterpret_cast<u64>(m_image);
        debugUtilsObjectNameInfo.pObjectName = m_debugName.c_str();

        VulkanWrapper::SetDebugUtilsObjectName(m_device.getDeviceInfo()._device, &debugUtilsObjectNameInfo);
    }
#endif //VULKAN_DEBUG_MARKERS

    VkMemoryPropertyFlags memoryFlags = 0;
    if (m_tiling == VK_IMAGE_TILING_OPTIMAL)
    {
        memoryFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
        if (imageUsage & VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT)
        {
            if (VulkanMemory::isSupportedMemoryType(m_device.getVulkanDeviceCaps().getDeviceMemoryProperties(), memoryFlags | VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT, true))
            {
                memoryFlags |= VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT;
            }
        }

        /*if (memoryFlags == VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
        {
            memoryFlags |= VK_MEMORY_PROPERTY_PROTECTED_BIT;
        }*/
    }
    else
    {
        //TODO
        ASSERT(false, "not impl");
    }

    m_memory = VulkanMemory::allocateImageMemory(*m_memoryAllocator, m_image, memoryFlags);
    if (m_memory == VulkanMemory::s_invalidMemory)
    {
        VulkanImage::destroy();

        LOG_ERROR("VulkanImage::create() is failed");
        return false;
    }
#if VULKAN_DEBUG
    m_memoryAllocator->linkVulkanObject(m_memory, this);
#endif //VULKAN_DEBUG

    if (!createViewImage())
    {
        VulkanImage::destroy();

        LOG_ERROR("VulkanImage::create() is failed");
        return false;
    }

    if (m_resolveImage)
    {
        if (!m_resolveImage->create())
        {
            VulkanImage::destroy();

            LOG_ERROR("VulkanImage::create() resolve is failed");
            return false;
        }
    }

    return true;
}

bool VulkanImage::create(VkImage image, VulkanSwapchain* swapchain)
{
    ASSERT(image, "image is nullptr");
    ASSERT(!m_image, "m_image already exist");
    m_image = image;
    m_relatedSwapchain = swapchain;
    std::fill(m_globalLayout.begin(), m_globalLayout.end(), VK_IMAGE_LAYOUT_UNDEFINED);
    ASSERT(hasUsageFlag(TextureUsage::TextureUsage_Backbuffer), "must be swapchain");

#if VULKAN_DEBUG_MARKERS
    if (m_device.getVulkanDeviceCaps()._debugUtilsObjectNameEnabled)
    {
        VkDebugUtilsObjectNameInfoEXT debugUtilsObjectNameInfo = {};
        debugUtilsObjectNameInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
        debugUtilsObjectNameInfo.pNext = nullptr;
        debugUtilsObjectNameInfo.objectType = VK_OBJECT_TYPE_IMAGE;
        debugUtilsObjectNameInfo.objectHandle = reinterpret_cast<u64>(m_image);
        debugUtilsObjectNameInfo.pObjectName = m_debugName.c_str();

        VulkanWrapper::SetDebugUtilsObjectName(m_device.getDeviceInfo()._device, &debugUtilsObjectNameInfo);
    }
#endif //VULKAN_DEBUG_MARKERS

    if (!createViewImage())
    {
        LOG_ERROR("VulkanImage::VulkanImage::create(img) is failed");
        return false;
    }

    return true;
}

void VulkanImage::clear(VulkanCommandBuffer* cmdBuffer, const color::Color& color)
{
#if VULKAN_DEBUG
    LOG_DEBUG("VulkanImage::clearColor [%f, %f, %f, %f]", color[0], color[1], color[2], color[3]);
#endif
    if (!m_image)
    {
        ASSERT(false, "nullptr");
        return;
    }

    VkImageLayout layout = cmdBuffer->getResourceStateTracker().getLayout(this, RenderTexture::Subresource());
    if (layout == VK_IMAGE_LAYOUT_UNDEFINED || layout == VK_IMAGE_LAYOUT_PREINITIALIZED)
    {
        if (VulkanImage::hasUsageFlag(TextureUsage::TextureUsage_Backbuffer))
        {
            layout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
        }
        else if (VulkanImage::hasUsageFlag(TextureUsage::TextureUsage_Attachment))
        {
            layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        }
        else if (VulkanImage::hasUsageFlag(TextureUsage::TextureUsage_Storage))
        {
            layout = VK_IMAGE_LAYOUT_GENERAL;
        }
        else
        {
            ASSERT(false, "unknown layout");
            layout = VK_IMAGE_LAYOUT_GENERAL;
        }
    }

    VkPipelineStageFlags stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    if (VulkanImage::hasUsageFlag(TextureUsage::TextureUsage_Backbuffer) || VulkanImage::hasUsageFlag(TextureUsage::TextureUsage_Attachment))
    {
        if (layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
        {
            stage |= VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        }
        else
        {
            stage |= VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        }
    }
    else if (VulkanImage::hasUsageFlag(TextureUsage::TextureUsage_Storage))
    {
        stage |= VK_PIPELINE_STAGE_VERTEX_SHADER_BIT | VK_PIPELINE_STAGE_GEOMETRY_SHADER_BIT | VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    }

    const VkClearColorValue* vkColor = reinterpret_cast<const VkClearColorValue*>(&color);
    ASSERT(cmdBuffer, "nullptr");
    cmdBuffer->cmdPipelineBarrier(this, stage, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    cmdBuffer->cmdClearImage(this, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, vkColor);
    cmdBuffer->cmdPipelineBarrier(this, VK_PIPELINE_STAGE_TRANSFER_BIT, stage, layout);
}

void VulkanImage::clear(VulkanCommandBuffer* cmdBuffer, f32 depth, u32 stencil)
{
#if VULKAN_DEBUG
    LOG_DEBUG("VulkanContext::clearDepthStencil [%f, %u]", depth, stencil);
#endif
    if (!m_image)
    {
        ASSERT(false, "nullptr");
        return;
    }

    VkImageLayout layout = cmdBuffer->getResourceStateTracker().getLayout(this, RenderTexture::Subresource());
    if (layout == VK_IMAGE_LAYOUT_UNDEFINED || layout == VK_IMAGE_LAYOUT_PREINITIALIZED)
    {
        layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    }

    const VkClearDepthStencilValue clearDepthStencilValue = { depth, stencil };

    cmdBuffer->cmdPipelineBarrier(this, VK_PIPELINE_STAGE_TRANSFER_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    cmdBuffer->cmdClearImage(this, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, &clearDepthStencilValue);
    cmdBuffer->cmdPipelineBarrier(this, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT, layout);
}

bool VulkanImage::upload(VulkanCommandBuffer* cmdBuffer, u32 size, const void* data)
{
    math::Dimension3D dim(m_dimension.width, m_dimension.height, m_dimension.depth);
    u64 calculatedSize = ImageFormat::calculateImageSize(dim, m_mipLevels, m_arrayLayers, VulkanImage::convertVkImageFormatToFormat(m_format));
    ASSERT(size == calculatedSize, "must be same");
    return VulkanImage::internalUpload(cmdBuffer, math::Dimension3D(0, 0, 0), dim, m_arrayLayers, m_mipLevels, calculatedSize, data);
}

bool VulkanImage::upload(VulkanCommandBuffer* cmdBuffer, const math::Dimension3D& size, u32 layers, u32 mips, const void* data)
{
    ASSERT(m_mipLevels == mips, "should be same");
    ASSERT(m_arrayLayers == layers, "should be same");
    ASSERT(m_samples == VkSampleCountFlagBits::VK_SAMPLE_COUNT_1_BIT, "wrong sample count");

    u64 calculatedSize = ImageFormat::calculateImageSize(size, mips, layers, VulkanImage::convertVkImageFormatToFormat(m_format));
    return VulkanImage::internalUpload(cmdBuffer, math::Dimension3D(0, 0, 0), size, layers, mips, calculatedSize, data);
}

bool VulkanImage::upload(VulkanCommandBuffer* cmdBuffer, const math::Dimension3D& offsets, const math::Dimension3D& size, u32 layers, const void* data)
{
    ASSERT(m_mipLevels == 1, "should be 1");
    ASSERT(m_arrayLayers == layers, "should be same");
    ASSERT(m_samples == VkSampleCountFlagBits::VK_SAMPLE_COUNT_1_BIT, "wrong sample count");

    ASSERT(size > offsets, "wrong offset");
    math::Dimension3D diffSize = (size - offsets);
    u64 calculatedSize = ImageFormat::calculateImageMipSize(diffSize, 0, VulkanImage::convertVkImageFormatToFormat(m_format)) * layers;
    ASSERT(calculatedSize > 0, "wrong size");

    return VulkanImage::internalUpload(cmdBuffer, offsets, size, layers, 1, calculatedSize, data);
}

bool VulkanImage::internalUpload(VulkanCommandBuffer* cmdBuffer, const math::Dimension3D& offsets, const math::Dimension3D& size, u32 layers, u32 mips, u64 dataSize, const void* data)
{
    if (!m_image)
    {
        ASSERT(false, "nullptr");
        return false;
    }

    if (size.getArea() == 0 || !data)
    {
        ASSERT(false, "wrong area");
        return false;
    }

    if (m_tiling == VK_IMAGE_TILING_OPTIMAL)
    {
        VulkanBuffer* stagingBuffer = m_device.getStaginBufferManager()->createStagingBuffer(dataSize);
        if (!stagingBuffer)
        {
            ASSERT(false, "staginBuffer is nullptr");
            return false;
        }
        void* stagingData = stagingBuffer->map();
        ASSERT(stagingData, "stagingData is nullptr");
        memcpy(stagingData, data, dataSize);
        stagingBuffer->unmap();

        ASSERT(!VulkanResource::isUsed(), "still submitted");
        m_device.getStaginBufferManager()->destroyAfterUse(stagingBuffer);

        auto calculateMipSize = [](const math::Dimension3D& size) -> math::Dimension3D
        {
            math::Dimension3D mipSize;
            mipSize._width = std::max(size._width / 2, 1U);
            mipSize._height = std::max(size._height / 2, 1U);
            mipSize._depth = std::max(size._depth / 2, 1U);

            return mipSize;
        };

        auto calculateMipOffset = [](const math::Dimension3D& size) -> math::Dimension3D
        {
            math::Dimension3D mipSize;
            mipSize._width = std::max(size._width / 2, 0U);
            mipSize._height = std::max(size._height / 2, 0U);
            mipSize._depth = std::max(size._depth / 2, 0U);

            return mipSize;
        };

        u64 bufferOffset = 0;
        u64 bufferDataSize = 0;
        std::vector<VkBufferImageCopy> bufferImageCopys;

        for (u32 layer = 0; layer < layers; ++layer)
        {
            math::Dimension3D mipSize = size;
            math::Dimension3D mipOffset = offsets;

            for (u32 mip = 0; mip < mips; ++mip)
            {
                bufferDataSize = ImageFormat::calculateImageMipSize(size, mip, VulkanImage::convertVkImageFormatToFormat(m_format));

                VkBufferImageCopy regions;
                regions.imageOffset = { static_cast<s32>(mipOffset._width), static_cast<s32>(mipOffset._height), static_cast<s32>(mipOffset._depth) };
                regions.imageExtent = { mipSize._width, mipSize._height, mipSize._depth };
                regions.imageSubresource.aspectMask = m_aspectMask;
                regions.imageSubresource.baseArrayLayer = layer;
                regions.imageSubresource.layerCount = 1;
                regions.imageSubresource.mipLevel = mip;
                regions.bufferRowLength = 0;
                regions.bufferImageHeight = 0;
                regions.bufferOffset = bufferOffset;
                bufferOffset += bufferDataSize;

                bufferImageCopys.push_back(regions);

                mipSize = calculateMipSize(mipSize);
                mipOffset = calculateMipOffset(mipOffset);
            }
        }

        VkImageLayout prevLayout = cmdBuffer->getResourceStateTracker().getLayout(this, RenderTexture::Subresource());
        ASSERT(m_usage & TextureUsage_Write, "should be write");
        VkPipelineStageFlags srcStageMask = 0;
        if (m_globalLayout.front() == VK_IMAGE_LAYOUT_UNDEFINED) //first time
        {
            srcStageMask = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        }
        else
        {
            srcStageMask = VK_PIPELINE_STAGE_TRANSFER_BIT;
            if (m_usage & TextureUsage_Sampled)
            {
                srcStageMask |= VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
            }
        }
        VkImageLayout newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        cmdBuffer->cmdPipelineBarrier(this, srcStageMask, VK_PIPELINE_STAGE_TRANSFER_BIT, newLayout);
       
        cmdBuffer->cmdCopyBufferToImage(stagingBuffer, this, newLayout, bufferImageCopys);

        VkPipelineStageFlags dstStageMask = VK_PIPELINE_STAGE_TRANSFER_BIT;
        if (prevLayout == VK_IMAGE_LAYOUT_UNDEFINED || prevLayout == VK_IMAGE_LAYOUT_PREINITIALIZED) //first time
        {
            if (m_usage & TextureUsage_Sampled)
            {
                newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            }
        }
        else
        {
            newLayout = prevLayout;
        }

        if (m_usage & TextureUsage_Sampled)
        {
            dstStageMask |= VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        }
        cmdBuffer->cmdPipelineBarrier(this, VK_PIPELINE_STAGE_TRANSFER_BIT, dstStageMask, newLayout);
    }
    else
    {
        ASSERT(false, "not impl");
    }

    return true;
}

bool VulkanImage::generateMipmaps(VulkanCommandBuffer* cmdBuffer, u32 layer)
{
    if (!m_image)
    {
        ASSERT(false, "nullptr");
        return false;
    }

    if (!VulkanImage::hasUsageFlag(TextureUsage::TextureUsage_GenerateMipmaps))
    {
        ASSERT(false, "must contains TextureUsage_GenerateMipmaps flag");
        return false;
    }

    ASSERT(m_mipLevels > 1, "image must be created with mipmaps");

    VkImageLayout layoutMips = cmdBuffer->getResourceStateTracker().getLayout(this, RenderTexture::makeSubresource(layer, 1, 1, m_mipLevels - 1));
    cmdBuffer->cmdPipelineBarrier(this, { layer, 1, 1, m_mipLevels - 1 }, VulkanTransitionState::selectStageFlagsByImageLayout(layoutMips), VK_PIPELINE_STAGE_TRANSFER_BIT, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

    VkImageLayout baseMip = cmdBuffer->getResourceStateTracker().getLayout(this, { layer, 1, 0, 1 });
    cmdBuffer->cmdPipelineBarrier(this, { layer, 1, 0, 1 }, VulkanTransitionState::selectStageFlagsByImageLayout(baseMip), VK_PIPELINE_STAGE_TRANSFER_BIT, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);

    for (u32 mip = 1; mip < m_mipLevels; ++mip)
    {
        VkImageBlit region = {};
        region.srcSubresource.aspectMask = VulkanImage::getImageAspectFlags(m_format);
        region.srcSubresource.mipLevel = mip - 1;
        region.srcSubresource.baseArrayLayer = 0;
        region.srcSubresource.layerCount = 1;
        region.dstSubresource = region.srcSubresource;
        region.dstSubresource.mipLevel = mip;
        region.srcOffsets[1] = { std::max(static_cast<s32>(m_dimension.width >> (mip - 1)), 1), std::max(static_cast<s32>(m_dimension.height >> (mip - 1)), 1), 1 };
        region.dstOffsets[1] = { std::max(static_cast<s32>(m_dimension.width >> mip), 1), std::max(static_cast<s32>(m_dimension.height >> mip), 1), 1 };

        cmdBuffer->cmdBlitImage(this, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, this, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, { region });
        cmdBuffer->cmdPipelineBarrier(this, { layer, 1, mip, 1 }, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
    }

    return true;
}

VkImageSubresourceRange VulkanImage::makeImageSubresourceRange(const VulkanImage* image, const RenderTexture::Subresource& resource)
{
    ASSERT(image, "nullptr");
    VkImageSubresourceRange imageSubresourceRange = {};
    imageSubresourceRange.aspectMask = image->m_aspectMask;
    imageSubresourceRange.baseArrayLayer = resource._baseLayer;
    imageSubresourceRange.layerCount = resource._layers;
    imageSubresourceRange.baseMipLevel = resource._baseMip;
    imageSubresourceRange.levelCount = resource._mips;

    return imageSubresourceRange;
}

RenderTexture::Subresource VulkanImage::makeVulkanImageSubresource(const VulkanImage* image, u32 layer, u32 mip)
{
    ASSERT(image, "nullptr");
    RenderTexture::Subresource resource = { layer, 1, mip, 1 };

    if (layer == k_generalLayer)
    {
        resource._baseLayer = 0;
        resource._layers = image->m_arrayLayers;
    }

    if (mip == k_allMipmapsLevels)
    {
        resource._baseMip = 0;
        resource._mips = image->m_mipLevels;
    }

    return resource;
}

VkImageSubresourceRange VulkanImage::makeImageSubresourceRangeWithAspect(const VulkanImage* image, const RenderTexture::Subresource& resource, VkImageAspectFlags aspect)
{
    VkImageSubresourceRange imageSubresourceRange = {};
    if (!aspect)
    {
        imageSubresourceRange.aspectMask = image->m_aspectMask;
    }
    else
    {
        ASSERT(image->m_aspectMask & aspect, "must be present");
        imageSubresourceRange.aspectMask = aspect;
    }

    imageSubresourceRange.baseArrayLayer = resource._baseLayer;
    imageSubresourceRange.layerCount = resource._layers;
    imageSubresourceRange.baseMipLevel = resource._baseMip;
    imageSubresourceRange.levelCount = resource._mips;

    return imageSubresourceRange;
}

VkImageView VulkanImage::getImageView(const RenderTexture::Subresource& resource, VkImageAspectFlags aspects) const
{
    VkImageSubresourceRange imageSubresourceRange = VulkanImage::makeImageSubresourceRangeWithAspect(this, resource, aspects);
    auto found = m_imageViews.find(DescInfo<VkImageSubresourceRange>(imageSubresourceRange));
    if (found == m_imageViews.cend())
    {
        ASSERT(false, "view isn't found");
    }

    return found->second;
}

VkImageLayout VulkanImage::getGlobalLayout(const RenderTexture::Subresource& resource) const
{
    u32 index = 1 + (resource._baseLayer * m_mipLevels + resource._baseMip);
    ASSERT(index < m_globalLayout.size(), "out of range");
    return m_globalLayout[index];
}

VkImageLayout VulkanImage::setGlobalLayout(VkImageLayout newlayout, const RenderTexture::Subresource& resource)
{
    VkImageLayout oldLayout = m_globalLayout.front();
    for (u32 layerIndex = 0; layerIndex < resource._layers; ++layerIndex)
    {
        for (u32 mipIndex = 0; mipIndex < resource._mips; ++mipIndex)
        {
            u32 index = 1 + ((resource._baseLayer + layerIndex) * m_mipLevels + (resource._baseMip + mipIndex));
            ASSERT(index < m_globalLayout.size(), "out of range");
            [[maybe_unused]] VkImageLayout layout = std::exchange(m_globalLayout[index], newlayout);
        }
    }

    m_globalLayout.front() = newlayout; //General layout. Need do for every layer and mip
    return oldLayout;
}

void VulkanImage::destroy()
{
    if (m_resolveImage)
    {
        m_resolveImage->destroy();

        V3D_DELETE(m_resolveImage, memory::MemoryLabel::MemoryRenderCore);
        m_resolveImage = nullptr;
    }

    for (auto& view : m_imageViews)
    {
        VulkanWrapper::DestroyImageView(m_device.getDeviceInfo()._device, view.second, VULKAN_ALLOCATOR);
    }
    m_imageViews.clear();

    if (hasUsageFlag(TextureUsage::TextureUsage_Backbuffer))
    {
        //not delete swapchain image
        m_image = VK_NULL_HANDLE;
    }
    else
    {
        if (m_image)
        {
            VulkanWrapper::DestroyImage(m_device.getDeviceInfo()._device, m_image, VULKAN_ALLOCATOR);
            m_image = VK_NULL_HANDLE;
        }

        if (m_memory != VulkanMemory::s_invalidMemory)
        {
#if VULKAN_DEBUG
            m_memoryAllocator->unlinkVulkanObject(m_memory, this);
#endif //VULKAN_DEBUG
            VulkanMemory::freeMemory(*m_memoryAllocator, m_memory);
        }
    }
}

bool VulkanImage::createViewImage()
{
    auto convertImageTypeToImageViewType = [](VkImageType type, bool cube = false, bool array = false) -> VkImageViewType 
    {
        switch (type)
        {
        case VK_IMAGE_TYPE_1D:
            if (array)
            {
                return VK_IMAGE_VIEW_TYPE_1D_ARRAY;
            }
            return VK_IMAGE_VIEW_TYPE_1D;

        case VK_IMAGE_TYPE_2D:
            if (cube)
            {
                 return VK_IMAGE_VIEW_TYPE_CUBE;
            }
            else if (array)
            {
                return VK_IMAGE_VIEW_TYPE_2D_ARRAY;
            }
            return VK_IMAGE_VIEW_TYPE_2D;

        case VK_IMAGE_TYPE_3D:
            return VK_IMAGE_VIEW_TYPE_3D;

        default:
            ASSERT(false, "not handle");
        }

        return VK_IMAGE_VIEW_TYPE_2D;
    };

    void* vkExtensions = nullptr;
#ifdef VK_EXT_astc_decode_mode
    VkImageViewASTCDecodeModeEXT imageViewASTCDecodeModeEXT = {};
    if (VulkanImage::isASTCFormat(m_format) && m_device.getVulkanDeviceCaps()._ASTC_TexturesDecompressed)
    {
        imageViewASTCDecodeModeEXT.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_ASTC_DECODE_MODE_EXT;
        imageViewASTCDecodeModeEXT.pNext = nullptr;
        imageViewASTCDecodeModeEXT.decodeMode = VK_FORMAT_R8G8B8A8_UNORM; //LDR only

        vkExtensions = &imageViewASTCDecodeModeEXT;
    }
#endif //VK_EXT_astc_decode_mode

    if (hasUsageFlag(TextureUsage::TextureUsage_Sampled))
    {
        if (VulkanImage::isColorFormat(m_format))
        {
            ASSERT(m_aspectMask == VK_IMAGE_ASPECT_COLOR_BIT, "must be color");
            VkImageViewCreateInfo imageViewCreateInfo = {};
            imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            imageViewCreateInfo.pNext = vkExtensions;
            imageViewCreateInfo.flags = 0;
            imageViewCreateInfo.image = m_image;
            imageViewCreateInfo.viewType = convertImageTypeToImageViewType(m_type, m_arrayLayers == 6U, m_arrayLayers > 1);
            imageViewCreateInfo.format = m_format;
            imageViewCreateInfo.components = { VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A }; //TODO get components from format
            imageViewCreateInfo.subresourceRange = VulkanImage::makeImageSubresourceRangeWithAspect(this, VulkanImage::makeVulkanImageSubresource(this), VK_IMAGE_ASPECT_COLOR_BIT);

            VkImageView view = VK_NULL_HANDLE;
            VkResult result = VulkanWrapper::CreateImageView(m_device.getDeviceInfo()._device, &imageViewCreateInfo, VULKAN_ALLOCATOR, &view);
            if (result != VK_SUCCESS)
            {
                LOG_ERROR("VulkanImage::createViewImage vkCreateImageView is failed. Error: %s", ErrorString(result).c_str());
                return false;
            }

            [[maybe_unused]] auto viewIter = m_imageViews.insert({ DescInfo<VkImageSubresourceRange>(imageViewCreateInfo.subresourceRange), view });
            ASSERT(viewIter.second, "already exsist");

#if VULKAN_DEBUG_MARKERS
            if (m_device.getVulkanDeviceCaps()._debugUtilsObjectNameEnabled)
            {
                std::string imageViewName = "ColorView_" + m_debugName;

                VkDebugUtilsObjectNameInfoEXT debugUtilsObjectNameInfo = {};
                debugUtilsObjectNameInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
                debugUtilsObjectNameInfo.pNext = nullptr;
                debugUtilsObjectNameInfo.objectType = VK_OBJECT_TYPE_IMAGE_VIEW;
                debugUtilsObjectNameInfo.objectHandle = reinterpret_cast<u64>(view);
                debugUtilsObjectNameInfo.pObjectName = imageViewName.c_str();

                VulkanWrapper::SetDebugUtilsObjectName(m_device.getDeviceInfo()._device, &debugUtilsObjectNameInfo);
            }
#endif //VULKAN_DEBUG_MARKERS
        }

        if (VulkanImage::isDepthStencilFormat(m_format)) //might be Depth only sampled
        {
            VkImageViewCreateInfo imageViewCreateInfo = {};
            imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            imageViewCreateInfo.pNext = vkExtensions;
            imageViewCreateInfo.flags = 0;
            imageViewCreateInfo.image = m_image;
            imageViewCreateInfo.viewType = convertImageTypeToImageViewType(m_type, m_arrayLayers == 6U, m_arrayLayers > 1);
            imageViewCreateInfo.format = m_format;
            imageViewCreateInfo.components = { VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY };
            imageViewCreateInfo.subresourceRange = VulkanImage::makeImageSubresourceRangeWithAspect(this, VulkanImage::makeVulkanImageSubresource(this), VK_IMAGE_ASPECT_DEPTH_BIT);

            VkImageView view = VK_NULL_HANDLE;
            VkResult result = VulkanWrapper::CreateImageView(m_device.getDeviceInfo()._device, &imageViewCreateInfo, VULKAN_ALLOCATOR, &view);
            if (result != VK_SUCCESS)
            {
                LOG_ERROR("VulkanImage::createViewImage vkCreateImageView is failed. Error: %s", ErrorString(result).c_str());
                return false;
            }

            [[maybe_unused]] auto viewIter = m_imageViews.insert({ DescInfo<VkImageSubresourceRange>(imageViewCreateInfo.subresourceRange), view });
            ASSERT(viewIter.second, "already exsist");

#if VULKAN_DEBUG_MARKERS
            if (m_device.getVulkanDeviceCaps()._debugUtilsObjectNameEnabled)
            {
                std::string imageViewName = "DepthView_" + m_debugName;

                VkDebugUtilsObjectNameInfoEXT debugUtilsObjectNameInfo = {};
                debugUtilsObjectNameInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
                debugUtilsObjectNameInfo.pNext = nullptr;
                debugUtilsObjectNameInfo.objectType = VK_OBJECT_TYPE_IMAGE_VIEW;
                debugUtilsObjectNameInfo.objectHandle = reinterpret_cast<u64>(view);
                debugUtilsObjectNameInfo.pObjectName = imageViewName.c_str();

                VulkanWrapper::SetDebugUtilsObjectName(m_device.getDeviceInfo()._device, &debugUtilsObjectNameInfo);
            }
#endif //VULKAN_DEBUG_MARKERS
        }
    }

    if (hasUsageFlag(TextureUsage::TextureUsage_Attachment))
    {
        for (u32 layer = 0; layer < m_arrayLayers; ++layer)
        {
            VkImageViewCreateInfo imageViewCreateInfo = {};
            imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            imageViewCreateInfo.pNext = vkExtensions;
            imageViewCreateInfo.flags = 0;
            imageViewCreateInfo.image = m_image;
            imageViewCreateInfo.viewType = convertImageTypeToImageViewType(m_type, false, m_arrayLayers > 1);
            imageViewCreateInfo.format = m_format;
            imageViewCreateInfo.components = { VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A };
            imageViewCreateInfo.subresourceRange = VulkanImage::makeImageSubresourceRange(this, VulkanImage::makeVulkanImageSubresource(this, static_cast<s32>(layer), 0));

            auto viewIter = m_imageViews.insert({ DescInfo<VkImageSubresourceRange>(imageViewCreateInfo.subresourceRange), VK_NULL_HANDLE });
            if (viewIter.second)
            {
                VkResult result = VulkanWrapper::CreateImageView(m_device.getDeviceInfo()._device, &imageViewCreateInfo, VULKAN_ALLOCATOR, &viewIter.first->second);
                if (result != VK_SUCCESS)
                {
                    LOG_ERROR("VulkanImage::createViewImage vkCreateImageView is failed. Error: %s", ErrorString(result).c_str());
                    return false;
                }
            }

            if (hasUsageFlag(TextureUsage::TextureUsage_GenerateMipmaps))
            {
                for (u32 mip = 0; mip < m_mipLevels; ++mip)
                {
                    VkImageViewCreateInfo imageViewCreateInfo = {};
                    imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
                    imageViewCreateInfo.pNext = vkExtensions;
                    imageViewCreateInfo.flags = 0;
                    imageViewCreateInfo.image = m_image;
                    imageViewCreateInfo.viewType = convertImageTypeToImageViewType(m_type, false, m_arrayLayers > 1);
                    imageViewCreateInfo.format = m_format;
                    imageViewCreateInfo.components = { VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A };
                    imageViewCreateInfo.subresourceRange = VulkanImage::makeImageSubresourceRange(this, VulkanImage::makeVulkanImageSubresource(this, static_cast<s32>(layer), static_cast<s32>(mip)));

                    auto viewIter = m_imageViews.insert({ DescInfo<VkImageSubresourceRange>(imageViewCreateInfo.subresourceRange), VK_NULL_HANDLE });
                    if (viewIter.second)
                    {
                        VkResult result = VulkanWrapper::CreateImageView(m_device.getDeviceInfo()._device, &imageViewCreateInfo, VULKAN_ALLOCATOR, &viewIter.first->second);
                        if (result != VK_SUCCESS)
                        {
                            LOG_ERROR("VulkanImage::createViewImage vkCreateImageView is failed. Error: %s", ErrorString(result).c_str());
                            return false;
                        }
                    }
                }
            }
        }
    }

    return true;
}

} //namespace vk
} //namespace renderer
} //namespace v3d
#endif //VULKAN_RENDER
