#include "VulkanComputePipeline.h"

#include "Utils/Logger.h"
#include "Renderer/Shader.h"
#include "Renderer/ShaderProgram.h"

#ifdef VULKAN_RENDER
#   include "VulkanDebug.h"
#   include "VulkanDeviceCaps.h"
#   include "VulkanDevice.h"
#   include "VulkanGraphicPipeline.h"

namespace v3d
{
namespace renderer
{
namespace vk
{

VulkanComputePipeline::VulkanComputePipeline(VulkanDevice* device, VulkanPipelineLayoutManager* pipelineLayoutManager, const std::string& name) noexcept
    : RenderPipeline(PipelineType::PipelineType_Compute)
    , m_device(*device)
    , m_pipelineLayoutManager(pipelineLayoutManager)

    , m_pipeline(VK_NULL_HANDLE)
    , m_module(VK_NULL_HANDLE)

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

bool VulkanComputePipeline::createShaderModules(const renderer::ShaderProgram* program)
{
    const Shader* computeShader = program->getShader(ShaderType::Compute);
    if (!computeShader)
    {
        ASSERT(false, "nullptr");
        return false;
    }

    VkShaderModuleCreateInfo shaderModuleCreateInfo = {};
    shaderModuleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    shaderModuleCreateInfo.pNext = nullptr; //VkShaderModuleValidationCacheCreateInfoEXT
    shaderModuleCreateInfo.flags = 0;
    shaderModuleCreateInfo.pCode = reinterpret_cast<const u32*>(computeShader->getBytecode());
    shaderModuleCreateInfo.codeSize = computeShader->getBytecodeSize();

    ASSERT(m_module == VK_NULL_HANDLE, "Is not null handle");
    VkResult result = VulkanWrapper::CreateShaderModule(m_device.getDeviceInfo()._device, &shaderModuleCreateInfo, VULKAN_ALLOCATOR, &m_module);
    if (result != VK_SUCCESS)
    {
        LOG_ERROR("VulkanComputePipeline::createShaderModules vkCreateShaderModule is failed. Error: %s", ErrorString(result).c_str());
        return false;
    }

    return true;
}

void VulkanComputePipeline::deleteShaderModules()
{
    if (m_module)
    {
        VulkanWrapper::DestroyShaderModule(m_device.getDeviceInfo()._device, m_module, VULKAN_ALLOCATOR);
        m_module = VK_NULL_HANDLE;
    }
}

bool VulkanComputePipeline::create(const ComputePipelineState& state)
{
    ASSERT(getType() == PipelineType::PipelineType_Compute, "invalid type");

    VkComputePipelineCreateInfo computePipelineCreateInfo = {};
    computePipelineCreateInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
    computePipelineCreateInfo.pNext = nullptr; //VkPipelineCreationFeedbackCreateInfoEXT
    computePipelineCreateInfo.flags = 0;
#if VULKAN_DEBUG
    if (m_device.getVulkanDeviceCaps()._pipelineExecutablePropertiesEnabled)
    {
        computePipelineCreateInfo.flags |= VK_PIPELINE_CREATE_CAPTURE_STATISTICS_BIT_KHR;
    }
#endif //VULKAN_DEBUG

    if (!VulkanComputePipeline::createShaderModules(state.getShaderProgram()))
    {
        LOG_ERROR("VulkanComputePipeline::create: shader module is failed");
        VulkanComputePipeline::destroy();
        return false;
    }

    VulkanPipelineLayoutManager::DescriptorSetLayoutCreator layoutDesc(m_device, state.getShaderProgram());
    m_pipelineLayoutDescription = layoutDesc._description;
    m_pipelineLayout = m_pipelineLayoutManager->acquirePipelineLayout(m_pipelineLayoutDescription);
    computePipelineCreateInfo.layout = m_pipelineLayout._pipelineLayout;

    const Shader* computeShader = state.getShaderProgram()->getShader(ShaderType::Compute);
    VkPipelineShaderStageCreateInfo pipelineShaderStageCreateInfo = {};
    {
        pipelineShaderStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        pipelineShaderStageCreateInfo.pNext = nullptr;
        pipelineShaderStageCreateInfo.flags = 0;
        pipelineShaderStageCreateInfo.pSpecializationInfo = nullptr;
        pipelineShaderStageCreateInfo.stage = VulkanGraphicPipeline::convertShaderTypeToVkStage(ShaderType::Compute);
        pipelineShaderStageCreateInfo.module = m_module;
        pipelineShaderStageCreateInfo.pName = computeShader->getEntryPoint().c_str();
    }
    computePipelineCreateInfo.stage = pipelineShaderStageCreateInfo;


    computePipelineCreateInfo.basePipelineIndex = 0;
    computePipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE;

    VkPipelineCache pipelineCache = VK_NULL_HANDLE;

    VkResult result = VulkanWrapper::CreateComputePipelines(m_device.getDeviceInfo()._device, pipelineCache, 1, &computePipelineCreateInfo, VULKAN_ALLOCATOR, &m_pipeline);
    if (result != VK_SUCCESS)
    {
        LOG_ERROR("VulkanComputePipeline::create vkCreateComputePipelines is failed. Error: %s", ErrorString(result).c_str());
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
    VulkanComputePipeline::pipelineStatistic();
#endif
    return true;
}

void VulkanComputePipeline::destroy()
{
    if (m_pipeline)
    {
        VulkanWrapper::DestroyPipeline(m_device.getDeviceInfo()._device, m_pipeline, VULKAN_ALLOCATOR);
        m_pipeline = VK_NULL_HANDLE;
    }
    VulkanComputePipeline::deleteShaderModules();

    if (m_pipelineLayout._pipelineLayout)
    {
        //release when manager will be destroyed
        //m_descriptorSetManager->removePipelineLayout(m_pipelineLayout);
        m_pipelineLayout._pipelineLayout = VK_NULL_HANDLE;
    }
}

bool VulkanComputePipeline::pipelineStatistic() const
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
                LOG_WARNING("VulkanComputePipeline::pipelineStatistic vkGetPipelineExecutableStatistics count is failed. Error: %s", ErrorString(result).c_str());
                return false;
            }
        }
        LOG_DEBUG("VulkanComputePipeline::pipelineStatistic: PipelineExecutableStatistics count %u", statisticCount);

        if (statisticCount > 0)
        {
            std::vector<VkPipelineExecutableStatisticKHR> statistics;
            statistics.resize(statisticCount, { VK_STRUCTURE_TYPE_PIPELINE_EXECUTABLE_STATISTIC_KHR, nullptr });
            VkResult result = VulkanWrapper::GetPipelineExecutableStatistics(m_device.getDeviceInfo()._device, &executableInfo, &statisticCount, statistics.data());
            if (result != VK_SUCCESS)
            {
                LOG_WARNING("VulkanComputePipeline::pipelineStatistic vkGetPipelineExecutableStatistics is failed. Error: %s", ErrorString(result).c_str());
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
                    LOG_DEBUG("VulkanComputePipeline::pipelineStatistic: [%s] %s: %u", statistic.name, statistic.description, statistic.value.i64);
                    break;

                case VK_PIPELINE_EXECUTABLE_STATISTIC_FORMAT_FLOAT64_KHR:
                    LOG_DEBUG("VulkanComputePipeline::pipelineStatistic: [%s] %s: %f", statistic.name, statistic.description, statistic.value.f64);
                    break;
                }
            }
        }

        return true;
    }

    return false;
}


VulkanComputePipelineManager::VulkanComputePipelineManager(VulkanDevice* device) noexcept
    : m_device(*device)
{
}

VulkanComputePipelineManager::~VulkanComputePipelineManager()
{
    VulkanComputePipelineManager::clear();
}

VulkanComputePipeline* VulkanComputePipelineManager::acquireGraphicPipeline(const ComputePipelineState& state)
{
    std::scoped_lock lock(m_mutex);

    VulkanComputePipeline* pipeline = nullptr;
    VulkanPipelineDesc desc(state.getShaderProgram());

    auto found = m_pipelineComputeList.emplace(desc, pipeline);
    if (found.second)
    {
        pipeline = V3D_NEW(VulkanComputePipeline, memory::MemoryLabel::MemoryRenderCore)(&m_device, m_device.getPipelineLayoutManager(), state.getName());
        if (!pipeline->create(state))
        {
            m_pipelineComputeList.erase(desc);

            ASSERT(false, "can't create pipeline");
            pipeline->destroy();
            V3D_FREE(pipeline, memory::MemoryLabel::MemoryRenderCore);

            return nullptr;
        }

        found.first->second = pipeline;
        return pipeline;
    }

    return found.first->second;
}

bool VulkanComputePipelineManager::removePipeline(VulkanComputePipeline* pipeline)
{
    ASSERT(pipeline->getType() == RenderPipeline::PipelineType::PipelineType_Graphic, "wrong type");
    std::scoped_lock lock(m_mutex);

    auto found = std::find_if(m_pipelineComputeList.begin(), m_pipelineComputeList.end(), [pipeline](auto& elem) -> bool
        {
            return elem.second == pipeline;
        });
    if (found == m_pipelineComputeList.cend())
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
    m_pipelineComputeList.erase(found);

    pipeline->destroy();
    V3D_DELETE(pipeline, memory::MemoryLabel::MemoryRenderCore);

    return true;
}

void VulkanComputePipelineManager::clear()
{
    std::scoped_lock lock(m_mutex);

    for (auto& iter : m_pipelineComputeList)
    {
        VulkanComputePipeline* pipeline = iter.second;
        if (pipeline->linked())
        {
            LOG_WARNING("VulkanGraphicPipelineManager::clear pipleline still linked, but reqested to delete");
            ASSERT(false, "pipeline");
        }

        pipeline->destroy();
        V3D_DELETE(pipeline, memory::MemoryLabel::MemoryRenderCore);
    }
    m_pipelineComputeList.clear();
}

} //namespace vk
} //namespace renderer
} //namespace v3d
#endif //VULKAN_RENDER