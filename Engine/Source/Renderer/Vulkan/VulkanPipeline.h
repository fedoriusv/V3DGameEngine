#pragma once

#include "Common.h"
#include "Pipeline.h"

#ifdef VULKAN_RENDER
#include "VulkanWrapper.h"

namespace v3d
{
namespace renderer
{
namespace vk
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    class VulkanGraphicPipeline final : public Pipeline
    {
    public:

        VulkanGraphicPipeline(VkDevice device);
        ~VulkanGraphicPipeline();

        bool create(const PipelineGraphicInfo* pipelineInfo) override;
        void destroy() override;

        static VkPolygonMode convertPolygonModeToVk(PolygonMode mode);
        static VkCullModeFlags convertCullModeToVk(CullMode mode);
        static VkFrontFace convertFrontFaceToVk(FrontFace face);

        static VkPrimitiveTopology convertPrimitiveTopologyToVk(PrimitiveTopology primitiveTopology);

    private:

        VkDevice     m_device;
        VkPipeline   m_pipeline;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace vk
} //namespace renderer
} //namespace v3d
#endif //VULKAN_RENDER
