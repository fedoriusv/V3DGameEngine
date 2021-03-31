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

        static VkPipelineStageFlags selectStageFlagsByImageLayout(VkImageLayout layout);

        VulkanTransitionState() noexcept = default;
        ~VulkanTransitionState() = default;

        void transitionImages(VulkanCommandBuffer* cmdBuffer, const std::vector<const Image*>& images, VkImageLayout layout);
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace vk
} //namespace renderer
} //namespace v3d
#endif  //VULKAN_RENDER