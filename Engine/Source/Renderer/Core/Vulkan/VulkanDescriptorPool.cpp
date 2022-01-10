#include "VulkanDescriptorPool.h"
#include "Utils/Logger.h"

#ifdef VULKAN_RENDER
#include "VulkanDebug.h"
#include "VulkanDeviceCaps.h"

namespace v3d
{
namespace renderer
{
namespace vk
{

VulkanDescriptorSetPool::VulkanDescriptorSetPool(VkDevice device, VkDescriptorPoolCreateFlags flag) noexcept
    : m_device(device)
    , m_flag(flag)
    , m_pool(VK_NULL_HANDLE)
{
}

VulkanDescriptorSetPool::~VulkanDescriptorSetPool()
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
    if (!m_descriptorSets.empty()/* & m_flag == VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT*/)
    {
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

VkDescriptorSet VulkanDescriptorSetPool::createDescriptorSet(const SetInfo& info, VkDescriptorSetLayout layout)
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

    auto insert = m_descriptorSets.emplace(info, set);
    if (!insert.second)
    {
        ASSERT(false, "already present");
        return insert.first->second;
    }

    insert.first->second = set;
    return set;
}

VkDescriptorSet VulkanDescriptorSetPool::getDescriptorSet(const SetInfo& info)
{
    auto found = m_descriptorSets.find(info);
    if (found != m_descriptorSets.cend())
    {
        return found->second;
    }

    return VK_NULL_HANDLE;
}

bool VulkanDescriptorSetPool::reset()
{
    VkResult result = VulkanWrapper::ResetDescriptorPool(m_device, m_pool, m_flag);
    if (result != VK_SUCCESS)
    {
        LOG_ERROR("VulkanDescriptorPool::reset vkResetDescriptorPool is failed. Error: %s", ErrorString(result).c_str());
        return false;
    }

    m_descriptorSets.clear();
    return true;
}

u64 VulkanDescriptorSetPool::getCountDescriptorSets() const
{
    return m_descriptorSets.size();
}

bool VulkanDescriptorSetPool::createDescriptorPool(u32 setsCount, const std::vector<VkDescriptorPoolSize>& sizes)
{
    VkDescriptorPoolCreateInfo descriptorPoolCreateInfo = {};
    descriptorPoolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    descriptorPoolCreateInfo.pNext = nullptr;
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

/////////////////////////////////////////////////////////////////////////////////////////////////////


VulkanDescriptorPoolProvider::VulkanDescriptorPoolProvider(VkDevice device, GenericDescriptorPools* stategy) noexcept
    : m_device(device)
    , m_descriptorPools(stategy)
{
}

VulkanDescriptorPoolProvider::~VulkanDescriptorPoolProvider()
{
    ASSERT(m_descriptorPools, "nullptr");
    delete m_descriptorPools;
}

VulkanDescriptorSetPool* VulkanDescriptorPoolProvider::acquirePool(const VulkanDescriptorSetLayoutDescription& desc, VkDescriptorPoolCreateFlags flag)
{
    ASSERT(m_descriptorPools, "nullptr");
    return m_descriptorPools->acquirePool(desc, m_device, flag);
}

void VulkanDescriptorPoolProvider::destroyPools()
{
    ASSERT(m_descriptorPools, "nullptr");
    m_descriptorPools->destroyPools();
}

void VulkanDescriptorPoolProvider::updatePools()
{
    ASSERT(m_descriptorPools, "nullptr");
    m_descriptorPools->updatePools();
}

void VulkanDescriptorPoolProvider::resetPools()
{
    ASSERT(m_descriptorPools, "nullptr");
    m_descriptorPools->resetPools();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////

const u32 GlobalDescriptorPools::s_maxSets;

std::vector<VkDescriptorPoolSize> GlobalDescriptorPools::s_poolSizes =
{
    { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,                 std::min(GlobalDescriptorPools::s_maxSets, 256U) },
    { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,         std::min(GlobalDescriptorPools::s_maxSets, 64U)  },

    { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,         std::min(GlobalDescriptorPools::s_maxSets, 128U) },
    { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,                  std::min(GlobalDescriptorPools::s_maxSets, 128U) },
    { VK_DESCRIPTOR_TYPE_SAMPLER,                        std::min(GlobalDescriptorPools::s_maxSets, 128U) },

    { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,                  std::min(GlobalDescriptorPools::s_maxSets, 128U) },
    { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,                 std::min(GlobalDescriptorPools::s_maxSets, 128U) },

    { VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER,           std::min(GlobalDescriptorPools::s_maxSets, 128U) },
    { VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER,           std::min(GlobalDescriptorPools::s_maxSets, 128U) },

    { VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT,               std::min(GlobalDescriptorPools::s_maxSets, 8U)   },
};

GlobalDescriptorPools::GlobalDescriptorPools() noexcept
{
    ASSERT(VulkanDeviceCaps::getInstance()->useGlobalDescriptorPool, "wrong strategy");
}

VulkanDescriptorSetPool* GlobalDescriptorPools::acquirePool(const VulkanDescriptorSetLayoutDescription& desc, VkDevice device, VkDescriptorPoolCreateFlags flag)
{
    VulkanDescriptorSetPool* pool = nullptr;
    if (_freeDescriptorPools.empty())
    {
        pool = new VulkanDescriptorSetPool(device, flag);
        ASSERT(pool, "nullptr");

        u32 setCount = 0;
        std::vector<VkDescriptorPoolSize>* sizes = nullptr;

        setCount = s_maxSets;
        sizes = &s_poolSizes;

        if (!pool->create(setCount, *sizes))
        {
            ASSERT(false, "fail");
            pool->destroy();

            delete pool;
            pool = nullptr;
        }
    }
    else
    {
        pool = _freeDescriptorPools.front();
        _freeDescriptorPools.pop_front();
        ASSERT(!pool->isCaptured(), "pool is captured");
    }

    _usedDescriptorPools.push_back(pool);
    return pool;
}

void GlobalDescriptorPools::destroyPools()
{
    ASSERT(_usedDescriptorPools.empty(), "strill used");
    for (auto& pool : _freeDescriptorPools)
    {
        ASSERT(!pool->isCaptured(), "still used");
        pool->destroy();
        delete pool;
    }
    _freeDescriptorPools.clear();
}

void GlobalDescriptorPools::updatePools()
{
    for (auto iter = _usedDescriptorPools.begin(); iter != _usedDescriptorPools.end();)
    {
        VulkanDescriptorSetPool* pool = (*iter);
        if (!pool->isCaptured())
        {
            _freeDescriptorPools.push_back(pool);
            iter = _usedDescriptorPools.erase(iter);
        }
        else
        {
            ++iter;
        }
    }
}

void GlobalDescriptorPools::resetPools()
{
    for (auto iter = _freeDescriptorPools.begin(); iter != _freeDescriptorPools.end(); ++iter)
    {
        VulkanDescriptorSetPool* pool = (*iter);
        ASSERT(!pool->isCaptured(), "pool is captured");
        pool->reset();
    }
}

GlobalDescriptorPools::~GlobalDescriptorPools()
{
    ASSERT(_freeDescriptorPools.empty(), "not empty");
    ASSERT(_usedDescriptorPools.empty(), "not empty");
}



const u32 LayoutDescriptorPools::s_maxSets;

LayoutDescriptorPools::LayoutDescriptorPools() noexcept
{
    ASSERT(!VulkanDeviceCaps::getInstance()->useGlobalDescriptorPool, "wrong strategy");
}

VulkanDescriptorSetPool* LayoutDescriptorPools::acquirePool(const VulkanDescriptorSetLayoutDescription& desc, VkDevice device, VkDescriptorPoolCreateFlags flag)
{
    LayoutPools* layoutPools = nullptr;

    auto poolsIter = _pools.emplace(desc, nullptr);
    if (!poolsIter.second)
    {
        layoutPools  = poolsIter.first->second;
    }
    else
    {
        layoutPools = new LayoutPools();
        poolsIter.first->second = layoutPools;
    }
    ASSERT(layoutPools, "nullptr");

    VulkanDescriptorSetPool* pool = nullptr;
    if (layoutPools->_freeList.empty())
    {
        pool = LayoutDescriptorPools::createPool(desc, device, flag);
        ASSERT(pool, "nullptr");

    }
    else
    {
        pool = layoutPools->_freeList.front();
        layoutPools->_freeList.pop_front();
        ASSERT(!pool->isCaptured(), "pool is captured");
    }

    layoutPools->_usedList.push_back(pool);
    return pool;
}

void LayoutDescriptorPools::destroyPools()
{
    for (auto& pools : _pools)
    {
        LayoutPools* layoutPools = pools.second;
        ASSERT(layoutPools->_usedList.empty(), "pools are still used");
        for (auto iter = layoutPools->_freeList.begin(); iter != layoutPools->_freeList.end(); ++iter)
        {
            VulkanDescriptorSetPool* pool = (*iter);
            pool->destroy();
            delete pool;
        }
        layoutPools->_usedList.clear();
        delete layoutPools;
    }
    _pools.clear();
}

void LayoutDescriptorPools::updatePools()
{
    for (auto& pools : _pools)
    {
        LayoutPools* layoutPools = pools.second;
        for (auto iter = layoutPools->_usedList.begin(); iter != layoutPools->_usedList.end();)
        {
            VulkanDescriptorSetPool* pool = (*iter);
            if (!pool->isCaptured())
            {
                layoutPools->_freeList.push_back(pool);
                iter = layoutPools->_usedList.erase(iter);
            }
            else
            {
                ++iter;
            }
        }
    }
}

void LayoutDescriptorPools::resetPools()
{
    for (auto& pools : _pools)
    {
        LayoutPools* layoutPools = pools.second;
        for (auto iter = layoutPools->_freeList.begin(); iter != layoutPools->_freeList.end(); ++iter)
        {
            VulkanDescriptorSetPool* pool = (*iter);
            ASSERT(!pool->isCaptured(), "pool is captured");
            pool->reset();
        }
    }
}

VulkanDescriptorSetPool* LayoutDescriptorPools::createPool(const VulkanDescriptorSetLayoutDescription& desc, VkDevice device, VkDescriptorPoolCreateFlags flag)
{
    VulkanDescriptorSetPool* pool = new VulkanDescriptorSetPool(device, flag);
    ASSERT(pool, "nullptr");

    std::array<VkDescriptorPoolSize, 11> sizesCount =
    {
        VkDescriptorPoolSize({ VK_DESCRIPTOR_TYPE_SAMPLER, 0U }),
        VkDescriptorPoolSize({ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 0U }),
        VkDescriptorPoolSize({ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 0U }),
        VkDescriptorPoolSize({ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 0U }),
        VkDescriptorPoolSize({ VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 0U }),
        VkDescriptorPoolSize({ VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 0U }),
        VkDescriptorPoolSize({ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 0U }),
        VkDescriptorPoolSize({ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 0U }),
        VkDescriptorPoolSize({ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 0U }),
        VkDescriptorPoolSize({ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 0U }),
        VkDescriptorPoolSize({ VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 0U })
    };

    for (auto& layout : desc._bindings)
    {
        auto iter = std::find_if(sizesCount.begin(), sizesCount.end(), [&layout](const VkDescriptorPoolSize& s) -> bool
            {
                return layout.descriptorType == s.type;
            });

        if (iter != sizesCount.end())
        {
            ++iter->descriptorCount;
        }
    }

    u32 setCount = LayoutDescriptorPools::s_maxSets;
    std::vector<VkDescriptorPoolSize> sizes;
    for (auto& layout : sizesCount)
    {
        if (layout.descriptorCount > 0)
        {
            u32 count = std::min(layout.descriptorCount * LayoutDescriptorPools::s_multipliers, LayoutDescriptorPools::s_maxSets);
            sizes.push_back({ layout.type, count });
        }
    }

    if (!pool->create(setCount, sizes))
    {
        ASSERT(false, "fail");
        pool->destroy();

        delete pool;
        pool = nullptr;
    }

    return pool;
}

LayoutDescriptorPools::~LayoutDescriptorPools()
{
    ASSERT(_pools.empty(), "not empty");
}

} //namespace vk
} //namespace renderer
} //namespace v3d
#endif //VULKAN_RENDER
