#include "VulkanImage.h"
#include "VulkanGraphicContext.h"
#include "VulkanCommandBufferManager.h"
#include "VulkanDebug.h"

#include "Utils/Logger.h"

#ifdef VULKAN_RENDER
namespace v3d
{
namespace renderer
{
namespace vk
{

std::tuple<VkAccessFlags, VkAccessFlags> VulkanImage::getAccessFlagsFromImageLayout(VkImageLayout oldLayout, VkImageLayout newLayout)
{
    VkAccessFlags srcFlag = 0;
    VkAccessFlags dstFlag = 0;

    switch (oldLayout)
    {
    case VK_IMAGE_LAYOUT_UNDEFINED:
        srcFlag = 0;
        break;

    case VK_IMAGE_LAYOUT_PREINITIALIZED:
        srcFlag = VK_ACCESS_HOST_WRITE_BIT;
        break;

    case VK_IMAGE_LAYOUT_GENERAL:
        srcFlag |= VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;
        srcFlag |= VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        srcFlag |= VK_ACCESS_TRANSFER_WRITE_BIT;
        srcFlag |= VK_ACCESS_TRANSFER_READ_BIT;
        srcFlag |= VK_ACCESS_SHADER_READ_BIT;
        srcFlag |= VK_ACCESS_SHADER_WRITE_BIT;
        break;

    case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
        srcFlag |= VK_ACCESS_TRANSFER_READ_BIT;
        srcFlag |= VK_ACCESS_TRANSFER_WRITE_BIT;
        break;

    case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
        srcFlag |= VK_ACCESS_TRANSFER_READ_BIT;
        srcFlag |= VK_ACCESS_TRANSFER_WRITE_BIT;
        break;

    case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
        srcFlag = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        break;

    case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
        srcFlag = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
        break;

    case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
        srcFlag = VK_ACCESS_SHADER_READ_BIT;
        break;

    case VK_IMAGE_LAYOUT_PRESENT_SRC_KHR:
        dstFlag = VK_ACCESS_MEMORY_READ_BIT;
        break;
    }

    switch (newLayout)
    {
    case VK_IMAGE_LAYOUT_GENERAL:
        dstFlag |= VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;
        dstFlag |= VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        dstFlag |= VK_ACCESS_TRANSFER_WRITE_BIT;
        dstFlag |= VK_ACCESS_TRANSFER_READ_BIT;
        dstFlag |= VK_ACCESS_SHADER_READ_BIT;
        dstFlag |= VK_ACCESS_SHADER_WRITE_BIT;
        break;

    case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
        dstFlag |= VK_ACCESS_TRANSFER_WRITE_BIT;
        dstFlag |= VK_ACCESS_TRANSFER_READ_BIT;
        break;

    case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
        dstFlag |= VK_ACCESS_TRANSFER_WRITE_BIT;
        dstFlag |= VK_ACCESS_TRANSFER_READ_BIT;
        break;

    case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
        dstFlag = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        break;

    case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
        dstFlag = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
        break;

    case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
        dstFlag = VK_ACCESS_SHADER_READ_BIT;
        break;

    case VK_IMAGE_LAYOUT_PRESENT_SRC_KHR:
        dstFlag = VK_ACCESS_MEMORY_READ_BIT;
        break;
    }

    return { srcFlag, dstFlag };
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

    /*case v3d::renderer::Format_R64_UInt:
         return VK_FORMAT_UNDEFINED;
    case v3d::renderer::Format_R64_SInt:
         return VK_FORMAT_UNDEFINED;
    case v3d::renderer::Format_R64_SFloat:
         return VK_FORMAT_UNDEFINED;
    case v3d::renderer::Format_R64G64_UInt:
         return VK_FORMAT_UNDEFINED;
    case v3d::renderer::Format_R64G64_SInt:
         return VK_FORMAT_UNDEFINED;
    case v3d::renderer::Format_R64G64_SFloat:
         return VK_FORMAT_UNDEFINED;
    case v3d::renderer::Format_R64G64B64_UInt:
         return VK_FORMAT_UNDEFINED;
    case v3d::renderer::Format_R64G64B64_SInt:
         return VK_FORMAT_UNDEFINED;
    case v3d::renderer::Format_R64G64B64_SFloat:
         return VK_FORMAT_UNDEFINED;
    case v3d::renderer::Format_R64G64B64A64_UInt:
         return VK_FORMAT_UNDEFINED;
    case v3d::renderer::Format_R64G64B64A64_SInt:
         return VK_FORMAT_UNDEFINED;
    case v3d::renderer::Format_R64G64B64A64_SFloat:
         return VK_FORMAT_UNDEFINED;
    case v3d::renderer::Format_B10G11R11_UFloat_Pack32:
         return VK_FORMAT_UNDEFINED;
    case v3d::renderer::Format_E5B9G9R9_UFloat_Pack32:
         return VK_FORMAT_UNDEFINED;*/

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

    /*case v3d::renderer::Format_BC1_RGB_UNorm_Block:
         return VK_FORMAT_UNDEFINED;
    case v3d::renderer::Format_BC1_RGB_SRGB_Block:
         return VK_FORMAT_UNDEFINED;
    case v3d::renderer::Format_BC1_RGBA_UNorm_Block:
         return VK_FORMAT_UNDEFINED;
    case v3d::renderer::Format_BC1_RGBA_SRGB_Block:
         return VK_FORMAT_UNDEFINED;
    case v3d::renderer::Format_BC2_UNorm_Block:
         return VK_FORMAT_UNDEFINED;
    case v3d::renderer::Format_BC2_SRGB_Block:
         return VK_FORMAT_UNDEFINED;
    case v3d::renderer::Format_BC3_UNorm_Block:
         return VK_FORMAT_UNDEFINED;
    case v3d::renderer::Format_BC3_SRGB_Block:
         return VK_FORMAT_UNDEFINED;
    case v3d::renderer::Format_BC4_UNorm_Block:
         return VK_FORMAT_UNDEFINED;
    case v3d::renderer::Format_BC4_SNorm_Block:
         return VK_FORMAT_UNDEFINED;
    case v3d::renderer::Format_BC5_UNorm_Block:
         return VK_FORMAT_UNDEFINED;
    case v3d::renderer::Format_BC5_SNorm_Block:
         return VK_FORMAT_UNDEFINED;
    case v3d::renderer::Format_BC6H_UFloat_Block:
         return VK_FORMAT_UNDEFINED;
    case v3d::renderer::Format_BC6H_SFloat_Block:
         return VK_FORMAT_UNDEFINED;
    case v3d::renderer::Format_BC7_UNorm_Block:
         return VK_FORMAT_UNDEFINED;
    case v3d::renderer::Format_BC7_SRGB_Block:
         return VK_FORMAT_UNDEFINED;
    case v3d::renderer::Format_ETC2_R8G8B8_UNorm_Block:
         return VK_FORMAT_UNDEFINED;
    case v3d::renderer::Format_ETC2_R8G8B8_SRGB_Block:
         return VK_FORMAT_UNDEFINED;
    case v3d::renderer::Format_ETC2_R8G8B8A1_UNorm_Block:
         return VK_FORMAT_UNDEFINED;
    case v3d::renderer::Format_ETC2_R8G8B8A1_SRGB_Block:
         return VK_FORMAT_UNDEFINED;
    case v3d::renderer::Format_ETC2_R8G8B8A8_UNorm_Block:
         return VK_FORMAT_UNDEFINED;
    case v3d::renderer::Format_ETC2_R8G8B8A8_SRGB_Block:
         return VK_FORMAT_UNDEFINED;
    case v3d::renderer::Format_EAC_R11_UNorm_Block:
         return VK_FORMAT_UNDEFINED;
    case v3d::renderer::Format_EAC_R11_SNorm_Block:
         return VK_FORMAT_UNDEFINED;
    case v3d::renderer::Format_EAC_R11G11_UNorm_Block:
         return VK_FORMAT_UNDEFINED;
    case v3d::renderer::Format_EAC_R11G11_SNorm_Block:
         return VK_FORMAT_UNDEFINED;
    case v3d::renderer::Format_ASTC_4x4_UNorm_Block:
         return VK_FORMAT_UNDEFINED;
    case v3d::renderer::Format_ASTC_4x4_SRGB_Block:
         return VK_FORMAT_UNDEFINED;
    case v3d::renderer::Format_ASTC_5x4_UNorm_Block:
         return VK_FORMAT_UNDEFINED;
    case v3d::renderer::Format_ASTC_5x4_SRGB_Block:
         return VK_FORMAT_UNDEFINED;
    case v3d::renderer::Format_ASTC_5x5_UNorm_Block:
         return VK_FORMAT_UNDEFINED;
    case v3d::renderer::Format_ASTC_5x5_SRGB_Block:
         return VK_FORMAT_UNDEFINED;
    case v3d::renderer::Format_ASTC_6x5_UNorm_Block:
         return VK_FORMAT_UNDEFINED;
    case v3d::renderer::Format_ASTC_6x5_SRGB_Block:
         return VK_FORMAT_UNDEFINED;
    case v3d::renderer::Format_ASTC_6x6_UNorm_Block:
         return VK_FORMAT_UNDEFINED;
    case v3d::renderer::Format_ASTC_6x6_SRGB_Block:
         return VK_FORMAT_UNDEFINED;
    case v3d::renderer::Format_ASTC_8x5_UNorm_Block:
         return VK_FORMAT_UNDEFINED;
    case v3d::renderer::Format_ASTC_8x5_SRGB_Block:
         return VK_FORMAT_UNDEFINED;
    case v3d::renderer::Format_ASTC_8x6_UNorm_Block:
         return VK_FORMAT_UNDEFINED;
    case v3d::renderer::Format_ASTC_8x6_SRGB_Block:
         return VK_FORMAT_UNDEFINED;
    case v3d::renderer::Format_ASTC_8x8_UNorm_Block:
         return VK_FORMAT_UNDEFINED;
    case v3d::renderer::Format_ASTC_8x8_SRGB_Block:
         return VK_FORMAT_UNDEFINED;
    case v3d::renderer::Format_ASTC_10x5_UNorm_Block:
         return VK_FORMAT_UNDEFINED;
    case v3d::renderer::Format_ASTC_10x5_SRGB_Block:
         return VK_FORMAT_UNDEFINED;
    case v3d::renderer::Format_ASTC_10x6_UNorm_Block:
         return VK_FORMAT_UNDEFINED;
    case v3d::renderer::Format_ASTC_10x6_SRGB_Block:
         return VK_FORMAT_UNDEFINED;
    case v3d::renderer::Format_ASTC_10x8_UNorm_Block:
         return VK_FORMAT_UNDEFINED;
    case v3d::renderer::Format_ASTC_10x8_SRGB_Block:
         return VK_FORMAT_UNDEFINED;
    case v3d::renderer::Format_ASTC_10x10_UNorm_Block:
         return VK_FORMAT_UNDEFINED;
    case v3d::renderer::Format_ASTC_10x10_SRGB_Block:
         return VK_FORMAT_UNDEFINED;
    case v3d::renderer::Format_ASTC_12x10_UNorm_Block:
         return VK_FORMAT_UNDEFINED;
    case v3d::renderer::Format_ASTC_12x10_SRGB_Block:
         return VK_FORMAT_UNDEFINED;
    case v3d::renderer::Format_ASTC_12x12_UNorm_Block:
         return VK_FORMAT_UNDEFINED;
    case v3d::renderer::Format_ASTC_12x12_SRGB_Block:
         return VK_FORMAT_UNDEFINED;*/

    default:
        ASSERT(false, "unknown");
    }

    return VK_FORMAT_UNDEFINED;
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

VulkanMemory::VulkanMemoryAllocator* VulkanImage::s_memoryAllocator = new SimpleVulkanMemoryAllocator();

VulkanImage::VulkanImage(VulkanMemory* memory, VkDevice device, VkImageType type, VkFormat format, VkExtent3D dimension, u32 mipsLevel, VkImageTiling tiling)
    : m_device(device)
    , m_type(type)
    , m_format(format)
    , m_dimension(dimension)
    , m_mipsLevel(mipsLevel)
    , m_layersLevel(1)

    , m_aspectMask(VulkanImage::getImageAspectFlags(format))

    , m_samples(VK_SAMPLE_COUNT_1_BIT)
    , m_tiling(VK_IMAGE_TILING_OPTIMAL)

    , m_image(VK_NULL_HANDLE)
    , m_imageView(VK_NULL_HANDLE)

    , m_layout((m_tiling == VK_IMAGE_TILING_OPTIMAL) ? VK_IMAGE_LAYOUT_UNDEFINED : VK_IMAGE_LAYOUT_PREINITIALIZED)
    , m_usage(VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT)

    , m_resolveImage(nullptr)

    , m_memory(VulkanMemory::s_invalidMemory)
    , m_memoryManager(memory)
{
    LOG_DEBUG("VulkanImage::VulkanImage constructor %llx", this);
}

VulkanImage::VulkanImage(VulkanMemory* memory, VkDevice device, VkFormat format, VkExtent3D dimension, VkSampleCountFlagBits samples)
    : m_device(device)
    , m_type(VK_IMAGE_TYPE_2D)
    , m_format(format)
    , m_dimension(dimension)
    , m_mipsLevel(1)
    , m_layersLevel(1)

    , m_aspectMask(VulkanImage::getImageAspectFlags(format))

    , m_samples(samples)
    , m_tiling(VK_IMAGE_TILING_OPTIMAL)

    , m_image(VK_NULL_HANDLE)
    , m_imageView(VK_NULL_HANDLE)

    , m_layout(VK_IMAGE_LAYOUT_UNDEFINED)
    , m_usage(VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_SAMPLED_BIT)

    , m_resolveImage(nullptr)

    , m_memory(VulkanMemory::s_invalidMemory)
    , m_memoryManager(memory)
{
    LOG_DEBUG("VulkanImage::VulkanImage constructor %llx", this);

    if (VulkanImage::isColorFormat(format))
    {
        m_usage |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    }
    else
    {
        m_usage |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    }
}

VulkanImage::~VulkanImage()
{
    LOG_DEBUG("VulkanImage::VulkanImage destructor %llx", this);

    ASSERT(!m_imageView, "m_imageView is not nullptr");
    ASSERT(!m_image, "image not nullptr");
}

bool VulkanImage::create()
{
    ASSERT(!m_image, "image already created");

    VkImageCreateInfo imageCreateInfo = {};
    imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageCreateInfo.pNext = nullptr;
    imageCreateInfo.flags = 0;
    if (m_layersLevel == 6U)
    {
        imageCreateInfo.flags |= VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
    }
    if (m_format == VK_FORMAT_UNDEFINED)
    {
        imageCreateInfo.flags |= VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT;
    }

    imageCreateInfo.imageType = m_type;
    imageCreateInfo.format = m_format;
    imageCreateInfo.extent = m_dimension;
    imageCreateInfo.mipLevels = m_mipsLevel;
    imageCreateInfo.arrayLayers = m_layersLevel;

    imageCreateInfo.samples = m_samples;
    imageCreateInfo.tiling = m_tiling;
    imageCreateInfo.usage = m_usage;

    imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    imageCreateInfo.queueFamilyIndexCount = 0;
    imageCreateInfo.pQueueFamilyIndices = nullptr;

    imageCreateInfo.initialLayout = m_layout;

    VkResult result = VulkanWrapper::CreateImage(m_device, &imageCreateInfo, VULKAN_ALLOCATOR, &m_image);
    if (result != VK_SUCCESS)
    {
        LOG_ERROR("VulkanFramebuffer::create vkCreateImage is failed. Error: %s", ErrorString(result).c_str());
        return false;
    }

    VkMemoryPropertyFlags flag = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    if (m_tiling == VK_IMAGE_TILING_LINEAR)
    {
        flag |= VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
        flag |= VulkanDeviceCaps::getInstance()->supportCoherentMemory ? VK_MEMORY_PROPERTY_HOST_COHERENT_BIT : VK_MEMORY_PROPERTY_HOST_CACHED_BIT;
    }
    m_memory = m_memoryManager->allocateImageMemory(*s_memoryAllocator, m_image, flag);
    if (m_memory == VulkanMemory::s_invalidMemory)
    {
        VulkanImage::destroy();

        LOG_ERROR("VulkanImage::VulkanImage::create() is failed");
        return false;
    }

    if (!createViewImage())
    {
        VulkanImage::destroy();

        LOG_ERROR("VulkanImage::VulkanImage::create() is failed");
        return false;
    }

    return true;
}

bool VulkanImage::create(VkImage image)
{
    ASSERT(image, "image is nullptr");
    ASSERT(!m_image, "m_image already exist");
    m_image = image;

    if (!createViewImage())
    {
        LOG_ERROR("VulkanImage::VulkanImage::create(img) is failed");
        return false;
    }

    return true;
}

void VulkanImage::clear(Context * context, const core::Vector4D & color)
{
    LOG_DEBUG("VulkanGraphicContext::clearColor [%f, %f, %f, %f]", color[0], color[1], color[2], color[3]);
    VkClearColorValue clearColorValue = { color[0], color[1], color[2], color[3] };

    VulkanGraphicContext* vulkanContext = static_cast<VulkanGraphicContext*>(context);
    VulkanCommandBuffer* commandBuffer = vulkanContext->getCurrentBuffer(CommandTargetType::CmdDrawBuffer);
    ASSERT(commandBuffer, "commandBuffer is nullptr");

    VkImageLayout layout = m_layout;
    vulkanContext->transferImageLayout(this,VK_PIPELINE_STAGE_TRANSFER_BIT | VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    commandBuffer->cmdClearImage(this, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, &clearColorValue);
    vulkanContext->transferImageLayout(this,VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT | VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, layout);
}

void VulkanImage::clear(Context * context, f32 depth, u32 stencil)
{
    LOG_DEBUG("VulkanGraphicContext::clearDepthStencil [%f, %u]", depth, stencil);
    VkClearDepthStencilValue clearDepthStencilValue = { depth, stencil };

    VulkanGraphicContext* vulkanContext = static_cast<VulkanGraphicContext*>(context);
    VulkanCommandBuffer* commandBuffer = vulkanContext->getCurrentBuffer(CommandTargetType::CmdDrawBuffer);
    ASSERT(commandBuffer, "commandBuffer is nullptr");

    VkImageLayout layout = m_layout;
    vulkanContext->transferImageLayout(this, VK_PIPELINE_STAGE_TRANSFER_BIT | VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    commandBuffer->cmdClearImage(this, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, &clearDepthStencilValue);
    vulkanContext->transferImageLayout(this, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT | VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, layout);
}

VkImageSubresourceRange VulkanImage::makeImageSubresourceRange(const VulkanImage * image)
{
    VkImageSubresourceRange imageSubresourceRange = {};
    imageSubresourceRange.aspectMask = image->m_aspectMask;
    imageSubresourceRange.baseMipLevel = 0;
    imageSubresourceRange.levelCount = image->m_mipsLevel;
    imageSubresourceRange.baseArrayLayer = 0;
    imageSubresourceRange.layerCount = image->m_layersLevel;

    return imageSubresourceRange;
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

VkImage VulkanImage::getHandle() const
{
    ASSERT(m_image, "nullptr");
    return m_image;
}

VkImageAspectFlags VulkanImage::getImageAspectFlags() const
{
    return m_aspectMask;
}

VkImageView VulkanImage::getImageView() const
{
    ASSERT(m_imageView, "nullptr");
    return m_imageView;
}

VkImageLayout VulkanImage::getLayout() const
{
    return m_layout;
}

void VulkanImage::setLayout(VkImageLayout layout)
{
    m_layout = layout;
}

void VulkanImage::destroy()
{
    if (m_imageView)
    {
        VulkanWrapper::DestroyImageView(m_device, m_imageView, VULKAN_ALLOCATOR);
        m_imageView = VK_NULL_HANDLE;
    }

    if (m_image)
    {
        VulkanWrapper::DestroyImage(m_device, m_image, VULKAN_ALLOCATOR);
        m_image = VK_NULL_HANDLE;
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
        }

        return VK_IMAGE_VIEW_TYPE_2D;
    };

    VkImageViewCreateInfo imageViewCreateInfo = {};
    imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    imageViewCreateInfo.pNext = nullptr;
    imageViewCreateInfo.flags = 0;
    imageViewCreateInfo.image = m_image;
    imageViewCreateInfo.viewType = convertImageTypeToImageViewType(m_type, false, m_layersLevel > 1);
    imageViewCreateInfo.format = m_format;
    imageViewCreateInfo.components = { VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A };
    imageViewCreateInfo.subresourceRange = VulkanImage::makeImageSubresourceRange(this);

    VkResult result = VulkanWrapper::CreateImageView(m_device, &imageViewCreateInfo, VULKAN_ALLOCATOR, &m_imageView);
    if (result != VK_SUCCESS)
    {
        LOG_ERROR("VulkanImage::createViewImage vkCreateImageView is failed. Error: %s", ErrorString(result).c_str());
        return false;
    }

    return true;
}

bool VulkanImage::createSampler()
{
    return false;
}

} //namespace vk
} //namespace renderer
} //namespace v3d
#endif //VULKAN_RENDER
