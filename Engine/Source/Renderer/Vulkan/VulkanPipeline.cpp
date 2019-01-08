#include "VulkanRenderpass.h"
#include "VulkanImage.h"
#include "VulkanDebug.h"

#include "Utils/Logger.h"
#include "VulkanPipeline.h"

#ifdef VULKAN_RENDER
namespace v3d
{
namespace renderer
{
namespace vk
{

VkPolygonMode VulkanGraphicPipeline::convertPolygonModeToVk(PolygonMode mode)
{
    switch (mode)
    {
    case PolygonMode::PolygonMode_Triangle:
        return VK_POLYGON_MODE_FILL;

    case PolygonMode::PolygonMode_Line:
        return VK_POLYGON_MODE_LINE;

    case PolygonMode::PolygonMode_Point:
        return VK_POLYGON_MODE_POINT;

    default:
        break;
    }

    ASSERT(false, "type not found");
    return VK_POLYGON_MODE_FILL;
}

VkCullModeFlags VulkanGraphicPipeline::convertCullModeToVk(CullMode mode)
{
    switch (mode)
    {
    case CullMode::CullMode_None:
        return VK_CULL_MODE_NONE;

    case CullMode::CullMode_Front:
        return VK_CULL_MODE_FRONT_BIT;

    case CullMode::CullMode_Back:
        return VK_CULL_MODE_BACK_BIT;

    case CullMode::CullMode_FrontAndBack:
        return VK_CULL_MODE_FRONT_AND_BACK;

    default:
        break;
    }

    ASSERT(false, "mode not found");
    return VK_CULL_MODE_BACK_BIT;
}

VkFrontFace VulkanGraphicPipeline::convertFrontFaceToVk(FrontFace face)
{
    switch (face)
    {
    case FrontFace::FrontFace_Clockwise:
        return VK_FRONT_FACE_CLOCKWISE;

    case FrontFace::FrontFace_CounterClockwise:
        return VK_FRONT_FACE_COUNTER_CLOCKWISE;

    default:
        break;
    }
    ASSERT(false, "mode not found");
    return VK_FRONT_FACE_COUNTER_CLOCKWISE;
}

VulkanGraphicPipeline::VulkanGraphicPipeline(VkDevice device)
    : Pipeline(PipelineType::PipelineType_Graphic)
    , m_device(device)
    , m_pipeline(VK_NULL_HANDLE)
{
    LOG_DEBUG("VulkanGraphicPipeline::VulkanGraphicPipeline constructor %llx", this);
}

VulkanGraphicPipeline::~VulkanGraphicPipeline()
{
    LOG_DEBUG("VulkanRenderPass::VulkanRenderPass destructor %llx", this);
    ASSERT(!m_pipeline, "not nullptr");
}

bool VulkanGraphicPipeline::create(const PipelineGraphicInfo* pipelineInfo)
{
    ASSERT(m_pipelineType == PipelineType::PipelineType_Graphic, "invalid type");

    ASSERT(pipelineInfo, "nullptr");
    const GraphicsPipelineState::GraphicsPipelineStateDesc& pipelineDesc = pipelineInfo->_pipelineDesc;

    VkPipelineCache pipelineCache = VK_NULL_HANDLE; //TODO

    VkGraphicsPipelineCreateInfo graphicsPipelineCreateInfo = {};
    graphicsPipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    graphicsPipelineCreateInfo.pNext = nullptr;
    graphicsPipelineCreateInfo.flags = 0; //add VK_EXT_discard_rectangles

    graphicsPipelineCreateInfo.basePipelineIndex = 0;
    graphicsPipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE;

    graphicsPipelineCreateInfo.layout = VK_NULL_HANDLE; //TODO

    graphicsPipelineCreateInfo.stageCount = 0;
    graphicsPipelineCreateInfo.pStages = nullptr;

    graphicsPipelineCreateInfo.subpass = 0; //TODO


    const GraphicsPipelineState::RasterizationState& rasterizationState = pipelineDesc._rasterizationState;
    VkPipelineRasterizationStateCreateInfo rasterizationStateCreateInfo = {};
    rasterizationStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizationStateCreateInfo.pNext = nullptr; //
    rasterizationStateCreateInfo.flags = 0;
    rasterizationStateCreateInfo.depthClampEnable = VK_FALSE;
    rasterizationStateCreateInfo.rasterizerDiscardEnable = VK_FALSE;
    rasterizationStateCreateInfo.polygonMode = VulkanGraphicPipeline::convertPolygonModeToVk(rasterizationState._polygonMode);
    rasterizationStateCreateInfo.cullMode = VulkanGraphicPipeline::convertCullModeToVk(rasterizationState._cullMode);
    rasterizationStateCreateInfo.frontFace = VulkanGraphicPipeline::convertFrontFaceToVk(rasterizationState._frontFace);
    rasterizationStateCreateInfo.depthBiasEnable = VK_FALSE;
    rasterizationStateCreateInfo.depthBiasConstantFactor = 0.0f;
    rasterizationStateCreateInfo.depthBiasClamp = 0.0f;
    rasterizationStateCreateInfo.depthBiasSlopeFactor = 0.0f;
    rasterizationStateCreateInfo.lineWidth = 1.0f;
    graphicsPipelineCreateInfo.pRasterizationState = &rasterizationStateCreateInfo;

    VkResult result = VulkanWrapper::CreateGraphicsPipelines(m_device, pipelineCache, 1, &graphicsPipelineCreateInfo, VULKAN_ALLOCATOR, &m_pipeline);
    if (result != VK_SUCCESS)
    {
        LOG_ERROR("VulkanGraphicPipeline::create vkCreateGraphicsPipelines is failed. Error: %s", ErrorString(result).c_str());
        return false;
    }

    return true;
}

void VulkanGraphicPipeline::destroy()
{
}

} //namespace vk
} //namespace renderer
} //namespace v3d
#endif //VULKAN_RENDER
