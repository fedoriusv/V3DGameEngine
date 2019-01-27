#include "VulkanDescriptorSet.h"
#include "VulkanDebug.h"
#include "VulkanDeviceCaps.h"

#include "Resource/Shader.h"

#include "Utils/Logger.h"


#ifdef VULKAN_RENDER
namespace v3d
{
namespace renderer
{
namespace vk
{

VkShaderStageFlagBits DescriptorSetManager::convertShaderTypeToVkStage(resource::ShaderType type)
{
    switch (type)
    {
    case resource::ShaderType_Vertex:
        return VK_SHADER_STAGE_VERTEX_BIT;

    case resource::ShaderType_Fragment:
        return VK_SHADER_STAGE_FRAGMENT_BIT;

    default:
        break;
    }

    ASSERT(false, "not faund");
    return VK_SHADER_STAGE_VERTEX_BIT;
}

DescriptorSetManager::DescriptorSetManager(VkDevice device) noexcept
    : m_device(device)
{
}

void DescriptorSetManager::createPipelineLayout()
{
    VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = {};
    pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutCreateInfo.pNext = nullptr;
    pipelineLayoutCreateInfo.flags = 0;

    //std::vector<VkDescriptorSetLayout> 

    //pipelineLayoutCreateInfo.setLayoutCount;
    //pipelineLayoutCreateInfo.pSetLayouts;
    ////if push const
    //pipelineLayoutCreateInfo.pushConstantRangeCount;
    //pipelineLayoutCreateInfo.pPushConstantRanges;


    VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
    VkResult result = VulkanWrapper::CreatePipelineLayout(m_device, &pipelineLayoutCreateInfo, VULKAN_ALLOCATOR, &pipelineLayout);
    if (result != VK_SUCCESS)
    {

        //return false;
    }
}

VkDescriptorSetLayout DescriptorSetManager::createDescriptorSetLayout(const std::vector<resource::Shader*> shaders)
{
    auto findShaderByType = [](const std::vector<resource::Shader*>& shaders, resource::ShaderType type) -> resource::Shader*
    {
        for (auto& shader : shaders)
        {
            if (shader->getShaderHeader()._type == type)
            {
                return shader;
            }
        }

        return nullptr;
    };

    VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo = {};
    descriptorSetLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    descriptorSetLayoutCreateInfo.pNext = nullptr; //VkDescriptorSetLayoutBindingFlagsCreateInfoEXT
    descriptorSetLayoutCreateInfo.flags = 0; //VK_DESCRIPTOR_SET_LAYOUT_CREATE_PUSH_DESCRIPTOR_BIT_KHR

    std::vector<VkDescriptorSetLayoutBinding> descriptorSetLayoutBindings;
    for (u32 type = resource::ShaderType::ShaderType_Vertex; type < resource::ShaderType_Count; ++type)
    {
        resource::Shader* shader = findShaderByType(shaders, (resource::ShaderType)type);
        if (!shader)
        {
            continue;
        }

        //const resource::Shader::ReflectionInfo info; //TODO:get shader
        //for (auto& uniform : info._uniformBuffers)
        //{
        //    VkDescriptorSetLayoutBinding descriptorSetLayoutBinding;
        //    descriptorSetLayoutBinding.descriptorType = VulkanDeviceCaps::getInstance()->useDynamicUniforms ? VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC : VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        //    descriptorSetLayoutBinding.binding = uniform._binding;
        //    descriptorSetLayoutBinding.stageFlags = convertShaderTypeToVkStage((resource::ShaderType)type);
        //    descriptorSetLayoutBinding.descriptorCount = 1;
        //    descriptorSetLayoutBinding.pImmutableSamplers = nullptr;

        //    descriptorSetLayoutBindings.push_back(descriptorSetLayoutBinding);
        //}

        //for (auto& image : info._sampledImages)
        //{
        //    VkDescriptorSetLayoutBinding descriptorSetLayoutBinding;
        //    descriptorSetLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER; //VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE
        //    descriptorSetLayoutBinding.binding = image._binding;
        //    descriptorSetLayoutBinding.stageFlags = convertShaderTypeToVkStage((resource::ShaderType)type);
        //    descriptorSetLayoutBinding.descriptorCount = 1;
        //    descriptorSetLayoutBinding.pImmutableSamplers = nullptr;

        //    descriptorSetLayoutBindings.push_back(descriptorSetLayoutBinding);
        //}

        //TODO:
    }
    descriptorSetLayoutCreateInfo.bindingCount = static_cast<u32>(descriptorSetLayoutBindings.size());
    descriptorSetLayoutCreateInfo.pBindings = descriptorSetLayoutBindings.data();

    VkDescriptorSetLayout descriptorSetLayout = VK_NULL_HANDLE;
    VkResult result = VulkanWrapper::CreateDescriptorSetLayout(m_device, &descriptorSetLayoutCreateInfo, VULKAN_ALLOCATOR, &descriptorSetLayout);
    if (result != VK_SUCCESS)
    {
        LOG_ERROR("DescriptorSet::createDescriptorSetLayout vkCreateDescriptorSetLayout is failed. Error: %s", ErrorString(result).c_str());
        return VK_NULL_HANDLE;
    }

    return descriptorSetLayout;
}

} //namespace vk
} //namespace renderer
} //namespace v3d
#endif //VULKAN_RENDER
