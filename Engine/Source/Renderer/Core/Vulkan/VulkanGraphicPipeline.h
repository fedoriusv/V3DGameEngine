#pragma once

#include "Common.h"
#include "Renderer/ObjectTracker.h"
#include "Renderer/BufferProperties.h"
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
    * @brief VulkanGraphicPipeline final class. Vulkan Render side
    */
    class VulkanGraphicPipeline final : public Pipeline, public VulkanResource
    {
    public:

        VulkanGraphicPipeline(VkDevice device, Context* context, RenderPassManager* renderpassManager, VulkanPipelineLayoutManager* pipelineLayoutManager, [[maybe_unused]] const std::string& name = "");
        ~VulkanGraphicPipeline();

        static VkPolygonMode convertPolygonModeToVk(PolygonMode mode);
        static VkCullModeFlags convertCullModeToVk(CullMode mode);
        static VkFrontFace convertFrontFaceToVk(FrontFace face);

        static VkPrimitiveTopology convertPrimitiveTopologyToVk(PrimitiveTopology primitiveTopology);
        static VkShaderStageFlagBits convertShaderTypeToVkStage(ShaderType type);

        static VkBlendFactor convertBlendFactorToVk(BlendFactor factor);
        static VkBlendOp convertBlendOperationToVk(BlendOperation blendOp);
        static VkLogicOp covertLogicOperationToVk(LogicalOperation logicalOp);
        static VkCompareOp convertCompareOperationToVk(CompareOperation compareOp);

        static VkVertexInputRate covertInputRateToVk(VertexInputAttribDescription::InputRate rate);

        VkPipeline getHandle() const;
        VkPipelineLayout getPipelineLayoutHandle() const;

        const VulkanPipelineLayout& getDescriptorSetLayouts() const;
        const VulkanPipelineLayoutDescription& getPipelineLayoutDescription() const;

    private:

        bool create(const PipelineGraphicInfo* pipelineInfo) override;
        bool create(const PipelineComputeInfo* pipelineInfo) override;

        void destroy() override;

        bool compileShaders(std::vector<std::tuple<const ShaderHeader*, const void*, u32>>& shaders) override;
        void deleteShaderModules();

        bool createCompatibilityRenderPass(const RenderPassDescription& renderpassDesc, RenderPass* &compatibilityRenderPass);

        bool pipelineStatistic() const;

        std::vector<VkShaderModule> m_modules;

        VkDevice     m_device;
        VkPipeline   m_pipeline;
        RenderPass*  m_compatibilityRenderPass;
        ObjectTracker<RenderPass> m_trackerRenderPass;

        VulkanPipelineLayoutDescription m_pipelineLayoutDescription;
        VulkanPipelineLayout m_pipelineLayout;

        Context* const m_context;
        RenderPassManager* const m_renderpassManager;
        VulkanPipelineLayoutManager* const m_pipelineLayoutManager;

#if VULKAN_DEBUG_MARKERS
        std::string m_debugName;
#endif //VULKAN_DEBUG_MARKERS
    };

    inline const VulkanPipelineLayout& VulkanGraphicPipeline::getDescriptorSetLayouts() const
    {
        return m_pipelineLayout;
    }

    inline const VulkanPipelineLayoutDescription& VulkanGraphicPipeline::getPipelineLayoutDescription() const
    {
        return m_pipelineLayoutDescription;
    }

    inline VkPipeline VulkanGraphicPipeline::getHandle() const
    {
        ASSERT(m_pipeline != VK_NULL_HANDLE, "nullptr");
        return m_pipeline;
    }

    inline VkPipelineLayout VulkanGraphicPipeline::getPipelineLayoutHandle() const
    {
        ASSERT(m_pipelineLayout._pipelineLayout != VK_NULL_HANDLE, "nullptr");
        return m_pipelineLayout._pipelineLayout;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace vk
} //namespace renderer
} //namespace v3d
#endif //VULKAN_RENDER
