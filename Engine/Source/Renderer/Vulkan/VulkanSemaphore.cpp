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

#if DEBUG_OBJECT_MEMORY
    std::set<VulkanSemaphore*> VulkanSemaphore::s_objects;
#endif //DEBUG_OBJECT_MEMORY

VulkanSemaphore::VulkanSemaphore(VulkanDevice* device, SemaphoreType type) noexcept
    : m_device(*device)
    , m_semaphore(VK_NULL_HANDLE)
    , m_type(type)
    , m_status(SemaphoreStatus::Free)
{
    LOG_DEBUG("VulkanSemaphore::VulkanSemaphore constructor %llx", this);
#if VULKAN_DEBUG_MARKERS
    m_debugName = "Semaphore";
    m_debugName.append(VulkanDebugUtils::k_addressPreffix);
    m_debugName.append(std::to_string(reinterpret_cast<const u64>(this)));
#endif //VULKAN_DEBUG_MARKERS

#if DEBUG_OBJECT_MEMORY
    s_objects.insert(this);
#endif //DEBUG_OBJECT_MEMORY
}

VulkanSemaphore::~VulkanSemaphore()
{
    LOG_DEBUG("VulkanSemaphore::VulkanSemaphore destructor %llx", this);
#if DEBUG_OBJECT_MEMORY
    s_objects.erase(this);
#endif //DEBUG_OBJECT_MEMORY
    ASSERT(m_semaphore == VK_NULL_HANDLE, "must be nullptr");
}

#if VULKAN_DEBUG_MARKERS
void VulkanSemaphore::fenceTracker(VulkanFence* fence, u64 value, u64 frame)
{
    if (m_device.getVulkanDeviceCaps()._debugUtilsObjectNameEnabled)
    {
        std::string debugName(m_debugName);
        debugName.append("_Fence:");
        debugName.append(std::to_string(reinterpret_cast<const u64>(fence)));
        debugName.append("_value:");
        debugName.append(std::to_string(value));

        VkDebugUtilsObjectNameInfoEXT debugUtilsObjectNameInfo = {};
        debugUtilsObjectNameInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
        debugUtilsObjectNameInfo.pNext = nullptr;
        debugUtilsObjectNameInfo.objectType = VK_OBJECT_TYPE_SEMAPHORE;
        debugUtilsObjectNameInfo.objectHandle = reinterpret_cast<u64>(m_semaphore);
        debugUtilsObjectNameInfo.pObjectName = debugName.c_str();

        VulkanWrapper::SetDebugUtilsObjectName(m_device.getDeviceInfo()._device, &debugUtilsObjectNameInfo);
    }
}
#endif

VulkanSemaphoreManager::VulkanSemaphoreManager(VulkanDevice* device) noexcept
    : m_device(*device)
{
}

VulkanSemaphoreManager::~VulkanSemaphoreManager()
{
    ASSERT(m_freeList.empty(), "not empty");
    ASSERT(m_usedList.empty(), "not empty");
}

VulkanSemaphore* VulkanSemaphoreManager::acquireFreeSemaphore()
{
    std::lock_guard lock(m_mutex);

    VulkanSemaphore* semaphore = nullptr;
    if (m_freeList.empty())
    {
        semaphore = createSemaphore(VulkanSemaphore::SemaphoreType::Binary);
    }
    else
    {
        semaphore = m_freeList.front();
        ASSERT(semaphore->m_status == VulkanSemaphore::SemaphoreStatus::Free, "nullptr");
        m_freeList.pop_front();
    }

    ASSERT(semaphore, "nullptr");
    m_usedList.push_back(semaphore);

    return semaphore;
}

void VulkanSemaphoreManager::clear()
{
    std::lock_guard lock(m_mutex);

    ASSERT(m_usedList.empty(), "must be empty");
    for (auto& semaphore : m_freeList)
    {
        VulkanSemaphoreManager::deleteSemaphore(semaphore);
    }
    m_freeList.clear();
}

void VulkanSemaphoreManager::updateStatus(bool forced)
{
    std::lock_guard lock(m_mutex);

    for (auto semaphoreIter = m_usedList.begin(); semaphoreIter != m_usedList.end();)
    {
        VulkanSemaphore* semaphore = *semaphoreIter;
        // We dont want to free signaled semaphores, it will be used farther
        if (!semaphore->isUsed() && (forced || semaphore->m_status != VulkanSemaphore::SemaphoreStatus::AssignForSignal))
        {
            m_freeList.push_back(semaphore);
            semaphoreIter = m_usedList.erase(semaphoreIter);

            VulkanSemaphoreManager::markSemaphore(semaphore, VulkanSemaphore::SemaphoreStatus::Free);

            continue;
        }

        ++semaphoreIter;
    }
}

bool VulkanSemaphoreManager::markSemaphore(VulkanSemaphore* semaphore, VulkanSemaphore::SemaphoreStatus status)
{
    ASSERT(semaphore, "nullptr");
    if (status == VulkanSemaphore::SemaphoreStatus::Free && semaphore->isUsed())
    {
        ASSERT(false, "must be unused");
        return false;
    }

    if (status == VulkanSemaphore::SemaphoreStatus::AssignForWaiting || status == VulkanSemaphore::SemaphoreStatus::AssignToPresent)
    {
        if (semaphore->m_status != VulkanSemaphore::SemaphoreStatus::AssignForSignal && semaphore->m_status != VulkanSemaphore::SemaphoreStatus::Signaled)
        {
            ASSERT(false, "must be attached to signal");
            return false;
        }
    }

    semaphore->m_status = status;
    return true;
}

VulkanSemaphore* VulkanSemaphoreManager::createSemaphore(VulkanSemaphore::SemaphoreType type, const std::string& name)
{
    VkSemaphore vkSemaphore = VK_NULL_HANDLE;

    VkSemaphoreTypeCreateInfo semaphoreTypeCreateInfo = {};
    semaphoreTypeCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO;
    semaphoreTypeCreateInfo.pNext = nullptr;
    if (m_device.getVulkanDeviceCaps()._timelineSemaphore && type == VulkanSemaphore::SemaphoreType::Timeline)
    {
        semaphoreTypeCreateInfo.semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE;
        semaphoreTypeCreateInfo.initialValue = 0;
    }
    else
    {
        ASSERT(type == VulkanSemaphore::SemaphoreType::Binary, "not binary");
        semaphoreTypeCreateInfo.semaphoreType = VK_SEMAPHORE_TYPE_BINARY;
        semaphoreTypeCreateInfo.initialValue = 0;
    }

    VkSemaphoreCreateInfo semaphoreCreateInfo = {};
    semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    semaphoreCreateInfo.pNext = &semaphoreTypeCreateInfo;
    semaphoreCreateInfo.flags = 0;

    VkResult result = VulkanWrapper::CreateSemaphore(m_device.getDeviceInfo()._device, &semaphoreCreateInfo, VULKAN_ALLOCATOR, &vkSemaphore);
    if (result != VK_SUCCESS)
    {
        LOG_ERROR("VulkanSemaphoreManager::createSemaphore vkCreateSemaphore result %d", ErrorString(result).c_str());
        return nullptr;
    }

    VulkanSemaphore* semaphore = V3D_NEW(VulkanSemaphore, memory::MemoryLabel::MemoryRenderCore)(&m_device, type);
    semaphore->m_semaphore = vkSemaphore;
    semaphore->m_status = VulkanSemaphore::SemaphoreStatus::Free;

#if VULKAN_DEBUG_MARKERS
    if (!name.empty())
    {
        semaphore->m_debugName = name;
    }

    if (m_device.getVulkanDeviceCaps()._debugUtilsObjectNameEnabled)
    {
        VkDebugUtilsObjectNameInfoEXT debugUtilsObjectNameInfo = {};
        debugUtilsObjectNameInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
        debugUtilsObjectNameInfo.pNext = nullptr;
        debugUtilsObjectNameInfo.objectType = VK_OBJECT_TYPE_SEMAPHORE;
        debugUtilsObjectNameInfo.objectHandle = reinterpret_cast<u64>(vkSemaphore);
        debugUtilsObjectNameInfo.pObjectName = semaphore->m_debugName.c_str();

        VulkanWrapper::SetDebugUtilsObjectName(m_device.getDeviceInfo()._device, &debugUtilsObjectNameInfo);
    }
#endif //VULKAN_DEBUG_MARKERS

    return semaphore;
}

void VulkanSemaphoreManager::deleteSemaphore(VulkanSemaphore* semaphore)
{
    ASSERT(semaphore && semaphore->m_semaphore, "nullptr");
    VulkanWrapper::DestroySemaphore(m_device.getDeviceInfo()._device, semaphore->m_semaphore, VULKAN_ALLOCATOR);
    semaphore->m_semaphore = VK_NULL_HANDLE;

    V3D_DELETE(semaphore, memory::MemoryLabel::MemoryRenderCore);
}

} //namespace vk
} //namespace renderer
} //namespace v3d
#endif //VULKAN_RENDER
