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

void VulkanTransitionState::transitionImages(VulkanCommandBuffer* cmdBuffer, const std::vector<const Image*>& images, TransitionOp transition)
{
    //TODO
    for (auto image : images)
    {
        const VulkanImage* vulkanImage = static_cast<const VulkanImage*>(image);

        VkPipelineStageFlags srcStage = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
        VkPipelineStageFlags dstStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        VkImageLayout newLayout = VulkanRenderPass::convertTransitionStateToImageLayout(transition);
        VkImageLayout oldLayout = vulkanImage->getLayout();

        //to shader read
        if (newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
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
        if (newLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
        {
            ASSERT(VulkanImage::isColorFormat(vulkanImage->getFormat()), "wrong layout");
            srcStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
            dstStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        }

        //to preset form attachment
        if (oldLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_PRESENT_SRC_KHR)
        {
            ASSERT(vulkanImage->isSwapchain(), "mast be only swapchain");
            srcStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
            dstStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        }

        //form present to attachment
        if (oldLayout == VK_IMAGE_LAYOUT_PRESENT_SRC_KHR && newLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
        {
            ASSERT(vulkanImage->isSwapchain(), "mast be only swapchain");
            srcStage = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
            dstStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        }

        cmdBuffer->cmdPipelineBarrier(vulkanImage, srcStage, dstStage, newLayout);
    }
}

} //namespace vk
} //namespace renderer
} //namespace v3d

#endif //VULKAN_RENDER