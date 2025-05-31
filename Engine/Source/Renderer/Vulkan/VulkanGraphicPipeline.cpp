#include "VulkanGraphicPipeline.h"

#include "Utils/Logger.h"
#include "Renderer/ShaderProgram.h"

#ifdef VULKAN_RENDER
#   include "VulkanDebug.h"
#   include "VulkanDevice.h"
#   include "VulkanSwapchain.h"
#   include "VulkanImage.h"
#   include "VulkanRenderpass.h"
//#include "VulkanDescriptorSet.h"

#if defined(USE_SPIRV)
#   define RUNTIME_PATCH_SPIRV_REMOVE_UNUSED_LOCATIONS 0
#   if RUNTIME_PATCH_SPIRV_REMOVE_UNUSED_LOCATIONS
#       include "Resource/SpirVPatch/ShaderSpirVPatcherRemoveUnusedLocations.h"
#       include "Resource/SpirVPatch/ShaderSpirVPatcherVertexTransform.h"
#       include "Resource/SpirVPatch/ShaderSpirVPatcherInvertOrdinate.h"
#   endif //PATCH_SPIRV_REMOVE_UNUSED_LOCATIONS
#endif //USE_SPIRV


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
    case PolygonMode::PolygonMode_Fill:
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

VkShaderStageFlagBits VulkanGraphicPipeline::convertShaderTypeToVkStage(ShaderType type)
{
    switch (type)
    {
    case ShaderType::Vertex:
        return VK_SHADER_STAGE_VERTEX_BIT;

    case ShaderType::Fragment:
        return VK_SHADER_STAGE_FRAGMENT_BIT;

    case ShaderType::Compute:
        return VK_SHADER_STAGE_COMPUTE_BIT;

    default:
        ASSERT(false, "type not found");
    }

    return VK_SHADER_STAGE_VERTEX_BIT;
}

VkBlendFactor VulkanGraphicPipeline::convertBlendFactorToVk(BlendFactor factor)
{
    switch (factor)
    {
    case BlendFactor::BlendFactor_Zero:
        return VK_BLEND_FACTOR_ZERO;

    case BlendFactor::BlendFactor_One:
        return VK_BLEND_FACTOR_ONE;

    case BlendFactor::BlendFactor_SrcColor:
        return VK_BLEND_FACTOR_SRC_COLOR;

    case BlendFactor::BlendFactor_OneMinusSrcColor:
        return VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR;

    case BlendFactor::BlendFactor_DstColor:
        return VK_BLEND_FACTOR_DST_COLOR;

    case BlendFactor::BlendFactor_OneMinusDstColor:
        return VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR;

    case BlendFactor::BlendFactor_SrcAlpha:
        return VK_BLEND_FACTOR_SRC_ALPHA;

    case BlendFactor::BlendFactor_OneMinusSrcAlpha:
        return VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;

    case BlendFactor::BlendFactor_DstAlpha:
        return VK_BLEND_FACTOR_DST_ALPHA;

    case BlendFactor::BlendFactor_OneMinusDstAlpha:
        return VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA;

    case BlendFactor::BlendFactor_ConstantColor:
        return VK_BLEND_FACTOR_CONSTANT_COLOR;

    case BlendFactor::BlendFactor_OneMinusConstantColor:
        return VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_COLOR;

    case BlendFactor::BlendFactor_ConstantAlpha:
        return VK_BLEND_FACTOR_CONSTANT_ALPHA;

    case BlendFactor::BlendFactor_OneMinusConstantAlpha:
        return VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_ALPHA;

    case BlendFactor::BlendFactor_SrcAlphaSaturate:
        return VK_BLEND_FACTOR_SRC_ALPHA_SATURATE;

    case BlendFactor::BlendFactor_Src1Color:
        return VK_BLEND_FACTOR_SRC1_COLOR;

    case BlendFactor::BlendFactor_OneMinusSrc1Color:
        return VK_BLEND_FACTOR_ONE_MINUS_SRC1_COLOR;

    case BlendFactor::BlendFactor_Src1Alpha:
        return VK_BLEND_FACTOR_SRC1_ALPHA;

    case BlendFactor::BlendFactor_OneMinusSrc1Alpha:
        return VK_BLEND_FACTOR_ONE_MINUS_SRC1_ALPHA;

    default:
        break;
    }

    ASSERT(false, "blend facor not found");
    return VK_BLEND_FACTOR_ZERO;
}

VkBlendOp VulkanGraphicPipeline::convertBlendOperationToVk(BlendOperation blendOp)
{
    switch (blendOp)
    {
    case BlendOperation::BlendOp_Add:
        return VK_BLEND_OP_ADD;

    case BlendOperation::BlendOp_Subtract:
        return VK_BLEND_OP_SUBTRACT;

    case BlendOperation::BlendOp_ReverseSubtract:
        return VK_BLEND_OP_REVERSE_SUBTRACT;

    case BlendOperation::BlendOp_Min:
        return VK_BLEND_OP_MIN;

    case BlendOperation::BlendOp_Max:
        return VK_BLEND_OP_MAX;

    default:
        break;
    }

    ASSERT(false, "blend op not found");
    return VK_BLEND_OP_ADD;
}

VkLogicOp VulkanGraphicPipeline::covertLogicOperationToVk(LogicalOperation logicalOp)
{
    switch (logicalOp)
    {
    case LogicalOperation::LogicalOp_Clear:
        return VK_LOGIC_OP_CLEAR;

    case LogicalOperation::LogicalOp_And:
        return VK_LOGIC_OP_AND;

    case LogicalOperation::LogicalOp_AndReverse:
        return VK_LOGIC_OP_AND_REVERSE;

    case LogicalOperation::LogicalOp_Copy:
        return VK_LOGIC_OP_COPY;

    case LogicalOperation::LogicalOp_AndInverted:
        return VK_LOGIC_OP_AND_INVERTED;

    case LogicalOperation::LogicalOp_NoOp:
        return VK_LOGIC_OP_NO_OP;

    case LogicalOperation::LogicalOp_Xor:
        return VK_LOGIC_OP_XOR;

    case LogicalOperation::LogicalOp_Or:
        return VK_LOGIC_OP_OR;

    case LogicalOperation::LogicalOp_Nor:
        return VK_LOGIC_OP_NOR;

    case LogicalOperation::LogicalOp_Equivalent:
        return VK_LOGIC_OP_EQUIVALENT;

    case LogicalOperation::LogicalOp_Invert:
        return VK_LOGIC_OP_INVERT;

    case LogicalOperation::LogicalOp_OrReverse:
        return VK_LOGIC_OP_OR_REVERSE;

    case LogicalOperation::LogicalOp_CopyInverted:
        return VK_LOGIC_OP_COPY_INVERTED;

    case LogicalOperation::LogicalOp_OrInverted:
        return VK_LOGIC_OP_OR_INVERTED;

    case LogicalOperation::LogicalOp_Nand:
        return VK_LOGIC_OP_NAND;

    case LogicalOperation::LogicalOp_Set:
        return VK_LOGIC_OP_SET;

    default:
        break;
    }

    ASSERT(false, "logical Op not found");
    return VK_LOGIC_OP_AND;
}

VkCompareOp VulkanGraphicPipeline::convertCompareOperationToVk(CompareOperation compareOp)
{
    switch (compareOp)
    {
    case CompareOperation::CompareOp_Never:
        return VK_COMPARE_OP_NEVER;

    case CompareOperation::CompareOp_Less:
        return VK_COMPARE_OP_LESS;

    case CompareOperation::CompareOp_Equal:
        return VK_COMPARE_OP_EQUAL;

    case CompareOperation::CompareOp_LessOrEqual:
        return VK_COMPARE_OP_LESS_OR_EQUAL;

    case CompareOperation::CompareOp_Greater:
        return VK_COMPARE_OP_GREATER;

    case CompareOperation::CompareOp_NotEqual:
        return VK_COMPARE_OP_NOT_EQUAL;

    case CompareOperation::CompareOp_GreaterOrEqual:
        return VK_COMPARE_OP_GREATER_OR_EQUAL;

    case CompareOperation::CompareOp_Always:
        return VK_COMPARE_OP_ALWAYS;

    default:
        break;
    }

    ASSERT(false, "compare Op not found");
    return VK_COMPARE_OP_GREATER;
}

VkVertexInputRate VulkanGraphicPipeline::covertInputRateToVk(InputRate rate)
{
    if (rate == InputRate::InputRate_Vertex)
    {
        return VK_VERTEX_INPUT_RATE_VERTEX;
    }
    else if (rate == InputRate::InputRate_Instance)
    {
        return VK_VERTEX_INPUT_RATE_INSTANCE;
    }

    ASSERT(false, "rate not found");
    return VK_VERTEX_INPUT_RATE_VERTEX;

}

VulkanGraphicPipeline::VulkanGraphicPipeline(VulkanDevice* device, VulkanRenderpassManager* renderpassManager, VulkanPipelineLayoutManager* pipelineLayoutManager, const std::string& name)
    : RenderPipeline(PipelineType::PipelineType_Graphic)
    , m_device(*device)
    , m_pipeline(VK_NULL_HANDLE)

    , m_compatibilityRenderPass(nullptr)
    , m_trackerRenderPass(nullptr, [](const std::vector<renderer::RenderPass*>&) {})

    , m_renderpassManager(renderpassManager)
    , m_pipelineLayoutManager(pipelineLayoutManager)
{
#if VULKAN_DEBUG
    LOG_DEBUG("VulkanGraphicPipeline::VulkanGraphicPipeline constructor %llx", this);
#endif //VULKAN_DEBUG
    m_modules.fill(VK_NULL_HANDLE);
#if VULKAN_DEBUG_MARKERS
    m_debugName = name.empty() ? "GraphicPipeline" : name;
    m_debugName.append(VulkanDebugUtils::k_addressPreffix);
    m_debugName.append(std::to_string(reinterpret_cast<const u64>(this)));
#endif //VULKAN_DEBUG_MARKERS
}

VulkanGraphicPipeline::~VulkanGraphicPipeline()
{
#if VULKAN_DEBUG
    LOG_DEBUG("VulkanRenderPass::VulkanRenderPass destructor %llx", this);
#endif //VULKAN_DEBUG
    ASSERT(!m_pipeline, "not nullptr");
}

bool VulkanGraphicPipeline::create(const GraphicsPipelineState& state)
{
    ASSERT(getType() == PipelineType::PipelineType_Graphic, "invalid type");

    const GraphicsPipelineStateDesc& pipelineDesc = state.getPipelineStateDesc();

    VkPipelineCache pipelineCache = VK_NULL_HANDLE; //TODO

    VkGraphicsPipelineCreateInfo graphicsPipelineCreateInfo = {};
    graphicsPipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    graphicsPipelineCreateInfo.pNext = nullptr;
    graphicsPipelineCreateInfo.flags = 0; //VkPipelineDiscardRectangleStateCreateInfoEXT
#if VULKAN_DEBUG
    if (m_device.getVulkanDeviceCaps()._pipelineExecutablePropertiesEnabled)
    {
        graphicsPipelineCreateInfo.flags |= VK_PIPELINE_CREATE_CAPTURE_STATISTICS_BIT_KHR;
    }
#endif //VULKAN_DEBUG
    graphicsPipelineCreateInfo.basePipelineIndex = 0;
    graphicsPipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE;

    ASSERT(!m_compatibilityRenderPass, "not nullptr");
    if (!createCompatibilityRenderPass(state.getRenderPassDesc(), m_compatibilityRenderPass))
    {
        LOG_ERROR("VulkanGraphicPipeline::create couldn't create renderpass for pipline");
        return false;
    }

    graphicsPipelineCreateInfo.renderPass = static_cast<VulkanRenderPass*>(m_compatibilityRenderPass)->getHandle();
    graphicsPipelineCreateInfo.subpass = 0; //TODO


    std::vector<VkPipelineShaderStageCreateInfo> pipelineShaderStageCreateInfos;
    if (!VulkanGraphicPipeline::createShaderModules(state.getShaderProgram()))
    {
        LOG_ERROR("VulkanGraphicPipeline::create can't create modules for pipeline");
        deleteShaderModules();
        return false;
    }

    u32 moduleIndex = 0;
    for (u32 type = toEnumType(ShaderType::First); type <= (u32)toEnumType(ShaderType::Last); ++type)
    {
        if (!m_modules[type])
        {
            continue;
        }

        VkPipelineShaderStageCreateInfo pipelineShaderStageCreateInfo = {};
        pipelineShaderStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        pipelineShaderStageCreateInfo.pNext = nullptr;
        pipelineShaderStageCreateInfo.flags = 0;
        pipelineShaderStageCreateInfo.stage = convertShaderTypeToVkStage((ShaderType)type);
        pipelineShaderStageCreateInfo.module = m_modules[moduleIndex];
        pipelineShaderStageCreateInfo.pName = state.getShaderProgram()->getShader((ShaderType)type)->getEntryPoint().c_str();
        pipelineShaderStageCreateInfo.pSpecializationInfo = nullptr;

        ++moduleIndex;

        pipelineShaderStageCreateInfos.push_back(pipelineShaderStageCreateInfo);
    }
    graphicsPipelineCreateInfo.stageCount = static_cast<u32>(pipelineShaderStageCreateInfos.size());
    graphicsPipelineCreateInfo.pStages = pipelineShaderStageCreateInfos.data();

    VulkanPipelineLayoutManager::DescriptorSetLayoutCreator layoutDesc(m_device, state.getShaderProgram());
    m_pipelineLayoutDescription = layoutDesc._description;
    m_pipelineLayout = m_pipelineLayoutManager->acquirePipelineLayout(m_pipelineLayoutDescription);
    graphicsPipelineCreateInfo.layout = m_pipelineLayout._pipelineLayout;


    const GraphicsPipelineStateDesc::RasterizationState& rasterizationState = pipelineDesc._rasterizationState;

    //VkPipelineRasterizationStateCreateInfo
    VkPipelineRasterizationStateCreateInfo rasterizationStateCreateInfo = {};
    rasterizationStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizationStateCreateInfo.pNext = nullptr; //VkPipelineRasterizationStateStreamCreateInfoEXT, VkPipelineRasterizationConservativeStateCreateInfoEXT
    rasterizationStateCreateInfo.flags = 0;
    rasterizationStateCreateInfo.depthClampEnable = VK_FALSE;
    rasterizationStateCreateInfo.rasterizerDiscardEnable = rasterizationState._discardRasterization ? VK_TRUE : VK_FALSE;
    rasterizationStateCreateInfo.polygonMode = VulkanGraphicPipeline::convertPolygonModeToVk(rasterizationState._polygonMode);
    rasterizationStateCreateInfo.cullMode = VulkanGraphicPipeline::convertCullModeToVk(rasterizationState._cullMode);
    rasterizationStateCreateInfo.frontFace = VulkanGraphicPipeline::convertFrontFaceToVk(rasterizationState._frontFace);
    if (!VulkanDevice::isDynamicStateSupported(VK_DYNAMIC_STATE_DEPTH_BIAS))
    {
        rasterizationStateCreateInfo.depthBiasEnable = (rasterizationState._depthBiasConstant != 0.f || rasterizationState._depthBiasClamp != 0.f || rasterizationState._depthBiasSlope != 0.f) ? VK_TRUE : VK_FALSE;
        rasterizationStateCreateInfo.depthBiasConstantFactor = rasterizationState._depthBiasConstant;
        rasterizationStateCreateInfo.depthBiasClamp = rasterizationState._depthBiasClamp;
        rasterizationStateCreateInfo.depthBiasSlopeFactor = rasterizationState._depthBiasSlope;
    }
    else
    {
        rasterizationStateCreateInfo.depthBiasEnable = VK_FALSE;
        ASSERT(false, "not impl");
    }

    if (!VulkanDevice::isDynamicStateSupported(VK_DYNAMIC_STATE_LINE_WIDTH))
    {
        rasterizationStateCreateInfo.lineWidth = 1.0f; //wideLines
    }
    graphicsPipelineCreateInfo.pRasterizationState = &rasterizationStateCreateInfo;


    //VkPipelineInputAssemblyStateCreateInfo
    VkPipelineInputAssemblyStateCreateInfo inputAssemblyStateCreateInfo = {};
    inputAssemblyStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssemblyStateCreateInfo.pNext = nullptr;
    inputAssemblyStateCreateInfo.flags = 0;
    inputAssemblyStateCreateInfo.topology = VulkanGraphicPipeline::convertPrimitiveTopologyToVk(pipelineDesc._vertexInputState._primitiveTopology);
    inputAssemblyStateCreateInfo.primitiveRestartEnable = VK_FALSE;
    graphicsPipelineCreateInfo.pInputAssemblyState = &inputAssemblyStateCreateInfo;


    //VkPipelineVertexInputStateCreateInfo
    VkPipelineVertexInputStateCreateInfo vertexInputStateCreateInfo = {};
    vertexInputStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputStateCreateInfo.pNext = nullptr; //VkPipelineVertexInputDivisorStateCreateInfoEXT
    vertexInputStateCreateInfo.flags = 0;

    const VertexInputAttributeDesc& inputAttrDesc = pipelineDesc._vertexInputState._inputAttributes;

    bool useStrideBindingDynamicState = false;
    std::vector<VkVertexInputBindingDescription> vertexInputBindingDescriptions;
    vertexInputBindingDescriptions.reserve(inputAttrDesc._countInputBindings);
    for (u32 index = 0; index < inputAttrDesc._countInputBindings; ++index)
    {
        VkVertexInputBindingDescription vertexInputBindingDescription = {};
        vertexInputBindingDescription.binding = inputAttrDesc._inputBindings[index]._binding;
        vertexInputBindingDescription.inputRate = VulkanGraphicPipeline::covertInputRateToVk(inputAttrDesc._inputBindings[index]._rate);
        if (VulkanDevice::isDynamicStateSupported(VK_DYNAMIC_STATE_VERTEX_INPUT_BINDING_STRIDE))
        {
            vertexInputBindingDescription.stride = 0; //ignore this value
            useStrideBindingDynamicState = true;
        }
        else
        {
            vertexInputBindingDescription.stride = inputAttrDesc._inputBindings[index]._stride;
        }
        vertexInputBindingDescriptions.push_back(vertexInputBindingDescription);
    }
    vertexInputStateCreateInfo.vertexBindingDescriptionCount = static_cast<u32>(vertexInputBindingDescriptions.size());
    vertexInputStateCreateInfo.pVertexBindingDescriptions = vertexInputBindingDescriptions.data();

    const Shader* vertexShader = state.getShaderProgram()->getShader(ShaderType::Vertex);
    ASSERT(vertexShader, "nullptr");
    std::vector<VkVertexInputAttributeDescription> vertexInputAttributeDescriptions;
    vertexInputAttributeDescriptions.reserve(inputAttrDesc._countInputAttributes);
    const Shader::Resources& res = vertexShader->getMappingResources();
    ASSERT(res._inputAttribute.size() == inputAttrDesc._countInputAttributes, "different sizes");
    for (u32 index = 0; index < inputAttrDesc._countInputAttributes; ++index)
    {
        VkVertexInputAttributeDescription vertexInputAttributeDescription = {};
        vertexInputAttributeDescription.binding = inputAttrDesc._inputAttributes[index]._binding;
        vertexInputAttributeDescription.location = res._inputAttribute[index]._location;
        if (res._inputAttribute[index]._format != inputAttrDesc._inputAttributes[index]._format)
        {
            VkFormat shaderFormat = VulkanImage::convertImageFormatToVkFormat(res._inputAttribute[index]._format);
            VkFormat descFormat = VulkanImage::convertImageFormatToVkFormat(inputAttrDesc._inputAttributes[index]._format);
            LOG_WARNING("VulkanGraphicPipeline::create: different input attribute formats index: %u, shader - %s, desc - %s ", index, VulkanImage::imageFormatStringVK(shaderFormat).c_str(), VulkanImage::imageFormatStringVK(descFormat).c_str());
        }
        vertexInputAttributeDescription.format = VulkanImage::convertImageFormatToVkFormat(inputAttrDesc._inputAttributes[index]._format);
        vertexInputAttributeDescription.offset = inputAttrDesc._inputAttributes[index]._offset;
        vertexInputAttributeDescriptions.push_back(vertexInputAttributeDescription);
    }
    vertexInputStateCreateInfo.vertexAttributeDescriptionCount = static_cast<u32>(vertexInputAttributeDescriptions.size());
    vertexInputStateCreateInfo.pVertexAttributeDescriptions = vertexInputAttributeDescriptions.data();

    graphicsPipelineCreateInfo.pVertexInputState = &vertexInputStateCreateInfo;

    const GraphicsPipelineStateDesc::BlendState& blendState = pipelineDesc._blendState;

    //VkPipelineColorBlendStateCreateInfo
    VkPipelineColorBlendStateCreateInfo pipelineColorBlendStateCreateInfo = {};
    pipelineColorBlendStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    pipelineColorBlendStateCreateInfo.pNext = nullptr; //VkPipelineColorBlendAdvancedStateCreateInfoEXT
    pipelineColorBlendStateCreateInfo.flags = 0;
    if (m_device.getVulkanDeviceCaps().getPhysicalDeviceFeatures().logicOp)
    {
        pipelineColorBlendStateCreateInfo.logicOpEnable = blendState._logicalOpEnable;
        pipelineColorBlendStateCreateInfo.logicOp = VulkanGraphicPipeline::covertLogicOperationToVk(blendState._logicalOp);
    }
    else
    {
        pipelineColorBlendStateCreateInfo.logicOpEnable = VK_FALSE;
        ASSERT(blendState._logicalOpEnable == VK_FALSE, "feature logicOp not supported");
    }

    if (!VulkanDevice::isDynamicStateSupported(VK_DYNAMIC_STATE_BLEND_CONSTANTS))
    {
        pipelineColorBlendStateCreateInfo.blendConstants[0] = blendState._constant[0];
        pipelineColorBlendStateCreateInfo.blendConstants[1] = blendState._constant[1];
        pipelineColorBlendStateCreateInfo.blendConstants[2] = blendState._constant[2];
        pipelineColorBlendStateCreateInfo.blendConstants[3] = blendState._constant[3];
    }

    //bool independentBlend = VulkanDeviceCaps::getInstance()->getPhysicalDeviceFeatures().independentBlend;
    std::vector<VkPipelineColorBlendAttachmentState> pipelineColorBlendAttachmentStates;
    for (u32 index = 0; index < state.getRenderPassDesc()._countColorAttachments; ++index)
    {
#if USE_MULTI_COLOR_BLEND_ATTACMENTS
        const GraphicsPipelineStateDesc::BlendState::ColorBlendAttachmentState& colorBlendState = blendState._colorBlendAttachments[index];
#else
        const GraphicsPipelineStateDesc::BlendState::ColorBlendAttachmentState& colorBlendState = blendState._colorBlendAttachments[0];
#endif
        VkPipelineColorBlendAttachmentState pipelineColorBlendAttachmentState = {};
        pipelineColorBlendAttachmentState.blendEnable = colorBlendState._colorBlendEnable;
        pipelineColorBlendAttachmentState.srcColorBlendFactor = VulkanGraphicPipeline::convertBlendFactorToVk(colorBlendState._srcBlendFacor);
        pipelineColorBlendAttachmentState.dstColorBlendFactor = VulkanGraphicPipeline::convertBlendFactorToVk(colorBlendState._dscBlendFacor);
        pipelineColorBlendAttachmentState.colorBlendOp = VulkanGraphicPipeline::convertBlendOperationToVk(colorBlendState._blendOp);
        pipelineColorBlendAttachmentState.srcAlphaBlendFactor = VulkanGraphicPipeline::convertBlendFactorToVk(colorBlendState._srcAlphaBlendFacor);
        pipelineColorBlendAttachmentState.dstAlphaBlendFactor = VulkanGraphicPipeline::convertBlendFactorToVk(colorBlendState._dscAlphaBlendFacor);
        pipelineColorBlendAttachmentState.alphaBlendOp = VulkanGraphicPipeline::convertBlendOperationToVk(colorBlendState._alphaBlendOp);
        pipelineColorBlendAttachmentState.colorWriteMask = colorBlendState._colorWriteMask;

        pipelineColorBlendAttachmentStates.push_back(pipelineColorBlendAttachmentState);
    }

    pipelineColorBlendStateCreateInfo.attachmentCount = static_cast<u32>(pipelineColorBlendAttachmentStates.size());
    pipelineColorBlendStateCreateInfo.pAttachments = pipelineColorBlendAttachmentStates.data();

    graphicsPipelineCreateInfo.pColorBlendState = &pipelineColorBlendStateCreateInfo;

    const GraphicsPipelineStateDesc::DepthStencilState& depthBlendState = state.getPipelineStateDesc()._depthStencilState;

    //VkPipelineDepthStencilStateCreateInfo
    VkPipelineDepthStencilStateCreateInfo pipelineDepthStencilStateCreateInfo = {};
    pipelineDepthStencilStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    pipelineDepthStencilStateCreateInfo.pNext = nullptr;
    pipelineDepthStencilStateCreateInfo.flags = 0;
    pipelineDepthStencilStateCreateInfo.depthTestEnable = depthBlendState._depthTestEnable;
    pipelineDepthStencilStateCreateInfo.depthWriteEnable = depthBlendState._depthWriteEnable;
    pipelineDepthStencilStateCreateInfo.depthCompareOp = VulkanGraphicPipeline::convertCompareOperationToVk(depthBlendState._compareOp);
    if (m_device.getVulkanDeviceCaps().getPhysicalDeviceFeatures().depthBounds)
    {
        pipelineDepthStencilStateCreateInfo.depthBoundsTestEnable = depthBlendState._depthBoundsTestEnable;
        if (!VulkanDevice::isDynamicStateSupported(VK_DYNAMIC_STATE_DEPTH_BOUNDS))
        {
            pipelineDepthStencilStateCreateInfo.minDepthBounds = depthBlendState._depthBounds._x;
            pipelineDepthStencilStateCreateInfo.maxDepthBounds = depthBlendState._depthBounds._y;
        }
    }
    else
    {
        pipelineDepthStencilStateCreateInfo.depthBoundsTestEnable = VK_FALSE;
        ASSERT(depthBlendState._depthBoundsTestEnable == VK_FALSE, "feature depthBounds not supported");
    }

    VkStencilOpState stencilOpState = {}; //TODO
    pipelineDepthStencilStateCreateInfo.stencilTestEnable = depthBlendState._stencilTestEnable;
    pipelineDepthStencilStateCreateInfo.front = stencilOpState;
    pipelineDepthStencilStateCreateInfo.back = stencilOpState;
    graphicsPipelineCreateInfo.pDepthStencilState = &pipelineDepthStencilStateCreateInfo;

    std::vector<VkDynamicState> dynamicStates = VulkanDevice::getDynamicStates();
    if (useStrideBindingDynamicState)
    {
        dynamicStates.push_back(VK_DYNAMIC_STATE_VERTEX_INPUT_BINDING_STRIDE);
    }

    //VkPipelineDynamicStateCreateInfo
    VkPipelineDynamicStateCreateInfo pipelineDynamicStateCreateInfo = {};
    pipelineDynamicStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    pipelineDynamicStateCreateInfo.pNext = nullptr;
    pipelineDynamicStateCreateInfo.flags = 0;
    pipelineDynamicStateCreateInfo.dynamicStateCount = static_cast<u32>(dynamicStates.size());
    pipelineDynamicStateCreateInfo.pDynamicStates = dynamicStates.data();
    graphicsPipelineCreateInfo.pDynamicState = &pipelineDynamicStateCreateInfo;

    //VkPipelineViewportStateCreateInfo
    VkPipelineViewportStateCreateInfo pipelineViewportStateCreateInfo = {};
    pipelineViewportStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    pipelineViewportStateCreateInfo.pNext = nullptr;
    pipelineViewportStateCreateInfo.flags = 0;

    if (!VulkanDevice::isDynamicStateSupported(VK_DYNAMIC_STATE_VIEWPORT))
    {
        VkViewport viewport = {};
        pipelineViewportStateCreateInfo.viewportCount = 1;
        pipelineViewportStateCreateInfo.pViewports = &viewport;

        ASSERT(false, "not implemented");
    }
    else
    {
        pipelineViewportStateCreateInfo.viewportCount = 1;
    }

    if (!VulkanDevice::isDynamicStateSupported(VK_DYNAMIC_STATE_SCISSOR))
    {
        VkRect2D scissor = {};
        pipelineViewportStateCreateInfo.scissorCount = 1;
        pipelineViewportStateCreateInfo.pScissors = &scissor;

        ASSERT(false, "not implemented");
    }
    else
    {
        pipelineViewportStateCreateInfo.scissorCount = 1;
    }
    graphicsPipelineCreateInfo.pViewportState = &pipelineViewportStateCreateInfo;

    //VkPipelineMultisampleStateCreateInfo
    VkPipelineMultisampleStateCreateInfo pipelineMultisampleStateCreateInfo = {};
    pipelineMultisampleStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    pipelineMultisampleStateCreateInfo.pNext = nullptr; //VkPipelineSampleLocationsStateCreateInfoEXT
    pipelineMultisampleStateCreateInfo.flags = 0;
    if (state.getRenderPassDesc()._attachmentsDesc[0]._samples > TextureSamples::TextureSamples_x1)
    {
        pipelineMultisampleStateCreateInfo.rasterizationSamples = VulkanImage::convertRenderTargetSamplesToVkSampleCount(state.getRenderPassDesc()._attachmentsDesc[0]._samples);
        pipelineMultisampleStateCreateInfo.sampleShadingEnable = VK_FALSE;
        pipelineMultisampleStateCreateInfo.minSampleShading = 0.0f;
        pipelineMultisampleStateCreateInfo.pSampleMask = nullptr;
        pipelineMultisampleStateCreateInfo.alphaToOneEnable = VK_FALSE;
        pipelineMultisampleStateCreateInfo.alphaToCoverageEnable = VK_FALSE;
    }
    else
    {
        pipelineMultisampleStateCreateInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    }

    graphicsPipelineCreateInfo.pMultisampleState = &pipelineMultisampleStateCreateInfo;


    //VkPipelineTessellationStateCreateInfo
    graphicsPipelineCreateInfo.pTessellationState = nullptr; //TODO


    VkResult result = VulkanWrapper::CreateGraphicsPipelines(m_device.getDeviceInfo()._device, pipelineCache, 1, &graphicsPipelineCreateInfo, VULKAN_ALLOCATOR, &m_pipeline);
    if (result != VK_SUCCESS)
    {
        LOG_ERROR("VulkanGraphicPipeline::create vkCreateGraphicsPipelines is failed. Error: %s", ErrorString(result).c_str());
        return false;
    }

#if VULKAN_DEBUG_MARKERS
    if (m_device.getVulkanDeviceCaps()._debugUtilsObjectNameEnabled)
    {
        VkDebugUtilsObjectNameInfoEXT debugUtilsObjectNameInfo = {};
        debugUtilsObjectNameInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
        debugUtilsObjectNameInfo.pNext = nullptr;
        debugUtilsObjectNameInfo.objectType = VK_OBJECT_TYPE_PIPELINE;
        debugUtilsObjectNameInfo.objectHandle = reinterpret_cast<u64>(m_pipeline);
        debugUtilsObjectNameInfo.pObjectName = m_debugName.c_str();

        VulkanWrapper::SetDebugUtilsObjectName(m_device.getDeviceInfo()._device, &debugUtilsObjectNameInfo);
    }
#endif //VULKAN_DEBUG_MARKERS

#if VULKAN_DEBUG
    VulkanGraphicPipeline::pipelineStatistic();
#endif
    return true;
}

void VulkanGraphicPipeline::destroy()
{
    if (m_pipeline)
    {
        VulkanWrapper::DestroyPipeline(m_device.getDeviceInfo()._device, m_pipeline, VULKAN_ALLOCATOR);
        m_pipeline = VK_NULL_HANDLE;
    }
    deleteShaderModules();

    if (m_pipelineLayout._pipelineLayout)
    {
        //release when manager will be destroyed
        //m_descriptorSetManager->removePipelineLayout(m_pipelineLayout);
        m_pipelineLayout._pipelineLayout = VK_NULL_HANDLE;
    }

    if (m_compatibilityRenderPass)
    {
        m_trackerRenderPass.release();
        if (!m_compatibilityRenderPass->linked())
        {
            m_renderpassManager->removeRenderPass(m_compatibilityRenderPass);
        }
        m_compatibilityRenderPass = nullptr;
    }
}

bool VulkanGraphicPipeline::createShaderModules(const renderer::ShaderProgram* program)
{
#if RUNTIME_PATCH_SPIRV_REMOVE_UNUSED_LOCATIONS
    resource::PatchRemoveUnusedLocations removeUnusedLocationPatch;
    for (auto& shader : shaders)
    {
        std::vector<u32> spirv;
        spirv.resize(std::get<2>(shader) / sizeof(u32));
        memcpy(spirv.data(), std::get<1>(shader), std::get<2>(shader));

        removeUnusedLocationPatch.collectDataFromSpirv(spirv);
    }
#endif //RUNTIME_PATCH_SPIRV_REMOVE_UNUSED_LOCATIONS

    for (u32 type = toEnumType(ShaderType::First); type <= (u32)toEnumType(ShaderType::Last); ++type)
    {
        const Shader* shader = program->getShader(ShaderType(type));
        if (!shader)
        {
            continue;
        }

        const void* source = shader->getBytecode();
        u32 size = shader->getBytecodeSize();
        if (!source || size == 0)
        {
            ASSERT(false, "invalid source");
            return false;
        }

#if RUNTIME_PATCH_SPIRV_REMOVE_UNUSED_LOCATIONS
        std::vector<u32> patchedSpirv;
        if (std::get<0>(shader) == ShaderType::Vertex)
        {
            patchedSpirv.resize(size / sizeof(u32));
            memcpy(patchedSpirv.data(), source, size);

            [[maybe_unused]] resource::ShaderPatcherSpirV patcher;
            if (patcher.process(&removeUnusedLocationPatch, patchedSpirv))
            {
                source = patchedSpirv.data();
                size = static_cast<u32>(patchedSpirv.size()) * sizeof(u32);
            }
#endif //RUNTIME_PATCH_SPIRV_REMOVE_UNUSED_LOCATIONS

#ifdef PLATFORM_ANDROID
            resource::PatchInvertOrdinate patchInvertOrdinate;
            if (patcher.process(&patchInvertOrdinate, patchedSpirv))
            {
                source = patchedSpirv.data();
                size = static_cast<u32>(patchedSpirv.size()) * sizeof(u32);
            }

            ASSERT(m_compatibilityRenderPass, "nullptr");
            if (VulkanDeviceCaps::getInstance()->preTransform && static_cast<VulkanRenderPass*>(m_compatibilityRenderPass)->isDrawingToSwapchain())
            {
                VkSurfaceTransformFlagBitsKHR preTransform = static_cast<VulkanContext*>(m_context)->getSwapchain()->getTransformFlag();
                if (preTransform & (VK_SURFACE_TRANSFORM_ROTATE_90_BIT_KHR | VK_SURFACE_TRANSFORM_ROTATE_270_BIT_KHR))
                {
                    f32 angleDegree = (preTransform == VK_SURFACE_TRANSFORM_ROTATE_90_BIT_KHR) ? 90.0f : 270.0f;
                    resource::PatchVertexTransform vertexTransform(angleDegree);
                    if (patcher.process(&vertexTransform, patchedSpirv))
                    {
                        source = patchedSpirv.data();
                        size = static_cast<u32>(patchedSpirv.size()) * sizeof(u32);
                    }
                }
            }
        }
#endif //PLATFORM_ANDROID

        VkShaderModuleCreateInfo shaderModuleCreateInfo = {};
        shaderModuleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        shaderModuleCreateInfo.pNext = nullptr; //VkShaderModuleValidationCacheCreateInfoEXT
        shaderModuleCreateInfo.flags = 0;
        shaderModuleCreateInfo.pCode = reinterpret_cast<const u32*>(source);
        shaderModuleCreateInfo.codeSize = size;

        VkShaderModule module = VK_NULL_HANDLE;
        VkResult result = VulkanWrapper::CreateShaderModule(m_device.getDeviceInfo()._device, &shaderModuleCreateInfo, VULKAN_ALLOCATOR, &module); //manager
        if (result != VK_SUCCESS)
        {
            LOG_ERROR("VulkanGraphicPipeline::compileShaders vkCreateShaderModule is failed. Error: %s", ErrorString(result).c_str());
            return false;
        }
        m_modules[type] = module;
    }

    return true;
}

void VulkanGraphicPipeline::deleteShaderModules()
{
    for (auto& module : m_modules)
    {
        VulkanWrapper::DestroyShaderModule(m_device.getDeviceInfo()._device, module, VULKAN_ALLOCATOR);
    }
    m_modules.fill(VK_NULL_HANDLE);
}

bool VulkanGraphicPipeline::createCompatibilityRenderPass(const RenderPassDesc& renderpassDesc, VulkanRenderPass* &compatibilityRenderPass)
{
    RenderPassDesc compatibilityRenderpassDesc(RenderPipeline::createCompatibilityRenderPassDescription(renderpassDesc));
    compatibilityRenderPass = m_renderpassManager->acquireRenderpass(compatibilityRenderpassDesc);
    if (!compatibilityRenderPass)
    {
        LOG_ERROR("VulkanGraphicPipeline::createCompatibilityRenderPass couldn't create renderpass for pipline");
        return false;
    }

    m_trackerRenderPass.attach(compatibilityRenderPass);
    return true;
}

bool VulkanGraphicPipeline::pipelineStatistic() const
{
    if (m_device.getVulkanDeviceCaps()._pipelineExecutablePropertiesEnabled)
    {
        VkPipelineExecutableInfoKHR executableInfo = {};
        executableInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_EXECUTABLE_INFO_KHR;
        executableInfo.pNext = nullptr;
        executableInfo.pipeline = m_pipeline;
        executableInfo.executableIndex = 0;

        u32 statisticCount = 0;
        {
            VkResult result = VulkanWrapper::GetPipelineExecutableStatistics(m_device.getDeviceInfo()._device, &executableInfo, &statisticCount, nullptr);
            if (result != VK_SUCCESS)
            {
                LOG_WARNING("VulkanGraphicPipeline::pipelineStatistic vkGetPipelineExecutableStatistics count is failed. Error: %s", ErrorString(result).c_str());
                return false;
            }
        }
        LOG_DEBUG("VulkanGraphicPipeline::pipelineStatistic: PipelineExecutableStatistics count %u", statisticCount);

        if (statisticCount > 0)
        {
            std::vector<VkPipelineExecutableStatisticKHR> statistics;
            statistics.resize(statisticCount, { VK_STRUCTURE_TYPE_PIPELINE_EXECUTABLE_STATISTIC_KHR, nullptr });
            VkResult result = VulkanWrapper::GetPipelineExecutableStatistics(m_device.getDeviceInfo()._device, &executableInfo, &statisticCount, statistics.data());
            if (result != VK_SUCCESS)
            {
                LOG_WARNING("VulkanGraphicPipeline::pipelineStatistic vkGetPipelineExecutableStatistics is failed. Error: %s", ErrorString(result).c_str());
                return false;
            }

            for (auto& statistic : statistics)
            {
                switch (statistic.format)
                {
                case VK_PIPELINE_EXECUTABLE_STATISTIC_FORMAT_BOOL32_KHR:
                case VK_PIPELINE_EXECUTABLE_STATISTIC_FORMAT_INT64_KHR:
                case VK_PIPELINE_EXECUTABLE_STATISTIC_FORMAT_UINT64_KHR:
                default:
                    LOG_DEBUG("VulkanGraphicPipeline::pipelineStatistic: [%s] %s: %u", statistic.name, statistic.description, statistic.value.i64);
                    break;

                case VK_PIPELINE_EXECUTABLE_STATISTIC_FORMAT_FLOAT64_KHR:
                    LOG_DEBUG("VulkanGraphicPipeline::pipelineStatistic: [%s] %s: %f", statistic.name, statistic.description, statistic.value.f64);
                    break;
                }
            }
        }

        return true;
    }

    return false;
}



VulkanGraphicPipelineManager::VulkanGraphicPipelineManager(VulkanDevice* device) noexcept
    : m_device(*device)
{
}

VulkanGraphicPipelineManager::~VulkanGraphicPipelineManager()
{
    VulkanGraphicPipelineManager::clear();
}

VulkanGraphicPipeline* VulkanGraphicPipelineManager::acquireGraphicPipeline(const GraphicsPipelineState& state)
{
    std::lock_guard lock(m_mutex);

    VulkanPipelineDesc desc(state.getPipelineStateDesc(), state.getRenderPassDesc(), state.getShaderProgram());
    DescInfo<VulkanPipelineDesc> key(desc);

    auto found = m_pipelineGraphicList.find(desc);
    if (found != m_pipelineGraphicList.cend())
    {
        return found->second;
    }

    VulkanGraphicPipeline* pipeline = V3D_NEW(VulkanGraphicPipeline, memory::MemoryLabel::MemoryRenderCore)(&m_device, m_device.getRenderpassManager(), m_device.getPipelineLayoutManager(), state.getName());
    if (!pipeline->create(state))
    {
        ASSERT(false, "can't create pipeline");
        pipeline->destroy();
        V3D_FREE(pipeline, memory::MemoryLabel::MemoryRenderCore);

        return nullptr;
    }
    [[maybe_unused]] auto inserted = m_pipelineGraphicList.emplace(key, pipeline);
    ASSERT(inserted.second, "must be valid insertion");

    return pipeline;
}

bool VulkanGraphicPipelineManager::removePipeline(VulkanGraphicPipeline* pipeline)
{
    ASSERT(pipeline->getType() == RenderPipeline::PipelineType::PipelineType_Graphic, "wrong type");
    std::lock_guard lock(m_mutex);

    auto found = std::find_if(m_pipelineGraphicList.begin(), m_pipelineGraphicList.end(), [pipeline](auto& elem) -> bool
        {
            return elem.second == pipeline;
        });
    if (found == m_pipelineGraphicList.cend())
    {
        LOG_DEBUG("VulkanGraphicPipelineManager pipeline not found");
        ASSERT(false, "pipeline");
        return false;
    }

    ASSERT(found->second == pipeline, "Different pointers");
    if (pipeline->linked())
    {
        LOG_WARNING("VulkanGraphicPipelineManager::removePipeline pipleline still linked, but reqested to delete");
        ASSERT(false, "pipeline");
        //return false;
    }
    m_pipelineGraphicList.erase(found);

    pipeline->destroy();
    V3D_DELETE(pipeline, memory::MemoryLabel::MemoryRenderCore);

    return true;
}

void VulkanGraphicPipelineManager::clear()
{
    std::lock_guard lock(m_mutex);

    for (auto& iter : m_pipelineGraphicList)
    {
        VulkanGraphicPipeline* pipeline = iter.second;
        if (pipeline->linked())
        {
            LOG_WARNING("VulkanGraphicPipelineManager::clear pipleline still linked, but reqested to delete");
            ASSERT(false, "pipeline");
        }

        pipeline->destroy();
        V3D_DELETE(pipeline, memory::MemoryLabel::MemoryRenderCore);
    }
    m_pipelineGraphicList.clear();
}

} //namespace vk
} //namespace renderer
} //namespace v3d
#endif //VULKAN_RENDER
