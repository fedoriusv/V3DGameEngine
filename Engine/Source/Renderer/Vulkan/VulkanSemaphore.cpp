#include "VulkanSemaphore.h"

#include "Utils/Logger.h"

#ifdef VULKAN_RENDER
#include "VulkanDebug.h"
#include "VulkanDeviceCaps.h"
#include "VulkanDevice.h"

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
#if VULKAN_DEBUG_MARKERS
    m_debugName = "Semaphore";
    m_debugName.append(VulkanDebugUtils::k_addressPreffix);
    m_debugName.append(std::to_string(reinterpret_cast<const u64>(this)));
#endif //VULKAN_DEBUG_MARKERS
}

VulkanSemaphore::~VulkanSemaphore()
{
    ASSERT(m_semaphore == VK_NULL_HANDLE, "must be nullptr");
}

VulkanSemaphoreManager::VulkanSemaphoreManager(VulkanDevice* device) noexcept
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
        VulkanSemaphoreManager::markSemaphore(semaphore, VulkanSemaphore::SemaphoreStatus::Free);
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
            VulkanSemaphoreManager::markSemaphore(semaphore, VulkanSemaphore::SemaphoreStatus::Signaled);

            m_freePools.push_back(semaphore);
            semaphoreIter = m_usedPools.erase(semaphoreIter);
        }
        else
        {
            ++semaphoreIter;
        }
    }
}

bool VulkanSemaphoreManager::markSemaphore(VulkanSemaphore* semaphore, VulkanSemaphore::SemaphoreStatus status)
{
    ASSERT(semaphore, "nullptr");
    /*if (status == VulkanSemaphore::SemaphoreStatus::AssignToSignal)
    {
        if (semaphore->m_semaphoreStatus != VulkanSemaphore::SemaphoreStatus::Free)
        {
            ASSERT(false, "must be free before signal");
            return false;
        }
    }*/

    if (status == VulkanSemaphore::SemaphoreStatus::AssignToWaiting)
    {
        if (semaphore->m_semaphoreStatus != VulkanSemaphore::SemaphoreStatus::AssignToSignal && semaphore->m_semaphoreStatus != VulkanSemaphore::SemaphoreStatus::Signaled)
        {
            ASSERT(false, "must be attached to signal");
            return false;
        }
    }

    semaphore->m_semaphoreStatus = status;
    return true;
}

VulkanSemaphore* VulkanSemaphoreManager::createSemaphore(const std::string& name)
{
    VkSemaphore vkSemaphore = VK_NULL_HANDLE;
    VkSemaphoreCreateInfo semaphoreCreateInfo = {};
    semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    semaphoreCreateInfo.pNext = nullptr;
    semaphoreCreateInfo.flags = 0;

    VkResult result = VulkanWrapper::CreateSemaphore(m_device->m_deviceInfo._device, &semaphoreCreateInfo, VULKAN_ALLOCATOR, &vkSemaphore);
    if (result != VK_SUCCESS)
    {
        LOG_ERROR("VulkanSemaphoreManager::createSemaphore vkCreateSemaphore result %d", ErrorString(result).c_str());
        return nullptr;
    }

    VulkanSemaphore* semaphore = new VulkanSemaphore();
    semaphore->m_semaphore = vkSemaphore;
    semaphore->m_semaphoreStatus = VulkanSemaphore::SemaphoreStatus::Free;

#if VULKAN_DEBUG_MARKERS
    if (!name.empty())
    {
        semaphore->m_debugName = name;
    }

    if (m_device->m_deviceCaps._debugUtilsObjectNameEnabled)
    {
        VkDebugUtilsObjectNameInfoEXT debugUtilsObjectNameInfo = {};
        debugUtilsObjectNameInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
        debugUtilsObjectNameInfo.pNext = nullptr;
        debugUtilsObjectNameInfo.objectType = VK_OBJECT_TYPE_SEMAPHORE;
        debugUtilsObjectNameInfo.objectHandle = reinterpret_cast<u64>(vkSemaphore);
        debugUtilsObjectNameInfo.pObjectName = semaphore->m_debugName.c_str();

        VulkanWrapper::SetDebugUtilsObjectName(m_device->m_deviceInfo._device, &debugUtilsObjectNameInfo);
    }
#endif //VULKAN_DEBUG_MARKERS

    return semaphore;
}

void VulkanSemaphoreManager::deleteSemaphore(VulkanSemaphore* semaphore)
{
    ASSERT(semaphore && semaphore->m_semaphore, "nullptr");
    VulkanWrapper::DestroySemaphore(m_device->m_deviceInfo._device, semaphore->m_semaphore, VULKAN_ALLOCATOR);
    semaphore->m_semaphore = VK_NULL_HANDLE;
}

} //namespace vk
} //namespace renderer
} //namespace v3d
#endif //VULKAN_RENDER
