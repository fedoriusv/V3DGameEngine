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
    class ShaderProgram;

namespace vk
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    class VulkanDevice;
    class VulkanCommandBuffer;

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief VulkanGraphicPipeline final class. Vulkan Render side
    */
    class VulkanGraphicPipeline final : public RenderPipeline, public VulkanResource
    {
    public:

        VulkanGraphicPipeline(VulkanDevice* device, const std::string& name = "");
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
        static VkStencilOp convertStencilOperationToVk(StencilOperation stencilOp);

        static VkVertexInputRate covertInputRateToVk(InputRate rate);

        VkPipeline getHandle() const;
        VkPipelineLayout getPipelineLayoutHandle() const;

        const VulkanPipelineLayout& getDescriptorSetLayouts() const;
        const VulkanPipelineLayoutDescription& getPipelineLayoutDescription() const;

        bool create(const GraphicsPipelineState& state);
        void destroy();

    private:

        bool createShaderModules(const renderer::ShaderProgram* program);
        void deleteShaderModules();
        bool createCompatibilityRenderPass(const RenderPassDesc& renderpassDesc, VulkanRenderPass* &compatibilityRenderPass);

        bool pipelineStatistic() const;


        VulkanDevice&                                               m_device;

        VkPipeline                                                  m_pipeline;
        VulkanRenderPass*                                           m_compatibilityRenderPass;
        ObjectTracker<RenderPass>                                   m_trackerRenderPass;
        std::array<VkShaderModule, toEnumType(ShaderType::Count)>   m_modules;

        VulkanPipelineLayoutDescription                             m_pipelineLayoutDescription;
        VulkanPipelineLayout                                        m_pipelineLayout;


#if VULKAN_DEBUG_MARKERS
        std::string m_debugName;
#endif //VULKAN_DEBUG_MARKERS

        friend VulkanCommandBuffer;
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
    * @brief VulkanGraphicPipelineManager class.
    * Multithreaded
    */
    class VulkanGraphicPipelineManager final
    {
    public:

        struct VulkanPipelineDesc
        {
            VulkanPipelineDesc() noexcept = default;

            explicit VulkanPipelineDesc(const GraphicsPipelineStateDesc& state, const RenderPassDesc& pass, const ShaderProgram* program) noexcept
                : _pipelineDesc(state)
                , _renderpassDesc(pass)
                , _program(program)
            {
            }

            bool operator==(const VulkanPipelineDesc& other) const
            {
                if (this != &other)
                {
                    if (_program != other._program)
                    {
                        return false;
                    }

                    if (_pipelineDesc != other._pipelineDesc)
                    {
                        return false;
                    }

                    if (_renderpassDesc != other._renderpassDesc)
                    {
                        return false;
                    }
                }

                return true;
            }

            GraphicsPipelineStateDesc _pipelineDesc;
            RenderPassDesc            _renderpassDesc;
            const ShaderProgram*      _program = nullptr;
        };

        explicit VulkanGraphicPipelineManager(VulkanDevice* device) noexcept;
        ~VulkanGraphicPipelineManager();

        [[nodiscard]] VulkanGraphicPipeline* acquireGraphicPipeline(const GraphicsPipelineState& state);

        bool removePipeline(VulkanGraphicPipeline* pipeline);
        void clear();

    private:

        VulkanGraphicPipelineManager(const VulkanGraphicPipelineManager&) = delete;
        VulkanGraphicPipelineManager& operator=(const VulkanGraphicPipelineManager&) = delete;

        VulkanDevice&    m_device;
        thread::Spinlock m_mutex;
        std::unordered_map<DescInfo<VulkanPipelineDesc>, VulkanGraphicPipeline*, DescInfo<VulkanPipelineDesc>::Hash, DescInfo<VulkanPipelineDesc>::Compare> m_pipelineGraphicList;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace vk
} //namespace renderer
} //namespace v3d
#endif //VULKAN_RENDER
