#include "VulkanTransitionState.h"

#ifdef VULKAN_RENDER
#include "VulkanImage.h"
#include "VulkanCommandBuffer.h"
#include "VulkanRenderpass.h"

namespace v3d
{
namespace renderer
{
namespace vk
{

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
        return VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
    }
}

void VulkanTransitionState::transitionImages(VulkanCommandBuffer* cmdBuffer, const std::vector<const Image*>& images, VkImageLayout layout)
{
    //TODO
    for (auto image : images)
    {
        const VulkanImage* vulkanImage = static_cast<const VulkanImage*>(image);

        VkPipelineStageFlags srcStage = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
        VkPipelineStageFlags dstStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        VkImageLayout oldLayout = vulkanImage->getLayout();

        //to shader read
        if (layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
        {
            dstStage = VK_PIPELINE_STAGE_VERTEX_SHADER_BIT | VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
            if (oldLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
            {
                if (VulkanImage::isColorFormat(vulkanImage->getFormat()))
                {
                    srcStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
                    dstStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
                }
                else
                {
                    srcStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
                    dstStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
                }
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
        if (oldLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL && layout == VK_IMAGE_LAYOUT_PRESENT_SRC_KHR)
        {
            ASSERT(vulkanImage->isSwapchain(), "mast be only swapchain");
            srcStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
            dstStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        }

        //form present to attachment
        if (oldLayout == VK_IMAGE_LAYOUT_PRESENT_SRC_KHR && layout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
        {
            ASSERT(vulkanImage->isSwapchain(), "mast be only swapchain");
            srcStage = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
            dstStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        }

        cmdBuffer->cmdPipelineBarrier(vulkanImage, srcStage, dstStage, layout);
    }
}

} //namespace vk
} //namespace renderer
} //namespace v3d

#endif //VULKAN_RENDER