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

    case SamplerFilter_Cubic:
        return VK_FILTER_CUBIC_IMG;

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
    case SamplerFilter_Cubic:
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

VulkanSampler::VulkanSampler(VkDevice device) noexcept
    : m_device(device)
    , m_sampler(VK_NULL_HANDLE)
{
}

VulkanSampler::~VulkanSampler()
{
    ASSERT(!m_sampler, "still exist");
}

bool VulkanSampler::create(const SamplerDescription& info)
{
    VkSamplerCreateInfo samplerCreateInfo = {};
    samplerCreateInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerCreateInfo.pNext = nullptr; //VkSamplerReductionModeCreateInfoEXT, VkSamplerYcbcrConversionInfo
    samplerCreateInfo.flags = 0;
    samplerCreateInfo.magFilter = VulkanSampler::convertSamplerFilterToVk(info._magFilter);
    samplerCreateInfo.minFilter = VulkanSampler::convertSamplerFilterToVk(info._minFilter);
    samplerCreateInfo.mipmapMode = VulkanSampler::convertMipmapSamplerFilterToVk(info._minFilter);
    samplerCreateInfo.anisotropyEnable = (VulkanSampler::convertAnisotropyCount(info._anisotropic) > 0) ? VK_TRUE : VK_FALSE;
    samplerCreateInfo.maxAnisotropy = VulkanSampler::convertAnisotropyCount(info._anisotropic);
    ASSERT(samplerCreateInfo.maxAnisotropy <= VulkanDeviceCaps::getInstance()->getPhysicalDeviceLimits().maxSamplerAnisotropy, "max aniso");
    samplerCreateInfo.addressModeU = VulkanSampler::convertSamplerWrapToVkSamplerAddressMode(info._wrapU);
    samplerCreateInfo.addressModeV = VulkanSampler::convertSamplerWrapToVkSamplerAddressMode(info._wrapV);
    samplerCreateInfo.addressModeW = VulkanSampler::convertSamplerWrapToVkSamplerAddressMode(info._wrapW);
    samplerCreateInfo.mipLodBias = info._lodBias;
    samplerCreateInfo.minLod = 0.0f;
    samplerCreateInfo.maxLod = FLT_MAX;
    samplerCreateInfo.compareEnable = (info._enableCompOp) ? VK_TRUE : VK_FALSE;
    samplerCreateInfo.compareOp = VulkanGraphicPipeline::convertCompareOperationToVk(info._compareOp);
    samplerCreateInfo.borderColor = VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK;
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
