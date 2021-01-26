#pragma once

#include "Common.h"
#include "Renderer/TextureProperties.h"

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

        VulkanTransitionState() noexcept = default;
        ~VulkanTransitionState() = default;

        void transitionImages(VulkanCommandBuffer* cmdBuffer, const std::vector<const Image*>& images, TransitionOp transition);
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace vk
} //namespace renderer
} //namespace v3d