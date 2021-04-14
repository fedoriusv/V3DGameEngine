#include "VulkanSemaphore.h"

#include "Utils/Logger.h"

#ifdef VULKAN_RENDER
#include "VulkanDebug.h"

namespace v3d
{
namespace renderer
{
namespace vk
{

VulkanSemaphore::VulkanSemaphore() noexcept
    : m_semaphore(VK_NULL_HANDLE)
    , m_semaphoreStatus(SemaphoreStatus::Free)
{
}

VulkanSemaphore::~VulkanSemaphore()
{
    ASSERT(m_semaphore == VK_NULL_HANDLE, "must be nullptr");
}

VkSemaphore VulkanSemaphore::getHandle() const
{
    ASSERT(m_semaphore, "nullptr");
    return m_semaphore;
}

VulkanSemaphoreManager::VulkanSemaphoreManager(VkDevice device) noexcept
    : m_device(device)
{
}

VulkanSemaphoreManager::~VulkanSemaphoreManager()
{
    ASSERT(m_freePools.empty(), "not empty");
    ASSERT(m_usedPools.empty(), "not empty");
}

VulkanSemaphore* VulkanSemaphoreManager::acquireSemaphore()
{
    VulkanSemaphore* semaphore = nullptr;
    if (m_freePools.empty())
    {
        semaphore = createSemaphore();
    }
    else
    {
        semaphore = m_freePools.front();
        semaphore->m_semaphoreStatus = VulkanSemaphore::SemaphoreStatus::Free;
        m_freePools.pop_front();
    }

    m_usedPools.push_back(semaphore);
    ASSERT(semaphore, "nullptr");
    return semaphore;
}

void VulkanSemaphoreManager::clear()
{
    ASSERT(m_usedPools.empty(), "must be empty");
    for (auto semaphore : m_freePools)
    {
        ASSERT(!semaphore->isCaptured(), "must be free");
        VulkanSemaphoreManager::deleteSemaphore(semaphore);
        delete semaphore;
    }
    m_freePools.clear();
}

void VulkanSemaphoreManager::updateSemaphores()
{
    for (auto semaphoreIter = m_usedPools.begin(); semaphoreIter != m_usedPools.end();)
    {
        VulkanSemaphore* semaphore = *semaphoreIter;
        if (!semaphore->isCaptured())
        {
            semaphore->m_semaphoreStatus = VulkanSemaphore::SemaphoreStatus::Signaled;

            m_freePools.push_back(semaphore);
            semaphoreIter = m_usedPools.erase(semaphoreIter);
        }
        else
        {
            ++semaphoreIter;
        }
    }
}

VulkanSemaphore* VulkanSemaphoreManager::createSemaphore()
{
    VkSemaphore vkSemaphore = VK_NULL_HANDLE;
    VkSemaphoreCreateInfo semaphoreCreateInfo = {};
    semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    semaphoreCreateInfo.pNext = nullptr;
    semaphoreCreateInfo.flags = 0;

    VkResult result = VulkanWrapper::CreateSemaphore(m_device, &semaphoreCreateInfo, VULKAN_ALLOCATOR, &vkSemaphore);
    if (result != VK_SUCCESS)
    {
        LOG_ERROR("VulkanSemaphoreManager::createSemaphore vkCreateSemaphore result %d", ErrorString(result).c_str());
        return nullptr;
    }

    VulkanSemaphore* semaphore = new VulkanSemaphore();
    semaphore->m_semaphore = vkSemaphore;
    semaphore->m_semaphoreStatus = VulkanSemaphore::SemaphoreStatus::Free;

    return semaphore;
}

void VulkanSemaphoreManager::deleteSemaphore(VulkanSemaphore* sem)
{
    ASSERT(sem && sem->m_semaphore, "nullptr");
    VulkanWrapper::DestroySemaphore(m_device, sem->m_semaphore, VULKAN_ALLOCATOR);
    sem->m_semaphore = VK_NULL_HANDLE;
}

} //namespace vk
} //namespace renderer
} //namespace v3d
#endif //VULKAN_RENDER
