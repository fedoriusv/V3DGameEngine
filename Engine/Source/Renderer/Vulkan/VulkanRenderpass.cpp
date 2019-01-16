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
    //TODO merge renderpasses
    std::vector<VkAttachmentDescription> attachmentDescriptions;
    attachmentDescriptions.reserve(m_descriptions.size());

    std::vector<VkAttachmentReference> colorAttachmentReferences;
    VkAttachmentReference depthStencilAttachmentReferences = {};

    bool depthStencil = false;
    for (u32 index = 0; index < m_descriptions.size(); ++index)
    {
        VulkanAttachmentDescription& attach = m_descriptions[index];
        ASSERT(attach._format != VK_FORMAT_UNDEFINED, "undefined format");

        VkAttachmentDescription vkAttach = {};
        vkAttach.format = attach._format;
        vkAttach.samples = attach._samples;
        vkAttach.flags = 0;// VK_ATTACHMENT_DESCRIPTION_MAY_ALIAS_BIT; //need check
        vkAttach.loadOp = attach._loadOp;
        vkAttach.storeOp = attach._storeOp;
        vkAttach.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE; //TODO: if use stencil need enable it
        vkAttach.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

        //TODO:
        vkAttach.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        vkAttach.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        attachmentDescriptions.push_back(vkAttach);

        if (VulkanImage::isColorFormat(attach._format))
        {
            VkAttachmentReference attachmentReference = {};
            attachmentReference.attachment = index;
            attachmentReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

            colorAttachmentReferences.push_back(attachmentReference);
        }
        else
        {
            depthStencilAttachmentReferences.attachment = index;
            depthStencilAttachmentReferences.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
            depthStencil = true;
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
        subpassDescription.inputAttachmentCount = 0; //TODO
        subpassDescription.pInputAttachments = nullptr;
        subpassDescription.colorAttachmentCount = static_cast<u32>(colorAttachmentReferences.size());
        subpassDescription.pColorAttachments = colorAttachmentReferences.data();
        subpassDescription.pResolveAttachments = nullptr;
        subpassDescription.pDepthStencilAttachment = depthStencil  ? &depthStencilAttachmentReferences : nullptr;
        subpassDescription.preserveAttachmentCount = 0;
        subpassDescription.pPreserveAttachments = nullptr;

        subpassDescriptions.push_back(subpassDescription);
    }

    VkRenderPassCreateInfo renderPassCreateInfo = {};
    renderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassCreateInfo.pNext = nullptr;
    renderPassCreateInfo.flags = 0;
    renderPassCreateInfo.attachmentCount = static_cast<u32>(attachmentDescriptions.size());
    renderPassCreateInfo.pAttachments = attachmentDescriptions.data();
    renderPassCreateInfo.subpassCount = static_cast<u32>(subpassDescriptions.size());
    renderPassCreateInfo.pSubpasses = subpassDescriptions.data();
    renderPassCreateInfo.dependencyCount = 0;
    renderPassCreateInfo.pDependencies = nullptr;

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
