#include "VulkanCommandBufferManager.h"

#include "Utils/Logger.h"

#ifdef VULKAN_RENDER
#   include "VulkanDebug.h"
#   include "VulkanDevice.h"
#   include "VulkanSemaphore.h"
#   include "VulkanImage.h"
#   include "VulkanSwapchain.h"

namespace v3d
{
namespace renderer
{
namespace vk
{

VulkanCommandBufferManager::VulkanCommandBufferManager(VulkanDevice* device, VulkanSemaphoreManager* semaphoreManager) noexcept
    : m_device(*device)
    , m_semaphoreManager(semaphoreManager)

    , m_poolFlag(VK_COMMAND_POOL_CREATE_TRANSIENT_BIT)
{
    LOG_DEBUG("VulkanCommandBufferManager constructor %llx", this);

    if (m_device.getVulkanDeviceCaps()._individuallyResetForCommandBuffers)
    {
        m_poolFlag |= VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    }
    m_commandPools.resize(m_device.getDeviceInfo()._queues.size());
}

VulkanCommandBufferManager::~VulkanCommandBufferManager()
{
    LOG_DEBUG("~VulkanCommandBufferManager destructor %llx", this);

    ASSERT(m_usedCmdBuffers.empty(), "already used");

    for (u32 level = 0; level < CommandBufferLevel::CommandBufferLevelCount; ++level)
    {
        while (!m_freeCmdBuffers[level].empty())
        {
            VulkanCommandBuffer* cmdBuffer = m_freeCmdBuffers[level].front();
            m_freeCmdBuffers[level].pop_front();

            V3D_DELETE(cmdBuffer, memory::MemoryLabel::MemoryRenderCore);
        }
    }

    for (auto& maskedPool : m_commandPools)
    {
        for (auto pool : maskedPool)
        {
            VulkanCommandBufferManager::destoryCommandPool(m_device.getDeviceInfo()._device, pool);
        }
    }
    m_commandPools.clear();
}

VulkanCommandBuffer* VulkanCommandBufferManager::acquireNewCmdBuffer(Device::DeviceMask queueMask, CommandBufferLevel level)
{
    if (!m_freeCmdBuffers[level].empty())
    {
        VulkanCommandBuffer* cmdBuffer = m_freeCmdBuffers[level].front();

        m_freeCmdBuffers[level].pop_front();
        ASSERT(cmdBuffer->m_status == VulkanCommandBuffer::CommandBufferStatus::Ready, "invalid state");

        m_usedCmdBuffers.push_back(cmdBuffer);

        return cmdBuffer;
    }

    for (auto& pool : m_commandPools[queueMask >> 1])
    {
        VkCommandBuffer buffer = VulkanCommandBufferManager::allocateCommandBuffer(m_device.getDeviceInfo()._device, pool, (level == CommandBufferLevel::PrimaryBuffer) ? VK_COMMAND_BUFFER_LEVEL_PRIMARY : VK_COMMAND_BUFFER_LEVEL_SECONDARY);
        if (buffer)
        {
            VulkanCommandBuffer* cmdBuffer = V3D_NEW(VulkanCommandBuffer, memory::MemoryLabel::MemoryRenderCore)(&m_device, level);
            cmdBuffer->init(queueMask, pool, buffer);

            m_usedCmdBuffers.push_back(cmdBuffer);

            return cmdBuffer;
        }
        else
        {
            LOG_WARNING("VulkanCommandBufferManager::acquireNewCmdBuffer AllocateCommandBuffers is failed. Get another pool");
        }
    }

    VkCommandPool pool = VulkanCommandBufferManager::createCommandPool(m_device.getDeviceInfo()._device, m_poolFlag, m_device.getQueueFamilyIndexByMask(queueMask));
    ASSERT(pool, "pool is nullptr");
    m_commandPools[queueMask >> 1].push_back(pool);

    VkCommandBuffer buffer = VulkanCommandBufferManager::allocateCommandBuffer(m_device.getDeviceInfo()._device, pool, (level == CommandBufferLevel::PrimaryBuffer) ? VK_COMMAND_BUFFER_LEVEL_PRIMARY : VK_COMMAND_BUFFER_LEVEL_SECONDARY);
    ASSERT(buffer, "buffer is nullptr");

    VulkanCommandBuffer* cmdBuffer = V3D_NEW(VulkanCommandBuffer, memory::MemoryLabel::MemoryRenderCore)(&m_device, level);
    cmdBuffer->init(queueMask, pool, buffer);

    m_usedCmdBuffers.push_back(cmdBuffer);

    return cmdBuffer;
}

bool VulkanCommandBufferManager::submit(VulkanCommandBuffer* buffer, const std::vector<VulkanSemaphore*>& signalSemaphores)
{
    ASSERT(buffer, "buffer is nullptr");
    if (buffer->getStatus() != VulkanCommandBuffer::CommandBufferStatus::End)
    {
        LOG_ERROR("VulkanCommandBufferManager::submit buffer current status not EndBuffer. skip submit");
        ASSERT(false, "invalid state");
        return false;
    }

    VkCommandBuffer cmdBuffer = buffer->getHandle();

    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.pNext = nullptr;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &cmdBuffer;

    static thread_local std::vector<VkSemaphore> internalWaitSemaphores;
    internalWaitSemaphores.clear();
    internalWaitSemaphores.reserve(buffer->m_semaphores.size());
    for (VulkanSemaphore* semaphore : buffer->m_semaphores)
    {
        internalWaitSemaphores.push_back(semaphore->getHandle());
        m_semaphoreManager->markSemaphore(semaphore, VulkanSemaphore::SemaphoreStatus::AssignToWaiting);
    }
    submitInfo.waitSemaphoreCount = static_cast<u32>(internalWaitSemaphores.size());
    submitInfo.pWaitSemaphores = internalWaitSemaphores.data();
    submitInfo.pWaitDstStageMask = buffer->m_stageMasks.data();

    static thread_local std::vector<VkSemaphore> internalSignalSemaphores;
    internalSignalSemaphores.clear();
    internalSignalSemaphores.reserve(signalSemaphores.size());
    for (VulkanSemaphore* semaphore : signalSemaphores)
    {
        internalSignalSemaphores.push_back(semaphore->getHandle());
        m_semaphoreManager->markSemaphore(semaphore, VulkanSemaphore::SemaphoreStatus::AssignToSignal);
    }
    submitInfo.signalSemaphoreCount = static_cast<u32>(internalSignalSemaphores.size());
    submitInfo.pSignalSemaphores = internalSignalSemaphores.data();


    VkResult result = VulkanWrapper::QueueSubmit(std::get<0>(m_device.getDeviceInfo()._queues[buffer->m_queueIndex]), 1, &submitInfo, buffer->m_fence);
    if (result != VK_SUCCESS)
    {
        LOG_ERROR("VulkanCommandBufferManager::submit vkQueueSubmit. Error %s", ErrorString(result).c_str());
        //TODO: do something

        buffer->m_status = VulkanCommandBuffer::CommandBufferStatus::Invalid;
        return false;
    }
    //Uncomment for debug
    //vkDeviceWaitIdle(m_device);

    buffer->m_status = VulkanCommandBuffer::CommandBufferStatus::Submit;
    return true;
}

void VulkanCommandBufferManager::updateCommandBuffers()
{
    for (auto iter = m_usedCmdBuffers.begin(); iter != m_usedCmdBuffers.end();)
    {
        VulkanCommandBuffer* cmdBuffer = (*iter);
        cmdBuffer->refreshFenceStatus();
        if (cmdBuffer->m_status == VulkanCommandBuffer::CommandBufferStatus::Finished)
        {
            iter = m_usedCmdBuffers.erase(iter);

            cmdBuffer->m_status = VulkanCommandBuffer::CommandBufferStatus::Ready;
            m_freeCmdBuffers[cmdBuffer->m_level].push_back(cmdBuffer);

            continue;
        }
        ++iter;
    }
}

void VulkanCommandBufferManager::waitCompletion()
{
    VkResult result = VulkanWrapper::DeviceWaitIdle(m_device.getDeviceInfo()._device);
    if (result != VK_SUCCESS)
    {
        LOG_ERROR("VulkanCommandBufferManager::waitCompete DeviceWaitIdle. Error %s", ErrorString(result).c_str());
    }
    VulkanCommandBufferManager::updateCommandBuffers();
}

void VulkanCommandBufferManager::waitQueueCompletion(VkQueue queue)
{
    VkResult result = VulkanWrapper::QueueWaitIdle(queue);
    if (result != VK_SUCCESS)
    {
        LOG_ERROR("VulkanCommandBufferManager::waitCompete QueueWaitIdle. Error %s", ErrorString(result).c_str());
    }
    VulkanCommandBufferManager::updateCommandBuffers();
}

void VulkanCommandBufferManager::resetPools()
{
    ASSERT(m_usedCmdBuffers.empty(), "already used");

    for (u32 level = 0; level < CommandBufferLevel::CommandBufferLevelCount; ++level)
    {
        while (!m_freeCmdBuffers[level].empty())
        {
            VulkanCommandBuffer* cmdBuffer = m_freeCmdBuffers[level].front();
            m_freeCmdBuffers[level].pop_front();

            VulkanCommandBufferManager::freeCommandBuffer(m_device.getDeviceInfo()._device, cmdBuffer->m_pool, cmdBuffer->m_commands);
            delete cmdBuffer;
        }
    }

    VkCommandPoolResetFlags flag = (m_poolFlag & VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT) ? 0 : VK_COMMAND_POOL_RESET_RELEASE_RESOURCES_BIT;
    for (auto& maskedPool : m_commandPools)
    {
        for (auto pool : maskedPool)
        {
            VkResult result = VulkanWrapper::ResetCommandPool(m_device.getDeviceInfo()._device, pool, flag);
            if (result != VK_SUCCESS)
            {
                LOG_ERROR("VulkanCommandBufferManager::resetPools vkResetCommandPool. Error %s", ErrorString(result).c_str());
            }
        }
    }
}

void VulkanCommandBufferManager::drawToSwapchain(VulkanCommandBuffer* buffer, const VulkanSwapchain* swapchain)
{
    buffer->m_acquireSemaphore = swapchain->getCurrentAcquireSemaphore();
    buffer->m_drawingToSwapchain = true;
}

VkCommandPool VulkanCommandBufferManager::createCommandPool(VkDevice device, VkCommandPoolCreateFlags flag, u32 familyIndex)
{
    VkCommandPoolCreateInfo commandPoolCreateInfo = {};
    commandPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    commandPoolCreateInfo.pNext = nullptr;
    commandPoolCreateInfo.flags = flag;
    commandPoolCreateInfo.queueFamilyIndex = familyIndex;

    VkCommandPool pool = VK_NULL_HANDLE;
    VkResult result = VulkanWrapper::CreateCommandPool(device, &commandPoolCreateInfo, VULKAN_ALLOCATOR, &pool);
    if (result != VK_SUCCESS)
    {
        LOG_ERROR("VulkanCommandBufferManager::createCommandPool vkCreateCommandPool. Error %s", ErrorString(result).c_str());
        return VK_NULL_HANDLE;
    }

    return pool;
}

void VulkanCommandBufferManager::destoryCommandPool(VkDevice device, VkCommandPool pool)
{
    VulkanWrapper::DestroyCommandPool(device, pool, VULKAN_ALLOCATOR);
    pool = VK_NULL_HANDLE;
}

VkCommandBuffer VulkanCommandBufferManager::allocateCommandBuffer(VkDevice device, VkCommandPool pool, VkCommandBufferLevel level)
{
    VkCommandBufferAllocateInfo commandBufferAllocateInfo = {};
    commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    commandBufferAllocateInfo.pNext = nullptr;;
    commandBufferAllocateInfo.level = level;
    commandBufferAllocateInfo.commandPool = pool;
    commandBufferAllocateInfo.commandBufferCount = 1;

    VkCommandBuffer buffer = VK_NULL_HANDLE;
    VkResult result = VulkanWrapper::AllocateCommandBuffers(device, &commandBufferAllocateInfo, &buffer);
    if (result != VK_SUCCESS)
    {
        LOG_ERROR("VulkanCommandBufferManager::allocateCommandBuffer vkAllocateCommandBuffers. Error %s. Get another pool", ErrorString(result).c_str());
        return VK_NULL_HANDLE;
    }

    return buffer;
}

void VulkanCommandBufferManager::freeCommandBuffer(VkDevice device, VkCommandPool pool, VkCommandBuffer buffer)
{
    VulkanWrapper::FreeCommandBuffers(device, pool, 1, &buffer);
    buffer = VK_NULL_HANDLE;
}

} //namespace vk
} //namespace renderer
} //namespace v3d
#endif //VULKAN_RENDER
