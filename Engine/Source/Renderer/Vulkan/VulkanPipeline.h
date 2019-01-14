#pragma once

#include "Common.h"
#include "Renderer/Pipeline.h"

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
        static VkShaderStageFlagBits convertShaderTypeToVkStage(resource::ShaderType type);

    private:

        bool compileShader(const resource::ShaderHeader* header, const void* source, u32 size) override;

        bool createShaderModules(const ShaderProgram* program, std::vector<VkPipelineShaderStageCreateInfo>& outPipelineShaderStageCreateInfo);
        void deleteShaderModules();

        std::vector<VkShaderModule> m_modules;

        VkDevice     m_device;
        VkPipeline   m_pipeline;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace vk
} //namespace renderer
} //namespace v3d
#endif //VULKAN_RENDER
