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

size_t SetInfo::Hash::operator()(const SetInfo& set) const
{
    ASSERT(set._key, "empty");
    return set._key;
}

bool SetInfo::Equal::operator()(const SetInfo& set0, const SetInfo& set1) const
{
    if (set0._bindingsInfo.size() != set1._bindingsInfo.size())
    {
        return false;
    }

    if (set0._bindingsInfo.empty() && memcmp(set0._bindingsInfo.data(), set1._bindingsInfo.data(), sizeof(BindingInfo) * set0._bindingsInfo.size()) != 0)
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

    auto& insert = m_descriptorSets.emplace(info, set);
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

u64 VulkanDescriptorSetPool::getCountDescriptorSets() const
{
    return m_descriptorSets.size();
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

VulkanDescriptorSetManager::VulkanDescriptorSetManager(VkDevice device) noexcept
    : m_device(device)
{
}

VulkanDescriptorSetManager::~VulkanDescriptorSetManager()
{
    if (VulkanDeviceCaps::getInstance()->useGlobalDescriptorPool)
    {
        m_genericPools.clearPools();
    }
    else
    {
        m_layoutPools.destroyPools();
    }
}

VkDescriptorSet VulkanDescriptorSetManager::acquireDescriptorSet(const VulkanDescriptorSetLayoutDescription& desc, const SetInfo& info, VkDescriptorSetLayout layoutSet, VulkanDescriptorSetPool*& pool)
{
    VkDescriptorPoolCreateFlags flag = 0;
    if (VulkanDeviceCaps::getInstance()->useGlobalDescriptorPool)
    {
        //finds in pools
        if (m_genericPools._currentDescriptorPool)
        {
            VkDescriptorSet set = m_genericPools._currentDescriptorPool->getDescriptorSet(info);
            if (set != VK_NULL_HANDLE)
            {
                pool = m_genericPools._currentDescriptorPool;
                return set;
            }
        }
        else
        {
            ASSERT(m_genericPools._usedDescriptorPools.empty(), "not empty");
            m_genericPools._currentDescriptorPool = m_genericPools.acquirePool(m_device, flag);
        }

        for (auto& usedPool : m_genericPools._usedDescriptorPools)
        {
            VkDescriptorSet set = usedPool->getDescriptorSet(info);
            if (set != VK_NULL_HANDLE)
            {
                pool = usedPool;
                return set;
            }
        }
        ASSERT(m_genericPools._currentDescriptorPool, "nullptr");

        //create new
        VkDescriptorSet vkDescriptorSet = m_genericPools._currentDescriptorPool->createDescriptorSet(info, layoutSet);
        if (vkDescriptorSet == VK_NULL_HANDLE)
        {
            //try another pool
            m_genericPools._usedDescriptorPools.push_back(m_genericPools._currentDescriptorPool);
            m_genericPools._currentDescriptorPool = m_genericPools.acquirePool(m_device, flag);
            ASSERT(m_genericPools._currentDescriptorPool, "nullptr");

            vkDescriptorSet = m_genericPools._currentDescriptorPool->createDescriptorSet(info, layoutSet);
            ASSERT(vkDescriptorSet != VK_NULL_HANDLE, "fail");
        }

        pool = m_genericPools._currentDescriptorPool;
        return vkDescriptorSet;
    }
    else
    {
        VkDescriptorSet vkDescriptorSet = VK_NULL_HANDLE;
        VulkanDescriptorSetPool* setPool = nullptr;

        auto poolsIter = m_layoutPools._pools.emplace(desc, nullptr);
        if (!poolsIter.second)
        {
            std::list<VulkanDescriptorSetPool*>& poolsList = *(poolsIter.first)->second;
            for (VulkanDescriptorSetPool* usedPool : poolsList)
            {

                ASSERT(usedPool, "nullptr");
                vkDescriptorSet = usedPool->getDescriptorSet(info);
                if (vkDescriptorSet != VK_NULL_HANDLE)
                {
                    pool = usedPool;
                    return vkDescriptorSet;
                }

                vkDescriptorSet = usedPool->createDescriptorSet(info, layoutSet);
                if (vkDescriptorSet == VK_NULL_HANDLE)
                {
                    pool = usedPool;
                    return vkDescriptorSet;
                }
            }
        }

        //create new
        setPool = m_layoutPools.createPool(desc, m_device, flag);
        ASSERT(setPool, "nullptr");


        if (poolsIter.first->second)
        {
            poolsIter.first->second->push_back(setPool);
        }
        else
        {
            std::list<VulkanDescriptorSetPool*>* poolsList = new std::list<VulkanDescriptorSetPool*>();
            poolsList->push_back(setPool);

            poolsIter.first->second = poolsList;
        }

        vkDescriptorSet = setPool->createDescriptorSet(info, layoutSet);
        ASSERT(vkDescriptorSet != VK_NULL_HANDLE, "fail");

        pool = setPool;
        return vkDescriptorSet;
    }

    ASSERT(false, "fail");
    return nullptr;
}



void VulkanDescriptorSetManager::updateDescriptorPools()
{
    if (VulkanDeviceCaps::getInstance()->useGlobalDescriptorPool)
    {
        m_genericPools.updatePools();
    }
}

std::vector<VkDescriptorPoolSize> VulkanDescriptorSetManager::GenericPools::s_poolSizes =
{
    { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,                 std::min(VulkanDescriptorSetManager::GenericPools::s_maxSets, 256U) },
    { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,         std::min(VulkanDescriptorSetManager::GenericPools::s_maxSets, 64U)  },

    { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,         std::min(VulkanDescriptorSetManager::GenericPools::s_maxSets, 128U) },
    { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,                  std::min(VulkanDescriptorSetManager::GenericPools::s_maxSets, 128U) },
    { VK_DESCRIPTOR_TYPE_SAMPLER,                        std::min(VulkanDescriptorSetManager::GenericPools::s_maxSets, 128U) },

    { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,                  std::min(VulkanDescriptorSetManager::GenericPools::s_maxSets, 128U) },
    { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,                 std::min(VulkanDescriptorSetManager::GenericPools::s_maxSets, 128U) },

    { VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER,           std::min(VulkanDescriptorSetManager::GenericPools::s_maxSets, 128U) },
    { VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER,           std::min(VulkanDescriptorSetManager::GenericPools::s_maxSets, 128U) },

    { VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT,               std::min(VulkanDescriptorSetManager::GenericPools::s_maxSets, 8U)   },
};

VulkanDescriptorSetManager::GenericPools::GenericPools() noexcept
    : _currentDescriptorPool(nullptr)
{
}

VulkanDescriptorSetPool* VulkanDescriptorSetManager::GenericPools::acquirePool(VkDevice device, VkDescriptorPoolCreateFlags flag)
{
    ASSERT(VulkanDeviceCaps::getInstance()->useGlobalDescriptorPool, "wrong strategy");
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
    }

    return pool;
}

void VulkanDescriptorSetManager::GenericPools::destroyPools()
{
    ASSERT(_usedDescriptorPools.empty(), "not enmpty");

    if (_currentDescriptorPool)
    {
        ASSERT(!_currentDescriptorPool->isCaptured(), "still used");
        _currentDescriptorPool->destroy();
        delete _currentDescriptorPool;
        _currentDescriptorPool = nullptr;
    }

    for (auto pool : _freeDescriptorPools)
    {
        pool->destroy();
        delete pool;
    }
    _freeDescriptorPools.clear();
}

void VulkanDescriptorSetManager::GenericPools::clearPools()
{
    ASSERT(_usedDescriptorPools.empty(), "strill used");
    for (auto& pool : _freeDescriptorPools)
    {
        ASSERT(!pool->isCaptured(), "still used");
        pool->destroy();
        delete pool;
    }
    _freeDescriptorPools.clear();

    if (_currentDescriptorPool)
    {
        ASSERT(!_currentDescriptorPool->isCaptured(), "still used");
        _currentDescriptorPool->destroy();

        delete _currentDescriptorPool;
        _currentDescriptorPool = nullptr;
    }
}

void VulkanDescriptorSetManager::GenericPools::updatePools()
{
    for (auto iter = _usedDescriptorPools.begin(); iter != _usedDescriptorPools.end();)
    {
        VulkanDescriptorSetPool* pool = (*iter);
        if (!pool->isCaptured())
        {
            _freeDescriptorPools.push_back(pool);
            iter = _usedDescriptorPools.erase(iter);

            pool->reset(0);
        }
        else
        {
            ++iter;
        }
    }
}

VulkanDescriptorSetManager::GenericPools::~GenericPools()
{
    ASSERT(!_currentDescriptorPool, "not nullptr");
    ASSERT(_freeDescriptorPools.empty(), "not empty");
    ASSERT(_usedDescriptorPools.empty(), "not empty");
}

VulkanDescriptorSetPool* VulkanDescriptorSetManager::LayoutPools::createPool(const VulkanDescriptorSetLayoutDescription& desc, VkDevice device, VkDescriptorPoolCreateFlags flag)
{
    ASSERT(!VulkanDeviceCaps::getInstance()->useGlobalDescriptorPool, "wrong strategy");
    
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

    u32 setCount = LayoutPools::s_maxSets;
    std::vector<VkDescriptorPoolSize> sizes;
    for (auto& layout : sizesCount)
    {
        if (layout.descriptorCount > 0)
        {
            u32 count = std::min(layout.descriptorCount * LayoutPools::s_multipliers, LayoutPools::s_maxSets);
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

void VulkanDescriptorSetManager::LayoutPools::destroyPools()
{
    for (auto& pools : _pools)
    {
        ASSERT(pools.second, "nullptr");
        for (auto& pool : *pools.second)
        {
            pool->destroy();
            delete pool;
        }
        pools.second->clear();

        delete pools.second;
    }
    _pools.clear();
}

VulkanDescriptorSetManager::LayoutPools::~LayoutPools()
{
    ASSERT(_pools.empty(), "not empty");
}

} //namespace vk
} //namespace renderer
} //namespace v3d
#endif //VULKAN_RENDER
