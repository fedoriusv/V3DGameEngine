#include "VulkanRenderpass.h"

#include "Utils/Logger.h"

#ifdef VULKAN_RENDER
#   include "VulkanDebug.h"
#   include "VulkanDeviceCaps.h"
#   include "VulkanDevice.h"
#   include "VulkanImage.h"
#   include "VulkanCommandBufferManager.h"

namespace v3d
{
namespace renderer
{
namespace vk
{

VkAttachmentLoadOp VulkanRenderPass::convertAttachLoadOpToVkAttachmentLoadOp(RenderTargetLoadOp loadOp)
{
    switch (loadOp)
    {
    case RenderTargetLoadOp::LoadOp_Load:
        return VK_ATTACHMENT_LOAD_OP_LOAD;

    case RenderTargetLoadOp::LoadOp_Clear:
        return VK_ATTACHMENT_LOAD_OP_CLEAR;

    case RenderTargetLoadOp::LoadOp_DontCare:
        return VK_ATTACHMENT_LOAD_OP_DONT_CARE;

    default:
        ASSERT(false, "unknown");
    }

    return VK_ATTACHMENT_LOAD_OP_CLEAR;
}
VkAttachmentStoreOp VulkanRenderPass::convertAttachStoreOpToVkAttachmentStoreOp(RenderTargetStoreOp storeOp)
{
    switch (storeOp)
    {
    case RenderTargetStoreOp::StoreOp_Store:
        return VK_ATTACHMENT_STORE_OP_STORE;

    case RenderTargetStoreOp::StoreOp_DontCare:
        return VK_ATTACHMENT_STORE_OP_DONT_CARE;

    default:
        ASSERT(false, "unknown");
    }

    return VK_ATTACHMENT_STORE_OP_STORE;
}

VulkanRenderPass::VulkanRenderPass(VulkanDevice* device, const RenderPassDesc& description, const std::string& name) noexcept
    : m_device(*device)
    , m_renderpass(VK_NULL_HANDLE)
    , m_viewsMask(0x0)
    , m_drawingToSwapchain(false)
{
    LOG_DEBUG("VulkanRenderpass::VulkanRenderpass constructor %llx", this);

    u32 countAttachments = (description._hasDepthStencilAttahment) ? description._countColorAttachments + 1 : description._countColorAttachments;
    m_descriptions.resize(countAttachments);
    for (u32 index = 0; index < description._countColorAttachments; ++index)
    {
        VulkanRenderPass::VulkanAttachmentDescription& desc = m_descriptions[index];
        desc._format = VulkanImage::convertImageFormatToVkFormat(description._attachmentsDesc[index]._format);
        desc._samples = VulkanImage::convertRenderTargetSamplesToVkSampleCount(description._attachmentsDesc[index]._samples);
        desc._loadOp = VulkanRenderPass::convertAttachLoadOpToVkAttachmentLoadOp(description._attachmentsDesc[index]._loadOp);
        desc._storeOp = VulkanRenderPass::convertAttachStoreOpToVkAttachmentStoreOp(description._attachmentsDesc[index]._storeOp);
        desc._initialLayout = VulkanTransitionState::convertTransitionStateToImageLayout(description._attachmentsDesc[index]._initTransition);
        desc._finalLayout = VulkanTransitionState::convertTransitionStateToImageLayout(description._attachmentsDesc[index]._finalTransition);
        desc._layer = AttachmentDesc::uncompressLayer(description._attachmentsDesc[index]._layer);
        desc._mip = 0;

        desc._swapchainImage = (description._attachmentsDesc[index]._backbuffer) ? true : false;
        desc._autoResolve = (description._attachmentsDesc[index]._autoResolve) ? true : false;
    }

    if (description._hasDepthStencilAttahment)
    {
        VulkanRenderPass::VulkanAttachmentDescription& desc = m_descriptions.back();
        desc._format = VulkanImage::convertImageFormatToVkFormat(description._attachmentsDesc.back()._format);
        desc._samples = VulkanImage::convertRenderTargetSamplesToVkSampleCount(description._attachmentsDesc.back()._samples);
        desc._loadOp = VulkanRenderPass::convertAttachLoadOpToVkAttachmentLoadOp(description._attachmentsDesc.back()._loadOp);
        desc._storeOp = VulkanRenderPass::convertAttachStoreOpToVkAttachmentStoreOp(description._attachmentsDesc.back()._storeOp);
        desc._stencilLoadOp = VulkanRenderPass::convertAttachLoadOpToVkAttachmentLoadOp(description._attachmentsDesc.back()._stencilLoadOp);
        desc._stensilStoreOp = VulkanRenderPass::convertAttachStoreOpToVkAttachmentStoreOp(description._attachmentsDesc.back()._stencilStoreOp);
        desc._initialLayout = VulkanTransitionState::convertTransitionStateToImageLayout(description._attachmentsDesc.back()._initTransition);
        desc._finalLayout = VulkanTransitionState::convertTransitionStateToImageLayout(description._attachmentsDesc.back()._finalTransition);
        desc._layer = AttachmentDesc::uncompressLayer(description._attachmentsDesc.back()._layer);
        desc._mip = 0;

        desc._swapchainImage = false;
        desc._autoResolve = (description._attachmentsDesc.back()._autoResolve) ? true : false;
    }

    m_viewsMask = description._viewsMask;
    m_drawingToSwapchain = std::any_of(m_descriptions.cbegin(), m_descriptions.cend(), [](auto& desc)
        {
            return desc._swapchainImage;
        });

#if VULKAN_DEBUG_MARKERS
    m_debugName = name.empty() ? "Renderpass" : name;
    m_debugName.append(VulkanDebugUtils::k_addressPreffix);
    m_debugName.append(std::to_string(reinterpret_cast<const u64>(this)));
#endif //VULKAN_DEBUG_MARKERS
}

VulkanRenderPass::~VulkanRenderPass()
{
    LOG_DEBUG("VulkanRenderpass::VulkanRenderpass destructor %llx", this);
    ASSERT(!m_renderpass, "not nullptr");
}

bool VulkanRenderPass::create()
{
    ASSERT(!m_renderpass, "not empty");
    u32 countLayouts = 0;
    for (auto& desc : m_descriptions)
    {
        ++countLayouts;
        if (desc._autoResolve && desc._samples > VK_SAMPLE_COUNT_1_BIT)
        {
            ++countLayouts;
        }
    }
    m_layout.resize(countLayouts, std::make_tuple(VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_UNDEFINED));

    if (m_device.getVulkanDeviceCaps()._supportRenderpass2)
    {
        if (!VulkanRenderPass::createRenderpass2())
        {
            return false;
        }
    }
    else //renderpass
    {
        if (!VulkanRenderPass::createRenderpass())
        {
            return false;
        }
    }

#if VULKAN_DEBUG_MARKERS
    if (m_device.getVulkanDeviceCaps()._debugUtilsObjectNameEnabled)
    {
        VkDebugUtilsObjectNameInfoEXT debugUtilsObjectNameInfo = {};
        debugUtilsObjectNameInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
        debugUtilsObjectNameInfo.pNext = nullptr;
        debugUtilsObjectNameInfo.objectType = VK_OBJECT_TYPE_RENDER_PASS;
        debugUtilsObjectNameInfo.objectHandle = reinterpret_cast<u64>(m_renderpass);
        debugUtilsObjectNameInfo.pObjectName = m_debugName.c_str();

        VulkanWrapper::SetDebugUtilsObjectName(m_device.getDeviceInfo()._device, &debugUtilsObjectNameInfo);
    }
#endif //VULKAN_DEBUG_MARKERS

    return true;
}

bool VulkanRenderPass::createRenderpass()
{
    std::vector<VkAttachmentDescription> attachmentDescriptions;
    attachmentDescriptions.reserve(m_descriptions.size());

    std::vector<VkAttachmentReference> colorAttachmentReferences;
    std::vector<VkAttachmentReference> resolveAttachmentReferences;

    VkAttachmentReference depthStencilAttachmentReferences = {};

    VkAccessFlags dependencyAccessFlags = 0;
    VkPipelineStageFlags dependencyStageFlags = 0;

    bool depthStencil = false;
    u32 index = 0;
    for (u32 descIndex = 0; descIndex < m_descriptions.size(); ++descIndex)
    {
        VulkanAttachmentDescription& attach = m_descriptions[descIndex];
        ASSERT(attach._format != VK_FORMAT_UNDEFINED, "undefined format");

        bool disableAutoresolve = !attach._autoResolve;
        if (VulkanImage::isDepthStencilFormat(attach._format))
        {
            disableAutoresolve = m_device.getVulkanDeviceCaps()._supportDepthAutoResolve ? disableAutoresolve : true;
        }

        if (attach._samples == VK_SAMPLE_COUNT_1_BIT || disableAutoresolve)
        {
            VkAttachmentDescription attachmentDescription = {};
            attachmentDescription.format = attach._format;
            attachmentDescription.samples = attach._samples;
            attachmentDescription.flags = 0; // VK_ATTACHMENT_DESCRIPTION_MAY_ALIAS_BIT; //need check
            attachmentDescription.loadOp = attach._loadOp;
            attachmentDescription.storeOp = attach._storeOp;
            attachmentDescription.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            attachmentDescription.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            attachmentDescription.initialLayout = attach._initialLayout;
            attachmentDescription.finalLayout = attach._finalLayout;

            if (VulkanImage::isColorFormat(attach._format))
            {
                VkAttachmentReference attachmentReference = {};
                attachmentReference.attachment = index;
                attachmentReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

                dependencyAccessFlags |= VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
                dependencyStageFlags |= VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

                m_layout[index] = { attachmentReference.layout, attach._finalLayout };
                index++;

                colorAttachmentReferences.push_back(attachmentReference);
            }
            else
            {
                depthStencil = true;

                attachmentDescription.stencilLoadOp = attach._stencilLoadOp;
                attachmentDescription.stencilStoreOp = attach._stensilStoreOp;

                depthStencilAttachmentReferences.attachment = index;
                depthStencilAttachmentReferences.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

                dependencyAccessFlags |= VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
                dependencyStageFlags |= VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;

                m_layout[index] = { depthStencilAttachmentReferences.layout, attach._finalLayout };
                index++;
            }

            attachmentDescriptions.push_back(attachmentDescription);
        }
        else
        {
            VkAttachmentDescription msaaAttachmentDescription = {};
            msaaAttachmentDescription.flags = 0;
            msaaAttachmentDescription.format = attach._format;
            msaaAttachmentDescription.samples = attach._samples;
            msaaAttachmentDescription.loadOp = attach._loadOp;
            msaaAttachmentDescription.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            msaaAttachmentDescription.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            msaaAttachmentDescription.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            msaaAttachmentDescription.initialLayout = attach._initialLayout;
            msaaAttachmentDescription.finalLayout = attach._finalLayout;

            VkAttachmentDescription resolveAttachmentDescription = {};
            resolveAttachmentDescription.flags = 0;
            resolveAttachmentDescription.format = attach._format;
            resolveAttachmentDescription.samples = VK_SAMPLE_COUNT_1_BIT;
            resolveAttachmentDescription.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            resolveAttachmentDescription.storeOp = attach._storeOp;
            resolveAttachmentDescription.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            resolveAttachmentDescription.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            resolveAttachmentDescription.initialLayout = attach._initialLayout;
            resolveAttachmentDescription.finalLayout = attach._finalLayout;

            if (VulkanImage::isColorFormat(attach._format))
            {
                VkAttachmentReference msaaAttachmentReference = {};
                msaaAttachmentReference.attachment = index;
                msaaAttachmentReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

                dependencyAccessFlags |= VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
                dependencyStageFlags |= VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

                m_layout[index] = { msaaAttachmentReference.layout, attach._finalLayout };
                index++;

                colorAttachmentReferences.push_back(msaaAttachmentReference);

                VkAttachmentReference resolveAttachmentReference = {};
                resolveAttachmentReference.attachment = index;
                resolveAttachmentReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

                m_layout[index] = { resolveAttachmentReference.layout, attach._finalLayout };
                index++;

                resolveAttachmentReferences.push_back(resolveAttachmentReference);
            }
            else
            {
                depthStencil = true;

                msaaAttachmentDescription.stencilLoadOp = attach._stencilLoadOp;
                msaaAttachmentDescription.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

                resolveAttachmentDescription.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
                resolveAttachmentDescription.stencilStoreOp = attach._stensilStoreOp;

                depthStencilAttachmentReferences.attachment = index;
                depthStencilAttachmentReferences.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

                dependencyAccessFlags |= VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
                dependencyStageFlags |= VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;

                m_layout[index] = { depthStencilAttachmentReferences.layout, attach._finalLayout };
                index++;
            }

            attachmentDescriptions.push_back(msaaAttachmentDescription);
            attachmentDescriptions.push_back(resolveAttachmentDescription);
        }
    }

    u32 countSubpasses = 1;
    std::vector<VkSubpassDescription> subpassDescriptions;
    subpassDescriptions.reserve(countSubpasses);

    for (u32 subpassIndex = 0; subpassIndex < countSubpasses; ++subpassIndex)
    {
        VkSubpassDescription subpassDescription = {};
        subpassDescription.flags = 0;
        subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpassDescription.inputAttachmentCount = 0;
        subpassDescription.pInputAttachments = nullptr;
        subpassDescription.colorAttachmentCount = static_cast<u32>(colorAttachmentReferences.size());
        subpassDescription.pColorAttachments = colorAttachmentReferences.data();
        subpassDescription.pResolveAttachments = !resolveAttachmentReferences.empty() ? resolveAttachmentReferences.data() : nullptr;
        subpassDescription.pDepthStencilAttachment = depthStencil ? &depthStencilAttachmentReferences : nullptr;
        subpassDescription.preserveAttachmentCount = 0;
        subpassDescription.pPreserveAttachments = nullptr;

        subpassDescriptions.push_back(subpassDescription);
    }


    void* vkExtensions = nullptr;

    VkRenderPassMultiviewCreateInfo renderPassMultiviewCreateInfo = {};
    std::vector<u32> viewMasksBySubpass;
    if (m_device.getVulkanDeviceCaps()._supportMultiview && m_viewsMask != 0)
    {
        viewMasksBySubpass.push_back(m_viewsMask);
        ASSERT(viewMasksBySubpass.size() == countSubpasses && viewMasksBySubpass.size() == 1, "mulitview support only one subpass");

        renderPassMultiviewCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_MULTIVIEW_CREATE_INFO;
        renderPassMultiviewCreateInfo.pNext = nullptr;
        renderPassMultiviewCreateInfo.subpassCount = static_cast<u32>(viewMasksBySubpass.size());
        renderPassMultiviewCreateInfo.pViewMasks = viewMasksBySubpass.data();
        renderPassMultiviewCreateInfo.dependencyCount = 0;
        renderPassMultiviewCreateInfo.pViewOffsets = nullptr;
        renderPassMultiviewCreateInfo.correlationMaskCount = 0;
        renderPassMultiviewCreateInfo.pCorrelationMasks = nullptr;

        vkExtensions = &renderPassMultiviewCreateInfo;
    }

    std::array<VkSubpassDependency, 1> subpassDependency;
    {
        VkSubpassDependency& subpassDependencyBottomExernal = subpassDependency[0];
        subpassDependencyBottomExernal.srcSubpass = 0;
        subpassDependencyBottomExernal.dstSubpass = VK_SUBPASS_EXTERNAL;
        subpassDependencyBottomExernal.srcStageMask = dependencyStageFlags;
        subpassDependencyBottomExernal.srcAccessMask = dependencyAccessFlags;
        subpassDependencyBottomExernal.dstStageMask = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        subpassDependencyBottomExernal.dstAccessMask = 0;
        subpassDependencyBottomExernal.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
    }

    VkRenderPassCreateInfo renderPassCreateInfo = {};
    renderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassCreateInfo.pNext = vkExtensions;
    renderPassCreateInfo.flags = 0;
    renderPassCreateInfo.attachmentCount = static_cast<u32>(attachmentDescriptions.size());
    renderPassCreateInfo.pAttachments = attachmentDescriptions.data();
    renderPassCreateInfo.subpassCount = static_cast<u32>(subpassDescriptions.size());
    renderPassCreateInfo.pSubpasses = subpassDescriptions.data();
    renderPassCreateInfo.dependencyCount = static_cast<u32>(subpassDependency.size());
    renderPassCreateInfo.pDependencies = subpassDependency.data();

#ifdef VK_QCOM_render_pass_transform
    if (m_device.getVulkanDeviceCaps()._renderpassTransformQCOM)
    {
        renderPassCreateInfo.flags |= VK_RENDER_PASS_CREATE_TRANSFORM_BIT_QCOM;
    }
#endif

    VkResult result = VulkanWrapper::CreateRenderPass(m_device.getDeviceInfo()._device, &renderPassCreateInfo, VULKAN_ALLOCATOR, &m_renderpass);
    if (result != VK_SUCCESS)
    {
        LOG_ERROR("VulkanRenderPass::create vkCreateRenderPass is failed. Error: %s", ErrorString(result).c_str());
        return false;
    }
#if VULKAN_DEBUG
    LOG_DEBUG("VulkanCommandBuffer::create RenderPass has been created %llx. Flags %d", m_renderpass, renderPassCreateInfo.flags);
#endif
    return true;
}

bool VulkanRenderPass::createRenderpass2()
{
    std::vector<VkAttachmentDescription2KHR> attachmentDescriptions;
    attachmentDescriptions.reserve(m_descriptions.size());

    std::vector<VkAttachmentReference2KHR> colorAttachmentReferences;
    std::vector<VkAttachmentReference2KHR> resolveAttachmentReferences;

    VkAttachmentReference2KHR depthStencilAttachmentReferences = {};
    depthStencilAttachmentReferences.sType = VK_STRUCTURE_TYPE_ATTACHMENT_REFERENCE_2_KHR;
    depthStencilAttachmentReferences.pNext = nullptr;

    VkAttachmentReference2KHR resolveDepthStencilAttachmentReferences = {};
    resolveDepthStencilAttachmentReferences.sType = VK_STRUCTURE_TYPE_ATTACHMENT_REFERENCE_2_KHR;
    resolveDepthStencilAttachmentReferences.pNext = nullptr;

    VkAccessFlags dependencyAccessFlags = 0;
    VkPipelineStageFlags dependencyStageFlags = 0;

    bool depthStencil = false;
    bool depthStencilAutoresolve = false;
    u32 index = 0;
    for (u32 descIndex = 0; descIndex < m_descriptions.size(); ++descIndex)
    {
        VulkanAttachmentDescription& attach = m_descriptions[descIndex];
        ASSERT(attach._format != VK_FORMAT_UNDEFINED, "undefined format");

        bool disableAutoresolve = !attach._autoResolve;
        if (VulkanImage::isDepthStencilFormat(attach._format))
        {
            disableAutoresolve = m_device.getVulkanDeviceCaps()._supportDepthAutoResolve ? disableAutoresolve : true;
        }

        if (attach._samples == VK_SAMPLE_COUNT_1_BIT || disableAutoresolve)
        {
            VkAttachmentDescription2KHR attachmentDescription = {};
            attachmentDescription.sType = VK_STRUCTURE_TYPE_ATTACHMENT_DESCRIPTION_2_KHR;
            attachmentDescription.pNext = nullptr;
            attachmentDescription.flags = 0;
            attachmentDescription.format = attach._format;
            attachmentDescription.samples = attach._samples;
            attachmentDescription.flags = 0; // VK_ATTACHMENT_DESCRIPTION_MAY_ALIAS_BIT; //need check
            attachmentDescription.loadOp = attach._loadOp;
            attachmentDescription.storeOp = attach._storeOp;
            attachmentDescription.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            attachmentDescription.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            attachmentDescription.initialLayout = attach._initialLayout;
            attachmentDescription.finalLayout = attach._finalLayout;

            if (VulkanImage::isColorFormat(attach._format))
            {
                VkAttachmentReference2KHR attachmentReference = {};
                attachmentReference.sType = VK_STRUCTURE_TYPE_ATTACHMENT_REFERENCE_2_KHR;
                attachmentReference.pNext = nullptr;
                attachmentReference.aspectMask = VulkanImage::getImageAspectFlags(attach._format);
                attachmentReference.attachment = index;
                attachmentReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

                dependencyAccessFlags |= VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
                dependencyStageFlags |= VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

                m_layout[index] = { attachmentReference.layout, attach._finalLayout };
                ++index;

                colorAttachmentReferences.push_back(attachmentReference);
            }
            else
            {
                depthStencil = true;

                attachmentDescription.stencilLoadOp = attach._stencilLoadOp;
                attachmentDescription.stencilStoreOp = attach._stensilStoreOp;

                depthStencilAttachmentReferences.attachment = index;
                depthStencilAttachmentReferences.aspectMask = VulkanImage::getImageAspectFlags(attach._format);
                depthStencilAttachmentReferences.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

                dependencyAccessFlags |= VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
                dependencyStageFlags |= VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;

                m_layout[index] = { depthStencilAttachmentReferences.layout, attach._finalLayout };
                ++index;
            }

            attachmentDescriptions.push_back(attachmentDescription);
        }
        else
        {
            VkAttachmentDescription2KHR msaaAttachmentDescription = {};
            msaaAttachmentDescription.sType = VK_STRUCTURE_TYPE_ATTACHMENT_DESCRIPTION_2_KHR;
            msaaAttachmentDescription.pNext = nullptr;
            msaaAttachmentDescription.flags = 0;
            msaaAttachmentDescription.format = attach._format;
            msaaAttachmentDescription.samples = attach._samples;
            msaaAttachmentDescription.flags = 0;
            msaaAttachmentDescription.loadOp = attach._loadOp;
            msaaAttachmentDescription.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            msaaAttachmentDescription.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            msaaAttachmentDescription.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            msaaAttachmentDescription.initialLayout = attach._initialLayout;
            msaaAttachmentDescription.finalLayout = attach._finalLayout;

            VkAttachmentDescription2KHR resolveAttachmentDescription = {};
            resolveAttachmentDescription.sType = VK_STRUCTURE_TYPE_ATTACHMENT_DESCRIPTION_2_KHR;
            resolveAttachmentDescription.pNext = nullptr;
            resolveAttachmentDescription.flags = 0;
            resolveAttachmentDescription.format = attach._format;
            resolveAttachmentDescription.samples = VK_SAMPLE_COUNT_1_BIT;
            resolveAttachmentDescription.flags = 0;
            resolveAttachmentDescription.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            resolveAttachmentDescription.storeOp = attach._storeOp;
            resolveAttachmentDescription.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            resolveAttachmentDescription.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            resolveAttachmentDescription.initialLayout = attach._initialLayout;
            resolveAttachmentDescription.finalLayout = attach._finalLayout;

            if (VulkanImage::isColorFormat(attach._format))
            {
                VkAttachmentReference2KHR msaaAttachmentReference = {};
                msaaAttachmentReference.sType = VK_STRUCTURE_TYPE_ATTACHMENT_REFERENCE_2_KHR;
                msaaAttachmentReference.pNext = nullptr;
                msaaAttachmentReference.aspectMask = VulkanImage::getImageAspectFlags(attach._format);
                msaaAttachmentReference.attachment = index;
                msaaAttachmentReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

                dependencyAccessFlags |= VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
                dependencyStageFlags |= VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

                m_layout[index] = { msaaAttachmentReference.layout, attach._finalLayout };
                ++index;

                colorAttachmentReferences.push_back(msaaAttachmentReference);

                VkAttachmentReference2KHR resolveAttachmentReference = {};
                resolveAttachmentReference.sType = VK_STRUCTURE_TYPE_ATTACHMENT_REFERENCE_2_KHR;
                resolveAttachmentReference.pNext = nullptr;
                resolveAttachmentReference.aspectMask = VulkanImage::getImageAspectFlags(attach._format);
                resolveAttachmentReference.attachment = index;
                resolveAttachmentReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

                m_layout[index] = { resolveAttachmentReference.layout, attach._finalLayout };
                ++index;

                resolveAttachmentReferences.push_back(resolveAttachmentReference);
            }
            else
            {
                depthStencil = true;

                msaaAttachmentDescription.stencilLoadOp = attach._stencilLoadOp;
                msaaAttachmentDescription.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

                resolveAttachmentDescription.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
                resolveAttachmentDescription.stencilStoreOp = attach._stensilStoreOp;

                depthStencilAttachmentReferences.attachment = index;
                depthStencilAttachmentReferences.aspectMask = VulkanImage::getImageAspectFlags(attach._format);
                depthStencilAttachmentReferences.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

                dependencyAccessFlags |= VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
                dependencyStageFlags |= VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;

                m_layout[index] = { depthStencilAttachmentReferences.layout, attach._finalLayout };
                ++index;

                if (m_device.getVulkanDeviceCaps()._supportDepthAutoResolve)
                {
                    depthStencilAutoresolve = true;

                    resolveDepthStencilAttachmentReferences.attachment = index;
                    resolveDepthStencilAttachmentReferences.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
                    resolveDepthStencilAttachmentReferences.aspectMask = VulkanImage::getImageAspectFlags(attach._format);

                    m_layout[index] = { resolveDepthStencilAttachmentReferences.layout, attach._finalLayout };
                    ++index;
                }
            }

            attachmentDescriptions.push_back(msaaAttachmentDescription);
            attachmentDescriptions.push_back(resolveAttachmentDescription);
        }
    }

    std::array<VkSubpassDescription2KHR, 1> subpassDescriptions;
    VkSubpassDescription2KHR& subpassDescription = subpassDescriptions[0];
    subpassDescription.sType = VK_STRUCTURE_TYPE_SUBPASS_DESCRIPTION_2_KHR;

    VkSubpassDescriptionDepthStencilResolveKHR subpassDescriptionDepthStencilResolve = {};
    subpassDescriptionDepthStencilResolve.sType = VK_STRUCTURE_TYPE_SUBPASS_DESCRIPTION_DEPTH_STENCIL_RESOLVE_KHR;
    subpassDescriptionDepthStencilResolve.pNext = nullptr;
    if (depthStencilAutoresolve)
    {
        subpassDescriptionDepthStencilResolve.pDepthStencilResolveAttachment = &resolveDepthStencilAttachmentReferences;
        subpassDescriptionDepthStencilResolve.depthResolveMode = VK_RESOLVE_MODE_SAMPLE_ZERO_BIT_KHR;
        subpassDescriptionDepthStencilResolve.stencilResolveMode = VK_RESOLVE_MODE_NONE_KHR;

        subpassDescription.pNext = &subpassDescriptionDepthStencilResolve;
    }
    else
    {
        subpassDescription.pNext = nullptr;
    }

    subpassDescription.flags = 0;
    subpassDescription.viewMask = 0;
    subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpassDescription.inputAttachmentCount = 0;
    subpassDescription.pInputAttachments = nullptr;
    subpassDescription.colorAttachmentCount = static_cast<u32>(colorAttachmentReferences.size());
    subpassDescription.pColorAttachments = colorAttachmentReferences.data();
    subpassDescription.pResolveAttachments = !resolveAttachmentReferences.empty() ? resolveAttachmentReferences.data() : nullptr;
    subpassDescription.pDepthStencilAttachment = depthStencil ? &depthStencilAttachmentReferences : nullptr;
    subpassDescription.preserveAttachmentCount = 0;
    subpassDescription.pPreserveAttachments = nullptr;
    if (m_device.getVulkanDeviceCaps()._supportMultiview && m_viewsMask != 0)
    {
        subpassDescription.viewMask = m_viewsMask;
    }
    else
    {
        subpassDescription.viewMask = 0;
    }

    std::array<VkSubpassDependency2KHR, 1> subpassDependency;
    {
        VkSubpassDependency2KHR& subpassDependencyBottomExernal = subpassDependency[0];
        subpassDependencyBottomExernal.sType = VK_STRUCTURE_TYPE_SUBPASS_DEPENDENCY_2;
        subpassDependencyBottomExernal.pNext = nullptr;
        subpassDependencyBottomExernal.srcSubpass = 0;
        subpassDependencyBottomExernal.dstSubpass = VK_SUBPASS_EXTERNAL;
        subpassDependencyBottomExernal.srcStageMask = dependencyStageFlags;
        subpassDependencyBottomExernal.srcAccessMask = dependencyAccessFlags;
        subpassDependencyBottomExernal.dstStageMask = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        subpassDependencyBottomExernal.dstAccessMask = 0;
        subpassDependencyBottomExernal.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
        subpassDependencyBottomExernal.viewOffset = 0;
    }

    VkRenderPassCreateInfo2KHR renderPassCreateInfo = {};
    renderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO_2_KHR;
    renderPassCreateInfo.pNext = nullptr;
    renderPassCreateInfo.flags = 0;
    renderPassCreateInfo.attachmentCount = static_cast<u32>(attachmentDescriptions.size());
    renderPassCreateInfo.pAttachments = attachmentDescriptions.data();
    renderPassCreateInfo.subpassCount = static_cast<u32>(subpassDescriptions.size());
    renderPassCreateInfo.pSubpasses = subpassDescriptions.data();
    renderPassCreateInfo.dependencyCount = static_cast<u32>(subpassDependency.size());
    renderPassCreateInfo.pDependencies = subpassDependency.data();
    renderPassCreateInfo.correlatedViewMaskCount = 0;
    renderPassCreateInfo.pCorrelatedViewMasks = nullptr;

#ifdef VK_QCOM_render_pass_transform
    if (m_device.getVulkanDeviceCaps()._renderpassTransformQCOM)
    {
        renderPassCreateInfo.flags |= VK_RENDER_PASS_CREATE_TRANSFORM_BIT_QCOM;
    }
#endif

    VkResult result = VulkanWrapper::CreateRenderPass2(m_device.getDeviceInfo()._device, &renderPassCreateInfo, VULKAN_ALLOCATOR, &m_renderpass);
    if (result != VK_SUCCESS)
    {
        LOG_ERROR("VulkanRenderPass::create vkCreateRenderPass2 is failed. Error: %s", ErrorString(result).c_str());
        return false;
    }
#if VULKAN_DEBUG
    LOG_DEBUG("VulkanCommandBuffer::create RenderPass2 has been created %llx. Flags %d", m_renderpass, renderPassCreateInfo.flags);
#endif
    return true;
}

void VulkanRenderPass::destroy()
{
    if (m_renderpass)
    {
        VulkanWrapper::DestroyRenderPass(m_device.getDeviceInfo()._device, m_renderpass, VULKAN_ALLOCATOR);
        m_renderpass = VK_NULL_HANDLE;
    }

    m_layout.clear();
}


VulkanRenderpassManager::VulkanRenderpassManager(VulkanDevice* device) noexcept
    : m_device(*device)
{
}

VulkanRenderpassManager::~VulkanRenderpassManager()
{
    VulkanRenderpassManager::clear();
}

VulkanRenderPass* VulkanRenderpassManager::acquireRenderpass(const RenderPassDesc& description, const std::string& name)
{
    std::scoped_lock lock(m_mutex);

    VulkanRenderPass* renderpass = nullptr;
    auto found = m_renderPassList.emplace(description, renderpass);
    if (found.second)
    {
        renderpass = V3D_NEW(VulkanRenderPass, memory::MemoryLabel::MemoryRenderCore)(&m_device, description, name);
        if (!renderpass->create())
        {
            renderpass->destroy();
            m_renderPassList.erase(found.first);

            ASSERT(false, "can't create renderpass");
            return nullptr;
        }
        found.first->second = renderpass;

        return renderpass;
    }

    return found.first->second;
}

bool VulkanRenderpassManager::removeRenderPass(VulkanRenderPass* renderpass)
{
    std::scoped_lock lock(m_mutex);

    auto found = std::find_if(m_renderPassList.begin(), m_renderPassList.end(), [renderpass](auto& elem) -> bool
        {
            return elem.second == renderpass;
        });
    if (found == m_renderPassList.cend())
    {
        LOG_DEBUG("RenderPassManager renderpass not found");
        ASSERT(false, "renderpass");
        return false;
    }

    ASSERT(found->second == renderpass, "Different pointers");
    if (renderpass->linked())
    {
        LOG_WARNING("RenderPassManager::removeRenderPass renderPass still linked, but reqested to delete");
        ASSERT(false, "renderpass");
        return false;
    }
    m_renderPassList.erase(found);

    renderpass->destroy();
    V3D_DELETE(renderpass, memory::MemoryLabel::MemoryRenderCore);

    return true;
}

void VulkanRenderpassManager::clear()
{
    for (auto& iter : m_renderPassList)
    {
        VulkanRenderPass* renderpass = iter.second;
        if (renderpass->linked())
        {
            LOG_WARNING("RenderPassManager::removeRenderPass renderPass still linked, but reqested to delete");
            ASSERT(false, "renderpass");
        }
        renderpass->destroy();
        V3D_DELETE(renderpass, memory::MemoryLabel::MemoryRenderCore);
    }
    m_renderPassList.clear();
}

} //namespace vk
} //namespace renderer
} //namespace v3d
#endif //VULKAN_RENDER
