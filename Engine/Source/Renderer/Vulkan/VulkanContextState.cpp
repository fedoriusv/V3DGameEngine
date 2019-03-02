#include "VulkanContextState.h"

#include "Utils/Logger.h"
#include "crc32c/crc32c.h"

#ifdef VULKAN_RENDER
#include "VulkanRenderpass.h"
#include "VulkanFramebuffer.h"
#include "VulkanPipeline.h"
#include "VulkanDescriptorSet.h"
#include "VulkanImage.h"
#include "VulkanBuffer.h"
#include "VulkanSampler.h"
#include "VulkanUnifromBuffer.h"
#include "VulkanDeviceCaps.h"

namespace v3d
{
namespace renderer
{
namespace vk
{

VulkanContextState::VulkanContextState(VkDevice device, VulkanDescriptorSetManager* descriptorSetManager, VulkanUniformBufferManager* unifromBufferManager) noexcept
    : m_device(device)
    , m_swapchainIndex(0)
    , m_descriptorSetManager(descriptorSetManager)
    , m_unifromBufferManager(unifromBufferManager)
{
    m_currentRenderpass = { nullptr, false };
    m_currentFramebuffer = { { nullptr }, false };
    m_currentPipeline = { nullptr, false };

    m_currentVertexBuffers.second = false;

    m_renderPassArea = { 0, 0, 0, 0 };
    m_renderPassClearValues.resize(1, { 0.0f, 0.0f, 0.0f, 0.0f });
}

void VulkanContextState::updateSwapchainIndex(u32 index)
{
    m_swapchainIndex = index;
}

void VulkanContextState::invalidateCommandBuffer(CommandTargetType type)
{
    if (type == CommandTargetType::CmdDrawBuffer)
    {
        m_stateCallbacks.clear();
    }
}

inline bool VulkanContextState::isCurrentRenderPass(const VulkanRenderPass* pass) const
{
    return m_currentRenderpass.first == pass;
}

inline bool VulkanContextState::isCurrentFramebuffer(const VulkanFramebuffer * framebuffer) const
{
    if (m_currentFramebuffer.first.size() == 1)
    {
        return m_currentFramebuffer.first[0] == framebuffer;
    }
    else
    {
        for (auto framebuff : m_currentFramebuffer.first)
        {
            if (framebuff == framebuffer)
            {
                return true;
            }
        }

        return false;
    }

    return false;
}

inline bool VulkanContextState::isCurrentPipeline(const VulkanGraphicPipeline * pipeline) const
{
    return m_currentPipeline.first == pipeline;
}

bool VulkanContextState::setCurrentRenderPass(VulkanRenderPass * pass)
{
    bool changed = !VulkanContextState::isCurrentRenderPass(pass);
    m_currentRenderpass.first = pass;
    m_currentRenderpass.second = changed;

    return changed;
}

bool VulkanContextState::setCurrentFramebuffer(VulkanFramebuffer * framebuffer)
{
    bool changed = !VulkanContextState::isCurrentFramebuffer(framebuffer);
    m_currentFramebuffer.first[0] = framebuffer;
    m_currentFramebuffer.second = changed;

    return changed;
}

bool VulkanContextState::setCurrentFramebuffer(std::vector<VulkanFramebuffer*>& framebuffers)
{
    bool changed = !VulkanContextState::isCurrentFramebuffer(framebuffers[0]);
    m_currentFramebuffer.first = std::move(framebuffers);
    m_currentFramebuffer.second = changed;

    return changed;
}

bool VulkanContextState::setCurrentPipeline(VulkanGraphicPipeline * pipeline)
{
    bool changed = !VulkanContextState::isCurrentPipeline(pipeline);
    m_currentPipeline.first = pipeline;
    m_currentPipeline.second = changed;

    return changed;
}

bool VulkanContextState::setCurrentVertexBuffers(StreamBufferDescription & desc)
{
    bool changed = m_currentVertexBuffers.first != desc;
    if (changed)
    {
        m_currentVertexBuffers.first = std::move(desc);
    }
    m_currentVertexBuffers.second = changed;

    return changed;
}

void VulkanContextState::setClearValues(const VkRect2D & area, std::vector<VkClearValue>& clearValues)
{
    m_renderPassArea = area;
    m_renderPassClearValues = std::move(clearValues);
}

VulkanRenderPass * VulkanContextState::getCurrentRenderpass() const
{
    ASSERT(m_currentRenderpass.first, "nullptr");
    return m_currentRenderpass.first;
}

VulkanFramebuffer * VulkanContextState::getCurrentFramebuffer() const
{
    ASSERT(!m_currentFramebuffer.first.empty(), "nullptr");
    if (m_currentFramebuffer.first.size() == 1)
    {
        return m_currentFramebuffer.first[0];
    }
    else
    {
        return m_currentFramebuffer.first[m_swapchainIndex];
    }
}

VulkanGraphicPipeline * VulkanContextState::getCurrentPipeline() const
{
    ASSERT(m_currentPipeline.first, "nullptr");
    return m_currentPipeline.first;
}

const StreamBufferDescription & VulkanContextState::getStreamBufferDescription() const
{
    return m_currentVertexBuffers.first;
}

bool VulkanContextState::setDynamicState(VkDynamicState state, const std::function<void()>& callback)
{
    auto iter = m_stateCallbacks.emplace(state, callback);
    if (iter.second)
    {
        return true;
    }

    iter.first->second = callback;
    return false;
}

void VulkanContextState::invokeDynamicStates()
{
    for (auto& callback : m_stateCallbacks)
    {
        callback.second();
    }
}

bool VulkanContextState::prepareDescriptorSets(VulkanCommandBuffer * cmdBuffer, std::vector<VkDescriptorSet>& sets, std::vector<u32>& offsets)
{
    //TODO:
    //m_currentBindingCache.clear();

    if (!m_currentSets.empty())
    {
        sets = m_currentSets;
        return true;
    }

    m_currentPool = m_descriptorSetManager->acquireDescriptorSets(m_currentPipeline.first->getDescriptorSetLayouts(), sets, offsets);
    ASSERT(m_currentPool, "m_currentPool");
    m_currentPool->captureInsideCommandBuffer(cmdBuffer, 0);

    if (sets.empty())
    {
        return false;
    }

    m_currentSets = sets;
    return updateDescriptorSet(cmdBuffer, sets);
}

bool VulkanContextState::updateDescriptorSet(VulkanCommandBuffer* cmdBuffer, std::vector<VkDescriptorSet>& sets)
{
    if (m_updatedBindings.empty())
    {
        return false;
    }

    std::vector<VkWriteDescriptorSet> writeDescriptorSets;
    writeDescriptorSets.reserve(m_updatedBindings.size());

    for (auto& binding : m_updatedBindings)
    {
        VkWriteDescriptorSet writeDescriptorSet = {};
        writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        writeDescriptorSet.pNext = nullptr; //VkWriteDescriptorSetInlineUniformBlockEXT
        writeDescriptorSet.dstSet = sets[binding._set];
        writeDescriptorSet.dstBinding = binding._binding;
        writeDescriptorSet.dstArrayElement = binding._arrayIndex;
        writeDescriptorSet.descriptorCount = 1;
        writeDescriptorSet.pTexelBufferView = nullptr;

        switch (binding._type)
        {
        case BindingType::BindingType_Uniform:
            writeDescriptorSet.pBufferInfo = &binding._bufferBinding._bufferInfo;
            writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;

            binding._bufferBinding._buffer->captureInsideCommandBuffer(cmdBuffer, 0);
            break;

        case BindingType::BindingType_DynamicUniform:
            writeDescriptorSet.pBufferInfo = &binding._bufferBinding._bufferInfo;
            writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;

            binding._bufferBinding._buffer->captureInsideCommandBuffer(cmdBuffer, 0);
            break;

        case BindingType::BindingType_Sampler:
            ASSERT(binding._imageBinding._imageInfo.imageView == VK_NULL_HANDLE, "image present");
            ASSERT(binding._imageBinding._imageInfo.sampler != VK_NULL_HANDLE, "sampler");
            writeDescriptorSet.pImageInfo = &binding._imageBinding._imageInfo;
            writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;

            binding._imageBinding._sampler->captureInsideCommandBuffer(cmdBuffer, 0);
            break;

        case BindingType::BindingType_Texture:
            ASSERT(binding._imageBinding._imageInfo.sampler == VK_NULL_HANDLE, "sampler present");
            ASSERT(binding._imageBinding._imageInfo.imageView != VK_NULL_HANDLE, "image");
            writeDescriptorSet.pImageInfo = &binding._imageBinding._imageInfo;
            writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;

            binding._imageBinding._image->captureInsideCommandBuffer(cmdBuffer, 0);
            break;

        case BindingType::BindingType_SamplerAndTexture:
            ASSERT(binding._imageBinding._imageInfo.imageView != VK_NULL_HANDLE, "image");
            //ASSERT(binding._imageBinding._imageInfo.sampler != VK_NULL_HANDLE, "sampler");
            writeDescriptorSet.pImageInfo = &binding._imageBinding._imageInfo;
            writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;

            binding._imageBinding._image->captureInsideCommandBuffer(cmdBuffer, 0);
            binding._imageBinding._sampler->captureInsideCommandBuffer(cmdBuffer, 0);
            break;

        default:
            ASSERT(false, "impl");
        }

        writeDescriptorSets.push_back(writeDescriptorSet);
    }

    VulkanWrapper::UpdateDescriptorSets(m_device, static_cast<u32>(writeDescriptorSets.size()), writeDescriptorSets.data(), 0, nullptr);
    m_updatedBindings.clear();

    return true;
}

void VulkanContextState::bindTexture(const VulkanImage* image, const VulkanSampler* sampler, u32 arrayIndex, const Shader::SampledImage& reflaction)
{
    m_currentSets.clear();

    /*auto& bindingList = m_descriptorSetsState[reflaction._set];
    if (bindingList.size() < reflaction._binding)
    {
        ASSERT(reflaction._binding < k_maxDescriptorBindingIndex, "invalid binding");
        bindingList.resize(reflaction._binding + 1);
    }
    BindingInfo& bindingInfo = bindingList[reflaction._binding];

    bool updated = false;
    if (!bindingInfo._imageBinding._image && !bindingInfo._imageBinding._sampler)
    {
        updated = true;
    }
    else if (bindingInfo._imageBinding._image != image || bindingInfo._imageBinding._sampler != sampler)
    {
        updated = true;
    }

    if (updated)
    {
        bindingInfo._type = BindingType::BindingType_SamplerAndTexture;
        bindingInfo._set = reflaction._set;
        bindingInfo._binding = reflaction._binding;
        bindingInfo._arrayIndex = arrayIndex;
        bindingInfo._imageBinding._image = image;
        bindingInfo._imageBinding._sampler = sampler;
        bindingInfo._imageBinding._imageInfo = VulkanContextState::makeVkDescriptorImageInfo(image, sampler);

        VulkanContextState::setBinding(bindingInfo);
    }*/

    BindingInfo bindingInfo;
    bindingInfo._type = BindingType::BindingType_SamplerAndTexture;
    bindingInfo._set = reflaction._set;
    bindingInfo._binding = reflaction._binding;
    bindingInfo._arrayIndex = arrayIndex;
    bindingInfo._imageBinding._image = image;
    bindingInfo._imageBinding._sampler = sampler;
    bindingInfo._imageBinding._imageInfo = VulkanContextState::makeVkDescriptorImageInfo(image, sampler);

    VulkanContextState::setBinding(bindingInfo);
}

void VulkanContextState::updateConstantBuffer(u32 arrayIndex, const Shader::UniformBuffer& reflaction, u32 offset, u32 size, const void* data)
{
    m_currentSets.clear();
    ASSERT(size <= reflaction._size, "over size");
    /*auto& bindingList = m_descriptorSetsState[reflaction._set];
    if (bindingList.size() < reflaction._binding + 1)
    {
        ASSERT(reflaction._binding < k_maxDescriptorBindingIndex, "invalid binding");
        bindingList.resize(reflaction._binding + 1);
    }
    BindingInfo& bindingInfo = bindingList[reflaction._binding];

    bool updated = false;
    VulkanUniformBuffer * uniformBuffer = nullptr;
    if (!bindingInfo._bufferBinding._buffer)
    {
        uniformBuffer = m_unifromBufferManager->acquireUnformBuffer(reflaction._size);
        updated = true;
    }
    else
    {
        uniformBuffer = bindingInfo._bufferBinding._uniform;
        ASSERT(uniformBuffer, "nulptr");

        updated = uniformBuffer->update(offset, size, data);
    }

    if (updated)
    {
        bindingInfo._set = reflaction._set;
        bindingInfo._binding = reflaction._binding;
        bindingInfo._type = VulkanDeviceCaps::getInstance()->useDynamicUniforms ? BindingType::BindingType_DynamicUniform : BindingType::BindingType_Uniform;
        bindingInfo._arrayIndex = arrayIndex;
        bindingInfo._bufferBinding._uniform = uniformBuffer;
        bindingInfo._bufferBinding._buffer = uniformBuffer->getBuffer();
        bindingInfo._bufferBinding._bufferInfo = VulkanContextState::makeVkDescriptorBufferInfo(uniformBuffer->getBuffer(), uniformBuffer->getOffset(), uniformBuffer->getSize());

        VulkanContextState::setBinding(bindingInfo);
    }*/

    VulkanUniformBuffer * uniformBuffer = m_unifromBufferManager->acquireUnformBuffer(reflaction._size);
    bool updated = uniformBuffer->update(offset, size, data);

    BindingInfo bindingInfo;
    bindingInfo._set = reflaction._set;
    bindingInfo._binding = reflaction._binding;
    bindingInfo._type = VulkanDeviceCaps::getInstance()->useDynamicUniforms ? BindingType::BindingType_DynamicUniform : BindingType::BindingType_Uniform;
    bindingInfo._arrayIndex = arrayIndex;
    bindingInfo._bufferBinding._uniform = uniformBuffer;
    bindingInfo._bufferBinding._buffer = uniformBuffer->getBuffer();
    bindingInfo._bufferBinding._bufferInfo = VulkanContextState::makeVkDescriptorBufferInfo(uniformBuffer->getBuffer(), uniformBuffer->getOffset(), uniformBuffer->getSize());

    VulkanContextState::setBinding(bindingInfo);
}

void VulkanContextState::invalidateDescriptorSetsState()
{
    bool resetCurrentPool = false;
    if (resetCurrentPool)
    {
        LOG_DEBUG("VulkanContextState::invalidateDescriptorSetsState Reset pool");
        if (m_currentPool->isCaptured())
        {
            ASSERT(false, "still used");
        }
        m_currentPool->reset(0);
    }
    m_descriptorSetManager->updateDescriptorPools();
}

VkDescriptorBufferInfo VulkanContextState::makeVkDescriptorBufferInfo(const VulkanBuffer* buffer, u64 offset, u64 range)
{
    ASSERT(range <= VulkanDeviceCaps::getInstance()->getPhysicalDeviceLimits().maxUniformBufferRange, "out range");
    VkDescriptorBufferInfo descriptorBufferInfo = {};
    descriptorBufferInfo.buffer = buffer->getHandle();
    descriptorBufferInfo.offset = offset;
    descriptorBufferInfo.range = range;

    return descriptorBufferInfo;
}

VkDescriptorImageInfo VulkanContextState::makeVkDescriptorImageInfo(const VulkanImage* image, const VulkanSampler* sampler)
{
    VkDescriptorImageInfo descriptorImageInfo = {};
    descriptorImageInfo.sampler = sampler->getHandle();
    descriptorImageInfo.imageView = image->getImageView();
    descriptorImageInfo.imageLayout = image->getLayout();

    return descriptorImageInfo;
}

void VulkanContextState::setBinding(BindingInfo & binding)
{
    u32 hash = crc32c::Crc32c(reinterpret_cast<u8*>(&binding), sizeof(BindingInfo));

    auto iter = m_currentBindingCache.find(hash);
    if (iter == m_currentBindingCache.cend())
    {
        m_currentBindingCache.insert({ hash, binding });
    }

    //TODO
    m_updatedBindings.push_back(binding);
}

VulkanContextState::BindingInfo::BindingInfo()
{
    memset(this, 0, sizeof(VulkanContextState::BindingInfo));
}

bool VulkanContextState::BindingInfo::operator==(const BindingInfo & info) const
{
    if (this == &info)
    {
        return true;
    }

    if (_set != info._set || _binding != info._binding 
        || _type != info._type || _arrayIndex != info._arrayIndex)
    {
        return false;
    }

    if ( memcmp(&_bufferBinding, &info._bufferBinding, sizeof(BindingBufferInfo)) ||
        memcmp(&_imageBinding, &info._imageBinding, sizeof(BindingImageInfo)) )
    {
        return false;
    }

    return true;
}

} //namespace vk
} //namespace renderer
} //namespace v3d
#endif //VULKAN_RENDER
