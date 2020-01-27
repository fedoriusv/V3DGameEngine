#include "VulkanDescriptorSet.h"
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

BindingInfo::BindingInfo() noexcept
{
    memset(this, 0, sizeof(BindingInfo));
}

bool BindingInfo::operator==(const BindingInfo& info) const
{
    if (this == &info)
    {
        return true;
    }

    if (_binding != info._binding || _type != info._type || _arrayIndex != info._arrayIndex)
    {
        return false;
    }

    if (memcmp(&_info._bufferInfo, &info._info._bufferInfo, sizeof(VkDescriptorBufferInfo)) ||
        memcmp(&_info._imageInfo, &info._info._imageInfo, sizeof(VkDescriptorImageInfo)))
    {
        return false;
    }

    return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////

VulkanDescriptorSetPool::VulkanDescriptorSetPool(VkDevice device, VkDescriptorPoolCreateFlags flag) noexcept
    : m_device(device)
    , m_flag(flag)
    , m_pool(VK_NULL_HANDLE)
{
    ASSERT(m_descriptorSets.empty(), "not empty");
    ASSERT(!m_pool, "not nullptr");
}

bool VulkanDescriptorSetPool::create(u32 setsCount, const std::vector<VkDescriptorPoolSize>& sizes)
{
    ASSERT(!m_pool, "not nullptr");
    if (!VulkanDescriptorSetPool::createDescriptorPool(setsCount, sizes))
    {
        return false;
    }

    return true;
}

void VulkanDescriptorSetPool::destroy()
{
    if (!m_descriptorSets.empty())
    {
        //VulkanDescriptorSetPool::freeDescriptorSets(m_descriptorSets.);
        m_descriptorSets.clear();
    }

    if (m_pool)
    {
        VulkanWrapper::DestroyDescriptorPool(m_device, m_pool, VULKAN_ALLOCATOR);
        m_pool = VK_NULL_HANDLE;
    }
}

bool VulkanDescriptorSetPool::allocateDescriptorSets(std::vector<VkDescriptorSetLayout>& layout, std::vector<VkDescriptorSet>& descriptorSets)
{
    VkDescriptorSetAllocateInfo descriptorSetAllocateInfo = {};
    descriptorSetAllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    descriptorSetAllocateInfo.pNext = nullptr; //VkDescriptorSetVariableDescriptorCountAllocateInfoEXT
    descriptorSetAllocateInfo.descriptorPool = m_pool;
    descriptorSetAllocateInfo.descriptorSetCount = static_cast<u32>(layout.size());
    descriptorSetAllocateInfo.pSetLayouts = layout.data();

    descriptorSets.resize(layout.size(), VK_NULL_HANDLE);
    VkResult result = VulkanWrapper::AllocateDescriptorSets(m_device, &descriptorSetAllocateInfo, descriptorSets.data());
    if (result == VK_ERROR_OUT_OF_POOL_MEMORY || result == VK_ERROR_FRAGMENTED_POOL || result == VK_ERROR_OUT_OF_DEVICE_MEMORY)
    {
#if VULKAN_DEBUG
        LOG_WARNING("VulkanDescriptorPool::createDescriptorSets vkAllocateDescriptorSets is failed. Error: %s", ErrorString(result).c_str());
#endif //VULKAN_DEBUG
        //VulkanDescriptorSetPool::freeDescriptorSets(descriptorSets);

        return false;
    }
    else if (result != VK_SUCCESS)
    {
        LOG_ERROR("VulkanDescriptorPool::createDescriptorSets vkAllocateDescriptorSets is failed. Error: %s", ErrorString(result).c_str());
        VulkanDescriptorSetPool::freeDescriptorSets(descriptorSets);
        return false;
    }

    return true;
}

bool VulkanDescriptorSetPool::freeDescriptorSets(std::vector<VkDescriptorSet>& descriptorSets)
{
    if (!(m_flag & VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT))
    {
        return false;
    }

    VkResult result = VulkanWrapper::FreeDescriptorSets(m_device, m_pool, static_cast<u32>(descriptorSets.size()), descriptorSets.data());
    if (result != VK_SUCCESS)
    {
        LOG_ERROR("VulkanDescriptorPool::destroyDescriptorSets vkFreeDescriptorSets is failed. Error: %s", ErrorString(result).c_str());
        return false;
    }

    return true;
}

bool VulkanDescriptorSetPool::allocateDescriptorSet(VkDescriptorSetLayout layout, VkDescriptorSet& descriptorSet)
{
    VkDescriptorSetAllocateInfo descriptorSetAllocateInfo = {};
    descriptorSetAllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    descriptorSetAllocateInfo.pNext = nullptr; //VkDescriptorSetVariableDescriptorCountAllocateInfoEXT
    descriptorSetAllocateInfo.descriptorPool = m_pool;
    descriptorSetAllocateInfo.descriptorSetCount = 1;
    descriptorSetAllocateInfo.pSetLayouts = &layout;

    descriptorSet = VK_NULL_HANDLE;
    VkResult result = VulkanWrapper::AllocateDescriptorSets(m_device, &descriptorSetAllocateInfo, &descriptorSet);
    if (result == VK_ERROR_OUT_OF_POOL_MEMORY || result == VK_ERROR_FRAGMENTED_POOL || result == VK_ERROR_OUT_OF_DEVICE_MEMORY)
    {
#if VULKAN_DEBUG
        LOG_WARNING("VulkanDescriptorPool::createDescriptorSet vkAllocateDescriptorSets is failed. Error: %s", ErrorString(result).c_str());
#endif //VULKAN_DEBUG
        //VulkanDescriptorSetPool::freeDescriptorSet(descriptorSets);

        return false;
    }
    else if (result != VK_SUCCESS)
    {
        LOG_ERROR("VulkanDescriptorPool::createDescriptorSet vkAllocateDescriptorSets is failed. Error: %s", ErrorString(result).c_str());
        VulkanDescriptorSetPool::freeDescriptorSet(descriptorSet);
        return false;
    }

    return true;
}

bool VulkanDescriptorSetPool::freeDescriptorSet(VkDescriptorSet& descriptorSet)
{
    if (!(m_flag & VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT))
    {
        return false;
    }

    VkResult result = VulkanWrapper::FreeDescriptorSets(m_device, m_pool, 1, &descriptorSet);
    if (result != VK_SUCCESS)
    {
        LOG_ERROR("VulkanDescriptorPool::freeDescriptorSet vkFreeDescriptorSets is failed. Error: %s", ErrorString(result).c_str());
        return false;
    }

    descriptorSet = VK_NULL_HANDLE;
    return true;
}

VkDescriptorSet VulkanDescriptorSetPool::createDescriptorSet(u64 hash, VkDescriptorSetLayout layout)
{
    VkDescriptorSet set = VK_NULL_HANDLE;
    bool result = VulkanDescriptorSetPool::allocateDescriptorSet(layout, set);
    if (!result)
    {
        if (set != VK_NULL_HANDLE)
        {
            VulkanDescriptorSetPool::freeDescriptorSet(set);
        }
        return VK_NULL_HANDLE;
    }

    auto& insert = m_descriptorSets.emplace(hash, set);
    if (!insert.second)
    {
        ASSERT(false, "already present");
        return insert.first->second;
    }

    insert.first->second = set;
    return set;
}

VkDescriptorSet VulkanDescriptorSetPool::getDescriptorSet(u64 hash)
{
    auto found = m_descriptorSets.find(hash);
    if (found != m_descriptorSets.cend())
    {
        return found->second;
    }

    return VK_NULL_HANDLE;
}

bool VulkanDescriptorSetPool::reset(VkDescriptorPoolResetFlags flag)
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

bool VulkanDescriptorSetPool::createDescriptorPool(u32 setsCount, const std::vector<VkDescriptorPoolSize>& sizes)
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
    { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,                 256 },
    { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,         64  },

    { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,         128 },
    { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,                  128 },
    { VK_DESCRIPTOR_TYPE_SAMPLER,                        128 },

    { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,                  128 },
    { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,                 128 },

    { VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER,           128 },
    { VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER,           128 },

    { VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT,               8   },
};

VulkanDescriptorSetManager::VulkanDescriptorSetManager(VkDevice device) noexcept
    : m_device(device)
    , m_currentDescriptorPool(nullptr)
{
}

VulkanDescriptorSetManager::~VulkanDescriptorSetManager()
{
    ASSERT(!m_currentDescriptorPool, "not nullptr");
    ASSERT(m_freeDescriptorPools.empty(), "not empty");
    ASSERT(m_usedDescriptorPools.empty(), "not empty");
}

void VulkanDescriptorSetManager::updateDescriptorPools()
{
    for (auto iter = m_usedDescriptorPools.begin(); iter != m_usedDescriptorPools.end();)
    {
        VulkanDescriptorSetPool* pool = (*iter);
        if (!pool->isCaptured())
        {
            m_freeDescriptorPools.push_back(pool);
            iter = m_usedDescriptorPools.erase(iter);

            pool->reset(0);
        }
        else
        {
            ++iter;
        }
    }
}

void VulkanDescriptorSetManager::clear()
{
    ASSERT(m_usedDescriptorPools.empty(), "strill used");
    for (auto& pool : m_freeDescriptorPools)
    {
        ASSERT(!pool->isCaptured(), "still used");
        pool->destroy();
        delete pool;
    }
    m_freeDescriptorPools.clear();

    if (m_currentDescriptorPool)
    {
        ASSERT(!m_currentDescriptorPool->isCaptured(), "still used");
        m_currentDescriptorPool->destroy();

        delete m_currentDescriptorPool;
        m_currentDescriptorPool = nullptr;
    }
}

VkDescriptorSet VulkanDescriptorSetManager::acquireDescriptorSet(const VulkanPipelineLayout& layout, const SetKey& key, VulkanDescriptorSetPool*& pool)
{
    VkDescriptorPoolCreateFlags flag = 0;
    /*if (VulkanDeviceCaps::getInstance()->useDynamicUniforms)
    {
        flag |= VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT_EXT;
    }*/

    //finds in pools
    if (m_currentDescriptorPool)
    {
        VkDescriptorSet set = m_currentDescriptorPool->getDescriptorSet(key._hash);
        if (set != VK_NULL_HANDLE)
        {
            pool = m_currentDescriptorPool;
            return set;
        }
    }
    else
    {
        ASSERT(m_usedDescriptorPools.empty(), "not empty");
        m_currentDescriptorPool = VulkanDescriptorSetManager::acquireFreePool(layout, flag);
    }

    for (auto& usedPool : m_usedDescriptorPools)
    {
        VkDescriptorSet set = usedPool->getDescriptorSet(key._hash);
        if (set != VK_NULL_HANDLE)
        {
            pool = usedPool;
            return set;
        }
    }
    ASSERT(m_currentDescriptorPool, "nullptr");

    //create new
    VkDescriptorSetLayout vkDescriptorSetLayout = layout._descriptorSetLayouts[key._set];
    ASSERT(vkDescriptorSetLayout != VK_NULL_HANDLE, "nullptr");
    VkDescriptorSet vkDescriptorSet = m_currentDescriptorPool->createDescriptorSet(key._hash, vkDescriptorSetLayout);
    if (vkDescriptorSet == VK_NULL_HANDLE)
    {
        //try another pool
        m_usedDescriptorPools.push_back(m_currentDescriptorPool);
        m_currentDescriptorPool = VulkanDescriptorSetManager::acquireFreePool(layout, flag);
        ASSERT(m_currentDescriptorPool, "nullptr");

        vkDescriptorSet = m_currentDescriptorPool->createDescriptorSet(key._hash, vkDescriptorSetLayout);
        ASSERT(vkDescriptorSet != VK_NULL_HANDLE, "fail");
    }

    pool = m_currentDescriptorPool;
    return vkDescriptorSet;
}

VulkanDescriptorSetPool* VulkanDescriptorSetManager::acquireFreePool(const VulkanPipelineLayout& layout, VkDescriptorPoolCreateFlags flag)
{
    VulkanDescriptorSetPool* newPool = nullptr;
    if (m_freeDescriptorPools.empty())
    {
        newPool = VulkanDescriptorSetManager::createPool(layout, flag);
    }
    else
    {
        newPool = m_freeDescriptorPools.front();
        m_freeDescriptorPools.pop_front();
    }

    return newPool;
}

VulkanDescriptorSetPool* VulkanDescriptorSetManager::createPool(const VulkanPipelineLayout& layout, VkDescriptorPoolCreateFlags flag)
{
    VulkanDescriptorSetPool* pool = new VulkanDescriptorSetPool(m_device, flag);
    ASSERT(pool, "nullptr");

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

void VulkanDescriptorSetManager::destroyPools()
{
    ASSERT(m_usedDescriptorPools.empty(), "not enmpty");

    if (m_currentDescriptorPool)
    {
        ASSERT(!m_currentDescriptorPool->isCaptured(), "still used");
        m_currentDescriptorPool->destroy();
        delete m_currentDescriptorPool;
        m_currentDescriptorPool = nullptr;
    }

    for (auto pool : m_freeDescriptorPools)
    {
        pool->destroy();
        delete pool;
    }
    m_freeDescriptorPools.clear();
}

} //namespace vk
} //namespace renderer
} //namespace v3d
#endif //VULKAN_RENDER
