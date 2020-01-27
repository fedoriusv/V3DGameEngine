#include "VulkanPipelineLayout.h"
#include "VulkanDebug.h"
#include "VulkanDeviceCaps.h"
#include "Renderer/Shader.h"

#include "Utils/Logger.h"

#include "crc32c/crc32c.h"

#ifdef VULKAN_RENDER
namespace v3d
{
namespace renderer
{
namespace vk
{

VulkanPipelineLayout::VulkanPipelineLayout() noexcept
    : _key(0)
    , _layout(VK_NULL_HANDLE)
{
    _descriptorSetLayouts.fill(VK_NULL_HANDLE);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////

VkShaderStageFlagBits VulkanPipelineLayoutManager::convertShaderTypeToVkStage(ShaderType type)
{
    switch (type)
    {
    case ShaderType_Vertex:
        return VK_SHADER_STAGE_VERTEX_BIT;

    case ShaderType_Fragment:
        return VK_SHADER_STAGE_FRAGMENT_BIT;

    default:
        break;
    }

    ASSERT(false, "not faund");
    return VK_SHADER_STAGE_VERTEX_BIT;
}

VulkanPipelineLayoutManager::VulkanPipelineLayoutManager(VkDevice device) noexcept
    : m_device(device)
{
}

VulkanPipelineLayoutManager::~VulkanPipelineLayoutManager()
{
    ASSERT(m_pipelinesLayouts.empty(), "not empty");
}

VulkanPipelineLayout VulkanPipelineLayoutManager::acquirePipelineLayout(const DescriptorSetDescription& desc)
{
    u64 hash = desc._hash;

    auto found = m_pipelinesLayouts.emplace(hash, VulkanPipelineLayout());
    if (found.second)
    {
        VulkanPipelineLayout& layout = found.first->second;
        layout._key = hash;
        layout._layout = VulkanPipelineLayoutManager::createPipelineLayout(desc, layout._descriptorSetLayouts);

        return layout;
    }

    return found.first->second;
}

bool VulkanPipelineLayoutManager::removePipelineLayout(const DescriptorSetDescription & desc)
{
    auto iter = m_pipelinesLayouts.find(desc._hash);
    if (iter == m_pipelinesLayouts.cend())
    {
        LOG_DEBUG("removePipelineLayout PipelineLayout not found");
        ASSERT(false, "PipelineLayout");
        return false;
    }

    VulkanPipelineLayout& layout = iter->second;
    VulkanPipelineLayoutManager::destroyPipelineLayout(layout._layout, layout._descriptorSetLayouts);

    m_pipelinesLayouts.erase(desc._hash);

    return true;
}

bool VulkanPipelineLayoutManager::removePipelineLayout(VulkanPipelineLayout & layout)
{
    auto iter = m_pipelinesLayouts.find(layout._key);
    if (iter == m_pipelinesLayouts.cend())
    {
        LOG_DEBUG("removePipelineLayout PipelineLayout not found");
        ASSERT(false, "PipelineLayout");
        return false;
    }

    VulkanPipelineLayout& pipelinelayout = iter->second;
    VulkanPipelineLayoutManager::destroyPipelineLayout(pipelinelayout._layout, pipelinelayout._descriptorSetLayouts);

    m_pipelinesLayouts.erase(layout._key);

    return true;
}

void VulkanPipelineLayoutManager::clear()
{
    for (auto& iter : m_pipelinesLayouts)
    {
        VulkanPipelineLayoutManager::destroyPipelineLayout(iter.second._layout, iter.second._descriptorSetLayouts);
    }
    m_pipelinesLayouts.clear();
}


VkPipelineLayout VulkanPipelineLayoutManager::createPipelineLayout(const DescriptorSetDescription& desc, DescriptorSetLayouts& descriptorSetLayouts)
{
    if (!VulkanPipelineLayoutManager::createDescriptorSetLayouts(desc, descriptorSetLayouts))
    {
        ASSERT(false, "error");
        VulkanPipelineLayoutManager::destroyDescriptorSetLayouts(descriptorSetLayouts);

        return VK_NULL_HANDLE;
    }

    std::vector<VkDescriptorSetLayout> vkDescriptorSetLayouts;
    vkDescriptorSetLayouts.reserve(k_maxDescriptorSetIndex);
    for (auto& set : descriptorSetLayouts)
    {
        if (set != VK_NULL_HANDLE)
        {
            vkDescriptorSetLayouts.push_back(set);
        }
    }

    VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = {};
    pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutCreateInfo.pNext = nullptr;
    pipelineLayoutCreateInfo.flags = 0;
    pipelineLayoutCreateInfo.setLayoutCount = static_cast<u32>(vkDescriptorSetLayouts.size());
    pipelineLayoutCreateInfo.pSetLayouts = vkDescriptorSetLayouts.data();
    pipelineLayoutCreateInfo.pushConstantRangeCount = static_cast<u32>(desc._pushConstant.size());
    pipelineLayoutCreateInfo.pPushConstantRanges = desc._pushConstant.data();

    VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
    VkResult result = VulkanWrapper::CreatePipelineLayout(m_device, &pipelineLayoutCreateInfo, VULKAN_ALLOCATOR, &pipelineLayout);
    if (result != VK_SUCCESS)
    {
        LOG_ERROR("VulkanDescriptorSetManager::createPipelineLayout vkCreatePipelineLayout is failed. Error: %s", ErrorString(result).c_str());
        return VK_NULL_HANDLE;
    }

    return pipelineLayout;
}

void VulkanPipelineLayoutManager::destroyPipelineLayout(VkPipelineLayout layout, DescriptorSetLayouts& descriptorSetLayouts)
{
    VulkanPipelineLayoutManager::destroyDescriptorSetLayouts(descriptorSetLayouts);
    VulkanWrapper::DestroyPipelineLayout(m_device, layout, VULKAN_ALLOCATOR);
}

bool VulkanPipelineLayoutManager::createDescriptorSetLayouts(const DescriptorSetDescription& desc, DescriptorSetLayouts& descriptorSetLayouts)
{
    descriptorSetLayouts.fill(VK_NULL_HANDLE);
    for (u32 setId = 0; setId < k_maxDescriptorSetIndex; ++setId)
    {
        auto& set = desc._descriptorSets[setId];
        if (set.empty())
        {
            descriptorSetLayouts[setId] = VK_NULL_HANDLE;
            continue;
        }

        void* vkExtensions = nullptr;

        VkDescriptorSetLayoutBindingFlagsCreateInfoEXT descriptorSetLayoutBindingFlagsCreateInfo = {};
        std::vector<VkDescriptorBindingFlagsEXT> descriptorBindingFlags(set.size(), 0);
        if (VulkanDeviceCaps::getInstance()->useDynamicUniforms)
        {
            for (u32 i = 0; i < set.size(); ++i)
            {
                if (set[i].descriptorType == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC)
                {
                    descriptorBindingFlags[i] |= VK_DESCRIPTOR_BINDING_UPDATE_UNUSED_WHILE_PENDING_BIT_EXT;
                }
            }

            descriptorSetLayoutBindingFlagsCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO_EXT;
            descriptorSetLayoutBindingFlagsCreateInfo.pNext = nullptr;
            descriptorSetLayoutBindingFlagsCreateInfo.pBindingFlags = descriptorBindingFlags.data();
            descriptorSetLayoutBindingFlagsCreateInfo.bindingCount = static_cast<u32>(descriptorBindingFlags.size());

            vkExtensions = &descriptorSetLayoutBindingFlagsCreateInfo;
        }

        VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo = {};
        descriptorSetLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        descriptorSetLayoutCreateInfo.pNext = vkExtensions;
        descriptorSetLayoutCreateInfo.flags = 0; //VK_DESCRIPTOR_SET_LAYOUT_CREATE_PUSH_DESCRIPTOR_BIT_KHR
        //descriptorSetLayoutCreateInfo.flags = VulkanDeviceCaps::getInstance()->useDynamicUniforms ? VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT_EXT : 0;
        descriptorSetLayoutCreateInfo.bindingCount = static_cast<u32>(set.size());
        descriptorSetLayoutCreateInfo.pBindings = set.data();

        VkDescriptorSetLayout descriptorSetLayout = VK_NULL_HANDLE;
        VkResult result = VulkanWrapper::CreateDescriptorSetLayout(m_device, &descriptorSetLayoutCreateInfo, VULKAN_ALLOCATOR, &descriptorSetLayout);
        if (result != VK_SUCCESS)
        {
            LOG_ERROR("DescriptorSetManager::createDescriptorSetLayout vkCreateDescriptorSetLayout is failed. Error: %s", ErrorString(result).c_str());
            return false;
        }

        descriptorSetLayouts[setId] = descriptorSetLayout;
    }

    return true;
}

void VulkanPipelineLayoutManager::destroyDescriptorSetLayouts(DescriptorSetLayouts& descriptorSetLayouts)
{
    for (auto& set : descriptorSetLayouts)
    {
        if (set != VK_NULL_HANDLE)
        {
            VulkanWrapper::DestroyDescriptorSetLayout(m_device, set, VULKAN_ALLOCATOR);
        }
    }
    descriptorSetLayouts.fill(VK_NULL_HANDLE);
}

VulkanPipelineLayoutManager::DescriptorSetDescription::DescriptorSetDescription(const std::array<const Shader*, ShaderType::ShaderType_Count>& shaders) noexcept
    : _hash(0)
{
    _descriptorSets.fill({});

    u32 maxSetIndex = 0;
    for (u32 setIndex = 0; setIndex < k_maxDescriptorSetIndex; ++setIndex)
    {
        std::vector<VkDescriptorSetLayoutBinding> descriptorSetLayoutBindings;
        for (u32 type = ShaderType::ShaderType_Vertex; type < ShaderType_Count; ++type)
        {
            const Shader* shader = shaders[type];
            if (!shader)
            {
                continue;
            }

            const Shader::ReflectionInfo& info = shader->getReflectionInfo();
            for (auto& uniform : info._uniformBuffers)
            {
                ASSERT(uniform._set < k_maxDescriptorSetIndex && uniform._binding < k_maxDescriptorBindingIndex, "range out");
                if (uniform._set != setIndex)
                {
                    continue;
                }

                VkDescriptorSetLayoutBinding descriptorSetLayoutBinding;
                descriptorSetLayoutBinding.descriptorType = VulkanDeviceCaps::getInstance()->useDynamicUniforms ? VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC : VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
                descriptorSetLayoutBinding.binding = uniform._binding;
                descriptorSetLayoutBinding.stageFlags = convertShaderTypeToVkStage((ShaderType)type);
                descriptorSetLayoutBinding.descriptorCount = uniform._array;
                descriptorSetLayoutBinding.pImmutableSamplers = nullptr;

                descriptorSetLayoutBindings.push_back(descriptorSetLayoutBinding);
            }

            for (auto& image : info._sampledImages)
            {
                if (image._set != setIndex)
                {
                    continue;
                }

                VkDescriptorSetLayoutBinding descriptorSetLayoutBinding;
                descriptorSetLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER; //VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE
                descriptorSetLayoutBinding.binding = image._binding;
                descriptorSetLayoutBinding.stageFlags = convertShaderTypeToVkStage((ShaderType)type);
                descriptorSetLayoutBinding.descriptorCount = image._array;
                descriptorSetLayoutBinding.pImmutableSamplers = nullptr;

                descriptorSetLayoutBindings.push_back(descriptorSetLayoutBinding);
            }
        }

        if (!descriptorSetLayoutBindings.empty())
        {
            maxSetIndex = std::max(maxSetIndex, setIndex);
            _descriptorSets[setIndex] = std::move(descriptorSetLayoutBindings);
        }
    }
    ASSERT(maxSetIndex < k_maxDescriptorSetIndex, "invalid max set index");

    for (u32 type = ShaderType::ShaderType_Vertex; type < ShaderType_Count; ++type)
    {
        const Shader* shader = shaders[type];
        if (!shader)
        {
            continue;
        }

        const Shader::ReflectionInfo& info = shader->getReflectionInfo();
        _pushConstant.reserve(info._pushConstant.size());
        for (auto& push : info._pushConstant)
        {
            VkPushConstantRange pushConstantRange = {};
            pushConstantRange.stageFlags = convertShaderTypeToVkStage((ShaderType)type);
            pushConstantRange.offset = push._offset;
            pushConstantRange.size = push._size;

            _pushConstant.push_back(pushConstantRange);
        }
    }

    u64 pushConstantHash = crc32c::Extend(static_cast<u32>(_pushConstant.size()), reinterpret_cast<u8*>(_pushConstant.data()), _pushConstant.size() * sizeof(VkPushConstantRange));

    u32 setHash = static_cast<u32>(_descriptorSets.size());
    for (auto& set : _descriptorSets)
    {
        u32 bindingCount = static_cast<u32>(set.size());
        setHash = crc32c::Extend(setHash, reinterpret_cast<u8*>(&bindingCount), sizeof(u32));
        setHash = crc32c::Extend(setHash, reinterpret_cast<u8*>(set.data()), set.size() * sizeof(VkDescriptorSetLayoutBinding));
    }
    _hash = setHash | pushConstantHash << 32;
}

} //namespace vk
} //namespace renderer
} //namespace v3d
#endif //VULKAN_RENDER
