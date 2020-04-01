#include "VulkanDescriptorSet.h"
#include "VulkanDebug.h"
#include "VulkanDeviceCaps.h"
#include "Renderer/Shader.h"

#include "Utils/Logger.h"

#include "crc32c/crc32c.h"

#ifdef VULKAN_RENDER
#include "VulkanDescriptorPool.h"
#include "VulkanSwapchain.h"

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

    if (!set0._bindingsInfo.empty() && memcmp(set0._bindingsInfo.data(), set1._bindingsInfo.data(), sizeof(BindingInfo) * set0._bindingsInfo.size()) != 0)
    {
        return false;
    }

    return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////

auto DescriptorPoolStrategy = []() -> GenericDescriptorPools *
{
    if (VulkanDeviceCaps::getInstance()->useGlobalDescriptorPool)
    {
        return new GlobalDescriptorPools();
    }
    else
    {
        return new LayoutDescriptorPools();
    }
};

VulkanDescriptorSetManager::VulkanDescriptorSetManager(VkDevice device, u32 swapchainImages) noexcept
    : m_device(device)
    , m_poolProvider(new VulkanDescriptorPoolProvider(device, DescriptorPoolStrategy()))
{
    m_currentPool.resize(swapchainImages, nullptr);
}

VulkanDescriptorSetManager::~VulkanDescriptorSetManager()
{
    if (m_poolProvider)
    {
        m_poolProvider->destroyPools();

        delete m_poolProvider;
        m_poolProvider = nullptr;
    }
}

VkDescriptorSet VulkanDescriptorSetManager::acquireDescriptorSet(const VulkanDescriptorSetLayoutDescription& desc, const SetInfo& info, VkDescriptorSetLayout layoutSet, VulkanDescriptorSetPool*& pool)
{
    VkDescriptorPoolCreateFlags flag = 0;
    u32 index = VulkanSwapchain::currentSwapchainIndex();
    ASSERT(m_currentPool.size() > index, "wrong index");

    u32 secondTry = 0;
    while (secondTry < 2)
    {
        if (!m_currentPool[index])
        {
            m_currentPool[index] = m_poolProvider->acquirePool(desc, flag);
        }

        VkDescriptorSet descriptorSet = m_currentPool[index]->getDescriptorSet(info);
        if (descriptorSet != VK_NULL_HANDLE)
        {
            pool = m_currentPool[index];
            return descriptorSet;
        }
        else
        {
            descriptorSet = m_currentPool[index]->createDescriptorSet(info, layoutSet);
            if (descriptorSet != VK_NULL_HANDLE)
            {
                pool = m_currentPool[index];
                return descriptorSet;
            }
        }

        m_currentPool[index] = nullptr;
        ++secondTry;
    }
    ASSERT(false, "descriptor set is not created");

    pool = nullptr;
    return VK_NULL_HANDLE;
}

void VulkanDescriptorSetManager::updateDescriptorPools()
{
    m_poolProvider->updatePools();
}

} //namespace vk
} //namespace renderer
} //namespace v3d
#endif //VULKAN_RENDER
