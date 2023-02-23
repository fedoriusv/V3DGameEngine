#include "VulkanComputePipeline.h"

#include "Utils/Logger.h"
#include "Renderer/Shader.h"

#ifdef VULKAN_RENDER
#include "VulkanDebug.h"
#include "VulkanDeviceCaps.h"
#include "VulkanGraphicPipeline.h"

namespace v3d
{
namespace renderer
{
namespace vk
{

VulkanComputePipeline::VulkanComputePipeline(VkDevice device, VulkanPipelineLayoutManager* pipelineLayoutManager, const std::string& name)
    : Pipeline(PipelineType::PipelineType_Compute)
    , m_device(device)

    , m_pipeline(VK_NULL_HANDLE)
    , m_module(VK_NULL_HANDLE)

    , m_pipelineLayoutManager(pipelineLayoutManager)
{
    LOG_DEBUG("VulkanComputePipeline::VulkanComputePipeline constructor %llx", this);

#if VULKAN_DEBUG_MARKERS
    m_debugName = name.empty() ? "ComputePipeline" : name;
    m_debugName.append(VulkanDebugUtils::k_addressPreffix);
    m_debugName.append(std::to_string(reinterpret_cast<const u64>(this)));
#endif //VULKAN_DEBUG_MARKERS
}

VulkanComputePipeline::~VulkanComputePipeline()
{
    LOG_DEBUG("VulkanComputePipeline::VulkanComputePipeline destructor %llx", this);
    ASSERT(m_pipeline == VK_NULL_HANDLE, "not deleted");
}

bool VulkanComputePipeline::create(const PipelineComputeInfo* pipelineInfo)
{
    ASSERT(getType() == PipelineType::PipelineType_Compute, "invalid type");
    ASSERT(pipelineInfo, "nullptr");

    VkComputePipelineCreateInfo computePipelineCreateInfo = {};
    computePipelineCreateInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
    computePipelineCreateInfo.pNext = nullptr; //VkPipelineCreationFeedbackCreateInfoEXT
    computePipelineCreateInfo.flags = 0;
#if VULKAN_DEBUG
    if (VulkanDeviceCaps::getInstance()->pipelineExecutablePropertiesEnabled)
    {
        computePipelineCreateInfo.flags |= VK_PIPELINE_CREATE_CAPTURE_STATISTICS_BIT_KHR;
    }
#endif //VULKAN_DEBUG

    const Shader* computeShader = pipelineInfo->_programDesc._shaders[toEnumType(ShaderType::Compute)];
    if (!computeShader || !Pipeline::createShader(computeShader))
    {
        LOG_ERROR("VulkanComputePipeline::create: shader module is failed");
        VulkanComputePipeline::destroy();
        return false;
    }

    VulkanPipelineLayoutManager::DescriptorSetLayoutCreator layoutDesc(pipelineInfo->_programDesc._shaders);
    m_pipelineLayoutDescription = layoutDesc._description;
    m_pipelineLayout = m_pipelineLayoutManager->acquirePipelineLayout(m_pipelineLayoutDescription);
    computePipelineCreateInfo.layout = m_pipelineLayout._pipelineLayout;

    VkPipelineShaderStageCreateInfo pipelineShaderStageCreateInfo = {};
    {
        pipelineShaderStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        pipelineShaderStageCreateInfo.pNext = nullptr;
        pipelineShaderStageCreateInfo.flags = 0;
        pipelineShaderStageCreateInfo.pSpecializationInfo = nullptr;
        pipelineShaderStageCreateInfo.stage = VulkanGraphicPipeline::convertShaderTypeToVkStage(ShaderType::Compute);
        pipelineShaderStageCreateInfo.module = m_module;
        pipelineShaderStageCreateInfo.pName = computeShader->getShaderHeader()._entryPoint.c_str();
    }
    computePipelineCreateInfo.stage = pipelineShaderStageCreateInfo;


    computePipelineCreateInfo.basePipelineIndex = 0;
    computePipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE;

    VkPipelineCache pipelineCache = VK_NULL_HANDLE;

    VkResult result = VulkanWrapper::CreateComputePipelines(m_device, pipelineCache, 1, &computePipelineCreateInfo, VULKAN_ALLOCATOR, &m_pipeline);
    if (result != VK_SUCCESS)
    {
        LOG_ERROR("VulkanComputePipeline::create vkCreateComputePipelines is failed. Error: %s", ErrorString(result).c_str());
        return false;
    }

#if VULKAN_DEBUG_MARKERS
    if (VulkanDeviceCaps::getInstance()->debugUtilsObjectNameEnabled)
    {
        VkDebugUtilsObjectNameInfoEXT debugUtilsObjectNameInfo = {};
        debugUtilsObjectNameInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
        debugUtilsObjectNameInfo.pNext = nullptr;
        debugUtilsObjectNameInfo.objectType = VK_OBJECT_TYPE_PIPELINE;
        debugUtilsObjectNameInfo.objectHandle = reinterpret_cast<u64>(m_pipeline);
        debugUtilsObjectNameInfo.pObjectName = m_debugName.c_str();

        VulkanWrapper::SetDebugUtilsObjectName(m_device, &debugUtilsObjectNameInfo);
}
#endif //VULKAN_DEBUG_MARKERS

#if VULKAN_DEBUG
    VulkanComputePipeline::pipelineStatistic();
#endif
    return true;
}

void VulkanComputePipeline::destroy()
{
    if (m_pipeline)
    {
        VulkanWrapper::DestroyPipeline(m_device, m_pipeline, VULKAN_ALLOCATOR);
        m_pipeline = VK_NULL_HANDLE;
    }

    if (m_module)
    {
        VulkanWrapper::DestroyShaderModule(m_device, m_module, VULKAN_ALLOCATOR);
        m_module = VK_NULL_HANDLE;
    }

    if (m_pipelineLayout._pipelineLayout)
    {
        //release when manager will be destroyed
        //m_descriptorSetManager->removePipelineLayout(m_pipelineLayout);
        m_pipelineLayout._pipelineLayout = VK_NULL_HANDLE;
    }
}

bool VulkanComputePipeline::compileShader(const ShaderHeader* header, const void* source, u32 size)
{
    VkShaderModuleCreateInfo shaderModuleCreateInfo = {};
    shaderModuleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    shaderModuleCreateInfo.pNext = nullptr; //VkShaderModuleValidationCacheCreateInfoEXT
    shaderModuleCreateInfo.flags = 0;
    shaderModuleCreateInfo.pCode = reinterpret_cast<const u32*>(source);
    shaderModuleCreateInfo.codeSize = size;

    ASSERT(m_module == VK_NULL_HANDLE, "Is not null handle");
    VkResult result = VulkanWrapper::CreateShaderModule(m_device, &shaderModuleCreateInfo, VULKAN_ALLOCATOR, &m_module);
    if (result != VK_SUCCESS)
    {
        LOG_ERROR("VulkanComputePipeline::compileShader vkCreateShaderModule is failed. Error: %s", ErrorString(result).c_str());
        return false;
    }

    return true;
}

bool VulkanComputePipeline::create(const PipelineGraphicInfo* pipelineInfo)
{
    ASSERT(false, "cant be graphic");
    return false;
}

bool VulkanComputePipeline::pipelineStatistic() const
{
    if (VulkanDeviceCaps::getInstance()->pipelineExecutablePropertiesEnabled)
    {
        VkPipelineExecutableInfoKHR executableInfo = {};
        executableInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_EXECUTABLE_INFO_KHR;
        executableInfo.pNext = nullptr;
        executableInfo.pipeline = m_pipeline;
        executableInfo.executableIndex = 0;

        u32 statisticCount = 0;
        {
            VkResult result = VulkanWrapper::GetPipelineExecutableStatistics(m_device, &executableInfo, &statisticCount, nullptr);
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
            VkResult result = VulkanWrapper::GetPipelineExecutableStatistics(m_device, &executableInfo, &statisticCount, statistics.data());
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

} //namespace vk
} //namespace renderer
} //namespace v3d
#endif //VULKAN_RENDER