#include "VulkanContextState.h"

#include "Utils/Logger.h"
#include "crc32c/crc32c.h"

#ifdef VULKAN_RENDER
#include "VulkanRenderpass.h"
#include "VulkanFramebuffer.h"
#include "VulkanPipeline.h"
#include "VulkanDescriptorSet.h"
#include "VulkanDescriptorPool.h"
#include "VulkanImage.h"
#include "VulkanBuffer.h"
#include "VulkanSampler.h"
#include "VulkanUnifromBuffer.h"
#include "VulkanDeviceCaps.h"
#include "VulkanSwapchain.h"

namespace v3d
{
namespace renderer
{
namespace vk
{

VulkanContextState::VulkanContextState(VkDevice device, VulkanDescriptorSetManager* descriptorSetManager, VulkanUniformBufferManager* unifromBufferManager) noexcept
    : m_device(device)
    , m_descriptorSetManager(descriptorSetManager)
    , m_unifromBufferManager(unifromBufferManager)
{
    m_currentRenderpass = { nullptr, false };
    m_currentFramebuffer = { { nullptr }, false };
    m_currentPipeline = { nullptr, false };

    m_currentVertexBuffers.second = false;

    m_renderPassArea = {{ 0, 0 }, { 0, 0 }};
    m_renderPassClearValues.resize(1, {{{0.0f, 0.0f, 0.0f, 0.0f}}});

    VulkanContextState::invalidateDescriptorSetsState();
    m_currentDesctiptorsSets.fill(VK_NULL_HANDLE);
}

void VulkanContextState::invalidateCommandBuffer(CommandTargetType type)
{
    if (type == CommandTargetType::CmdDrawBuffer)
    {
        m_stateCallbacks.clear();
    }
}

bool VulkanContextState::isCurrentRenderPass(const VulkanRenderPass* pass) const
{
    return m_currentRenderpass.first == pass;
}

bool VulkanContextState::isCurrentFramebuffer(const VulkanFramebuffer * framebuffer) const
{
    if (m_currentFramebuffer.first.size() == 1)
    {
        return m_currentFramebuffer.first[0] == framebuffer;
    }
    else
    {
        for (auto& framebuff : m_currentFramebuffer.first)
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

bool VulkanContextState::isCurrentPipeline(const VulkanGraphicPipeline * pipeline) const
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
    bool changed = !VulkanContextState::isCurrentFramebuffer(framebuffers.back());
    m_currentFramebuffer.first = std::move(framebuffers);
    m_currentFramebuffer.second = changed;

    return changed;
}

bool VulkanContextState::setCurrentPipeline(VulkanGraphicPipeline* pipeline)
{
    bool changed = !VulkanContextState::isCurrentPipeline(pipeline);
    m_currentPipeline.first = pipeline;
    m_currentPipeline.second = changed;

    return changed;
}

bool VulkanContextState::setCurrentVertexBuffers(const StreamBufferDescription & desc)
{
    bool changed = m_currentVertexBuffers.first != desc;
    if (changed)
    {
        m_currentVertexBuffers.first = desc;//std::move(desc);
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
        return m_currentFramebuffer.first[VulkanSwapchain::currentSwapchainIndex()];
    }
}

VulkanGraphicPipeline* VulkanContextState::getCurrentPipeline() const
{
    //ASSERT(m_currentPipeline.first, "nullptr");
    return m_currentPipeline.first;
}

const StreamBufferDescription& VulkanContextState::getStreamBufferDescription() const
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

void VulkanContextState::invokeDynamicStates(bool clear)
{
    for (auto& callback : m_stateCallbacks)
    {
        callback.second();
    }

    if (clear)
    {
        m_stateCallbacks.clear();
    }
}

bool VulkanContextState::prepareDescriptorSets(VulkanCommandBuffer* cmdBuffer, std::vector<VkDescriptorSet>& sets, std::vector<u32>& offsets)
{
    if (m_currentPipeline.first->getDescriptorSetLayouts()._setLayouts.empty())
    {
        return false;
    }

#if VULKAN_DEBUG
    if (!checkBindingsAndPipelineLayout())
    {
        ASSERT(false, "not same");
        return false;
    }
#endif

    for (u32 setId = 0; setId < k_maxDescriptorSetIndex; ++setId)
    {
        BindingState& bindSet = m_currentBindingSlots[setId];
        if (bindSet.isDirty())
        {
            SetInfo info;
            bindSet.apply(cmdBuffer, 0, info);

            VulkanDescriptorSetPool* pool = nullptr;
            VkDescriptorSet set = m_descriptorSetManager->acquireDescriptorSet(VulkanDescriptorSetLayoutDescription(m_currentPipeline.first->getPipelineLayoutDescription()._bindingsSet[setId]), info,
                m_currentPipeline.first->getDescriptorSetLayouts()._setLayouts[setId], pool);
            if (set == VK_NULL_HANDLE)
            {
                ASSERT(false, "fail to allocate descriptor set");
                return false;
            }

            ASSERT(pool, "nullptr");
            pool->captureInsideCommandBuffer(cmdBuffer, 0);

            if (m_currentDesctiptorsSets[setId] != set)
            {
                m_currentDesctiptorsSets[setId] = set;
                if (VulkanDeviceCaps::getInstance()->useDynamicUniforms)
                {
                    auto updatedSet = m_updatedDescriptorsSets.insert(set);
                    if (updatedSet.second)
                    {
                        VulkanContextState::updateDescriptorSet(cmdBuffer, set, bindSet);
                    }
                }
                else
                {
                    VulkanContextState::updateDescriptorSet(cmdBuffer, set, bindSet);
                }
            }

            sets.push_back(set);

            bindSet.extractBufferOffsets(offsets);
            bindSet.reset();
        }
    }

    if (sets.empty())
    {
        return false;
    }

    return true;
}

void VulkanContextState::updateDescriptorSet(VulkanCommandBuffer* cmdBuffer, VkDescriptorSet set, const BindingState& state)
{
    std::vector<VkWriteDescriptorSet> writeDescriptorSets;
    writeDescriptorSets.reserve(k_maxDescriptorSetIndex);

    for (const auto& binding : state._set)
    {
        const BindingInfo& bindingInfo = std::get<0>(binding);
        if (bindingInfo._type == BindingType::BindingType_Unknown || !state.isActiveBinding(bindingInfo._binding))
        {
            continue;
        }

        VkWriteDescriptorSet writeDescriptorSet = {};
        writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        writeDescriptorSet.pNext = nullptr; //VkWriteDescriptorSetInlineUniformBlockEXT
        writeDescriptorSet.dstSet = set;
        writeDescriptorSet.dstBinding = bindingInfo._binding;
        writeDescriptorSet.dstArrayElement = bindingInfo._arrayIndex;
        writeDescriptorSet.descriptorCount = 1;
        writeDescriptorSet.pTexelBufferView = nullptr;

        switch (bindingInfo._type)
        {
        case BindingType::BindingType_Uniform:
            ASSERT(bindingInfo._info._bufferInfo.range <= VulkanDeviceCaps::getInstance()->getPhysicalDeviceLimits().maxUniformBufferRange, "out of max range");
            writeDescriptorSet.pBufferInfo = &bindingInfo._info._bufferInfo;
            writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;

            break;

        case BindingType::BindingType_DynamicUniform:
            ASSERT(bindingInfo._info._bufferInfo.range <= VulkanDeviceCaps::getInstance()->getPhysicalDeviceLimits().maxUniformBufferRange, "out of max range");
            //TODO Will update range if it gets errors
            /*VkDescriptorBufferInfo bufferInfo(bindingInfo._info._bufferInfo);
            bufferInfo.offset = 0;
            bufferInfo.range = state._offsets[index] + bufferInfo.range;*/
            writeDescriptorSet.pBufferInfo = &bindingInfo._info._bufferInfo;
            writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;

            break;

        case BindingType::BindingType_Sampler:
            ASSERT(bindingInfo._info._imageInfo.imageView == VK_NULL_HANDLE, "image present");
            ASSERT(bindingInfo._info._imageInfo.sampler != VK_NULL_HANDLE, "sampler");
            writeDescriptorSet.pImageInfo = &bindingInfo._info._imageInfo;
            writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;

            break;

        case BindingType::BindingType_Texture:
            ASSERT(bindingInfo._info._imageInfo.sampler == VK_NULL_HANDLE, "sampler present");
            ASSERT(bindingInfo._info._imageInfo.imageView != VK_NULL_HANDLE, "image");
            writeDescriptorSet.pImageInfo = &bindingInfo._info._imageInfo;
            writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;

            break;

        case BindingType::BindingType_SamplerAndTexture:
            ASSERT(bindingInfo._info._imageInfo.imageView != VK_NULL_HANDLE, "image");
            //ASSERT(binding._imageBinding._imageInfo.sampler != VK_NULL_HANDLE, "sampler");
            writeDescriptorSet.pImageInfo = &bindingInfo._info._imageInfo;
            writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;

            break;

        default:
            ASSERT(false, "impl");
        }

        writeDescriptorSets.push_back(writeDescriptorSet);
    }

    if (!writeDescriptorSets.empty())
    {
        VulkanWrapper::UpdateDescriptorSets(m_device, static_cast<u32>(writeDescriptorSets.size()), writeDescriptorSets.data(), 0, nullptr);
    }
}

void VulkanContextState::bindTexture(const VulkanImage* image, const VulkanSampler* sampler, u32 arrayIndex, const Shader::Image& info)
{
    ASSERT(info._set < VulkanDeviceCaps::getInstance()->maxDescriptorSetIndex, "Set index. Out of range");
    ASSERT(info._binding < VulkanDeviceCaps::getInstance()->maxDescriptorBindingIndex, "Binding index. Out of range");

    BindingState& bindingSlot = m_currentBindingSlots[info._set];
    bindingSlot.bind(BindingType::BindingType_SamplerAndTexture, info._binding, -1, arrayIndex, image, sampler);
}

void VulkanContextState::bindTexture(const VulkanImage* image, u32 arrayIndex, const Shader::Image& info)
{
    ASSERT(info._set < VulkanDeviceCaps::getInstance()->maxDescriptorSetIndex, "Set index. Out of range");
    ASSERT(info._binding < VulkanDeviceCaps::getInstance()->maxDescriptorBindingIndex, "Binding index. Out of range");

    BindingState& bindingSlot = m_currentBindingSlots[info._set];
    bindingSlot.bind(BindingType::BindingType_Texture, info._binding, -1, arrayIndex, image, nullptr);
}

void VulkanContextState::bindSampler(const VulkanSampler* sampler, const Shader::Sampler& info)
{
    ASSERT(info._set < VulkanDeviceCaps::getInstance()->maxDescriptorSetIndex, "Set index. Out of range");
    ASSERT(info._binding < VulkanDeviceCaps::getInstance()->maxDescriptorBindingIndex, "Binding index. Out of range");

    BindingState& bindingSlot = m_currentBindingSlots[info._set];
    bindingSlot.bind(BindingType::BindingType_Sampler, info._binding, -1, 0, nullptr, sampler);
}

void VulkanContextState::updateConstantBuffer(const Shader::UniformBuffer& info, u32 offset, u32 size, const void* data)
{
    ASSERT(info._set < VulkanDeviceCaps::getInstance()->maxDescriptorSetIndex, "Set index. Out of range");
    ASSERT(info._binding < VulkanDeviceCaps::getInstance()->maxDescriptorBindingIndex, "Binding index. Out of range");

    ASSERT(size <= info._size * info._array, "over size");
    VulkanUniformBuffer* uniformBuffer = m_unifromBufferManager->acquireUnformBuffer(info._size);
    [[maybe_unused]] bool updated = uniformBuffer->update(offset, size, data);

    BindingType type = VulkanDeviceCaps::getInstance()->useDynamicUniforms ? BindingType::BindingType_DynamicUniform : BindingType::BindingType_Uniform;

    BindingState& bindingSlot = m_currentBindingSlots[info._set];
    bindingSlot.bind(type, info._binding, 0, uniformBuffer->getBuffer(), uniformBuffer->getOffset(), uniformBuffer->getSize());
}

void VulkanContextState::invalidateDescriptorSetsState()
{
    for (auto& slot : m_currentBindingSlots)
    {
        slot.reset();
    }
}

void VulkanContextState::updateDescriptorStates()
{
    VulkanContextState::invalidateDescriptorSetsState();
    m_currentDesctiptorsSets.fill(VK_NULL_HANDLE);

    m_descriptorSetManager->updateDescriptorPools();
    m_updatedDescriptorsSets.clear();
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

VkDescriptorImageInfo VulkanContextState::makeVkDescriptorImageInfo(const VulkanImage* image, const VulkanSampler* sampler, VkImageLayout layout, s32 layer)
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
            descriptorImageInfo.imageView = vkImage->getImageView(layer, VK_IMAGE_ASPECT_DEPTH_BIT);
        }
        else
        {
            descriptorImageInfo.imageView = vkImage->getImageView(layer, VK_IMAGE_ASPECT_COLOR_BIT);
        }
    }

    return descriptorImageInfo;
}

bool VulkanContextState::checkBindingsAndPipelineLayout()
{
    //TODO compare m_currentPipeline.first->getDescriptorSetLayouts()._setLayouts and m_currentBindingSlots
    //TODO check image usage flags
    return true;
}

void VulkanContextState::BindingState::reset()
{
    _dirtyFlag = 0;
    _activeBindingsFlags = 0;

    memset(&_set, 0, sizeof(_set));

    _usedResources.clear();
    _offsets.clear();
}

void VulkanContextState::BindingState::extractBufferOffsets(std::vector<u32>& offsets)
{
    offsets = std::move(_offsets);
}

bool VulkanContextState::BindingState::isActiveBinding(u32 binding) const
{
    return (_activeBindingsFlags >> binding) & 1;
}

bool VulkanContextState::BindingState::isDirty() const
{
    return _dirtyFlag;
}

void VulkanContextState::BindingState::bind(BindingType type, u32 binding, u32 layer, u32 arrayIndex, const VulkanImage* image, const VulkanSampler* sampler)
{
    BindingInfo& bindingInfo = std::get<0>(_set[binding]);
    bindingInfo._binding = binding;
    bindingInfo._arrayIndex = arrayIndex;
    bindingInfo._type = type;
    bindingInfo._info._imageInfo = VulkanContextState::makeVkDescriptorImageInfo(image, sampler, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, layer);

    BindingData& bindingData = std::get<1>(_set[binding]);
    bindingData._dataBinding = BindingData::BindingImageData { image, sampler };

    _activeBindingsFlags |= 1 << binding;
    _dirtyFlag = true;

    if (image)
    {
        _usedResources.insert(image);
    }

    if (sampler)
    {
        _usedResources.insert(sampler);
    }
}

void VulkanContextState::BindingState::bind(BindingType type, u32 binding, u32 arrayIndex, const VulkanBuffer* buffer, u64 offset, u64 range)
{
    BindingInfo& bindingInfo = std::get<0>(_set[binding]);
    bindingInfo._binding = binding;
    bindingInfo._arrayIndex = arrayIndex;
    bindingInfo._type = type;
    bindingInfo._info._bufferInfo = VulkanContextState::makeVkDescriptorBufferInfo(buffer, offset, range);

    BindingData& bindingData = std::get<1>(_set[binding]);
    bindingData._dataBinding = BindingData::BindingBufferData{ buffer, offset, range };

    _activeBindingsFlags |= 1 << binding;
    _dirtyFlag = true;

    if (type == BindingType::BindingType_DynamicUniform)
    {
        bindingInfo._info._bufferInfo.offset = 0;
        _offsets.push_back(static_cast<u32>(offset));
    }

    _usedResources.insert(buffer);
}

void VulkanContextState::BindingState::apply(VulkanCommandBuffer* cmdBuffer, u64 frame, SetInfo& setInfo)
{
    setInfo._bindingsInfo.clear();
    setInfo._bindingsInfo.reserve(k_maxDescriptorBindingIndex);

    u32 hash = 0;
    for (u32 i = 0; i < k_maxDescriptorBindingIndex; ++i)
    {
        if (!BindingState::isActiveBinding(i))
        {
            continue;
        }

        setInfo._bindingsInfo.push_back(std::get<0>(_set[i]));

        hash = crc32c::Extend(hash, reinterpret_cast<u8*>(&std::get<0>(_set[i])), sizeof(BindingInfo));
    }
    setInfo._key = hash;

    for (auto& res : _usedResources)
    {
        res->captureInsideCommandBuffer(cmdBuffer, frame);
    }
}

} //namespace vk
} //namespace renderer
} //namespace v3d
#endif //VULKAN_RENDER
