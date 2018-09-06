#pragma once

#include "Common.h"
#include "Utils/NonCopyable.h"

#ifdef VULKAN_RENDER
#include "VulkanWrapper.h"

namespace v3d
{
namespace renderer
{
namespace vk
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    class VulkanPipelineState final : public utils::NonCopyable
    {
    public:

        VulkanPipelineState() {};
        ~VulkanPipelineState() {};

    private:

        class VulkanPipeline* m_pipeline;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace vk
} //namespace renderer
} //namespace v3d
#endif //VULKAN_RENDER
