#pragma once

#include "Common.h"
#include "Renderer/ObjectTracker.h"
#include "Renderer/Pipeline.h"

#ifdef VULKAN_RENDER
#include "VulkanWrapper.h"
#include "VulkanResource.h"
#include "VulkanPipelineLayout.h"
#include "VulkanRenderpass.h"

namespace v3d
{
namespace resource
{
    class Shader;
} //namespace resource
namespace renderer
{
    class RenderPassManager;

namespace vk
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief VulkanComputePipeline final class. Vulkan Render side
    */
    class VulkanComputePipeline final : public Pipeline, public VulkanResource
    {
    public:

        VulkanComputePipeline() = delete;
        VulkanComputePipeline(const VulkanComputePipeline&) = delete;

        VulkanComputePipeline(VkDevice device, VulkanPipelineLayoutManager* pipelineLayoutManager);
        ~VulkanComputePipeline();

        VkPipeline getHandle() const;
        VkPipelineLayout getPipelineLayoutHandle() const;

        const VulkanPipelineLayout& getDescriptorSetLayouts() const;
        const VulkanPipelineLayoutDescription& getPipelineLayoutDescription() const;

    private:

        bool create(const PipelineGraphicInfo* pipelineInfo) override;
        bool create(const PipelineComputeInfo* pipelineInfo) override;

        void destroy() override;

        bool compileShader(const ShaderHeader* header, const void* source, u32 size) override;

        VkDevice m_device;

        VkPipeline m_pipeline;
        VkShaderModule m_module;

        VulkanPipelineLayoutDescription m_pipelineLayoutDescription;
        VulkanPipelineLayout m_pipelineLayout;

        VulkanPipelineLayoutManager* const m_pipelineLayoutManager;

        bool pipelineStatistic() const;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace vk
} //namespace renderer
} //namespace v3d
#endif //VULKAN_RENDER
