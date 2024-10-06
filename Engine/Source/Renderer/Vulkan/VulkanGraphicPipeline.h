#pragma once

#include "Renderer/Render.h"
#include "Renderer/RenderTargetState.h"
#include "Renderer/Pipeline.h"

#ifdef VULKAN_RENDER
#   include "VulkanWrapper.h"
#   include "VulkanResource.h"
#   include "VulkanPipelineLayout.h"
#   include "VulkanRenderpass.h"

namespace v3d
{
namespace renderer
{
namespace vk
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    class VulkanDevice;

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief VulkanGraphicPipeline final class. Vulkan Render side
    */
    class VulkanGraphicPipeline final : public RenderPipeline, public VulkanResource
    {
    public:

        VulkanGraphicPipeline(VulkanDevice* device, VulkanRenderpassManager* renderpassManager, VulkanPipelineLayoutManager* pipelineLayoutManager, const std::string& name = "");
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

        static VkVertexInputRate covertInputRateToVk(InputRate rate);

        VkPipeline getHandle() const;
        VkPipelineLayout getPipelineLayoutHandle() const;

        const VulkanPipelineLayout& getDescriptorSetLayouts() const;
        const VulkanPipelineLayoutDescription& getPipelineLayoutDescription() const;

    private:

        bool create(const PipelineGraphicInfo* pipelineInfo) override;
        bool create(const PipelineComputeInfo* pipelineInfo) override;
        void destroy() override;

        void deleteShaderModules();
        bool createCompatibilityRenderPass(const RenderPassDesc& renderpassDesc, VulkanRenderPass* &compatibilityRenderPass);

        bool pipelineStatistic() const;

        std::vector<VkShaderModule> m_modules;

        VulkanDevice&               m_device;
        VkPipeline                  m_pipeline;
        VulkanRenderPass*           m_compatibilityRenderPass;
        //ObjectTracker<RenderPass> m_trackerRenderPass;

        VulkanPipelineLayoutDescription m_pipelineLayoutDescription;
        VulkanPipelineLayout m_pipelineLayout;

        VulkanRenderpassManager* const m_renderpassManager;
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

    /**
    * @brief VulkanGraphicPipelineManager class
    */
    class VulkanGraphicPipelineManager final
    {
    public:

        struct VulkanPipelineDesc
        {
            VulkanPipelineDesc() noexcept;
            explicit VulkanPipelineDesc(const GraphicsPipelineStateDesc& desc) noexcept;

            GraphicsPipelineStateDesc _pipelineDesc;
            RenderPassDesc            _renderpassDesc;
            //ShaderProgramDescription         _programDesc;
        };

        explicit VulkanGraphicPipelineManager(VulkanDevice* device) noexcept;
        ~VulkanGraphicPipelineManager();

        [[nodiscard]] VulkanGraphicPipeline* acquireGraphicPipeline(const GraphicsPipelineStateDesc& description);

        bool removePipeline(VulkanGraphicPipeline* pipeline);
        void clear();

    private:

        VulkanGraphicPipelineManager(const VulkanGraphicPipelineManager&) = delete;
        VulkanGraphicPipelineManager& operator=(const VulkanGraphicPipelineManager&) = delete;

        VulkanDevice& m_device;
        std::unordered_map<DescInfo<VulkanPipelineDesc>, VulkanGraphicPipeline*, DescInfo<VulkanPipelineDesc>::Hash, DescInfo<VulkanPipelineDesc>::Compare> m_pipelineGraphicList;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace vk
} //namespace renderer
} //namespace v3d
#endif //VULKAN_RENDER
