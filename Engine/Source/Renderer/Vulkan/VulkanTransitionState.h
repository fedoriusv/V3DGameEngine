#pragma once

#include "Renderer/Render.h"

#ifdef VULKAN_RENDER
#   include "VulkanFunctions.h"

namespace v3d
{
namespace renderer
{
namespace vk
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    class VulkanImage;
    class VulkanCommandBuffer;

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief VulkanTransitionState class. Render side
    * TODO move to cmd buffer
    */
    struct VulkanTransitionState
    {
        static VkImageLayout convertTransitionStateToImageLayout(TransitionOp state);

        static VkPipelineStageFlags selectStageFlagsByImageLayout(VkImageLayout layout);
        static std::tuple<VkAccessFlags, VkAccessFlags> getAccessFlagsFromImageLayout(VkImageLayout oldLayout, VkImageLayout newLayout);

        static void transitionImage(VulkanCommandBuffer* cmdBuffer, std::tuple<const VulkanImage*, RenderTexture::Subresource>& images, VkImageLayout layout, bool toCompute = false);
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace vk
} //namespace renderer
} //namespace v3d
#endif  //VULKAN_RENDER