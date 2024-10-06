#include "VulkanPipelineLayout.h"

#include "Utils/Logger.h"
#include "Renderer/Shader.h"

#ifdef VULKAN_RENDER
#include "VulkanDebug.h"
#include "VulkanDevice.h"
#include "VulkanDeviceCaps.h"

namespace v3d
{
namespace renderer
{
namespace vk
{

VulkanDescriptorSetLayoutDescription::VulkanDescriptorSetLayoutDescription() noexcept
    : _key(0)
{
}

VulkanDescriptorSetLayoutDescription::VulkanDescriptorSetLayoutDescription(const std::vector<VkDescriptorSetLayoutBinding>& bindings) noexcept
    : _key(0)
    , _bindings(bindings)
{
    _key = crc32c::Extend(static_cast<u32>(_bindings.size()), reinterpret_cast<u8*>(_bindings.data()), _bindings.size() * sizeof(VkDescriptorSetLayoutBinding));
}

bool VulkanDescriptorSetLayoutDescription::Equal::operator()(const VulkanDescriptorSetLayoutDescription& descl, const VulkanDescriptorSetLayoutDescription& descr) const
{
    if (descl._bindings.size() != descr._bindings.size())
    {
        return false;
    }

    if (!descl._bindings.empty() && memcmp(descl._bindings.data(), descr._bindings.data(), descl._bindings.size() * sizeof(VkDescriptorSetLayoutBinding)) != 0)
    {
        return false;
    }

    return true;
}

size_t VulkanDescriptorSetLayoutDescription::Hash::operator()(const VulkanDescriptorSetLayoutDescription& desc) const
{
    ASSERT(desc._key, "no init");
    return desc._key;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////

VulkanPipelineLayoutDescription::VulkanPipelineLayoutDescription() noexcept
    : _key(0)
{
}

bool VulkanPipelineLayoutDescription::Equal::operator()(const VulkanPipelineLayoutDescription& descl, const VulkanPipelineLayoutDescription& descr) const
{
    //bindings
    for (u32 i = 0; i < k_maxDescriptorSetCount; ++i)
    {
        if (descl._bindingsSet[i].size() != descr._bindingsSet[i].size())
        {
            return false;
        }

        if (!descl._bindingsSet[i].empty() && memcmp(descl._bindingsSet[i].data(), descr._bindingsSet[i].data(), descl._bindingsSet[i].size() * sizeof(VkDescriptorSetLayoutBinding)) != 0)
        {
            return false;
        }
    }

    if (descl._pushConstant.size() != descr._pushConstant.size())
    {
        return false;
    }

    if (!descl._pushConstant.empty() && memcmp(descl._pushConstant.data(), descr._pushConstant.data(), descl._pushConstant.size() * sizeof(VkPushConstantRange))!= 0)
    {
        return false;
    }

    return true;
}

size_t VulkanPipelineLayoutDescription::Hash::operator()(const VulkanPipelineLayoutDescription& desc) const
{
    ASSERT(desc._key, "no init");
    return desc._key;
}

VulkanPipelineLayout::VulkanPipelineLayout() noexcept
    : _pipelineLayout(VK_NULL_HANDLE)
{
    _setLayouts.fill(VK_NULL_HANDLE);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////

VkShaderStageFlagBits VulkanPipelineLayoutManager::convertShaderTypeToVkStage(ShaderType type)
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
        break;
    }

    ASSERT(false, "not faund");
    return VK_SHADER_STAGE_VERTEX_BIT;
}

VulkanPipelineLayoutManager::VulkanPipelineLayoutManager(VulkanDevice* device) noexcept
    : m_device(*device)
{
}

VulkanPipelineLayoutManager::~VulkanPipelineLayoutManager()
{
    ASSERT(m_pipelinesLayouts.empty(), "not empty");
}

VulkanPipelineLayout VulkanPipelineLayoutManager::acquirePipelineLayout(const VulkanPipelineLayoutDescription& desc)
{
    auto found = m_pipelinesLayouts.emplace(desc, VulkanPipelineLayout());
    if (found.second)
    {
        VulkanPipelineLayout& layout = found.first->second;

        layout._setLayouts.fill(VK_NULL_HANDLE);
        for (u32 setId = 0; setId < k_maxDescriptorSetCount; ++setId)
        {
            auto& set = desc._bindingsSet[setId];
            if (set.empty())
            {
                layout._setLayouts[setId] = VK_NULL_HANDLE;
                continue;
            }

            layout._setLayouts[setId] = VulkanPipelineLayoutManager::acquireDescriptorSetLayout(set);
            if (!layout._setLayouts[setId])
            {
                ASSERT(false, "error");
                VulkanPipelineLayoutManager::destroyDescriptorSetLayouts(layout._setLayouts);
                return layout;
            }
        }

        layout._pipelineLayout = VulkanPipelineLayoutManager::createPipelineLayout(desc, layout._setLayouts);
        if (layout._pipelineLayout == VK_NULL_HANDLE)
        {
            ASSERT(false, "error");
            VulkanPipelineLayoutManager::destroyDescriptorSetLayouts(layout._setLayouts);
        }

        return layout;
    }

    return found.first->second;
}

bool VulkanPipelineLayoutManager::removePipelineLayout(const VulkanPipelineLayoutDescription& desc)
{
    auto iter = m_pipelinesLayouts.find(desc);
    if (iter == m_pipelinesLayouts.cend())
    {
        LOG_DEBUG("removePipelineLayout PipelineLayout not found");
        ASSERT(false, "PipelineLayout");
        return false;
    }

    VulkanPipelineLayout& pipelinelayout = iter->second;
    VulkanWrapper::DestroyPipelineLayout(m_device.getDeviceInfo()._device, pipelinelayout._pipelineLayout, VULKAN_ALLOCATOR);

    m_pipelinesLayouts.erase(desc);

    return true;
}

void VulkanPipelineLayoutManager::clear()
{
    for (auto& layout : m_pipelinesLayouts)
    {
        VulkanWrapper::DestroyPipelineLayout(m_device.getDeviceInfo()._device, layout.second._pipelineLayout, VULKAN_ALLOCATOR);
    }
    m_pipelinesLayouts.clear();

    for (auto& iter : m_descriptorSetLayouts)
    {
        VulkanWrapper::DestroyDescriptorSetLayout(m_device.getDeviceInfo()._device, iter.second, VULKAN_ALLOCATOR);
    }
    m_descriptorSetLayouts.clear();
}


VkPipelineLayout VulkanPipelineLayoutManager::createPipelineLayout(const VulkanPipelineLayoutDescription& desc, const std::array<VkDescriptorSetLayout, k_maxDescriptorSetCount>& descriptorSetLayouts)
{
    std::vector<VkDescriptorSetLayout> vkDescriptorSetLayouts;
    vkDescriptorSetLayouts.reserve(k_maxDescriptorSetCount);
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
    VkResult result = VulkanWrapper::CreatePipelineLayout(m_device.getDeviceInfo()._device, &pipelineLayoutCreateInfo, VULKAN_ALLOCATOR, &pipelineLayout);
    if (result != VK_SUCCESS)
    {

        VulkanWrapper::DestroyPipelineLayout(m_device.getDeviceInfo()._device, pipelineLayout, VULKAN_ALLOCATOR);
        LOG_ERROR("VulkanDescriptorSetManager::createPipelineLayout vkCreatePipelineLayout is failed. Error: %s", ErrorString(result).c_str());
        return VK_NULL_HANDLE;
    }

    return pipelineLayout;
}

VkDescriptorSetLayout VulkanPipelineLayoutManager::acquireDescriptorSetLayout(const std::vector<VkDescriptorSetLayoutBinding>& bindings)
{
    VkDescriptorSetLayout layout = VK_NULL_HANDLE;
    VulkanDescriptorSetLayoutDescription vulkanDescriptorSetDescription(bindings);
    auto found = m_descriptorSetLayouts.emplace(vulkanDescriptorSetDescription, layout);
    if (found.second)
    {
        VkDescriptorSetLayout& descriptorSetLayout = found.first->second;
        descriptorSetLayout = VulkanPipelineLayoutManager::createDescriptorSetLayout(bindings);

        return descriptorSetLayout;
    }

    return found.first->second;
}

VkDescriptorSetLayout VulkanPipelineLayoutManager::createDescriptorSetLayout(const std::vector<VkDescriptorSetLayoutBinding>& bindings)
{
    void* vkExtensions = nullptr;

    VkDescriptorSetLayoutBindingFlagsCreateInfoEXT descriptorSetLayoutBindingFlagsCreateInfo = {};
    std::vector<VkDescriptorBindingFlagsEXT> descriptorBindingFlags(bindings.size(), 0);
    if (m_device.getVulkanDeviceCaps()._useLateDescriptorSetUpdate)
    {
        for (u32 i = 0; i < bindings.size(); ++i)
        {
            descriptorBindingFlags[i] |= VK_DESCRIPTOR_BINDING_UPDATE_UNUSED_WHILE_PENDING_BIT_EXT;
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
    descriptorSetLayoutCreateInfo.bindingCount = static_cast<u32>(bindings.size());
    descriptorSetLayoutCreateInfo.pBindings = bindings.data();

    VkDescriptorSetLayout descriptorSetLayout = VK_NULL_HANDLE;
    VkResult result = VulkanWrapper::CreateDescriptorSetLayout(m_device.getDeviceInfo()._device, &descriptorSetLayoutCreateInfo, VULKAN_ALLOCATOR, &descriptorSetLayout);
    if (result != VK_SUCCESS)
    {
        LOG_ERROR("DescriptorSetManager::createDescriptorSetLayout vkCreateDescriptorSetLayout is failed. Error: %s", ErrorString(result).c_str());
        return VK_NULL_HANDLE;
    }

    return descriptorSetLayout;
}

void VulkanPipelineLayoutManager::destroyDescriptorSetLayouts(std::array<VkDescriptorSetLayout, k_maxDescriptorSetCount>& descriptorSetLayouts)
{
    for (auto& set : descriptorSetLayouts)
    {
        if (set != VK_NULL_HANDLE)
        {
            VulkanWrapper::DestroyDescriptorSetLayout(m_device.getDeviceInfo()._device, set, VULKAN_ALLOCATOR);
        }
    }
    descriptorSetLayouts.fill(VK_NULL_HANDLE);
}

VulkanPipelineLayoutManager::DescriptorSetLayoutCreator::DescriptorSetLayoutCreator(const std::array<Shader*, toEnumType(ShaderType::Count)>& shaders) noexcept
{
    //_description._pushConstant.clear();
    //_description._bindingsSet.fill({});

    //u32 maxSetIndex = 0;
    //for (u32 setIndex = 0; setIndex < k_maxDescriptorSetCount; ++setIndex)
    //{
    //    std::vector<VkDescriptorSetLayoutBinding> descriptorSetLayoutBindings;
    //    for (u32 type = toEnumType(ShaderType::Vertex); type < (u32)toEnumType(ShaderType::Count); ++type)
    //    {
    //        const Shader* shader = shaders[type];
    //        if (!shader)
    //        {
    //            continue;
    //        }

    //        const Shader::ReflectionInfo& info = shader->getReflectionInfo();
    //        for (auto& uniform : info._uniformBuffers)
    //        {
    //            ASSERT(uniform._set < k_maxDescriptorSetCount && uniform._binding < k_maxDescriptorBindingCount, "range out");
    //            if (uniform._set != setIndex)
    //            {
    //                continue;
    //            }

    //            VkDescriptorSetLayoutBinding descriptorSetLayoutBinding;
    //            descriptorSetLayoutBinding.descriptorType = VulkanDeviceCaps::getInstance()->useDynamicUniforms ? VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC : VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    //            descriptorSetLayoutBinding.binding = uniform._binding;
    //            descriptorSetLayoutBinding.stageFlags = convertShaderTypeToVkStage((ShaderType)type);
    //            descriptorSetLayoutBinding.descriptorCount = uniform._array;
    //            descriptorSetLayoutBinding.pImmutableSamplers = nullptr;

    //            descriptorSetLayoutBindings.push_back(descriptorSetLayoutBinding);
    //        }

    //        for (auto& sampledImage : info._sampledImages)
    //        {
    //            if (sampledImage._set != setIndex)
    //            {
    //                continue;
    //            }

    //            VkDescriptorSetLayoutBinding descriptorSetLayoutBinding;
    //            descriptorSetLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    //            descriptorSetLayoutBinding.binding = sampledImage._binding;
    //            descriptorSetLayoutBinding.stageFlags = convertShaderTypeToVkStage((ShaderType)type);
    //            descriptorSetLayoutBinding.descriptorCount = sampledImage._array;
    //            descriptorSetLayoutBinding.pImmutableSamplers = nullptr;

    //            descriptorSetLayoutBindings.push_back(descriptorSetLayoutBinding);
    //        }

    //        for (auto& sampler : info._samplers)
    //        {
    //            if (sampler._set != setIndex)
    //            {
    //                continue;
    //            }

    //            VkDescriptorSetLayoutBinding descriptorSetLayoutBinding;
    //            descriptorSetLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
    //            descriptorSetLayoutBinding.binding = sampler._binding;
    //            descriptorSetLayoutBinding.stageFlags = convertShaderTypeToVkStage((ShaderType)type);
    //            descriptorSetLayoutBinding.descriptorCount = 1;
    //            descriptorSetLayoutBinding.pImmutableSamplers = nullptr;

    //            descriptorSetLayoutBindings.push_back(descriptorSetLayoutBinding);
    //        }

    //        for (auto& image : info._images)
    //        {
    //            if (image._set != setIndex)
    //            {
    //                continue;
    //            }

    //            VkDescriptorSetLayoutBinding descriptorSetLayoutBinding;
    //            descriptorSetLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
    //            descriptorSetLayoutBinding.binding = image._binding;
    //            descriptorSetLayoutBinding.stageFlags = convertShaderTypeToVkStage((ShaderType)type);
    //            descriptorSetLayoutBinding.descriptorCount = image._array;
    //            descriptorSetLayoutBinding.pImmutableSamplers = nullptr;

    //            descriptorSetLayoutBindings.push_back(descriptorSetLayoutBinding);
    //        }

    //        for (auto& storageImage : info._storageImages)
    //        {
    //            if (storageImage._set != setIndex)
    //            {
    //                continue;
    //            }

    //            VkDescriptorSetLayoutBinding descriptorSetLayoutBinding;
    //            descriptorSetLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
    //            descriptorSetLayoutBinding.binding = storageImage._binding;
    //            descriptorSetLayoutBinding.stageFlags = convertShaderTypeToVkStage((ShaderType)type);
    //            descriptorSetLayoutBinding.descriptorCount = storageImage._array;
    //            descriptorSetLayoutBinding.pImmutableSamplers = nullptr;

    //            descriptorSetLayoutBindings.push_back(descriptorSetLayoutBinding);
    //        }
    //    }

    //    if (!descriptorSetLayoutBindings.empty())
    //    {
    //        maxSetIndex = std::max(maxSetIndex, setIndex);
    //        _description._bindingsSet[setIndex] = std::move(descriptorSetLayoutBindings);
    //    }
    //}
    //ASSERT(maxSetIndex < k_maxDescriptorSetCount, "invalid max set index");

    //for (u32 type = toEnumType(ShaderType::Vertex); type < (u32)toEnumType(ShaderType::Count); ++type)
    //{
    //    const Shader* shader = shaders[type];
    //    if (!shader)
    //    {
    //        continue;
    //    }

    //    const Shader::ReflectionInfo& info = shader->getReflectionInfo();
    //    _description._pushConstant.reserve(info._pushConstant.size());
    //    for (auto& push : info._pushConstant)
    //    {
    //        VkPushConstantRange pushConstantRange = {};
    //        pushConstantRange.stageFlags = convertShaderTypeToVkStage((ShaderType)type);
    //        pushConstantRange.offset = push._offset;
    //        pushConstantRange.size = push._size;

    //        _description._pushConstant.push_back(pushConstantRange);
    //    }
    //}

    //u64 pushConstantHash = crc32c::Extend(static_cast<u32>(_description._pushConstant.size()), reinterpret_cast<u8*>(_description._pushConstant.data()), _description._pushConstant.size() * sizeof(VkPushConstantRange));

    //u32 setHash = static_cast<u32>(_description._bindingsSet.size()); //always k_maxDescriptorSetIndex
    //for (auto& set : _description._bindingsSet)
    //{
    //    u32 bindingCount = static_cast<u32>(set.size());
    //    setHash = crc32c::Extend(setHash, reinterpret_cast<u8*>(&bindingCount), sizeof(u32));
    //    setHash = crc32c::Extend(setHash, reinterpret_cast<u8*>(set.data()), set.size() * sizeof(VkDescriptorSetLayoutBinding));
    //}
    //_description._key = setHash | pushConstantHash << 32;
}

} //namespace vk
} //namespace renderer
} //namespace v3d
#endif //VULKAN_RENDER
