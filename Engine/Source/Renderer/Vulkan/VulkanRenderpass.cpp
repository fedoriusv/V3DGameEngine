#include "VulkanRenderpass.h"

#include "Utils/Logger.h"

#ifdef VULKAN_RENDER
#include "VulkanDebug.h"
#include "VulkanDeviceCaps.h"
#include "VulkanTransitionState.h"
#include "VulkanImage.h"

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

VulkanRenderPass::VulkanRenderPass(VkDevice device, const RenderPassDescription& description, const std::string& name) noexcept
    : RenderPass(description)
    , m_device(device)
    , m_renderpass(VK_NULL_HANDLE)
{
    LOG_DEBUG("VulkanRenderPass::VulkanRenderPass constructor %llx", this);

    u32 countAttachments = (description._desc._hasDepthStencilAttahment) ? description._desc._countColorAttachments + 1 : description._desc._countColorAttachments;
    m_descriptions.resize(countAttachments);
    for (u32 index = 0; index < description._desc._countColorAttachments; ++index)
    {
        VulkanRenderPass::VulkanAttachmentDescription& desc = m_descriptions[index];
        desc._format = VulkanImage::convertImageFormatToVkFormat(description._desc._attachments[index]._format);
        desc._samples = VulkanImage::convertRenderTargetSamplesToVkSampleCount(description._desc._attachments[index]._samples);
        desc._loadOp = VulkanRenderPass::convertAttachLoadOpToVkAttachmentLoadOp(description._desc._attachments[index]._loadOp);
        desc._storeOp = VulkanRenderPass::convertAttachStoreOpToVkAttachmentStoreOp(description._desc._attachments[index]._storeOp);
        desc._initialLayout = VulkanTransitionState::convertTransitionStateToImageLayout(description._desc._attachments[index]._initTransition);
        desc._finalLayout = VulkanTransitionState::convertTransitionStateToImageLayout(description._desc._attachments[index]._finalTransition);
        desc._layer = AttachmentDescription::uncompressLayer(description._desc._attachments[index]._layer);
        desc._mip = 0;

        desc._swapchainImage = (description._desc._attachments[index]._backbuffer) ? true : false;
        desc._autoResolve = (description._desc._attachments[index]._autoResolve) ? true : false;
    }

    if (description._desc._hasDepthStencilAttahment)
    {
        VulkanRenderPass::VulkanAttachmentDescription& desc = m_descriptions.back();
        desc._format = VulkanImage::convertImageFormatToVkFormat(description._desc._attachments.back()._format);
        desc._samples = VulkanImage::convertRenderTargetSamplesToVkSampleCount(description._desc._attachments.back()._samples);
        desc._loadOp = VulkanRenderPass::convertAttachLoadOpToVkAttachmentLoadOp(description._desc._attachments.back()._loadOp);
        desc._storeOp = VulkanRenderPass::convertAttachStoreOpToVkAttachmentStoreOp(description._desc._attachments.back()._storeOp);
        desc._stencilLoadOp = VulkanRenderPass::convertAttachLoadOpToVkAttachmentLoadOp(description._desc._attachments.back()._stencilLoadOp);
        desc._stensilStoreOp = VulkanRenderPass::convertAttachStoreOpToVkAttachmentStoreOp(description._desc._attachments.back()._stencilStoreOp);
        desc._initialLayout = VulkanTransitionState::convertTransitionStateToImageLayout(description._desc._attachments.back()._initTransition);
        desc._finalLayout = VulkanTransitionState::convertTransitionStateToImageLayout(description._desc._attachments.back()._finalTransition);
        desc._layer = AttachmentDescription::uncompressLayer(description._desc._attachments.back()._layer);
        desc._mip = 0;

        desc._swapchainImage = false;
        desc._autoResolve = (description._desc._attachments.back()._autoResolve) ? true : false;
    }

#if VULKAN_DEBUG_MARKERS
    m_debugName = name.empty() ? "Renderpass" : name;
    m_debugName.append(VulkanDebugUtils::k_addressPreffix);
    m_debugName.append(std::to_string(reinterpret_cast<const u64>(this)));
#endif //VULKAN_DEBUG_MARKERS
}

VulkanRenderPass::~VulkanRenderPass()
{
    LOG_DEBUG("VulkanRenderPass::VulkanRenderPass destructor %llx", this);
    ASSERT(!m_renderpass, "not nullptr");
}

VkRenderPass VulkanRenderPass::getHandle() const
{
    ASSERT(m_renderpass != VK_NULL_HANDLE, "nullptr");
    return m_renderpass;
}

const VulkanRenderPass::VulkanAttachmentDescription& VulkanRenderPass::getAttachmentDescription(u32 index) const
{
    ASSERT(index < m_descriptions.size(), "range out");
    return m_descriptions[index];
}

bool VulkanRenderPass::isDrawingToSwapchain() const
{
    return (VulkanRenderPass::getDescription()._countColorAttachments == 1) && (VulkanRenderPass::getDescription()._attachments[0]._backbuffer);
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

    if (VulkanDeviceCaps::getInstance()->supportRenderpass2)
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
    if (VulkanDeviceCaps::getInstance()->debugUtilsObjectNameEnabled)
    {
        VkDebugUtilsObjectNameInfoEXT debugUtilsObjectNameInfo = {};
        debugUtilsObjectNameInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
        debugUtilsObjectNameInfo.pNext = nullptr;
        debugUtilsObjectNameInfo.objectType = VK_OBJECT_TYPE_RENDER_PASS;
        debugUtilsObjectNameInfo.objectHandle = reinterpret_cast<u64>(m_renderpass);
        debugUtilsObjectNameInfo.pObjectName = m_debugName.c_str();

        VulkanWrapper::SetDebugUtilsObjectName(m_device, &debugUtilsObjectNameInfo);
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

    bool depthStencil = false;
    u32 index = 0;
    for (u32 descIndex = 0; descIndex < m_descriptions.size(); ++descIndex)
    {
        VulkanAttachmentDescription& attach = m_descriptions[descIndex];
        ASSERT(attach._format != VK_FORMAT_UNDEFINED, "undefined format");

        bool disableAutoresolve = !attach._autoResolve;
        if (VulkanImage::isDepthStencilFormat(attach._format))
        {
            disableAutoresolve = VulkanDeviceCaps::getInstance()->supportDepthAutoResolve ? disableAutoresolve : true;
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
    if (VulkanDeviceCaps::getInstance()->supportMultiview && m_desc._desc._viewsMask != 0)
    {
        viewMasksBySubpass.push_back(m_desc._desc._viewsMask);
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

    VkRenderPassCreateInfo renderPassCreateInfo = {};
    renderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassCreateInfo.pNext = vkExtensions;
    renderPassCreateInfo.flags = 0;
    renderPassCreateInfo.attachmentCount = static_cast<u32>(attachmentDescriptions.size());
    renderPassCreateInfo.pAttachments = attachmentDescriptions.data();
    renderPassCreateInfo.subpassCount = static_cast<u32>(subpassDescriptions.size());
    renderPassCreateInfo.pSubpasses = subpassDescriptions.data();
    renderPassCreateInfo.dependencyCount = 0;     //dependencies.size();
    renderPassCreateInfo.pDependencies = nullptr; //ependencies.data();

#ifdef VK_QCOM_render_pass_transform
    if (VulkanDeviceCaps::getInstance()->renderpassTransformQCOM)
    {
        renderPassCreateInfo.flags |= VK_RENDER_PASS_CREATE_TRANSFORM_BIT_QCOM;
    }
#endif

    VkResult result = VulkanWrapper::CreateRenderPass(m_device, &renderPassCreateInfo, VULKAN_ALLOCATOR, &m_renderpass);
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
            disableAutoresolve = VulkanDeviceCaps::getInstance()->supportDepthAutoResolve ? disableAutoresolve : true;
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
                depthStencilAttachmentReferences.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
                depthStencilAttachmentReferences.aspectMask = VulkanImage::getImageAspectFlags(attach._format);

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
                depthStencilAttachmentReferences.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
                depthStencilAttachmentReferences.aspectMask = VulkanImage::getImageAspectFlags(attach._format);

                m_layout[index] = { depthStencilAttachmentReferences.layout, attach._finalLayout };
                ++index;

                if (VulkanDeviceCaps::getInstance()->supportDepthAutoResolve)
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

    u32 countSubpasses = 1;
    std::vector<VkSubpassDescription2KHR> subpassDescriptions;
    subpassDescriptions.reserve(countSubpasses);

    for (u32 subpassIndex = 0; subpassIndex < countSubpasses; ++subpassIndex)
    {
        VkSubpassDescription2KHR subpassDescription = {};
        subpassDescription.sType = VK_STRUCTURE_TYPE_SUBPASS_DESCRIPTION_2_KHR;

        VkSubpassDescriptionDepthStencilResolveKHR subpassDescriptionDepthStencilResolve = {};
        subpassDescriptionDepthStencilResolve.sType = VK_STRUCTURE_TYPE_SUBPASS_DESCRIPTION_DEPTH_STENCIL_RESOLVE_KHR;
        subpassDescriptionDepthStencilResolve.pNext = nullptr;
        if (depthStencilAutoresolve)
        {
            subpassDescriptionDepthStencilResolve.pDepthStencilResolveAttachment = &resolveDepthStencilAttachmentReferences;
            subpassDescriptionDepthStencilResolve.depthResolveMode = VK_RESOLVE_MODE_AVERAGE_BIT_KHR;
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
        if (VulkanDeviceCaps::getInstance()->supportMultiview && m_desc._desc._viewsMask != 0)
        {
            subpassDescription.viewMask = m_desc._desc._viewsMask;
        }
        else
        {
            subpassDescription.viewMask = 0;
        }

        subpassDescriptions.push_back(subpassDescription);
    }

    VkRenderPassCreateInfo2KHR renderPassCreateInfo = {};
    renderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO_2_KHR;
    renderPassCreateInfo.pNext = nullptr;
    renderPassCreateInfo.flags = 0;
    renderPassCreateInfo.attachmentCount = static_cast<u32>(attachmentDescriptions.size());
    renderPassCreateInfo.pAttachments = attachmentDescriptions.data();
    renderPassCreateInfo.subpassCount = static_cast<u32>(subpassDescriptions.size());
    renderPassCreateInfo.pSubpasses = subpassDescriptions.data();
    renderPassCreateInfo.dependencyCount = 0;     //dependencies.size();
    renderPassCreateInfo.pDependencies = nullptr; //ependencies.data();
    renderPassCreateInfo.correlatedViewMaskCount = 0;
    renderPassCreateInfo.pCorrelatedViewMasks = nullptr;

#ifdef VK_QCOM_render_pass_transform
    if (VulkanDeviceCaps::getInstance()->renderpassTransformQCOM)
    {
        renderPassCreateInfo.flags |= VK_RENDER_PASS_CREATE_TRANSFORM_BIT_QCOM;
    }
#endif

    VkResult result = VulkanWrapper::CreateRenderPass2(m_device, &renderPassCreateInfo, VULKAN_ALLOCATOR, &m_renderpass);
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
        VulkanWrapper::DestroyRenderPass(m_device, m_renderpass, VULKAN_ALLOCATOR);
        m_renderpass = VK_NULL_HANDLE;
    }

    m_layout.clear();
}

} //namespace vk
} //namespace renderer
} //namespace v3d
#endif //VULKAN_RENDER
