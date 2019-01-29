#include "VulkanContextState.h"

#ifdef VULKAN_RENDER
namespace v3d
{
namespace renderer
{
namespace vk
{

VulkanContextState::VulkanContextState(VkDevice device) noexcept
    : m_device(device)
{
}

void VulkanContextState::invalidateCommandBuffer(CommandTargetType type)
{
    if (type == CommandTargetType::CmdDrawBuffer)
    {
        m_stateCallbacks.clear();
    }
}

bool VulkanContextState::setDynamicState(VkDynamicState state, const std::function<void()>& callback)
{
    auto iter = m_stateCallbacks.emplace(state, callback);
    if (iter.second)
    {
        return true;
    }

    return false;
}

void VulkanContextState::invokeDynamicStates()
{
    for (auto& callback : m_stateCallbacks)
    {
        callback.second();
    }
}


void VulkanContextState::updateDescriptorSet()
{
 //VulkanWrapper::UpdateDescriptorSets()
}

} //namespace vk
} //namespace renderer
} //namespace v3d
#endif //VULKAN_RENDER
