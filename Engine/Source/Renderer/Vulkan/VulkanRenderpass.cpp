#include "VulkanRenderpass.h"
#include "VulkanImage.h"
#include "VulkanDebug.h"


#include "Utils/Logger.h"


#ifdef VULKAN_RENDER
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

VkImageLayout VulkanRenderPass::convertTransitionStateToImageLayout(TransitionOp state)
{
    switch (state)
    {
    case TransitionOp::TransitionOp_Undefined:
        return VK_IMAGE_LAYOUT_UNDEFINED;

    case TransitionOp::TransitionOp_ShaderRead:
        return VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    case TransitionOp::TransitionOp_ColorAttachmet:
        return VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    case TransitionOp::TransitionOp_DepthStencilAttachmet:
        return VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    case TransitionOp::TransitionOp_Present:
        return VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    default:
        ASSERT(false, "unknown");
    }

    return VK_IMAGE_LAYOUT_UNDEFINED;
}


VulkanRenderPass::VulkanRenderPass(VkDevice device, const std::vector<VulkanAttachmentDescription>& desc)
    : m_device(device)
    , m_renderpass(VK_NULL_HANDLE)

    , m_descriptions(desc)
{
    LOG_DEBUG("VulkanRenderPass::VulkanRenderPass constructor %llx", this);
}

VulkanRenderPass::~VulkanRenderPass()
{
    LOG_DEBUG("VulkanRenderPass::VulkanRenderPass destructor %llx", this);
    ASSERT(!m_renderpass, "not nullptr");
}

VkRenderPass VulkanRenderPass::getHandle() const
{
    return m_renderpass;
}

bool VulkanRenderPass::create()
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

        if (attach._samples == VK_SAMPLE_COUNT_1_BIT)
        {
            VkAttachmentDescription attachmentDescription = {};
            attachmentDescription.format = attach._format;
            attachmentDescription.samples = attach._samples;
            attachmentDescription.flags = 0;// VK_ATTACHMENT_DESCRIPTION_MAY_ALIAS_BIT; //need check
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
                index++;
                colorAttachmentReferences.push_back(attachmentReference);
            }
            else
            {
                attachmentDescription.stencilLoadOp = attach._stencilLoadOp;
                attachmentDescription.stencilStoreOp = attach._stensilStoreOp;

                depthStencilAttachmentReferences.attachment = index;
                depthStencilAttachmentReferences.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
                index++;
                depthStencil = true;
            }

            attachmentDescriptions.push_back(attachmentDescription);
        }
        else
        {
            VkAttachmentDescription msaaAttachmentDescription = {};
            msaaAttachmentDescription.format = attach._format;
            msaaAttachmentDescription.samples = attach._samples;
            msaaAttachmentDescription.flags = 0;
            msaaAttachmentDescription.loadOp = attach._loadOp;
            msaaAttachmentDescription.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            msaaAttachmentDescription.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            msaaAttachmentDescription.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            msaaAttachmentDescription.initialLayout = attach._initialLayout;
            msaaAttachmentDescription.finalLayout = attach._finalLayout;

            VkAttachmentDescription resolveAttachmentDescription = {};
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
                VkAttachmentReference msaaAttachmentReference = {};
                msaaAttachmentReference.attachment = index;
                msaaAttachmentReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
                index++;
                colorAttachmentReferences.push_back(msaaAttachmentReference);

                VkAttachmentReference resolveAttachmentReference = {};
                resolveAttachmentReference.attachment = index;
                resolveAttachmentReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
                index++;
                resolveAttachmentReferences.push_back(resolveAttachmentReference);
            }
            else
            {
                msaaAttachmentDescription.stencilLoadOp = attach._stencilLoadOp;
                msaaAttachmentDescription.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

                resolveAttachmentDescription.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
                resolveAttachmentDescription.stencilStoreOp = attach._stensilStoreOp;

                depthStencilAttachmentReferences.attachment = index;
                depthStencilAttachmentReferences.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
                index++;
                depthStencil = true;
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
        subpassDescription.pDepthStencilAttachment = depthStencil  ? &depthStencilAttachmentReferences : nullptr;
        subpassDescription.preserveAttachmentCount = 0;
        subpassDescription.pPreserveAttachments = nullptr;

        subpassDescriptions.push_back(subpassDescription);
    }

    //////
    //std::array<VkSubpassDependency, 2> dependencies;
    //dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;								// Producer of the dependency 
    //dependencies[0].dstSubpass = 0;													// Consumer is our single subpass that will wait for the execution depdendency
    //dependencies[0].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
    //dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    //dependencies[0].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
    //dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    //dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

    //dependencies[1].srcSubpass = 0;													// Producer of the dependency is our single subpass
    //dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;								// Consumer are all commands outside of the renderpass
    //dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    //dependencies[1].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
    //dependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    //dependencies[1].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
    //dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
    //////

    VkRenderPassCreateInfo renderPassCreateInfo = {};
    renderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassCreateInfo.pNext = nullptr;
    renderPassCreateInfo.flags = 0;
    renderPassCreateInfo.attachmentCount = static_cast<u32>(attachmentDescriptions.size());
    renderPassCreateInfo.pAttachments = attachmentDescriptions.data();
    renderPassCreateInfo.subpassCount = static_cast<u32>(subpassDescriptions.size());
    renderPassCreateInfo.pSubpasses = subpassDescriptions.data();
    //renderPassCreateInfo.dependencyCount = dependencies.size();
    //renderPassCreateInfo.pDependencies = dependencies.data();
    renderPassCreateInfo.dependencyCount = 0;//dependencies.size();
    renderPassCreateInfo.pDependencies = nullptr;//ependencies.data();

    VkResult result = VulkanWrapper::CreateRenderPass(m_device, &renderPassCreateInfo, VULKAN_ALLOCATOR, &m_renderpass);
    if (result != VK_SUCCESS)
    {
        LOG_ERROR("VulkanRenderPass::create vkCreateRenderPass is failed. Error: %s", ErrorString(result).c_str());
        return false;
    }

    return true;
}

void VulkanRenderPass::destroy()
{
    VulkanWrapper::DestroyRenderPass(m_device, m_renderpass, VULKAN_ALLOCATOR);
    m_renderpass = VK_NULL_HANDLE;
}

} //namespace vk
} //namespace renderer
} //namespace v3d
#endif //VULKAN_RENDER
