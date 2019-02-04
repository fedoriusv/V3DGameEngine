#include "VulkanDescriptorSet.h"
#include "VulkanDebug.h"
#include "VulkanDeviceCaps.h"

#include "Resource/Shader.h"
#include "Utils/Logger.h"

#include "crc32c/crc32c.h"

#ifdef VULKAN_RENDER
namespace v3d
{
namespace renderer
{
namespace vk
{

VulkanPipelineLayout::VulkanPipelineLayout()
    : _key(0)
    , _layout(VK_NULL_HANDLE)
{
}

VulkanDescriptorPool::VulkanDescriptorPool(VkDevice device, VkDescriptorPoolCreateFlags flag) noexcept
    : m_device(device)
    , m_pool(VK_NULL_HANDLE)
    , m_flag(flag)
{
    ASSERT(m_descriptorSets.empty(), "not empty");
    ASSERT(!m_pool, "not nullptr");
}

bool VulkanDescriptorPool::create(u32 setsCount, const std::vector<VkDescriptorPoolSize>& sizes)
{
    ASSERT(!m_pool, "not nullptr");
    if (!VulkanDescriptorPool::createDescriptorPool(setsCount, sizes))
    {
        return false;
    }

    return true;
}

void VulkanDescriptorPool::destroy()
{
    if (!m_descriptorSets.empty())
    {
        VulkanDescriptorPool::freeDescriptorSet(m_descriptorSets);
        m_descriptorSets.clear();
    }

    if (m_pool)
    {
        VulkanWrapper::DestroyDescriptorPool(m_device, m_pool, VULKAN_ALLOCATOR);
        m_pool = VK_NULL_HANDLE;
    }
}

bool VulkanDescriptorPool::allocateDescriptorSet(const VulkanPipelineLayout& layout, std::vector<VkDescriptorSet>& descriptorSets)
{
    VkDescriptorSetAllocateInfo descriptorSetAllocateInfo = {};
    descriptorSetAllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    descriptorSetAllocateInfo.pNext = nullptr; //VkDescriptorSetVariableDescriptorCountAllocateInfoEXT
    descriptorSetAllocateInfo.descriptorPool = m_pool;
    descriptorSetAllocateInfo.descriptorSetCount = static_cast<u32>(layout._descriptorSetLayouts.size());
    descriptorSetAllocateInfo.pSetLayouts = layout._descriptorSetLayouts.data();

    descriptorSets.resize(layout._descriptorSetLayouts.size(), VK_NULL_HANDLE);
    VkResult result = VulkanWrapper::AllocateDescriptorSets(m_device, &descriptorSetAllocateInfo, descriptorSets.data());
    if (result == VK_ERROR_OUT_OF_POOL_MEMORY || result == VK_ERROR_FRAGMENTED_POOL)
    {
        LOG_ERROR("VulkanDescriptorPool::createDescriptorSet vkAllocateDescriptorSets is failed. Error: %s", ErrorString(result).c_str());
        VulkanDescriptorPool::freeDescriptorSet(descriptorSets);

        return false;
    }
    else if (result != VK_SUCCESS)
    {
        LOG_ERROR("VulkanDescriptorPool::createDescriptorSet vkAllocateDescriptorSets is failed. Error: %s", ErrorString(result).c_str());
        return false;
    }

    m_descriptorSets.insert(m_descriptorSets.cend(), descriptorSets.cbegin(), descriptorSets.cend());
    return true;
}

bool VulkanDescriptorPool::freeDescriptorSet(std::vector<VkDescriptorSet>& descriptorSets)
{
    if (m_flag & ~VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT)
    {
        return false;
    }

    VkResult result = VulkanWrapper::FreeDescriptorSets(m_device, m_pool, static_cast<u32>(descriptorSets.size()), descriptorSets.data());
    if (result != VK_SUCCESS)
    {
        LOG_ERROR("VulkanDescriptorPool::destroyDescriptorSet vkFreeDescriptorSets is failed. Error: %s", ErrorString(result).c_str());
        return false;
    }

    return true;
}

bool VulkanDescriptorPool::reset(VkDescriptorPoolResetFlags flag)
{
    VkResult result = VulkanWrapper::ResetDescriptorPool(m_device, m_pool, flag);
    if (result != VK_SUCCESS)
    {
        LOG_ERROR("VulkanDescriptorPool::reset vkResetDescriptorPool is failed. Error: %s", ErrorString(result).c_str());
        return false;
    }

    m_descriptorSets.clear();
    return true;
}

bool VulkanDescriptorPool::createDescriptorPool(u32 setsCount, const std::vector<VkDescriptorPoolSize>& sizes)
{
    VkDescriptorPoolCreateInfo descriptorPoolCreateInfo = {};
    descriptorPoolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    descriptorPoolCreateInfo.pNext = nullptr; //VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO
    descriptorPoolCreateInfo.flags = m_flag;
    descriptorPoolCreateInfo.maxSets = setsCount;
    descriptorPoolCreateInfo.poolSizeCount = static_cast<u32>(sizes.size());
    descriptorPoolCreateInfo.pPoolSizes = sizes.data();

    VkResult result = VulkanWrapper::CreateDescriptorPool(m_device, &descriptorPoolCreateInfo, VULKAN_ALLOCATOR, &m_pool);
    if (result != VK_SUCCESS)
    {
        LOG_ERROR("VulkanDescriptorPool::createDescriptorPool vkResetDescriptorPool is failed. Error: %s", ErrorString(result).c_str());
        return false;
    }

    return true;
}

u32 VulkanDescriptorSetManager::s_maxSets = 256;

std::vector<VkDescriptorPoolSize> VulkanDescriptorSetManager::s_poolSizes =
{
    { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,                 128 },
    { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,         128 },
    { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,                  128 },
    { VK_DESCRIPTOR_TYPE_SAMPLER,                        128 },
    { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,                  128 },
    { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,                 128 },
    { VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER,           128 },
    { VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER,           128 },
    { VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT,               128 },
};

VkShaderStageFlagBits VulkanDescriptorSetManager::convertShaderTypeToVkStage(ShaderType type)
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

VulkanDescriptorSetManager::VulkanDescriptorSetManager(VkDevice device) noexcept
    : m_device(device)
{
}

VulkanPipelineLayout VulkanDescriptorSetManager::acquirePipelineLayout(const DescriptorSetDescription& desc)
{
    u64 hash = desc._hash;

    auto found = m_pipelinesLayouts.emplace(hash, VulkanPipelineLayout());
    if (found.second)
    {
        VulkanPipelineLayout& layout = found.first->second;
        layout._key = hash;
        layout._layout = VulkanDescriptorSetManager::createPipelineLayout(desc, layout._descriptorSetLayouts);

        return layout;
    }

    return found.first->second;
}

bool VulkanDescriptorSetManager::removePipelineLayout(const DescriptorSetDescription & desc)
{
    auto iter = m_pipelinesLayouts.find(desc._hash);
    if (iter == m_pipelinesLayouts.cend())
    {
        LOG_DEBUG("removePipelineLayout PipelineLayout not found");
        ASSERT(false, "PipelineLayout");
        return false;
    }

    VulkanPipelineLayout& layout = iter->second;
    VulkanDescriptorSetManager::destroyPipelineLayout(layout._layout, layout._descriptorSetLayouts);

    m_pipelinesLayouts.erase(desc._hash);

    return true;
}

bool VulkanDescriptorSetManager::removePipelineLayout(VulkanPipelineLayout & layout)
{
    auto iter = m_pipelinesLayouts.find(layout._key);
    if (iter == m_pipelinesLayouts.cend())
    {
        LOG_DEBUG("removePipelineLayout PipelineLayout not found");
        ASSERT(false, "PipelineLayout");
        return false;
    }

    VulkanPipelineLayout& pipelinelayout = iter->second;
    VulkanDescriptorSetManager::destroyPipelineLayout(pipelinelayout._layout, pipelinelayout._descriptorSetLayouts);

    m_pipelinesLayouts.erase(layout._key);

    return true;
}

VulkanDescriptorPool* VulkanDescriptorSetManager::acquireDescriptorSets(const VulkanPipelineLayout& layout, std::vector<VkDescriptorSet>& sets, std::vector<u32>& offsets)
{
    for (auto& pool : m_descriptorPools)
    {
        if (pool->allocateDescriptorSet(layout, sets))
        {
            return pool;
        }
    }

    VkDescriptorPoolCreateFlags flag = 0;
    VulkanDescriptorPool* newPool = VulkanDescriptorSetManager::createPool(layout, flag);
    m_descriptorPools.push_back(newPool);

    bool result = newPool->allocateDescriptorSet(layout, sets);
    ASSERT(result, "fail");
    return newPool;
}

VkPipelineLayout VulkanDescriptorSetManager::createPipelineLayout(const DescriptorSetDescription& desc, std::vector<VkDescriptorSetLayout>& descriptorSetLayouts)
{
    if (!VulkanDescriptorSetManager::createDescriptorSetLayouts(desc, descriptorSetLayouts))
    {
        ASSERT(false, "error");
        VulkanDescriptorSetManager::destroyDescriptorSetLayouts(descriptorSetLayouts);

        return VK_NULL_HANDLE;
    }

    VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = {};
    pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutCreateInfo.pNext = nullptr;
    pipelineLayoutCreateInfo.flags = 0;
    pipelineLayoutCreateInfo.setLayoutCount = static_cast<u32>(descriptorSetLayouts.size());
    pipelineLayoutCreateInfo.pSetLayouts = descriptorSetLayouts.data();
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

void VulkanDescriptorSetManager::destroyPipelineLayout(VkPipelineLayout layout, std::vector<VkDescriptorSetLayout>& descriptorSetLayouts)
{
    VulkanDescriptorSetManager::destroyDescriptorSetLayouts(descriptorSetLayouts);
    VulkanWrapper::DestroyPipelineLayout(m_device, layout, VULKAN_ALLOCATOR);
}

bool VulkanDescriptorSetManager::createDescriptorSetLayouts(const DescriptorSetDescription& desc, std::vector<VkDescriptorSetLayout>& descriptorSetLayouts)
{
    descriptorSetLayouts.reserve(desc._descriptorSets.size());
    for (auto& set : desc._descriptorSets)
    {
        if (set.empty())
        {
            continue;
        }

        VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo = {};
        descriptorSetLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        descriptorSetLayoutCreateInfo.pNext = nullptr; //VkDescriptorSetLayoutBindingFlagsCreateInfoEXT
        descriptorSetLayoutCreateInfo.flags = 0; //VK_DESCRIPTOR_SET_LAYOUT_CREATE_PUSH_DESCRIPTOR_BIT_KHR
        descriptorSetLayoutCreateInfo.bindingCount = static_cast<u32>(set.size());
        descriptorSetLayoutCreateInfo.pBindings = set.data();

        VkDescriptorSetLayout descriptorSetLayout = VK_NULL_HANDLE;
        VkResult result = VulkanWrapper::CreateDescriptorSetLayout(m_device, &descriptorSetLayoutCreateInfo, VULKAN_ALLOCATOR, &descriptorSetLayout);
        if (result != VK_SUCCESS)
        {
            LOG_ERROR("DescriptorSetManager::createDescriptorSetLayout vkCreateDescriptorSetLayout is failed. Error: %s", ErrorString(result).c_str());
            return false;
        }

        descriptorSetLayouts.push_back(descriptorSetLayout);
    }

    return true;
}

void VulkanDescriptorSetManager::destroyDescriptorSetLayouts(std::vector<VkDescriptorSetLayout>& descriptorSetLayouts)
{
    for (auto& set : descriptorSetLayouts)
    {
        VulkanWrapper::DestroyDescriptorSetLayout(m_device, set, VULKAN_ALLOCATOR);
    }
}

VulkanDescriptorPool * VulkanDescriptorSetManager::createPool(const VulkanPipelineLayout& layout, VkDescriptorPoolCreateFlags flag)
{
    VulkanDescriptorPool* pool = new VulkanDescriptorPool(m_device, flag);

    u32 setCount = 0;
    std::vector<VkDescriptorPoolSize>* sizes = nullptr;
    if (VulkanDeviceCaps::getInstance()->useGlobalDescriptorPool)
    {
        setCount = s_maxSets;
        sizes = &VulkanDescriptorSetManager::s_poolSizes;
    }
    else
    {
        ASSERT(false, "not implemented");
        setCount = static_cast<u32>(layout._descriptorSetLayouts.size());
        for (auto& set : layout._descriptorSetLayouts)
        {
            //TODO: need shader
        }
    }

    if (!pool->create(setCount, *sizes))
    {
        ASSERT(false, "fail");
        pool->destroy();
    }

    return pool;
}

VulkanDescriptorSetManager::DescriptorSetDescription::DescriptorSetDescription(const std::array<resource::Shader*, ShaderType::ShaderType_Count>& shaders) noexcept
    : _hash(0)
{
    _descriptorSets.fill({});

    u32 maxSetIndex = 0;
    for (u32 setIndex = 0; setIndex < k_maxDescriptorSetIndex; ++setIndex)
    {
        std::vector<VkDescriptorSetLayoutBinding> descriptorSetLayoutBindings;
        for (u32 type = ShaderType::ShaderType_Vertex; type < ShaderType_Count; ++type)
        {
            const resource::Shader* shader = shaders[type];
            if (!shader)
            {
                continue;
            }

            const resource::Shader::ReflectionInfo& info = shader->getReflectionInfo();
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
        const resource::Shader* shader = shaders[type];
        if (!shader)
        {
            continue;
        }

        const resource::Shader::ReflectionInfo& info = shader->getReflectionInfo();
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

}//namespace vk
} //namespace renderer
} //namespace v3d
#endif //VULKAN_RENDER
