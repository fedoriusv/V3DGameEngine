#pragma once

#include "Common.h"
#include "Renderer/TextureProperties.h"
#include "Renderer/Image.h"

#ifdef VULKAN_RENDER
#include "VulkanFunctions.h"

namespace v3d
{
namespace renderer
{
namespace vk
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    class VulkanCommandBuffer;

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief VulkanTransitionState class. Render side
    */
    class VulkanTransitionState final
    {
    public:

        static VkImageLayout convertTransitionStateToImageLayout(TransitionOp state);

        static VkPipelineStageFlags selectStageFlagsByImageLayout(VkImageLayout layout);
        static std::tuple<VkAccessFlags, VkAccessFlags> getAccessFlagsFromImageLayout(VkImageLayout oldLayout, VkImageLayout newLayout);

        VulkanTransitionState() noexcept = default;
        ~VulkanTransitionState() = default;

        void transitionImages(VulkanCommandBuffer* cmdBuffer, const std::vector<std::tuple<const Image*, Image::Subresource>>& images, VkImageLayout layout, bool toCompute = false);
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace vk
} //namespace renderer
} //namespace v3d
#endif  //VULKAN_RENDER