#include "VulkanRenderpass.h"
#include "VulkanImage.h"
#include "VulkanDebug.h"

#include "Utils/Logger.h"
#include "VulkanPipeline.h"

#include "Resource/Shader.h"

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
    return VK_FRONT_FACE_CLOCKWISE;
}

VkPrimitiveTopology VulkanGraphicPipeline::convertPrimitiveTopologyToVk(PrimitiveTopology primitiveTopology)
{
    switch (primitiveTopology)
    {
    case PrimitiveTopology::PrimitiveTopology_PointList:
        return VK_PRIMITIVE_TOPOLOGY_POINT_LIST;

    case PrimitiveTopology::PrimitiveTopology_LineList:
        return VK_PRIMITIVE_TOPOLOGY_LINE_LIST;

    case PrimitiveTopology::PrimitiveTopology_LineStrip:
        return VK_PRIMITIVE_TOPOLOGY_LINE_STRIP;

    case PrimitiveTopology::PrimitiveTopology_TriangleList:
        return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

    case PrimitiveTopology::PrimitiveTopology_TriangleStrip:
        return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;

    case PrimitiveTopology::PrimitiveTopology_TriangleFan:
        return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN;

    case PrimitiveTopology::PrimitiveTopology_LineListWithAdjacency:
        return VK_PRIMITIVE_TOPOLOGY_LINE_LIST_WITH_ADJACENCY;

    case PrimitiveTopology::PrimitiveTopology_LineStripWithAdjacency:
        return VK_PRIMITIVE_TOPOLOGY_LINE_STRIP_WITH_ADJACENCY;

    case PrimitiveTopology::PrimitiveTopology_TriangleListWithAdjacency:
        return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST_WITH_ADJACENCY;

    case PrimitiveTopology::PrimitiveTopology_TriangleStripWithAdjacency:
        return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP_WITH_ADJACENCY;

    case PrimitiveTopology::PrimitiveTopology_PatchList:
        return VK_PRIMITIVE_TOPOLOGY_PATCH_LIST;

    default:
        break;
    }
    ASSERT(false, "primitiveTopology not found");
    return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
}

VkShaderStageFlagBits VulkanGraphicPipeline::convertShaderTypeToVkStage(resource::ShaderType type)
{
    switch (type)
    {
    case resource::ShaderType::ShaderType_Vertex:
        return VK_SHADER_STAGE_VERTEX_BIT;

    case resource::ShaderType::ShaderType_Fragment:
        return VK_SHADER_STAGE_FRAGMENT_BIT;

    default:
        ASSERT(false, "type not found");
    }

    return VK_SHADER_STAGE_VERTEX_BIT;
}

VulkanGraphicPipeline::VulkanGraphicPipeline(VkDevice device, RenderPassManager* renderpassManager)
    : Pipeline(PipelineType::PipelineType_Graphic)
    , m_device(device)
    , m_pipeline(VK_NULL_HANDLE)

    , m_renderpassManager(renderpassManager)
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
    ASSERT(getType() == PipelineType::PipelineType_Graphic, "invalid type");

    ASSERT(pipelineInfo, "nullptr");
    const GraphicsPipelineStateDescription& pipelineDesc = pipelineInfo->_pipelineDesc;

    VkPipelineCache pipelineCache = VK_NULL_HANDLE; //TODO

    VkGraphicsPipelineCreateInfo graphicsPipelineCreateInfo = {};
    graphicsPipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    graphicsPipelineCreateInfo.pNext = nullptr;
    graphicsPipelineCreateInfo.flags = 0; //VkPipelineDiscardRectangleStateCreateInfoEXT

    graphicsPipelineCreateInfo.basePipelineIndex = 0;
    graphicsPipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE;

    auto findShaderByType = [](std::vector<resource::Shader*> shaders, resource::ShaderType type) -> const resource::Shader*
    {
        for (auto shader : shaders)
        {
            if (shader->getShaderHeader()._type == type)
            {
                return shader;
            }
        }

        return nullptr;
    };

    std::vector<VkPipelineShaderStageCreateInfo> pipelineShaderStageCreateInfos;
    for (u32 type = resource::ShaderType::ShaderType_Vertex; type < resource::ShaderType_Count; ++type)
    {
        VkPipelineShaderStageCreateInfo pipelineShaderStageCreateInfo = {};
        if (!createShaderModule(findShaderByType(pipelineInfo->_shaders, (resource::ShaderType)type), pipelineShaderStageCreateInfo))
        {
            LOG_ERROR("VulkanGraphicPipeline::create couldn't create modules for pipeline");
            deleteShaderModules();
            return false;
        }

        pipelineShaderStageCreateInfos.push_back(pipelineShaderStageCreateInfo);
    }
    graphicsPipelineCreateInfo.stageCount = static_cast<u32>(pipelineShaderStageCreateInfos.size());
    graphicsPipelineCreateInfo.pStages = pipelineShaderStageCreateInfos.data();

    //graphicsPipelineCreateInfo.layout = VK_NULL_HANDLE; //TODO

    const ShaderProgramDescription& shaderProgramInfo = pipelineInfo->_programDesc;
    RenderPass* pass = m_renderpassManager->acquireRenderPass(pipelineInfo->_renderpassDesc);
    if (!pass)
    {
        LOG_ERROR("VulkanGraphicPipeline::create couldn't create renderpass for pipline");
        return false;
    }

    graphicsPipelineCreateInfo.renderPass = static_cast<VulkanRenderPass*>(pass)->getHandle();
    graphicsPipelineCreateInfo.subpass = 0; //TODO

    const GraphicsPipelineStateDescription::RasterizationState& rasterizationState = pipelineDesc._rasterizationState;
    VkPipelineRasterizationStateCreateInfo rasterizationStateCreateInfo = {};
    rasterizationStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizationStateCreateInfo.pNext = nullptr; //VkPipelineRasterizationStateStreamCreateInfoEXT, VkPipelineRasterizationConservativeStateCreateInfoEXT
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


    VkPipelineInputAssemblyStateCreateInfo inputAssemblyStateCreateInfo = {};
    inputAssemblyStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssemblyStateCreateInfo.pNext = nullptr;
    inputAssemblyStateCreateInfo.flags = 0;
    inputAssemblyStateCreateInfo.topology = VulkanGraphicPipeline::convertPrimitiveTopologyToVk(pipelineDesc._primitiveTopology);
    inputAssemblyStateCreateInfo.primitiveRestartEnable = VK_FALSE;
    graphicsPipelineCreateInfo.pInputAssemblyState = &inputAssemblyStateCreateInfo;


    VkPipelineVertexInputStateCreateInfo vertexInputStateCreateInfo = {};
    vertexInputStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputStateCreateInfo.pNext = nullptr; //VkPipelineVertexInputDivisorStateCreateInfoEXT
    vertexInputStateCreateInfo.flags = 0;

    VkVertexInputBindingDescription vertexInputBindingDescription = {};
    vertexInputBindingDescription.binding = 0;
    vertexInputBindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    vertexInputBindingDescription.stride = 0;

    vertexInputStateCreateInfo.vertexBindingDescriptionCount = 1;
    vertexInputStateCreateInfo.pVertexBindingDescriptions = &vertexInputBindingDescription;

    std::vector<VkVertexInputAttributeDescription> vertexInputAttributeDescriptions;
    vertexInputAttributeDescriptions.reserve(shaderProgramInfo._inputAttachment.size());
    for (auto& attr : shaderProgramInfo._inputAttachment)
    {
        VkVertexInputAttributeDescription vertexInputAttributeDescription = {};
        vertexInputAttributeDescription.binding = 0;
        vertexInputAttributeDescription.location = attr.second._location;
        vertexInputAttributeDescription.format = VulkanImage::convertImageFormatToVkFormat(attr.second._format);
        vertexInputAttributeDescription.offset = attr.second._offset;
        vertexInputAttributeDescriptions.push_back(vertexInputAttributeDescription);
    }
    vertexInputStateCreateInfo.vertexAttributeDescriptionCount = static_cast<u32>(vertexInputAttributeDescriptions.size());
    vertexInputStateCreateInfo.pVertexAttributeDescriptions = vertexInputAttributeDescriptions.data();

    graphicsPipelineCreateInfo.pVertexInputState = &vertexInputStateCreateInfo;


    VkPipelineColorBlendStateCreateInfo pipelineColorBlendStateCreateInfo = {};
    pipelineColorBlendStateCreateInfo.sType;
    pipelineColorBlendStateCreateInfo.pNext;
    pipelineColorBlendStateCreateInfo.flags;
    pipelineColorBlendStateCreateInfo.logicOpEnable;
    pipelineColorBlendStateCreateInfo.logicOp;
    pipelineColorBlendStateCreateInfo.blendConstants;

    std::vector<VkPipelineColorBlendAttachmentState> pipelineColorBlendAttachmentStates;
    //for
    VkPipelineColorBlendAttachmentState pipelineColorBlendAttachmentState = {};
    pipelineColorBlendAttachmentState.blendEnable;
    pipelineColorBlendAttachmentState.srcColorBlendFactor;
    pipelineColorBlendAttachmentState.dstColorBlendFactor;
    pipelineColorBlendAttachmentState.colorBlendOp;
    pipelineColorBlendAttachmentState.srcAlphaBlendFactor;
    pipelineColorBlendAttachmentState.dstAlphaBlendFactor;
    pipelineColorBlendAttachmentState.alphaBlendOp;
    pipelineColorBlendAttachmentState.colorWriteMask;

    pipelineColorBlendAttachmentStates.push_back(pipelineColorBlendAttachmentState);
    //

    pipelineColorBlendStateCreateInfo.attachmentCount = static_cast<u32>(pipelineColorBlendAttachmentStates.size());
    pipelineColorBlendStateCreateInfo.pAttachments = pipelineColorBlendAttachmentStates.data();

    graphicsPipelineCreateInfo.pColorBlendState = &pipelineColorBlendStateCreateInfo;

    VkPipelineDepthStencilStateCreateInfo pipelineDepthStencilStateCreateInfo = {};
    pipelineDepthStencilStateCreateInfo.sType;
    pipelineDepthStencilStateCreateInfo.pNext;
    pipelineDepthStencilStateCreateInfo.flags;
    //TODO:

    graphicsPipelineCreateInfo.pDepthStencilState = &pipelineDepthStencilStateCreateInfo;

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
    if (m_pipeline)
    {
        VulkanWrapper::DestroyPipeline(m_device, m_pipeline, VULKAN_ALLOCATOR);
        m_pipeline = VK_NULL_HANDLE;
    }
    deleteShaderModules();
}

bool VulkanGraphicPipeline::compileShader(const resource::ShaderHeader* header, const void * source, u32 size)
{
    if (!source || size > 0)
    {
        return false;
    }

    VkShaderModuleCreateInfo shaderModuleCreateInfo = {};
    shaderModuleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    shaderModuleCreateInfo.pNext = nullptr; //VkShaderModuleValidationCacheCreateInfoEXT
    shaderModuleCreateInfo.flags = 0;
    shaderModuleCreateInfo.pCode = reinterpret_cast<const u32*>(source);
    shaderModuleCreateInfo.codeSize = size;

    VkShaderModule module = VK_NULL_HANDLE;
    VkResult result = VulkanWrapper::CreateShaderModule(m_device, &shaderModuleCreateInfo, VULKAN_ALLOCATOR, &module); //manager
    if (result != VK_SUCCESS)
    {
        LOG_ERROR("VulkanGraphicPipeline::compileShader vkCreateShaderModule is failed. Error: %s", ErrorString(result).c_str());
        return false;
    }
    m_modules.push_back(module);

    return true;
}

bool VulkanGraphicPipeline::createShaderModule(const resource::Shader* shader, VkPipelineShaderStageCreateInfo& outPipelineShaderStageCreateInfo)
{
    if (!shader || VulkanGraphicPipeline::createShader(shader))
    {
        return false;
    }

    const resource::ShaderHeader& header = shader->getShaderHeader();

    outPipelineShaderStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    outPipelineShaderStageCreateInfo.pNext = nullptr;
    outPipelineShaderStageCreateInfo.flags = 0;
    outPipelineShaderStageCreateInfo.stage = convertShaderTypeToVkStage((resource::ShaderType)header._type);
    outPipelineShaderStageCreateInfo.module = m_modules.back();
    outPipelineShaderStageCreateInfo.pName = header._entyPoint.c_str();
    outPipelineShaderStageCreateInfo.pSpecializationInfo = nullptr;

    return true;
}

void VulkanGraphicPipeline::deleteShaderModules()
{
    for (auto& module : m_modules)
    {
        VulkanWrapper::DestroyShaderModule(m_device, module, VULKAN_ALLOCATOR);
    }
    m_modules.clear();
}

} //namespace vk
} //namespace renderer
} //namespace v3d
#endif //VULKAN_RENDER
