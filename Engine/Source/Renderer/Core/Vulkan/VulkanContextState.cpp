#include "VulkanContextState.h"

#include "Utils/Logger.h"
#include "crc32c/crc32c.h"

#ifdef VULKAN_RENDER
#include "VulkanDeviceCaps.h"
#include "VulkanRenderpass.h"
#include "VulkanFramebuffer.h"
#include "VulkanGraphicPipeline.h"
#include "VulkanImage.h"
#include "VulkanBuffer.h"
#include "VulkanSampler.h"
#include "VulkanUnifromBuffer.h"
#include "VulkanSwapchain.h"
#include "VulkanQueryPool.h"

#include "Renderer/Core/RenderFrameProfiler.h"

namespace v3d
{
namespace renderer
{
namespace vk
{
#if FRAME_PROFILER_ENABLE
extern RenderFrameProfiler* g_CPUProfiler;
#endif //FRAME_PROFILER_ENABLE

VulkanContextState::VulkanContextState(VkDevice device, VulkanDescriptorSetManager* descriptorSetManager, VulkanUniformBufferManager* unifromBufferManager, VulkanQueryPoolManager* queryPoolManager) noexcept
    : m_device(device)

    , m_descriptorSetManager(descriptorSetManager)
    , m_unifromBufferManager(unifromBufferManager)
    , m_queryPoolManager(queryPoolManager)
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

bool VulkanContextState::setCurrentRenderPass(VulkanRenderPass* pass)
{
    bool changed = !VulkanContextState::isCurrentRenderPass(pass);
    m_currentRenderpass.first = pass;
    m_currentRenderpass.second = changed;

    return changed;
}

bool VulkanContextState::setCurrentFramebuffer(VulkanFramebuffer* framebuffer)
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

bool VulkanContextState::setCurrentPipeline(Pipeline* pipeline)
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

VulkanFramebuffer* VulkanContextState::getCurrentFramebuffer() const
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

        case BindingType::BindingType_StorageImage:
            ASSERT(bindingInfo._info._imageInfo.sampler == VK_NULL_HANDLE, "sampler present");
            ASSERT(bindingInfo._info._imageInfo.imageView != VK_NULL_HANDLE, "image");
            writeDescriptorSet.pImageInfo = &bindingInfo._info._imageInfo;
            writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
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

void VulkanContextState::bindTexture(const VulkanImage* image, const VulkanSampler* sampler, u32 arrayIndex, const Shader::Image& info, const Image::Subresource& subresource)
{
    ASSERT(info._set < VulkanDeviceCaps::getInstance()->maxDescriptorSetIndex, "Set index. Out of range");
    ASSERT(info._binding < VulkanDeviceCaps::getInstance()->maxDescriptorBindingIndex, "Binding index. Out of range");

    BindingState& bindingSlot = m_currentBindingSlots[info._set];
    bindingSlot.bind(BindingType::BindingType_SamplerAndTexture, info._binding, arrayIndex, image, subresource, sampler);
}

void VulkanContextState::bindTexture(const VulkanImage* image, u32 arrayIndex, const Shader::Image& info, const Image::Subresource& subresource)
{
    ASSERT(info._set < VulkanDeviceCaps::getInstance()->maxDescriptorSetIndex, "Set index. Out of range");
    ASSERT(info._binding < VulkanDeviceCaps::getInstance()->maxDescriptorBindingIndex, "Binding index. Out of range");

    BindingState& bindingSlot = m_currentBindingSlots[info._set];
    bindingSlot.bind(BindingType::BindingType_Texture, info._binding, arrayIndex, image, subresource, nullptr);
}

void VulkanContextState::bindSampler(const VulkanSampler* sampler, const Shader::Sampler& info)
{
    ASSERT(info._set < VulkanDeviceCaps::getInstance()->maxDescriptorSetIndex, "Set index. Out of range");
    ASSERT(info._binding < VulkanDeviceCaps::getInstance()->maxDescriptorBindingIndex, "Binding index. Out of range");

    BindingState& bindingSlot = m_currentBindingSlots[info._set];
    bindingSlot.bind(BindingType::BindingType_Sampler, info._binding, 0, nullptr, {}, sampler);
}

void VulkanContextState::bindStorageImage(const VulkanImage* image, u32 arrayIndex, const Shader::StorageImage& info, const Image::Subresource& subresource)
{
    ASSERT(info._set < VulkanDeviceCaps::getInstance()->maxDescriptorSetIndex, "Set index. Out of range");
    ASSERT(info._binding < VulkanDeviceCaps::getInstance()->maxDescriptorBindingIndex, "Binding index. Out of range");

    BindingState& bindingSlot = m_currentBindingSlots[info._set];
    bindingSlot.bind(BindingType::BindingType_StorageImage, info._binding, arrayIndex, image, subresource, nullptr);
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

const VulkanRenderQueryState* VulkanContextState::bindQuery(const VulkanQuery* query, u32 index, const std::string& tag)
{
    ASSERT(index < query->getSize(), "range out");
    auto inserted = m_currentRenderQueryState.emplace(query, nullptr);
    if (inserted.second)
    {
        VulkanRenderQueryState* renderQuery = m_queryPoolManager->acquireRenderQuery(query->getType(), query->getSize());
        ASSERT(renderQuery, "nullptr");
        renderQuery->_query = query;

        inserted.first->second = renderQuery;
    }

    inserted.first->second->_recorded[index] = true;
    if (!tag.empty())
    {
        inserted.first->second->_tags[index] = tag;
    }

    return inserted.first->second;
}

void VulkanContextState::invalidateRenderQueriesState()
{
    m_currentRenderQueryState.clear();
}

void VulkanContextState::prepareRenderQueries(const std::function<VulkanCommandBuffer* ()>& cmdBufferGetter)
{
    if (m_currentRenderQueryState.empty())
    {
        return;
    }

    if (VulkanDeviceCaps::getInstance()->hostQueryReset)
    {
        std::for_each(m_currentRenderQueryState.begin(), m_currentRenderQueryState.end(), [this](auto& renderQueryIter) -> void
            {
                VulkanQueryPool* pool = renderQueryIter.second->_pool;
                VulkanWrapper::ResetQueryPool(m_device, pool->getHandle(), 0, pool->getCount());
            });
    }
    else
    {
        VulkanCommandBuffer* cmdBuffer = cmdBufferGetter();
        ASSERT(cmdBuffer, "nullptr");

        std::for_each(m_currentRenderQueryState.begin(), m_currentRenderQueryState.end(), [cmdBuffer](auto& renderQueryIter) -> void
            {
                cmdBuffer->cmdResetQueryPool(renderQueryIter.second->_pool);
            });
    }
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

VkDescriptorImageInfo VulkanContextState::makeVkDescriptorImageInfo(const VulkanImage* image, const VulkanSampler* sampler, VkImageLayout layout, const Image::Subresource& subresource)
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
    _offsets.clear();
}

void VulkanContextState::BindingState::extractBufferOffsets(std::vector<u32>& offsets)
{
    offsets = _offsets;
}

bool VulkanContextState::BindingState::isActiveBinding(u32 binding) const
{
    return (_activeBindingsFlags >> binding) & 1;
}

bool VulkanContextState::BindingState::isDirty() const
{
    return _dirtyFlag;
}

void VulkanContextState::BindingState::bind(BindingType type, u32 binding, u32 arrayIndex, const VulkanImage* image, const Image::Subresource& subresource, const VulkanSampler* sampler)
{
    BindingInfo& bindingInfo = std::get<0>(_set[binding]);
    bindingInfo._binding = binding;
    bindingInfo._arrayIndex = arrayIndex;
    bindingInfo._type = type;
    bindingInfo._info._imageInfo = VulkanContextState::makeVkDescriptorImageInfo(image, sampler, (type == BindingType::BindingType_StorageImage) ? VK_IMAGE_LAYOUT_GENERAL : VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, subresource);

    BindingData& bindingData = std::get<1>(_set[binding]);
    bindingData._dataBinding.emplace<BindingData::BindingImageData>(image, sampler);

    _activeBindingsFlags |= 1 << binding;
    _dirtyFlag = true;
}

void VulkanContextState::BindingState::bind(BindingType type, u32 binding, u32 arrayIndex, const VulkanBuffer* buffer, u64 offset, u64 range)
{
    BindingInfo& bindingInfo = std::get<0>(_set[binding]);
    bindingInfo._binding = binding;
    bindingInfo._arrayIndex = arrayIndex;
    bindingInfo._type = type;
    bindingInfo._info._bufferInfo = VulkanContextState::makeVkDescriptorBufferInfo(buffer, offset, range);

    BindingData& bindingData = std::get<1>(_set[binding]);
    bindingData._dataBinding.emplace<BindingData::BindingBufferData>(buffer, offset, range);

    _activeBindingsFlags |= 1 << binding;
    _dirtyFlag = true;

    if (type == BindingType::BindingType_DynamicUniform)
    {
        bindingInfo._info._bufferInfo.offset = 0;
        _offsets.push_back(static_cast<u32>(offset));
    }
}

void VulkanContextState::BindingState::apply(VulkanCommandBuffer* cmdBuffer, u64 frame, SetInfo& setInfo)
{
    u32 hash = 0;
    for (u32 i = 0; i < k_maxDescriptorBindingIndex; ++i)
    {
        if (!BindingState::isActiveBinding(i))
        {
            continue;
        }

        setInfo._bindingsInfo[i] = std::get<0>(_set[i]);
        ++setInfo._size;

        hash = crc32c::Extend(hash, reinterpret_cast<u8*>(&std::get<0>(_set[i])), sizeof(BindingInfo));

        BindingData& bindingData = std::get<1>(_set[i]);
        switch (setInfo._bindingsInfo[i]._type)
        {
            case BindingType::BindingType_Uniform:
            case BindingType::BindingType_DynamicUniform:
            {
                const VulkanResource* buffer = std::get<2>(bindingData._dataBinding)._buffer;
                ASSERT(buffer, "nullptr");
                buffer->captureInsideCommandBuffer(cmdBuffer, frame);
            }
            break;

            case BindingType::BindingType_Sampler:
            case BindingType::BindingType_Texture:
            case BindingType::BindingType_SamplerAndTexture:
            case BindingType::BindingType_StorageImage:
            {
                const VulkanResource* image = std::get<1>(bindingData._dataBinding)._image;
                if (image)
                {
                    image->captureInsideCommandBuffer(cmdBuffer, frame);
                }

                const VulkanResource* sampler = std::get<1>(bindingData._dataBinding)._sampler;
                if (sampler)
                {
                    sampler->captureInsideCommandBuffer(cmdBuffer, frame);
                }
            }
            break;

            case BindingType::BindingType_Unknown:
            default:
                ASSERT(false, "wrong bind resource");
        }

    }
    setInfo._key = hash;
}

} //namespace vk
} //namespace renderer
} //namespace v3d
#endif //VULKAN_RENDER
