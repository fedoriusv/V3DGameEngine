#include "VulkanContextState.h"

#include "crc32c/crc32c.h"

#ifdef VULKAN_RENDER
#include "VulkanRenderpass.h"
#include "VulkanFramebuffer.h"
#include "VulkanPipeline.h"
#include "VulkanDescriptorSet.h"
#include "VulkanImage.h"
#include "VulkanBuffer.h"
#include "VulkanUnifromBuffer.h"
#include "VulkanDeviceCaps.h"

namespace v3d
{
namespace renderer
{
namespace vk
{

VulkanContextState::VulkanContextState(VkDevice device, VulkanDescriptorSetManager* descriptorSetManager) noexcept
    : m_device(device)
    , m_descriptorSetManager(descriptorSetManager)
{
    m_currentRenderpass = { nullptr, false };
    m_currentFramebuffer = { nullptr, false };
    m_currentPipeline = { nullptr, false };

    m_currentVertexBuffers.second = false;
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
    return m_currentFramebuffer.first == framebuffer;
}

inline bool VulkanContextState::isCurrentPipeline(const VulkanGraphicPipeline * pipeline) const
{
    return m_currentPipeline.first == pipeline;
}

bool VulkanContextState::setCurrentRenderPass(VulkanRenderPass * pass)
{
    bool changed = VulkanContextState::isCurrentRenderPass(pass);
    m_currentRenderpass.first = pass;
    m_currentRenderpass.second = changed;

    return changed;
}

bool VulkanContextState::setCurrentFramebuffer(VulkanFramebuffer * framebuffer)
{
    bool changed = VulkanContextState::isCurrentFramebuffer(framebuffer);
    m_currentFramebuffer.first = framebuffer;
    m_currentFramebuffer.second = changed;

    return changed;
}

bool VulkanContextState::setCurrentPipeline(VulkanGraphicPipeline * pipeline)
{
    bool changed = VulkanContextState::isCurrentPipeline(pipeline);
    m_currentPipeline.first = pipeline;
    m_currentPipeline.second = changed;

    return changed;
}

bool VulkanContextState::setCurrentVertexBuffers(StreamBufferDescription & desc)
{
    bool changed = m_currentVertexBuffers.first == desc;
    if (changed)
    {
        m_currentVertexBuffers.first = std::move(desc);
    }
    m_currentVertexBuffers.second = changed;

    return changed;
}

VulkanRenderPass * VulkanContextState::getCurrentRenderpass() const
{
    return m_currentRenderpass.first;
}

VulkanFramebuffer * VulkanContextState::getCurrentFramebuffer() const
{
    return m_currentFramebuffer.first;
}

VulkanGraphicPipeline * VulkanContextState::getCurrentPipeline() const
{
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


bool VulkanContextState::acquireDescriptorSets(std::vector<VkDescriptorSet>& sets, std::vector<u32>& offsets)
{
    //TODO:
    m_currentBindingCache.clear();
    m_currentPool = m_descriptorSetManager->acquireDescriptorSets(m_currentPipeline.first->getDescriptorSetLayouts(), sets, offsets);
    if (!m_currentPool)
    {
        ASSERT(false, "error");
        return false;
    }
    m_currentSets = sets;
    return true;
}

void VulkanContextState::updateDescriptorSet()
{
    bool changed = false;

    std::vector<VkWriteDescriptorSet> writeDescriptorSets;
    writeDescriptorSets.reserve(m_updatedBindings.size());

    for (auto& bindingSet : m_updatedBindings)
    {
        VkWriteDescriptorSet writeDescriptorSet = {};
        writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        writeDescriptorSet.pNext = nullptr; //VkWriteDescriptorSetInlineUniformBlockEXT
        writeDescriptorSet.dstSet = m_currentSets[bindingSet._set];
        writeDescriptorSet.dstBinding = bindingSet._binding;
        writeDescriptorSet.dstArrayElement = bindingSet._arrayIndex;
        writeDescriptorSet.descriptorCount = 1;
        writeDescriptorSet.descriptorType = bindingSet._type;
        writeDescriptorSet.pTexelBufferView = nullptr;

        switch (bindingSet._type)
        {
        case VK_DESCRIPTOR_TYPE_SAMPLER:
        case VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
        case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
            writeDescriptorSet.pImageInfo = &bindingSet._imageInfo;
            break;

        case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
        case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC:
            writeDescriptorSet.pBufferInfo = &bindingSet._bufferInfo;;
            break;

        default:
            ASSERT(false, "impl");
        }

        writeDescriptorSets.push_back(writeDescriptorSet);
    }
    VulkanWrapper::UpdateDescriptorSets(m_device, static_cast<u32>(writeDescriptorSets.size()), writeDescriptorSets.data(), 0, nullptr);

    m_updatedBindings.clear();
}

void VulkanContextState::bindTexture(const VulkanImage* image, VulkanSampler* sampler, u32 arrayIndex, const resource::Shader::SampledImage& reflaction)
{
    BindingInfo bindingInfo;
    bindingInfo._type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    bindingInfo._set = reflaction._set;
    bindingInfo._binding = reflaction._binding;
    bindingInfo._arrayIndex = arrayIndex;
    bindingInfo._imageInfo.imageView = image->getImageView();
    bindingInfo._imageInfo.imageLayout = image->getLayout();
    bindingInfo._imageInfo.sampler = VK_NULL_HANDLE; //TODO:


    VulkanContextState::setBinding(bindingInfo);
}

void VulkanContextState::bindUnifrom(const VulkanUnifromBuffer * uniform, u32 arrayIndex, const resource::Shader::UniformBuffer & reflaction)
{
    BindingInfo bindingInfo;
    bindingInfo._type = VulkanDeviceCaps::getInstance()->useDynamicUniforms ? VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC : VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    bindingInfo._set = reflaction._set;
    bindingInfo._binding = reflaction._binding;
    bindingInfo._arrayIndex = arrayIndex;
    bindingInfo._bufferInfo.buffer = uniform->_buffer->getHandle();
    bindingInfo._bufferInfo.offset = uniform->_offset;
    bindingInfo._bufferInfo.range = uniform->_size;

    VulkanContextState::setBinding(bindingInfo);
}

void VulkanContextState::setBinding(BindingInfo & binding)
{
    u32 hash = crc32c::Crc32c(reinterpret_cast<u8*>(&binding), sizeof(BindingInfo));

    auto iter = m_currentBindingCache.find(hash);
    if (iter == m_currentBindingCache.cend())
    {
        m_updatedBindings.push_back(binding);
        m_currentBindingCache.insert({ hash, binding });
    }
}

VulkanContextState::BindingInfo::BindingInfo()
{
    memset(this, 0, sizeof(VulkanContextState::BindingInfo));
}

} //namespace vk
} //namespace renderer
} //namespace v3d
#endif //VULKAN_RENDER
