#include "Common.h"

#ifdef VULKAN_RENDER
#include "VulkanWrapper.h"
#include "VulkanCommandBufferManager.h"

namespace v3d
{
namespace renderer
{
namespace vk
{
    class VulkanContextState
    {
    public:

        explicit VulkanContextState(VkDevice device) noexcept;

        void invalidateCommandBuffer(CommandTargetType type);

        bool setDynamicState(VkDynamicState state, const std::function<void()>& callback); //check logic
        void invokeDynamicStates();

        void updateDescriptorSet();

        void bindTexture();
        void bindUnifrom();

    private:

        VkDevice m_device;

        std::map<VkDynamicState, std::function<void()>> m_stateCallbacks;
    };

} //namespace vk
} //namespace renderer
} //namespace v3d
#endif //VULKAN_RENDER
