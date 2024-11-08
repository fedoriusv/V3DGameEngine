#include "VulkanDescriptorPool.h"
#include "Utils/Logger.h"

#ifdef VULKAN_RENDER
#   include "VulkanDebug.h"
#   include "VulkanDeviceCaps.h"
#   include "VulkanDevice.h"

namespace v3d
{
namespace renderer
{
namespace vk
{

VulkanDescriptorSetPool::VulkanDescriptorSetPool(VulkanDevice* device, VkDescriptorPoolCreateFlags flag, const std::string& name) noexcept
    : m_device(*device)
    , m_pool(VK_NULL_HANDLE)
    , m_freeIndex(0)
    , m_poolSize(0)
    , m_flag(flag)
{
    LOG_DEBUG("VulkanDescriptorSetPool::VulkanDescriptorSetPool constructor %llx", this);
#if VULKAN_DEBUG_MARKERS
    m_debugName = name.empty() ? "DescriptorPool" : name;
    m_debugName.append(VulkanDebugUtils::k_addressPreffix);
    m_debugName.append(std::to_string(reinterpret_cast<const u64>(this)));
#endif //VULKAN_DEBUG_MARKERS
}

VulkanDescriptorSetPool::~VulkanDescriptorSetPool()
{
    ASSERT(m_pool == VK_NULL_HANDLE, "not nullptr");
}

bool VulkanDescriptorSetPool::create(u32 setsCount, const std::vector<VkDescriptorPoolSize>& sizes)
{
    ASSERT(!m_pool, "not nullptr");
    VkDescriptorPoolCreateInfo descriptorPoolCreateInfo = {};
    descriptorPoolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    descriptorPoolCreateInfo.pNext = nullptr;
    descriptorPoolCreateInfo.flags = m_flag;
    descriptorPoolCreateInfo.maxSets = setsCount;
    descriptorPoolCreateInfo.poolSizeCount = static_cast<u32>(sizes.size());
    descriptorPoolCreateInfo.pPoolSizes = sizes.data();

    VkResult result = VulkanWrapper::CreateDescriptorPool(m_device.getDeviceInfo()._device, &descriptorPoolCreateInfo, VULKAN_ALLOCATOR, &m_pool);
    if (result != VK_SUCCESS)
    {
        LOG_ERROR("VulkanDescriptorSetPool::create vkCreateDescriptorPool is failed. Error: %s", ErrorString(result).c_str());
        return false;
    }

    m_freeIndex = 0;
    m_poolSize = setsCount;

#if VULKAN_DEBUG_MARKERS
    if (m_device.getVulkanDeviceCaps()._debugUtilsObjectNameEnabled)
    {
        VkDebugUtilsObjectNameInfoEXT debugUtilsObjectNameInfo = {};
        debugUtilsObjectNameInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
        debugUtilsObjectNameInfo.pNext = nullptr;
        debugUtilsObjectNameInfo.objectType = VK_OBJECT_TYPE_DESCRIPTOR_POOL;
        debugUtilsObjectNameInfo.objectHandle = reinterpret_cast<u64>(m_pool);
        debugUtilsObjectNameInfo.pObjectName = m_debugName.c_str();

        VulkanWrapper::SetDebugUtilsObjectName(m_device.getDeviceInfo()._device, &debugUtilsObjectNameInfo);
    }
#endif //VULKAN_DEBUG_MARKERS

    return true;
}

void VulkanDescriptorSetPool::destroy()
{
    if (m_pool)
    {
        VulkanWrapper::DestroyDescriptorPool(m_device.getDeviceInfo()._device, m_pool, VULKAN_ALLOCATOR);
        m_pool = VK_NULL_HANDLE;
    }
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
    VkResult result = VulkanWrapper::AllocateDescriptorSets(m_device.getDeviceInfo()._device, &descriptorSetAllocateInfo, &descriptorSet);
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

    VkResult result = VulkanWrapper::FreeDescriptorSets(m_device.getDeviceInfo()._device, m_pool, 1, &descriptorSet);
    if (result != VK_SUCCESS)
    {
        LOG_ERROR("VulkanDescriptorPool::freeDescriptorSet vkFreeDescriptorSets is failed. Error: %s", ErrorString(result).c_str());
        return false;
    }

    descriptorSet = VK_NULL_HANDLE;
    return true;
}

bool VulkanDescriptorSetPool::allocateDescriptorSets(VkDescriptorSetLayout layout, u32 count, std::vector<VkDescriptorSet>& descriptorSets)
{
    VkDescriptorSetAllocateInfo descriptorSetAllocateInfo = {};
    descriptorSetAllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    descriptorSetAllocateInfo.pNext = nullptr; //VkDescriptorSetVariableDescriptorCountAllocateInfoEXT
    descriptorSetAllocateInfo.descriptorPool = m_pool;
    descriptorSetAllocateInfo.descriptorSetCount = count;
    descriptorSetAllocateInfo.pSetLayouts = &layout;

    VkResult result = VulkanWrapper::AllocateDescriptorSets(m_device.getDeviceInfo()._device, &descriptorSetAllocateInfo, descriptorSets.data());
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

    VkResult result = VulkanWrapper::FreeDescriptorSets(m_device.getDeviceInfo()._device, m_pool, static_cast<u32>(descriptorSets.size()), descriptorSets.data());
    if (result != VK_SUCCESS)
    {
        LOG_ERROR("VulkanDescriptorPool::destroyDescriptorSets vkFreeDescriptorSets is failed. Error: %s", ErrorString(result).c_str());
        return false;
    }

    return true;
}

bool VulkanDescriptorSetPool::reset()
{
    VkResult result = VulkanWrapper::ResetDescriptorPool(m_device.getDeviceInfo()._device, m_pool, m_flag);
    if (result != VK_SUCCESS)
    {
        LOG_ERROR("VulkanDescriptorPool::reset vkResetDescriptorPool is failed. Error: %s", ErrorString(result).c_str());
        return false;
    }

    m_freeIndex = 0;

    return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////


VulkanDescriptorPoolProvider::VulkanDescriptorPoolProvider(VulkanDevice* device, GenericDescriptorPools* stategy) noexcept
    : m_device(*device)
    , m_descriptorPools(stategy)
{
}

VulkanDescriptorPoolProvider::~VulkanDescriptorPoolProvider()
{
    ASSERT(m_descriptorPools, "nullptr");
    V3D_DELETE(m_descriptorPools, memory::MemoryLabel::MemoryRenderCore);
}

VulkanDescriptorSetPool* VulkanDescriptorPoolProvider::acquireDescriptorSetPool(const VulkanDescriptorSetLayoutDescription& desc, VkDescriptorPoolCreateFlags flag)
{
    ASSERT(m_descriptorPools, "nullptr");
    return m_descriptorPools->acquirePool(desc, &m_device, flag);
}

void VulkanDescriptorPoolProvider::destroyDescriptorSetPools()
{
    ASSERT(m_descriptorPools, "nullptr");
    m_descriptorPools->destroyPools();
}

void VulkanDescriptorPoolProvider::updateDescriptorSetPools()
{
    ASSERT(m_descriptorPools, "nullptr");
    m_descriptorPools->updatePools();
}

void VulkanDescriptorPoolProvider::resetDescriptorSetPools()
{
    ASSERT(m_descriptorPools, "nullptr");
    m_descriptorPools->resetPools();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////

std::vector<VkDescriptorPoolSize> GlobalDescriptorPools::descriptorPoolSize(u32 countSets)
{
    return
    {
        { VK_DESCRIPTOR_TYPE_SAMPLER,                        std::min(countSets, 1024U) },
        { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,                  std::min(countSets, 1024U) },

        { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,                 std::min(countSets, 2048U) },
        { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,         std::min(countSets, 512U) },

        { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,                  std::min(countSets, 256U) },
        { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,                 std::min(countSets, 256U) },
        { VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER,           std::min(countSets, 128U) },
        { VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER,           std::min(countSets, 128U) },
    };
};

VulkanDescriptorSetPool* GlobalDescriptorPools::acquirePool(const VulkanDescriptorSetLayoutDescription& desc, VulkanDevice* device, VkDescriptorPoolCreateFlags flag)
{
    ASSERT(device->getVulkanDeviceCaps()._useGlobalDescriptorPool, "wrong strategy");

    VulkanDescriptorSetPool* pool = nullptr;
    if (_currentDescriptorPool)
    {
        return _currentDescriptorPool;
    }

    if (_freeDescriptorPools.empty())
    {
        pool = V3D_NEW(VulkanDescriptorSetPool, memory::MemoryLabel::MemoryRenderCore)(device, flag);
        ASSERT(pool, "nullptr");

        u32 countSets = device->getVulkanDeviceCaps()._globalDescriptorPoolSize;
        std::vector<VkDescriptorPoolSize> sizes = GlobalDescriptorPools::descriptorPoolSize(countSets);
        if (!pool->create(countSets, sizes))
        {
            ASSERT(false, "fail");
            pool->destroy();

            V3D_DELETE(pool, memory::MemoryLabel::MemoryRenderCore);
            pool = nullptr;
        }
    }
    else
    {
        pool = _freeDescriptorPools.front();
        _freeDescriptorPools.pop_front();
        ASSERT(!pool->isUsed(), "pool is captured");
    }

    _currentDescriptorPool = pool;
    return pool;
}

void GlobalDescriptorPools::destroyPools()
{
    ASSERT(!_currentDescriptorPool, "empty");
    ASSERT(_usedDescriptorPools.empty(), "strill used");
    for (auto& pool : _freeDescriptorPools)
    {
        ASSERT(!pool->isUsed(), "still used");
        pool->destroy();
        V3D_DELETE(pool, memory::MemoryLabel::MemoryRenderCore);
    }
    _freeDescriptorPools.clear();
}

void GlobalDescriptorPools::updatePools()
{
    if (_currentDescriptorPool)
    {
        _usedDescriptorPools.push_back(_currentDescriptorPool);
        _currentDescriptorPool = nullptr;
    }

    for (auto iter = _usedDescriptorPools.begin(); iter != _usedDescriptorPools.end();)
    {
        VulkanDescriptorSetPool* pool = (*iter);
        if (!pool->isUsed())
        {
            pool->reset();
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
        ASSERT(!pool->isUsed(), "pool is captured");
        pool->reset();
    }
}

GlobalDescriptorPools::~GlobalDescriptorPools()
{
    ASSERT(_freeDescriptorPools.empty(), "not empty");
    ASSERT(_usedDescriptorPools.empty(), "not empty");
}


const u32 LayoutDescriptorPools::s_maxSets;

VulkanDescriptorSetPool* LayoutDescriptorPools::acquirePool(const VulkanDescriptorSetLayoutDescription& desc, VulkanDevice* device, VkDescriptorPoolCreateFlags flag)
{
    ASSERT(!device->getVulkanDeviceCaps()._useGlobalDescriptorPool, "wrong strategy");
    LayoutPools* layoutPools = nullptr;

    auto poolsIter = _pools.emplace(desc, nullptr);
    if (!poolsIter.second)
    {
        layoutPools  = poolsIter.first->second;
    }
    else
    {
        layoutPools = V3D_NEW(LayoutPools, memory::MemoryLabel::MemoryRenderCore)();
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
        ASSERT(!pool->isUsed(), "pool is captured");
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
            V3D_DELETE(pool, memory::MemoryLabel::MemoryRenderCore);
        }
        layoutPools->_usedList.clear();
        V3D_DELETE(layoutPools, memory::MemoryLabel::MemoryRenderCore);
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
            if (!pool->isUsed())
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
            ASSERT(!pool->isUsed(), "pool is captured");
            pool->reset();
        }
    }
}

VulkanDescriptorSetPool* LayoutDescriptorPools::createPool(const VulkanDescriptorSetLayoutDescription& desc, VulkanDevice* device, VkDescriptorPoolCreateFlags flag)
{
    VulkanDescriptorSetPool* pool = V3D_NEW(VulkanDescriptorSetPool, memory::MemoryLabel::MemoryRenderCore)(device, flag);
    ASSERT(pool, "nullptr");

    std::array<VkDescriptorPoolSize, 9> sizesCount =
    {
        VkDescriptorPoolSize({ VK_DESCRIPTOR_TYPE_SAMPLER, 0U }),
        VkDescriptorPoolSize({ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 0U }),
        VkDescriptorPoolSize({ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 0U }),
        VkDescriptorPoolSize({ VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 0U }),
        VkDescriptorPoolSize({ VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 0U }),
        VkDescriptorPoolSize({ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 0U }),
        VkDescriptorPoolSize({ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 0U }),
        VkDescriptorPoolSize({ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 0U }),
        VkDescriptorPoolSize({ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 0U }),
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

        V3D_DELETE(pool, memory::MemoryLabel::MemoryRenderCore);
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
