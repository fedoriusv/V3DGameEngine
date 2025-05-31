#include "VulkanCommandBuffer.h"
#include "Utils/Logger.h"

#ifdef VULKAN_RENDER
#   include "VulkanResource.h"
#   include "VulkanDeviceCaps.h"
#   include "VulkanDebug.h"
#   include "VulkanDevice.h"
#   include "VulkanImage.h"
#   include "VulkanSemaphore.h"
#   include "VulkanSwapchain.h"

#   include "VulkanBuffer.h"
#   include "VulkanRenderpass.h"
#   include "VulkanFramebuffer.h"
#   include "VulkanCommandBufferManager.h"
#   include "VulkanGraphicPipeline.h"
#   include "VulkanComputePipeline.h"
#   include "VulkanFence.h"
//#include "VulkanQueryPool.h"

namespace v3d
{
namespace renderer
{
namespace vk
{

VulkanCommandBuffer::VulkanCommandBuffer(VulkanDevice* device, CommandBufferLevel level, VulkanCommandBuffer* primaryBuffer) noexcept
    : m_device(*device)

    , m_pool(VK_NULL_HANDLE)
    , m_commands(VK_NULL_HANDLE)

    , m_level(level)
    , m_status(CommandBufferStatus::Invalid)
    , m_queueIndex(~0U)

    , m_fence(V3D_NEW(VulkanFence, memory::MemoryLabel::MemoryRenderCore)(&m_device))

    , m_isInsideRenderPass(false)
{
#if VULKAN_DEBUG
    LOG_DEBUG("VulkanCommandBuffer constructor %llx", this);
#endif //VULKAN_DEBUG
    if (m_level == CommandBufferLevel::SecondaryBuffer)
    {
        ASSERT(primaryBuffer, "primaryBuffer is nullptr");
        m_primaryBuffer = primaryBuffer;
    }

    memset(&m_renderpassState, 0, sizeof(RenderPassState));

#if VULKAN_DEBUG_MARKERS
    m_debugName = "CommandBuffer";
    m_debugName.append(VulkanDebugUtils::k_addressPreffix);
    m_debugName.append(std::to_string(reinterpret_cast<const u64>(this)));
#endif //VULKAN_DEBUG_MARKERS
}

VulkanCommandBuffer::~VulkanCommandBuffer()
{
#if VULKAN_DEBUG
    LOG_DEBUG("~VulkanCommandBuffer destructor %llx", this);
#endif //VULKAN_DEBUG
    //released form pool manager
    m_commands = VK_NULL_HANDLE;
    V3D_DELETE(m_fence, memory::MemoryLabel::MemoryRenderCore);

    m_primaryBuffer = nullptr;
    m_secondaryBuffers.clear();
}

void VulkanCommandBuffer::addSemaphore(VkPipelineStageFlags mask, VulkanSemaphore* semaphore)
{
    if (m_level == CommandBufferLevel::SecondaryBuffer) [[unlikely]]
    {
        ASSERT(m_primaryBuffer, "m_primaryBuffer is nullptr");
        m_primaryBuffer->addSemaphore(mask, semaphore);
    }

    ASSERT(std::find(m_semaphores.cbegin(), m_semaphores.cend(), semaphore) == m_semaphores.cend(), "added multiply times");
    m_stageMasks.push_back(mask);
    m_semaphores.push_back(semaphore);
}

void VulkanCommandBuffer::addSemaphores(VkPipelineStageFlags mask, const std::vector<VulkanSemaphore*>& semaphores)
{
    for (VulkanSemaphore* semaphore : semaphores)
    {
        VulkanCommandBuffer::addSemaphore(mask, semaphore);
    }
}

bool VulkanCommandBuffer::waitCompletion(u64 timeout)
{
    if (m_status == CommandBufferStatus::Finished)
    {
        return true;
    }

    if (m_status == CommandBufferStatus::Submit)
    {
        timeout = (timeout == 0) ? u64(~0ULL) : timeout;
        VkFence vkFence = m_fence->getHandle();
        VkResult result = VulkanWrapper::WaitForFences(m_device.getDeviceInfo()._device, 1, &vkFence, VK_TRUE, timeout);
        if (result != VK_SUCCESS)
        {
            LOG_WARNING("VulkanCommandBuffer::waitComplete vkWaitForFences. Error %s", ErrorString(result).c_str());
            return false;
        }
        m_status = CommandBufferStatus::Finished;
        m_fence->incrementValue();

        result = VulkanWrapper::ResetFences(m_device.getDeviceInfo()._device, 1, &vkFence);
        ASSERT(result == VK_SUCCESS, "must be reseted");

        return true;
    }

    ASSERT(false, "not started");
    return false;
}

void VulkanCommandBuffer::refreshFenceStatus()
{
    if (m_status == CommandBufferStatus::Submit) [[likely]]
    {
        if (m_level == CommandBufferLevel::PrimaryBuffer) [[likely]]
        {
            VkFence vkFence = m_fence->getHandle();
            VkResult result = VulkanWrapper::GetFenceStatus(m_device.getDeviceInfo()._device, vkFence);
            if (result == VK_SUCCESS) [[likely]]
            {
                m_status = CommandBufferStatus::Finished;
                m_fence->incrementValue();

                result = VulkanWrapper::ResetFences(m_device.getDeviceInfo()._device, 1, &vkFence);
                ASSERT(result == VK_SUCCESS, "must be reseted");
            }
            else if (result == VK_NOT_READY && VulkanCommandBuffer::isSafeFrame(m_renderpassState._activeSwapchain ? m_renderpassState._activeSwapchain->getCurrentFrameIndex() : 0))
            {
                VkResult result = VulkanWrapper::WaitForFences(m_device.getDeviceInfo()._device, 1, &vkFence, VK_TRUE, u64(~0ULL));
                ASSERT(result == VK_SUCCESS, "must be finished");

                m_status = CommandBufferStatus::Finished;
                m_fence->incrementValue();

                result = VulkanWrapper::ResetFences(m_device.getDeviceInfo()._device, 1, &vkFence);
                ASSERT(result == VK_SUCCESS, "must be reseted");
            }
        }
        else
        {
            ASSERT(false, "not supported");
        }
    }
}

void VulkanCommandBuffer::init(Device::DeviceMask queueMask, VkCommandPool pool, VkCommandBuffer buffer)
{
    m_pool = pool;
    m_commands = buffer;
    m_status = VulkanCommandBuffer::CommandBufferStatus::Ready;
    m_queueIndex = queueMask >> 1;

    //Has crash inside renderdoc
#if VULKAN_DEBUG_MARKERS
    if (m_device.getVulkanDeviceCaps()._debugUtilsObjectNameEnabled)
    {
        VkDebugUtilsObjectNameInfoEXT debugUtilsObjectNameInfo = {};
        debugUtilsObjectNameInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
        debugUtilsObjectNameInfo.pNext = nullptr;
        debugUtilsObjectNameInfo.objectType = VK_OBJECT_TYPE_COMMAND_BUFFER;
        debugUtilsObjectNameInfo.objectHandle = reinterpret_cast<u64>(m_commands);
        debugUtilsObjectNameInfo.pObjectName = m_debugName.c_str();

        VulkanWrapper::SetDebugUtilsObjectName(m_device.getDeviceInfo()._device, &debugUtilsObjectNameInfo);
    }
#endif //VULKAN_DEBUG_MARKERS
}

void VulkanCommandBuffer::resetStatus()
{
    m_status = VulkanCommandBuffer::CommandBufferStatus::Ready;
    memset(&m_renderpassState, 0, sizeof(RenderPassState));

    m_stageMasks.clear();
    m_semaphores.clear();

    for (auto& secondaryBuffer : m_secondaryBuffers)
    {
        secondaryBuffer->resetStatus();
    }

#if VULKAN_DEBUG
    m_resources.clear();
#endif //VULKAN_DEBUG
}

bool VulkanCommandBuffer::isSafeFrame(u64 frame) const
{
    if (m_renderpassState._activeSwapchain)
    {
        u64 countSwapchains = m_renderpassState._activeSwapchain->getSwapchainImageCount();
        return (m_capturedFrameIndex + countSwapchains) < frame;
    }

    return false;
}

void VulkanCommandBuffer::beginCommandBuffer()
{
    ASSERT(m_status == CommandBufferStatus::Ready, "invalid state");

    VkCommandBufferBeginInfo commandBufferBeginInfo = {};
    commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    commandBufferBeginInfo.pNext = nullptr;

    if (m_level == CommandBufferLevel::PrimaryBuffer) [[likely]]
    {
        commandBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        commandBufferBeginInfo.pInheritanceInfo = nullptr;
    }
    else
    {
        ASSERT(false, "secondary not implemented");
        VkCommandBufferInheritanceInfo commandBufferInheritanceInfo = {};

        commandBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
        commandBufferBeginInfo.pInheritanceInfo = &commandBufferInheritanceInfo;
    }

    VkResult result = VulkanWrapper::BeginCommandBuffer(m_commands, &commandBufferBeginInfo);
    if (result != VK_SUCCESS)
    {
        LOG_ERROR("VulkanCommandBuffer::beginCommandBuffer vkBeginCommandBuffer. Error %s", ErrorString(result).c_str());
    }

#if VULKAN_DEBUG_MARKERS
    if (m_device.getVulkanDeviceCaps()._debugUtilsObjectNameEnabled)
    {
        VkDebugUtilsLabelEXT debugUtilsLabel = {};
        debugUtilsLabel.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT;
        debugUtilsLabel.pNext = nullptr;
        debugUtilsLabel.pLabelName = m_debugName.c_str();
        debugUtilsLabel.color[0] = 1.0f;
        debugUtilsLabel.color[1] = 1.0f;
        debugUtilsLabel.color[2] = 1.0f;
        debugUtilsLabel.color[3] = 1.0f;

        VulkanWrapper::CmdBeginDebugUtilsLabel(m_commands, &debugUtilsLabel);
    }
#endif //VULKAN_DEBUG_MARKERS

    m_status = CommandBufferStatus::Begin;
}

void VulkanCommandBuffer::endCommandBuffer()
{
#if VULKAN_DEBUG_MARKERS
    if (m_device.getVulkanDeviceCaps()._debugUtilsObjectNameEnabled)
    {
        VulkanWrapper::CmdEndDebugUtilsLabel(m_commands);
    }
#endif //VULKAN_DEBUG_MARKERS

#if TRACE_PROFILER_GPU_ENABLE
    ASSERT(m_tracyContext, "must be valid");
    TracyVkCollect(m_tracyContext, m_commands);
#endif

    ASSERT(m_status == CommandBufferStatus::Begin, "invalid state");
    VulkanWrapper::EndCommandBuffer(m_commands);

    m_status = CommandBufferStatus::End;
}

void VulkanCommandBuffer::cmdBeginRenderpass(VulkanRenderPass* pass, VulkanFramebuffer* framebuffer, const VkRect2D& area, const std::vector<VkClearValue>& clearValues)
{
    ASSERT(m_status == CommandBufferStatus::Begin, "not started");
    ASSERT(m_level == CommandBufferLevel::PrimaryBuffer, "primary only");
#if VULKAN_DEBUG
    LOG_DEBUG("VulkanCommandBuffer::cmdBeginRenderpass area [%d, %d, %u, %u], framebuffer area [width %u, height %u]", area.offset.x, area.offset.y, area.extent.width, area.extent.height, framebuffer->getArea().m_width, framebuffer->getArea().m_height);
#endif

    VulkanCommandBuffer::captureResource(pass);
    VulkanCommandBuffer::captureResource(framebuffer);

    m_renderpassState._renderpass = pass;
    m_renderpassState._framebuffer = framebuffer;

    u32 index = 0;
    for (u32 index = 0; index < pass->getCountAttachments(); ++index)
    {

        auto& image = framebuffer->getImages()[index];
        VulkanImage* vkImage = OBJECT_FROM_HANDLE(image, VulkanImage);
        VulkanCommandBuffer::captureResource(vkImage);

        VkImageLayout layout = pass->getAttachmentLayout<0>(index);
        const VulkanRenderPass::VulkanAttachmentDescription& attach = pass->getAttachmentDescription(index);
        m_resourceStates.setLayout(vkImage, layout, VulkanImage::makeVulkanImageSubresource(vkImage, attach._layer, attach._mip));

        ++index;

        if (vkImage->hasUsageFlag(TextureUsage_Backbuffer))
        {
            m_renderpassState._activeSwapchain = VulkanImage::getSwapchainFromImage(vkImage);
        }

#if VULKAN_DEBUG
        LOG_DEBUG("VulkanCommandBuffer::cmdBeginRenderpass framebuffer image (width %u, height %u)", vkImage->getSize().width, vkImage->getSize().height);
#endif
    }

    VkRenderPassBeginInfo renderPassBeginInfo = {};
    renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassBeginInfo.pNext = nullptr;
    renderPassBeginInfo.renderPass = pass->getHandle();
    renderPassBeginInfo.framebuffer = framebuffer->getHandle();
    renderPassBeginInfo.renderArea = area;
    renderPassBeginInfo.clearValueCount = static_cast<u32>(framebuffer->getImages().size());
    renderPassBeginInfo.pClearValues = clearValues.data();

#ifdef PLATFORM_ANDROID
#   ifdef VK_QCOM_render_pass_transform
    VkRenderPassTransformBeginInfoQCOM renderPassTransformBeginInfoQCOM = {};
    if (m_device->m_deviceCaps._renderpassTransformQCOM && pass->isDrawingToSwapchain())
    {
        VkSurfaceTransformFlagBitsKHR preTransform = static_cast<VulkanContext*>(m_context)->getSwapchain()->getTransformFlag();
        if (preTransform != VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR)
        {
            renderPassTransformBeginInfoQCOM.sType = m_device->m_deviceCaps._fixRenderPassTransformQCOMDriverIssue ? (VkStructureType)1000282000 : (VkStructureType)VK_STRUCTURE_TYPE_RENDER_PASS_TRANSFORM_BEGIN_INFO_QCOM;
            renderPassTransformBeginInfoQCOM.pNext = nullptr;
            renderPassTransformBeginInfoQCOM.transform = preTransform;

            renderPassBeginInfo.pNext = &renderPassTransformBeginInfoQCOM;
            LOG_DEBUG("VulkanCommandBuffer::cmdBeginRenderpass VkRenderPassTransformBeginInfoQCOM, transform %d", preTransform);
        }
    }
#   endif //VK_QCOM_render_pass_transform
#endif //PLATFORM_ANDROID

    if (m_device.getVulkanDeviceCaps()._supportRenderpass2)
    {
        VkSubpassBeginInfoKHR subpassBeginInfo = {};
        subpassBeginInfo.sType = VK_STRUCTURE_TYPE_SUBPASS_BEGIN_INFO_KHR;
        subpassBeginInfo.pNext = nullptr;
        subpassBeginInfo.contents = VK_SUBPASS_CONTENTS_INLINE;

#if VULKAN_DEBUG
        LOG_DEBUG("VulkanCommandBuffer::CmdBeginRenderPass2 area (width %u, height %u)", renderPassBeginInfo.renderArea.extent.width, renderPassBeginInfo.renderArea.extent.height);
#endif
        VulkanWrapper::CmdBeginRenderPass2(m_commands, &renderPassBeginInfo, &subpassBeginInfo);
    }
    else
    {
#if VULKAN_DEBUG
        LOG_DEBUG("VulkanCommandBuffer::CmdBeginRenderPass area (width %u, height %u)", renderPassBeginInfo.renderArea.extent.width, renderPassBeginInfo.renderArea.extent.height);
#endif
        VulkanWrapper::CmdBeginRenderPass(m_commands, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
    }

    m_isInsideRenderPass = true;
}

void VulkanCommandBuffer::cmdEndRenderPass()
{
    ASSERT(m_status == CommandBufferStatus::Begin && m_isInsideRenderPass, "invalid state");

    if (m_device.getVulkanDeviceCaps()._supportRenderpass2)
    {
        VkSubpassEndInfoKHR subpassEndInfo = {};
        subpassEndInfo.sType = VK_STRUCTURE_TYPE_SUBPASS_END_INFO_KHR;
        subpassEndInfo.pNext = nullptr;

        VulkanWrapper::CmdEndRenderPass2(m_commands, &subpassEndInfo);
    }
    else
    {
        VulkanWrapper::CmdEndRenderPass(m_commands);
    }

    const VulkanRenderPass* pass = m_renderpassState._renderpass;
    const VulkanFramebuffer* framebuffer = m_renderpassState._framebuffer;

    u32 index = 0;
    ASSERT(pass && framebuffer, "nullptr");
    for (u32 index = 0; index < pass->getCountAttachments(); ++index)
    {
        VulkanImage* image = OBJECT_FROM_HANDLE(framebuffer->getImages()[index], VulkanImage);
        ASSERT(image, "nullptr");
        VkImageLayout layout = m_renderpassState._renderpass->getAttachmentLayout<1>(index);
        const VulkanRenderPass::VulkanAttachmentDescription& attach = pass->getAttachmentDescription(index);
        m_resourceStates.setLayout(image, layout, VulkanImage::makeVulkanImageSubresource(image, attach._layer, attach._mip));
    }

    m_isInsideRenderPass = false;
}

} //namespace vk
} //namespace renderer
} //namespace v3d
#endif //VULKAN_RENDER
