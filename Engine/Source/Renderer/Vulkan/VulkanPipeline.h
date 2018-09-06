#pragma once

#include "Common.h"

#ifdef VULKAN_RENDER
#include "VulkanWrapper.h"

namespace v3d
{
namespace renderer
{
namespace vk
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    class VulkanPipeline final //cloneble
    {
    public:

        VulkanPipeline() {};
        ~VulkanPipeline() {};
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace vk
} //namespace renderer
} //namespace v3d
#endif //VULKAN_RENDER
