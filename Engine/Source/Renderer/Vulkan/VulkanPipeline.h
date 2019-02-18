#pragma once

#include "Common.h"
#include "Renderer/Pipeline.h"
#include "Renderer/BufferProperties.h"

#ifdef VULKAN_RENDER
#include "VulkanWrapper.h"
#include "VulkanResource.h"
#include "VulkanDescriptorSet.h"

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
    * VulkanGraphicPipeline final class. Vulkan Render side
    */
    class VulkanGraphicPipeline final : public Pipeline, public VulkanResource
    {
    public:

        VulkanGraphicPipeline(VkDevice device, RenderPassManager* renderpassManager, VulkanDescriptorSetManager* descriptorSetManager);
        ~VulkanGraphicPipeline();

        bool create(const PipelineGraphicInfo* pipelineInfo) override;
        void destroy() override;

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

    private:

        bool compileShader(const ShaderHeader* header, const void* source, u32 size) override;

        bool createShaderModule(const Shader* shader, VkPipelineShaderStageCreateInfo& outPipelineShaderStageCreateInfo);
        void deleteShaderModules();

        bool createCompatibilityRenderPass(const RenderPass::RenderPassInfo& renderpassDesc, RenderPass* &compatibilityRenderPass);

        std::vector<VkShaderModule> m_modules;

        VkDevice     m_device;
        VkPipeline   m_pipeline;
        RenderPass*  m_compatibilityRenderPass;
        VulkanPipelineLayout m_pipelineLayout;

        RenderPassManager* const m_renderpassManager;
        VulkanDescriptorSetManager* const m_descriptorSetManager;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace vk
} //namespace renderer
} //namespace v3d
#endif //VULKAN_RENDER
