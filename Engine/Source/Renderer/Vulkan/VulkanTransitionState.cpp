#include "VulkanTransitionState.h"

#ifdef VULKAN_RENDER
#include "VulkanImage.h"
#include "VulkanCommandBuffer.h"
#include "VulkanRenderpass.h"

#define GENERAL_LAYER_FOR_SHADER_ONLY 0

namespace v3d
{
namespace renderer
{
namespace vk
{

VkImageLayout VulkanTransitionState::convertTransitionStateToImageLayout(TransitionOp state)
{
    switch (state)
    {
    case TransitionOp::TransitionOp_Undefined:
        return VK_IMAGE_LAYOUT_UNDEFINED;

    case TransitionOp::TransitionOp_ShaderRead:
        return VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    case TransitionOp::TransitionOp_ColorAttachment:
        return VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    case TransitionOp::TransitionOp_DepthStencilAttachment:
        return VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    case TransitionOp::TransitionOp_GeneralGraphic:
    case TransitionOp::TransitionOp_GeneralCompute:
        return VK_IMAGE_LAYOUT_GENERAL;

    case TransitionOp::TransitionOp_Present:
        return VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    default:
        ASSERT(false, "unknown");
    }

    return VK_IMAGE_LAYOUT_UNDEFINED;
}

VkPipelineStageFlags VulkanTransitionState::selectStageFlagsByImageLayout(VkImageLayout layout)
{
    switch (layout)
    {
    case VK_IMAGE_LAYOUT_UNDEFINED:
    case VK_IMAGE_LAYOUT_PREINITIALIZED:
        return VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;

    case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
        return VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;

    case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
        return VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

    case VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL:
    case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
        return  VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;

    case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
    case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
        return VK_PIPELINE_STAGE_TRANSFER_BIT;

    default:
        ASSERT(false, "check");
        return VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
    }
}

std::tuple<VkAccessFlags, VkAccessFlags> VulkanTransitionState::getAccessFlagsFromImageLayout(VkImageLayout oldLayout, VkImageLayout newLayout)
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
#if GENERAL_LAYER_FOR_SHADER_ONLY
        srcFlag |= VK_ACCESS_SHADER_READ_BIT;
        srcFlag |= VK_ACCESS_SHADER_WRITE_BIT;
#else
        srcFlag |= VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;
        srcFlag |= VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        srcFlag |= VK_ACCESS_TRANSFER_WRITE_BIT;
        srcFlag |= VK_ACCESS_TRANSFER_READ_BIT;
        srcFlag |= VK_ACCESS_SHADER_READ_BIT;
        srcFlag |= VK_ACCESS_SHADER_WRITE_BIT;
#endif
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

    default:
        ASSERT(false, "not handled");

    }

    switch (newLayout)
    {
    case VK_IMAGE_LAYOUT_GENERAL:
#if GENERAL_LAYER_FOR_SHADER_ONLY
        dstFlag |= VK_ACCESS_SHADER_READ_BIT;
        dstFlag |= VK_ACCESS_SHADER_WRITE_BIT;
#else
        dstFlag |= VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;
        dstFlag |= VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        dstFlag |= VK_ACCESS_TRANSFER_WRITE_BIT;
        dstFlag |= VK_ACCESS_TRANSFER_READ_BIT;
        dstFlag |= VK_ACCESS_SHADER_READ_BIT;
        dstFlag |= VK_ACCESS_SHADER_WRITE_BIT;
#endif
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

    case VK_IMAGE_LAYOUT_UNDEFINED:
        dstFlag = 0;
        break;

    default:
        ASSERT(false, "not handled");
    }

    return { srcFlag, dstFlag };
}

void VulkanTransitionState::transitionImage(VulkanCommandBuffer* cmdBuffer, std::tuple<const VulkanImage*, RenderTexture::Subresource>& image, VkImageLayout layout, bool toCompute)
{
    const VulkanImage* vulkanImage = static_cast<const VulkanImage*>(std::get<0>(image));

    VkPipelineStageFlags srcStage = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
    VkPipelineStageFlags dstStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    VkImageLayout oldLayout = vulkanImage->getLayout(std::get<1>(image));

    //to general
    if (layout == VK_IMAGE_LAYOUT_GENERAL)
    {
        dstStage = toCompute ? VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT : VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT;
        if (oldLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
        {
            srcStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        }
        else if (oldLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
        {
            srcStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
        }
        else if (oldLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
        {
            srcStage = VK_PIPELINE_STAGE_VERTEX_SHADER_BIT | VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        }
    }

    //to shader read
    if (layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
    {
        dstStage = VK_PIPELINE_STAGE_VERTEX_SHADER_BIT | VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        if (oldLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
        {
            srcStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
            dstStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        }
        else if (oldLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
        {
            srcStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
            dstStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        }
        else if (oldLayout == VK_IMAGE_LAYOUT_GENERAL)
        {
            srcStage = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT; //TODO
            dstStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        }
    }

    //to color attachment
    if (layout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
    {
        ASSERT(VulkanImage::isColorFormat(vulkanImage->getFormat()), "wrong layout");
        srcStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        dstStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    }

    //to preset form attachment
    if (layout == VK_IMAGE_LAYOUT_PRESENT_SRC_KHR && oldLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
    {
        ASSERT(vulkanImage->hasUsageFlag(TextureUsage::TextureUsage_Backbuffer), "mast be only swapchain");
        srcStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dstStage = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
    }

    //form present to attachment
    if (layout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL && oldLayout == VK_IMAGE_LAYOUT_PRESENT_SRC_KHR)
    {
        ASSERT(vulkanImage->hasUsageFlag(TextureUsage::TextureUsage_Backbuffer), "mast be only swapchain");
        srcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        dstStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    }

    cmdBuffer->cmdPipelineBarrier(vulkanImage, srcStage, dstStage, layout, std::get<1>(image));
}

} //namespace vk
} //namespace renderer
} //namespace v3d

#endif //VULKAN_RENDER