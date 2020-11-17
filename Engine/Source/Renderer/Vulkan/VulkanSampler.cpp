#include "VulkanSampler.h"

#include "Utils/Logger.h"

#ifdef VULKAN_RENDER
#include "VulkanDebug.h"
#include "VulkanDeviceCaps.h"
#include "VulkanPipeline.h"

namespace v3d
{
namespace renderer
{
namespace vk
{

VkFilter VulkanSampler::convertSamplerFilterToVk(SamplerFilter filter)
{
    switch (filter)
    {
    case SamplerFilter_Nearest:
        return VK_FILTER_NEAREST;

    case SamplerFilter_Bilinear:
    case SamplerFilter_Trilinear:
        return VK_FILTER_LINEAR;

    default:
        break;
    }

    ASSERT(false, "filter not found");
    return VK_FILTER_NEAREST;
}

VkSamplerMipmapMode VulkanSampler::convertMipmapSamplerFilterToVk(SamplerFilter filter)
{
    switch (filter)
    {
    case SamplerFilter_Nearest:
    case SamplerFilter_Bilinear:
        return VK_SAMPLER_MIPMAP_MODE_NEAREST;

    case SamplerFilter_Trilinear:
        return VK_SAMPLER_MIPMAP_MODE_LINEAR;

    default:
        break;
    }

    ASSERT(false, "filter not found");
    return VK_SAMPLER_MIPMAP_MODE_NEAREST;
}

f32 VulkanSampler::convertAnisotropyCount(SamplerAnisotropic level)
{
    switch (level)
    {
    case SamplerAnisotropic::SamplerAnisotropic_None:
        return 0.f;

    case SamplerAnisotropic::SamplerAnisotropic_2x:
        return 2.f;

    case SamplerAnisotropic::SamplerAnisotropic_4x:
        return 4.f;

    case SamplerAnisotropic::SamplerAnisotropic_8x:
        return 8.f;

    case SamplerAnisotropic::SamplerAnisotropic_16x:
        return 16.f;

    default:
        break;
    }

    ASSERT(false, "count not found");
    return 0.f;
}

VkSamplerAddressMode VulkanSampler::convertSamplerWrapToVkSamplerAddressMode(SamplerWrap wrap)
{
    switch (wrap)
    {
    case SamplerWrap::TextureWrap_Repeat:
        return VK_SAMPLER_ADDRESS_MODE_REPEAT;

    case SamplerWrap::TextureWrap_MirroredRepeat:
        return VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;

    case SamplerWrap::TextureWrap_ClampToEdge:
        return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;

    case SamplerWrap::TextureWrap_ClampToBorder:
        return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;

    case SamplerWrap::TextureWrap_MirroredClampToEdge:
        ASSERT(VulkanDeviceCaps::getInstance()->enableSamplerMirrorClampToEdge, "TextureWrap_MirroredClampToEdge disabled");
        return VK_SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE;

    default:
        break;
    }

    ASSERT(false, "not found");
    return VK_SAMPLER_ADDRESS_MODE_REPEAT;
}

VulkanSampler::VulkanSampler(VkDevice device, const SamplerDescription& desc) noexcept
    : Sampler(desc)
    , m_device(device)
    , m_sampler(VK_NULL_HANDLE)
{
}

VulkanSampler::~VulkanSampler()
{
    ASSERT(!m_sampler, "still exist");
}

bool VulkanSampler::create()
{
    void* vkExtension = nullptr;
    VkSamplerCustomBorderColorCreateInfoEXT samplerCustomBorderColorCreateInfoEXT = {};
    bool enableCustomBorderColor = false;
    if (VulkanDeviceCaps::getInstance()->supportSamplerBorderColor && 
        (m_desc._desc._wrapU == SamplerWrap::TextureWrap_ClampToBorder || m_desc._desc._wrapV == SamplerWrap::TextureWrap_ClampToBorder || m_desc._desc._wrapW == SamplerWrap::TextureWrap_ClampToBorder))
    {
        samplerCustomBorderColorCreateInfoEXT.sType = VK_STRUCTURE_TYPE_SAMPLER_CUSTOM_BORDER_COLOR_CREATE_INFO_EXT;
        samplerCustomBorderColorCreateInfoEXT.pNext = nullptr;
        samplerCustomBorderColorCreateInfoEXT.format = VK_FORMAT_UNDEFINED;
        samplerCustomBorderColorCreateInfoEXT.customBorderColor.float32[0] = m_desc._desc._borderColor[0];
        samplerCustomBorderColorCreateInfoEXT.customBorderColor.float32[1] = m_desc._desc._borderColor[1];
        samplerCustomBorderColorCreateInfoEXT.customBorderColor.float32[2] = m_desc._desc._borderColor[2];
        samplerCustomBorderColorCreateInfoEXT.customBorderColor.float32[3] = m_desc._desc._borderColor[3];

        vkExtension = &samplerCustomBorderColorCreateInfoEXT;
        enableCustomBorderColor = true;
    }

    VkSamplerCreateInfo samplerCreateInfo = {};
    samplerCreateInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerCreateInfo.pNext = vkExtension;
    samplerCreateInfo.flags = 0;
    samplerCreateInfo.magFilter = VulkanSampler::convertSamplerFilterToVk(m_desc._desc._filter);
    samplerCreateInfo.minFilter = VulkanSampler::convertSamplerFilterToVk(m_desc._desc._filter);
    samplerCreateInfo.mipmapMode = VulkanSampler::convertMipmapSamplerFilterToVk(m_desc._desc._filter);
    samplerCreateInfo.anisotropyEnable = (VulkanSampler::convertAnisotropyCount(m_desc._desc._anisotropic) > 0) ? VK_TRUE : VK_FALSE;
    samplerCreateInfo.maxAnisotropy = VulkanSampler::convertAnisotropyCount(m_desc._desc._anisotropic);
    ASSERT(samplerCreateInfo.maxAnisotropy <= VulkanDeviceCaps::getInstance()->getPhysicalDeviceLimits().maxSamplerAnisotropy, "max aniso");
    samplerCreateInfo.addressModeU = VulkanSampler::convertSamplerWrapToVkSamplerAddressMode(m_desc._desc._wrapU);
    samplerCreateInfo.addressModeV = VulkanSampler::convertSamplerWrapToVkSamplerAddressMode(m_desc._desc._wrapV);
    samplerCreateInfo.addressModeW = VulkanSampler::convertSamplerWrapToVkSamplerAddressMode(m_desc._desc._wrapW);
    samplerCreateInfo.mipLodBias = m_desc._desc._lodBias;
    samplerCreateInfo.minLod = 0.0f;
    samplerCreateInfo.maxLod = FLT_MAX;
    samplerCreateInfo.compareEnable = (m_desc._desc._enableCompOp) ? VK_TRUE : VK_FALSE;
    samplerCreateInfo.compareOp = VulkanGraphicPipeline::convertCompareOperationToVk(m_desc._desc._compareOp);
    samplerCreateInfo.borderColor = enableCustomBorderColor ? VK_BORDER_COLOR_FLOAT_CUSTOM_EXT : VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK;
    samplerCreateInfo.unnormalizedCoordinates = VK_FALSE;

    VkResult result = VulkanWrapper::CreateSampler(m_device, &samplerCreateInfo, VULKAN_ALLOCATOR, &m_sampler);
    if (result != VK_SUCCESS)
    {
        LOG_ERROR("VulkanSampler::create vkCreateSampler is failed. Error: %s", ErrorString(result).c_str());
        return false;
    }

    return true;
}

void VulkanSampler::destroy()
{
    if (m_sampler)
    {
        VulkanWrapper::DestroySampler(m_device, m_sampler, VULKAN_ALLOCATOR);
        m_sampler = VK_NULL_HANDLE;
    }
}

VkSampler VulkanSampler::getHandle() const
{
    ASSERT(m_sampler, "nullptr");
    return m_sampler;
}

} //namespace vk
} //namespace renderer
} //namespace v3d
#endif //VULKAN_RENDER
