#pragma once

#include "Common.h"
#include "Renderer/TextureProperties.h"

#ifdef VULKAN_RENDER
#include "VulkanFunctions.h"

namespace v3d
{
namespace renderer
{
    class Image;

namespace vk
{
    class VulkanCommandBuffer;

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    class VulkanTransitionState final
    {
    public:

        static VkImageLayout convertTransitionStateToImageLayout(TransitionOp state);

        static VkPipelineStageFlags selectStageFlagsByImageLayout(VkImageLayout layout);
        static std::tuple<VkAccessFlags, VkAccessFlags> getAccessFlagsFromImageLayout(VkImageLayout oldLayout, VkImageLayout newLayout);

        VulkanTransitionState() noexcept = default;
        ~VulkanTransitionState() = default;

        void transitionImages(VulkanCommandBuffer* cmdBuffer, const std::vector<const Image*>& images, VkImageLayout layout, s32 layer = k_generalLayer, s32 mip = k_allMipmapsLevels, bool toCompute = false);
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace vk
} //namespace renderer
} //namespace v3d
#endif  //VULKAN_RENDER