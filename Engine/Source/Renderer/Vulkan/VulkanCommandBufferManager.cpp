#include "VulkanCommandBufferManager.h"
#include "VulkanDebug.h"
#include "VulkanGraphicContext.h"
#include "VulkanDeviceCaps.h"
#include "Utils/Logger.h"

#ifdef VULKAN_RENDER
namespace v3d
{
namespace renderer
{
namespace vk
{

VulkanCommandBufferManager::VulkanCommandBufferManager(Context* const context, const DeviceInfo* info, VkQueue queue)
    : m_device(info->_device)
    , m_queue(queue)
    , m_familyIndex(info->_queueFamilyIndex)

    , m_context(context)

    , m_poolFlag(0)
{
    if (VulkanDeviceCaps::getInstance()->individuallyResetForCommandBuffers)
    {
        m_poolFlag = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    }

    LOG_DEBUG("VulkanCommandBufferManager constructor %llx", this);

    VkCommandPool pool = VulkanCommandBufferManager::createCommandPool(m_device, m_poolFlag, m_familyIndex);
    m_commandPools.push_back(pool);
}

VulkanCommandBufferManager::~VulkanCommandBufferManager()
{
    LOG_DEBUG("~VulkanCommandBufferManager destructor %llx", this);

    ASSERT(m_usedCmdBuffers.empty(), "already used");

    for (u32 level = 0; level < VulkanCommandBuffer::CommandBufferLevelCount; ++level)
    {
        while (!m_freeCmdBuffers[level].empty())
        {
            VulkanCommandBuffer* cmdBuffer = m_freeCmdBuffers[level].front();
            m_freeCmdBuffers[level].pop_front();

            delete cmdBuffer;
        }
    }

    for (auto& pool : m_commandPools)
    {
        VulkanCommandBufferManager::destoryCommandPool(m_device, pool);
    }
    m_commandPools.clear();
}

VulkanCommandBuffer * VulkanCommandBufferManager::acquireNewCmdBuffer(VulkanCommandBuffer::CommandBufferLevel level)
{
    if (!m_freeCmdBuffers[level].empty())
    {
        VulkanCommandBuffer* cmdBuffer = m_freeCmdBuffers[level].front();

        m_freeCmdBuffers[level].pop_front();
        ASSERT(cmdBuffer->m_status == VulkanCommandBuffer::CommandBufferStatus::Ready, "invalid state");

        m_usedCmdBuffers.push_back(cmdBuffer);

        return cmdBuffer;
    }

    for (auto& pool : m_commandPools)
    {
        VkCommandBuffer buffer = VulkanCommandBufferManager::allocateCommandBuffer(m_device, pool, (level == VulkanCommandBuffer::PrimaryBuffer) ? VK_COMMAND_BUFFER_LEVEL_PRIMARY : VK_COMMAND_BUFFER_LEVEL_SECONDARY);
        if (buffer)
        {
            VulkanCommandBuffer* cmdBuffer = new VulkanCommandBuffer(m_context, m_device, level);
            cmdBuffer->init(pool, buffer);

            m_usedCmdBuffers.push_back(cmdBuffer);

            return cmdBuffer;
        }
        else
        {
            LOG_WARNING("VulkanCommandBufferManager::acquireNewCmdBuffer AllocateCommandBuffers is failed. Get another pool");
        }
    }

    VkCommandPool pool = VulkanCommandBufferManager::createCommandPool(m_device, m_poolFlag, m_familyIndex);
    ASSERT(pool, "pool is nullptr");
    m_commandPools.push_back(pool);

    VkCommandBuffer buffer = VulkanCommandBufferManager::allocateCommandBuffer(m_device, pool, (level == VulkanCommandBuffer::PrimaryBuffer) ? VK_COMMAND_BUFFER_LEVEL_PRIMARY : VK_COMMAND_BUFFER_LEVEL_SECONDARY);
    ASSERT(buffer, "buffer is nullptr");

    VulkanCommandBuffer* cmdBuffer = new VulkanCommandBuffer(m_context, m_device, level);
    cmdBuffer->init(pool, buffer);

    m_usedCmdBuffers.push_back(cmdBuffer);

    return cmdBuffer;
}

bool VulkanCommandBufferManager::submit(VulkanCommandBuffer* buffer, VkSemaphore signalSemaphore)
{
    ASSERT(buffer, "buffer is nullptr");
    if (buffer->getStatus() != VulkanCommandBuffer::CommandBufferStatus::End)
    {
        LOG_ERROR("VulkanCommandBufferManager::submit buffer current status not EndBuffer. skip submit");
        ASSERT(false, "invalid state");
        return false;
    }

    VkCommandBuffer cmdBuffer = buffer->getHandle1();

    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.pNext = nullptr;
    submitInfo.waitSemaphoreCount = static_cast<u32>(buffer->m_semaphores.size());
    submitInfo.pWaitSemaphores = buffer->m_semaphores.data();
    submitInfo.pWaitDstStageMask = buffer->m_stageMasks.data();
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &cmdBuffer;
    submitInfo.signalSemaphoreCount = (signalSemaphore != VK_NULL_HANDLE) ? 1 : 0;
    submitInfo.pSignalSemaphores = &signalSemaphore;

    VkResult result = VulkanWrapper::QueueSubmit(m_queue, 1, &submitInfo, buffer->m_fence);
    if (result != VK_SUCCESS)
    {
        LOG_ERROR("VulkanCommandBufferManager::submit vkQueueSubmit. Error %s", ErrorString(result).c_str());
        //TODO:

        buffer->m_status = VulkanCommandBuffer::CommandBufferStatus::Invalid;
        return false;
    }


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

void VulkanCommandBufferManager::waitCompete()
{
    VkResult result = VulkanWrapper::QueueWaitIdle(m_queue);
    if (result != VK_SUCCESS)
    {
        LOG_ERROR("VulkanCommandBufferManager::waitCompete QueueWaitIdle. Error %s", ErrorString(result).c_str());
    }
    VulkanCommandBufferManager::updateCommandBuffers();
}

void VulkanCommandBufferManager::resetPools()
{
    ASSERT(m_usedCmdBuffers.empty(), "already used");

    for (u32 level = 0; level < VulkanCommandBuffer::CommandBufferLevelCount; ++level)
    {
        while (!m_freeCmdBuffers[level].empty())
        {
            VulkanCommandBuffer* cmdBuffer = m_freeCmdBuffers[level].front();
            m_freeCmdBuffers[level].pop_front();

            VulkanCommandBufferManager::freeCommandBuffer(m_device, cmdBuffer->m_pool, cmdBuffer->m_commands);
            delete cmdBuffer;
        }
    }

    VkCommandPoolResetFlags flag = (m_poolFlag == VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT) ? 0 : VK_COMMAND_POOL_RESET_RELEASE_RESOURCES_BIT;
    for (auto& pool : m_commandPools)
    {
        VkResult result = VulkanWrapper::ResetCommandPool(m_device, pool, flag);
        if (result != VK_SUCCESS)
        {
            LOG_ERROR("VulkanCommandBufferManager::resetPools vkResetCommandPool. Error %s", ErrorString(result).c_str());
        }
    }
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
