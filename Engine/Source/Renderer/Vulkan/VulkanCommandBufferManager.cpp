#include "VulkanCommandBufferManager.h"

#include "Utils/Logger.h"

#ifdef VULKAN_RENDER
#   include "VulkanDebug.h"
#   include "VulkanDevice.h"
#   include "VulkanSemaphore.h"
#   include "VulkanImage.h"
#   include "VulkanSwapchain.h"
#   include "VulkanCommandBuffer.h"

namespace v3d
{
namespace renderer
{
namespace vk
{

#define GENERAL_LAYER_FOR_SHADER_ONLY 1

VkImageLayout VulkanTransitionState::convertTransitionStateToImageLayout(TransitionOp state)
{
    switch (state)
    {
    case TransitionOp::TransitionOp_Undefined:
        return VK_IMAGE_LAYOUT_UNDEFINED;

    case TransitionOp::TransitionOp_ShaderRead:
        return VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    case TransitionOp::TransitionOp_ColorAttachment:
        return VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    case TransitionOp::TransitionOp_DepthStencilAttachment:
        return VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    case TransitionOp::TransitionOp_GeneralGraphic:
    case TransitionOp::TransitionOp_GeneralCompute:
        return VK_IMAGE_LAYOUT_GENERAL;

    case TransitionOp::TransitionOp_Present:
        return VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    default:
        ASSERT(false, "unknown");
    }

    return VK_IMAGE_LAYOUT_UNDEFINED;
}

VkPipelineStageFlags VulkanTransitionState::selectStageFlagsByImageLayout(VkImageLayout layout)
{
    switch (layout)
    {
    case VK_IMAGE_LAYOUT_UNDEFINED:
    case VK_IMAGE_LAYOUT_PREINITIALIZED:
        return VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;

    case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
        return VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;

    case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
        return VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

    case VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL:
    case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
        return  VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;

    case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
    case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
        return VK_PIPELINE_STAGE_TRANSFER_BIT;

    default:
        ASSERT(false, "check");
        return VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
    }
}

std::tuple<VkAccessFlags, VkAccessFlags> VulkanTransitionState::getAccessFlagsFromImageLayout(VkImageLayout oldLayout, VkImageLayout newLayout)
{
    VkAccessFlags srcFlag = 0;
    VkAccessFlags dstFlag = 0;

    switch (oldLayout)
    {
    case VK_IMAGE_LAYOUT_UNDEFINED:
        srcFlag = 0;
        break;

    case VK_IMAGE_LAYOUT_PREINITIALIZED:
        srcFlag = VK_ACCESS_HOST_WRITE_BIT;
        break;

    case VK_IMAGE_LAYOUT_GENERAL:
#if GENERAL_LAYER_FOR_SHADER_ONLY
        srcFlag |= VK_ACCESS_SHADER_READ_BIT;
        srcFlag |= VK_ACCESS_SHADER_WRITE_BIT;
#else
        srcFlag |= VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;
        srcFlag |= VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        srcFlag |= VK_ACCESS_TRANSFER_WRITE_BIT;
        srcFlag |= VK_ACCESS_TRANSFER_READ_BIT;
        srcFlag |= VK_ACCESS_SHADER_READ_BIT;
        srcFlag |= VK_ACCESS_SHADER_WRITE_BIT;
#endif
        break;

    case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
        srcFlag |= VK_ACCESS_TRANSFER_READ_BIT;
        srcFlag |= VK_ACCESS_TRANSFER_WRITE_BIT;
        break;

    case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
        srcFlag |= VK_ACCESS_TRANSFER_READ_BIT;
        srcFlag |= VK_ACCESS_TRANSFER_WRITE_BIT;
        break;

    case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
        srcFlag = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        break;

    case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
        srcFlag = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
        break;

    case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
        srcFlag = VK_ACCESS_SHADER_READ_BIT;
        break;

    case VK_IMAGE_LAYOUT_PRESENT_SRC_KHR:
        dstFlag = VK_ACCESS_MEMORY_READ_BIT;
        break;

    default:
        ASSERT(false, "not handled");

    }

    switch (newLayout)
    {
    case VK_IMAGE_LAYOUT_GENERAL:
#if GENERAL_LAYER_FOR_SHADER_ONLY
        dstFlag |= VK_ACCESS_SHADER_READ_BIT;
        dstFlag |= VK_ACCESS_SHADER_WRITE_BIT;
#else
        dstFlag |= VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;
        dstFlag |= VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        dstFlag |= VK_ACCESS_TRANSFER_WRITE_BIT;
        dstFlag |= VK_ACCESS_TRANSFER_READ_BIT;
        dstFlag |= VK_ACCESS_SHADER_READ_BIT;
        dstFlag |= VK_ACCESS_SHADER_WRITE_BIT;
#endif
        break;

    case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
        dstFlag |= VK_ACCESS_TRANSFER_WRITE_BIT;
        dstFlag |= VK_ACCESS_TRANSFER_READ_BIT;
        break;

    case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
        dstFlag |= VK_ACCESS_TRANSFER_WRITE_BIT;
        dstFlag |= VK_ACCESS_TRANSFER_READ_BIT;
        break;

    case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
        dstFlag = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        break;

    case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
        dstFlag = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
        break;

    case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
        dstFlag = VK_ACCESS_SHADER_READ_BIT;
        break;

    case VK_IMAGE_LAYOUT_PRESENT_SRC_KHR:
        dstFlag = VK_ACCESS_MEMORY_READ_BIT;
        break;

    case VK_IMAGE_LAYOUT_UNDEFINED:
        dstFlag = 0;
        break;

    default:
        ASSERT(false, "not handled");
    }

    return { srcFlag, dstFlag };
}

std::tuple<VkPipelineStageFlags, VkPipelineStageFlags> VulkanTransitionState::getPipelineStageFlagsFromImageLayout(VulkanImage* vulkanImage, VkImageLayout oldLayout, VkImageLayout newLayout, bool toCompute)
{
    VkPipelineStageFlags srcStage = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
    VkPipelineStageFlags dstStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;

    //to general
    if (newLayout == VK_IMAGE_LAYOUT_GENERAL)
    {
        dstStage = toCompute ? VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT : VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT;
        if (oldLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
        {
            srcStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        }
        else if (oldLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
        {
            srcStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
        }
        else if (oldLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
        {
            srcStage = VK_PIPELINE_STAGE_VERTEX_SHADER_BIT | VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        }
    }

    //to shader read
    if (newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
    {
        dstStage = VK_PIPELINE_STAGE_VERTEX_SHADER_BIT | VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        if (oldLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
        {
            srcStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
            dstStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        }
        else if (oldLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
        {
            srcStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
            dstStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        }
        else if (oldLayout == VK_IMAGE_LAYOUT_GENERAL)
        {
            srcStage = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT; //TODO
            dstStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        }
    }

    //to color attachment
    if (newLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
    {
        ASSERT(VulkanImage::isColorFormat(vulkanImage->getFormat()), "wrong layout");
        srcStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        dstStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    }

    //to preset form attachment
    if (newLayout == VK_IMAGE_LAYOUT_PRESENT_SRC_KHR && oldLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
    {
        ASSERT(vulkanImage->hasUsageFlag(TextureUsage::TextureUsage_Backbuffer), "mast be only swapchain");
        srcStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dstStage = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
    }

    //form present to attachment
    if (newLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL && oldLayout == VK_IMAGE_LAYOUT_PRESENT_SRC_KHR)
    {
        ASSERT(vulkanImage->hasUsageFlag(TextureUsage::TextureUsage_Backbuffer), "mast be only swapchain");
        srcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        dstStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    }

    return { srcStage, dstStage };
}

void VulkanTransitionState::transitionImage(VulkanCommandBuffer* cmdBuffer, std::tuple<VulkanImage*, RenderTexture::Subresource>& image, VkImageLayout newLayout, bool toCompute)
{
    VulkanImage* vulkanImage = static_cast<VulkanImage*>(std::get<0>(image));
    RenderTexture::Subresource& resource = std::get<1>(image);

    VkImageLayout oldLayout = cmdBuffer->getResourceStateTracker().getLayout(vulkanImage, std::get<1>(image));
    auto [srcStage, dstStage] = VulkanTransitionState::getPipelineStageFlagsFromImageLayout(vulkanImage, oldLayout, newLayout, toCompute);

    cmdBuffer->cmdPipelineBarrier(vulkanImage, resource, srcStage, dstStage, newLayout);
}


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

#if TRACE_PROFILER_GPU_ENABLE
    ASSERT(m_tracyContext == nullptr, "must be nullptr");
    m_tracyContext = TracyVkContextHostCalibrated(m_device.getDeviceInfo()._instance, m_device.getDeviceInfo()._physicalDevice, m_device.getDeviceInfo()._device, &vkGetInstanceProcAddr, &vkGetDeviceProcAddr);
#endif
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

#if TRACE_PROFILER_GPU_ENABLE
    ASSERT(m_tracyContext != nullptr, "must be valid");
    TracyVkDestroy(m_tracyContext);
    m_tracyContext = nullptr;
#endif
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
#if TRACE_PROFILER_GPU_ENABLE
            ASSERT(m_tracyContext, "must be valid");
            cmdBuffer->m_tracyContext = m_tracyContext;
#endif
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
#if TRACE_PROFILER_GPU_ENABLE
    ASSERT(m_tracyContext, "must be valid");
    cmdBuffer->m_tracyContext = m_tracyContext;
#endif

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
        m_semaphoreManager->markSemaphore(semaphore, VulkanSemaphore::SemaphoreStatus::AssignForWaiting);
        buffer->captureResource(semaphore);
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
        m_semaphoreManager->markSemaphore(semaphore, VulkanSemaphore::SemaphoreStatus::AssignForSignal);
        buffer->captureResource(semaphore);
    }
    submitInfo.signalSemaphoreCount = static_cast<u32>(internalSignalSemaphores.size());
    submitInfo.pSignalSemaphores = internalSignalSemaphores.data();

#if VULKAN_DEBUG_MARKERS
	if (m_device.getVulkanDeviceCaps()._debugUtilsObjectNameEnabled)
	{
		std::string debugName;
		debugName.append("Fence:");
		debugName.append(std::to_string(reinterpret_cast<const u64>(buffer->m_fence)));
		debugName.append("_value:");
		debugName.append(std::to_string(buffer->m_fence->getValue()));

		VkDebugUtilsObjectNameInfoEXT debugUtilsObjectNameInfo = {};
		debugUtilsObjectNameInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
		debugUtilsObjectNameInfo.pNext = nullptr;
		debugUtilsObjectNameInfo.objectType = VK_OBJECT_TYPE_FENCE;
		debugUtilsObjectNameInfo.objectHandle = reinterpret_cast<u64>(buffer->m_fence->getHandle());
		debugUtilsObjectNameInfo.pObjectName = debugName.c_str();

		VulkanWrapper::SetDebugUtilsObjectName(m_device.getDeviceInfo()._device, &debugUtilsObjectNameInfo);
	}
#endif

    VkResult result = VulkanWrapper::QueueSubmit(std::get<0>(m_device.getDeviceInfo()._queues[buffer->m_queueIndex]), 1, &submitInfo, buffer->m_fence->getHandle());
    if (result != VK_SUCCESS)
    {
        LOG_ERROR("VulkanCommandBufferManager::submit vkQueueSubmit. Error %s", ErrorString(result).c_str());
        //TODO: do something

        buffer->m_status = VulkanCommandBuffer::CommandBufferStatus::Invalid;
        return false;
    }

    buffer->m_status = VulkanCommandBuffer::CommandBufferStatus::Submit;
    buffer->m_capturedFrameIndex = buffer->getActiveSwapchain() ? buffer->getActiveSwapchain()->getCurrentFrameIndex() : 0;

    //DEBUG: Uncomment for debug
    //vkDeviceWaitIdle(m_device);

    return true;
}

void VulkanCommandBufferManager::updateStatus()
{
    for (auto iter = m_usedCmdBuffers.begin(); iter != m_usedCmdBuffers.end();)
    {
        VulkanCommandBuffer* cmdBuffer = (*iter);
        cmdBuffer->refreshFenceStatus();
        if (cmdBuffer->m_status == VulkanCommandBuffer::CommandBufferStatus::Finished)
        {
            iter = m_usedCmdBuffers.erase(iter);

            cmdBuffer->resetStatus();
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
    VulkanCommandBufferManager::updateStatus();
}

void VulkanCommandBufferManager::waitQueueCompletion(VkQueue queue)
{
    VkResult result = VulkanWrapper::QueueWaitIdle(queue);
    if (result != VK_SUCCESS)
    {
        LOG_ERROR("VulkanCommandBufferManager::waitCompete QueueWaitIdle. Error %s", ErrorString(result).c_str());
    }
    VulkanCommandBufferManager::updateStatus();
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
