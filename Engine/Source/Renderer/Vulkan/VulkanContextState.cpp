#include "VulkanContextState.h"


#ifdef VULKAN_RENDER
#include "VulkanRenderpass.h"
#include "VulkanFramebuffer.h"
#include "VulkanPipeline.h"
#include "VulkanDescriptorSet.h"
#include "VulkanImage.h"

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
    u32 indexSet = 0;
    for (auto& set : m_currentSets)
    {
        VkWriteDescriptorSet writeDescriptorSet = {};
        writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        writeDescriptorSet.pNext = nullptr; //VkWriteDescriptorSetInlineUniformBlockEXT
        writeDescriptorSet.dstSet = set;
        writeDescriptorSet.dstBinding;
        writeDescriptorSet.dstArrayElement;
        writeDescriptorSet.descriptorCount;
        writeDescriptorSet.descriptorType;
        writeDescriptorSet.pImageInfo;
        writeDescriptorSet.pBufferInfo;
        writeDescriptorSet.pTexelBufferView;

        writeDescriptorSets.push_back(writeDescriptorSet);
    }
    

    VulkanWrapper::UpdateDescriptorSets(m_device, static_cast<u32>(writeDescriptorSets.size()), writeDescriptorSets.data(), 0, nullptr);
}

void VulkanContextState::bindTexture(VulkanImage* image, VulkanSampler* sampler, u32 arrayIndex, const resource::Shader::SampledImage& reflaction)
{
    /*BindingImageInfo* bindingInfo = new BindingImageInfo();
    bindingInfo->_set = reflaction._set;
    bindingInfo->_binding = reflaction._binding;
    bindingInfo->_type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    bindingInfo->_arrayIndex = arrayIndex;
    bindingInfo->_image = image;
    bindingInfo->_sampler = sampler;*/
}

VulkanContextState::BindingInfo::BindingInfo()
    : _set(0)
    , _binding(0)
    , _type(VK_DESCRIPTOR_TYPE_SAMPLER)
{
}

VulkanContextState::BindingInfo::~BindingInfo()
{
}

VulkanContextState::BindingImageInfo::BindingImageInfo()
    : _image(nullptr)
    , _sampler(nullptr)
{
    _type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
}

VulkanContextState::BindingImageInfo::~BindingImageInfo()
{
}

VulkanContextState::BindingBufferInfo::BindingBufferInfo()
    : _buffer(nullptr)
    , _offset(0)
    , _size(0)
{
}

VulkanContextState::BindingBufferInfo::~BindingBufferInfo()
{
}

} //namespace vk
} //namespace renderer
} //namespace v3d
#endif //VULKAN_RENDER
