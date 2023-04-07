#pragma once

#include "Common.h"
#include "Renderer/Core/Pipeline.h"

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

        explicit VulkanComputePipeline(VkDevice device, VulkanPipelineLayoutManager* pipelineLayoutManager, [[maybe_unused]] const std::string& name = "") noexcept;
        ~VulkanComputePipeline();

        VkPipeline getHandle() const;
        VkPipelineLayout getPipelineLayoutHandle() const;

        const VulkanPipelineLayout& getDescriptorSetLayouts() const;
        const VulkanPipelineLayoutDescription& getPipelineLayoutDescription() const;

    private:

        VulkanComputePipeline() = delete;
        VulkanComputePipeline(const VulkanComputePipeline&) = delete;

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

#if VULKAN_DEBUG_MARKERS
        std::string m_debugName;
#endif //VULKAN_DEBUG_MARKERS
    };

    inline const VulkanPipelineLayout& VulkanComputePipeline::getDescriptorSetLayouts() const
    {
        return m_pipelineLayout;
    }

    inline const VulkanPipelineLayoutDescription& VulkanComputePipeline::getPipelineLayoutDescription() const
    {
        return m_pipelineLayoutDescription;
    }

    inline VkPipeline VulkanComputePipeline::getHandle() const
    {
        ASSERT(m_pipeline != VK_NULL_HANDLE, "nullptr");
        return m_pipeline;
    }

    inline VkPipelineLayout VulkanComputePipeline::getPipelineLayoutHandle() const
    {
        ASSERT(m_pipelineLayout._pipelineLayout != VK_NULL_HANDLE, "nullptr");
        return m_pipelineLayout._pipelineLayout;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace vk
} //namespace renderer
} //namespace v3d
#endif //VULKAN_RENDER
