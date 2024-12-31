#include "VulkanDescriptorSet.h"

#include "crc32c/crc32c.h"

#include "Utils/Logger.h"
#include "Renderer/Shader.h"

#ifdef VULKAN_RENDER
#include "VulkanDebug.h"
#include "VulkanDeviceCaps.h"
#include "VulkanDescriptorPool.h"
#include "VulkanSwapchain.h"
#include "VulkanDevice.h"
#include "VulkanImage.h"
#include "VulkanBuffer.h"
#include "VulkanSampler.h"

namespace v3d
{
namespace renderer
{
namespace vk
{

/////////////////////////////////////////////////////////////////////////////////////////////////////

VkDescriptorBufferInfo makeVkDescriptorBufferInfo(const VulkanBuffer* buffer, u64 offset, u64 range)
{
    VkDescriptorBufferInfo descriptorBufferInfo = {};
    descriptorBufferInfo.buffer = buffer->getHandle();
    descriptorBufferInfo.offset = offset;
    descriptorBufferInfo.range = range;

    return descriptorBufferInfo;
}


VkDescriptorImageInfo makeVkDescriptorImageInfo(const VulkanImage* image, const VulkanSampler* sampler, VkImageLayout layout, const RenderTexture::Subresource& subresource)
{
    VkDescriptorImageInfo descriptorImageInfo = {};
    descriptorImageInfo.imageLayout = layout;

    if (sampler)
    {
        descriptorImageInfo.sampler = sampler->getHandle();
    }

    if (image)
    {
        const VulkanImage* vkImage = (image->getResolveImage()) ? image->getResolveImage() : image;
        if (vkImage->getImageAspectFlags() & VK_IMAGE_ASPECT_DEPTH_BIT)
        {
            descriptorImageInfo.imageView = vkImage->getImageView(subresource, VK_IMAGE_ASPECT_DEPTH_BIT);
        }
        else
        {
            descriptorImageInfo.imageView = vkImage->getImageView(subresource, VK_IMAGE_ASPECT_COLOR_BIT);
        }
    }

    return descriptorImageInfo;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////

auto DescriptorPoolStrategy = [](VulkanDevice* device) -> GenericDescriptorPools *
{
    if (device->getVulkanDeviceCaps()._useGlobalDescriptorPool)
    {
        return V3D_NEW(GlobalDescriptorPools, memory::MemoryLabel::MemoryRenderCore)();
    }
    else
    {
        return V3D_NEW(LayoutDescriptorPools, memory::MemoryLabel::MemoryRenderCore)();
    }
};

VulkanDescriptorSetManager::VulkanDescriptorSetManager(VulkanDevice* device, u32 swapchainImages) noexcept
    : m_device(*device)
    , m_poolProvider(V3D_NEW(VulkanDescriptorPoolProvider, memory::MemoryLabel::MemoryRenderCore)(device, DescriptorPoolStrategy(device)))
{
}

VulkanDescriptorSetManager::~VulkanDescriptorSetManager()
{
    if (m_poolProvider)
    {
        m_poolProvider->destroyDescriptorSetPools();

        V3D_DELETE(m_poolProvider, memory::MemoryLabel::MemoryRenderCore);
        m_poolProvider = nullptr;
    }
}

std::tuple<VulkanDescriptorSetPool*, VkDescriptorSet, u32> VulkanDescriptorSetManager::acquireFreeDescriptorSet(const VulkanDescriptorSetLayoutDescription& desc, VkDescriptorSetLayout layoutSet)
{
    VkDescriptorPoolCreateFlags flag = 0;

    u32 secondTry = 0;
    while (secondTry < 2)
    {
        VulkanDescriptorSetPool* pool = m_poolProvider->acquireDescriptorSetPool(desc, layoutSet, flag);
        ASSERT(pool, "nullptr");
        auto&& [descriptorSet, offset] = pool->getFreeDescriptorSet(layoutSet);
        if (descriptorSet != VK_NULL_HANDLE)
        {
            return { pool, descriptorSet, offset };
        }

        ++secondTry;
    }
    ASSERT(false, "descriptor set is not created");

    return { nullptr, VK_NULL_HANDLE, 0 };
}

void VulkanDescriptorSetManager::updateDescriptorSet(VulkanCommandBuffer* cmdBuffer, VkDescriptorSet set, const SetInfo& setInfo)
{
    std::array<VkWriteDescriptorSet, k_maxDescriptorBindingCount> writeDescriptorSets;
    u32 writeDescriptorCount = 0;

    for (u32 index = 0; index < k_maxDescriptorBindingCount; ++index)
    {
        auto& binding = setInfo._bindings[index];
        if (binding._type == BindingType::Unknown || !setInfo.isActiveBinding(binding._binding))
        {
            continue;
        }

        VkWriteDescriptorSet& writeDescriptorSet = writeDescriptorSets[writeDescriptorCount];
        writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        writeDescriptorSet.pNext = nullptr; //VkWriteDescriptorSetInlineUniformBlockEXT
        writeDescriptorSet.dstSet = set;
        writeDescriptorSet.dstBinding = binding._binding;
        writeDescriptorSet.dstArrayElement = binding._arrayIndex;
        writeDescriptorSet.descriptorCount = 1;
        writeDescriptorSet.pTexelBufferView = nullptr;

        switch (binding._type)
        {
        case BindingType::Uniform:
            ASSERT(binding._info._bufferInfo.range <= m_device.getVulkanDeviceCaps().getPhysicalDeviceLimits().maxUniformBufferRange, "out of max range");
            writeDescriptorSet.pBufferInfo = &binding._info._bufferInfo;
            writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            break;

        case BindingType::DynamicUniform:
            ASSERT(binding._info._bufferInfo.range <= m_device.getVulkanDeviceCaps().getPhysicalDeviceLimits().maxUniformBufferRange, "out of max range");
            //TODO Will update range if it gets errors
            /*VkDescriptorBufferInfo bufferInfo(bindingInfo._info._bufferInfo);
            bufferInfo.offset = 0;
            bufferInfo.range = state._offsets[index] + bufferInfo.range;*/
            writeDescriptorSet.pBufferInfo = &binding._info._bufferInfo;
            writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
            break;

        case BindingType::Sampler:
            ASSERT(binding._info._imageInfo.imageView == VK_NULL_HANDLE, "image present");
            ASSERT(binding._info._imageInfo.sampler != VK_NULL_HANDLE, "sampler");
            writeDescriptorSet.pImageInfo = &binding._info._imageInfo;
            writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;

            ASSERT(setInfo._resource[index], "invalid");
            cmdBuffer->captureResource(setInfo._resource[index]);
            break;

        case BindingType::Texture:
            ASSERT(binding._info._imageInfo.sampler == VK_NULL_HANDLE, "sampler present");
            ASSERT(binding._info._imageInfo.imageView != VK_NULL_HANDLE, "image");
            writeDescriptorSet.pImageInfo = &binding._info._imageInfo;
            writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;

            ASSERT(setInfo._resource[index], "invalid");
            cmdBuffer->captureResource(setInfo._resource[index]);
            break;

        case BindingType::RWTexture:
            ASSERT(binding._info._imageInfo.sampler == VK_NULL_HANDLE, "sampler present");
            ASSERT(binding._info._imageInfo.imageView != VK_NULL_HANDLE, "image");
            writeDescriptorSet.pImageInfo = &binding._info._imageInfo;
            writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;

            ASSERT(setInfo._resource[index], "invalid");
            cmdBuffer->captureResource(setInfo._resource[index]);
            break;

        default:
            ASSERT(false, "impl");
        }

        ++writeDescriptorCount;
    }

    if (writeDescriptorCount > 0)
    {
        VulkanWrapper::UpdateDescriptorSets(m_device.getDeviceInfo()._device, writeDescriptorCount, writeDescriptorSets.data(), 0, nullptr);
    }
}

void VulkanDescriptorSetManager::updateStatus()
{
    m_poolProvider->updateDescriptorSetPools();
}

} //namespace vk
} //namespace renderer
} //namespace v3d
#endif //VULKAN_RENDER
