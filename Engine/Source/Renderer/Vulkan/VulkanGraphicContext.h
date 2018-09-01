#pragma once

#include "Context.h"
#include "VulkanDeviceCaps.h"

#ifdef VULKAN_RENDER
#include "VulkanWrapper.h"
namespace v3d
{
namespace renderer
{
namespace vk
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    class VulkanGraphicContext final : public Context
    {
    public:

        struct DeviceInfo
        {
            VkInstance  _instance;
            VkDevice    _device;
            u32         _queueIndex;
        };

        VulkanGraphicContext(const platform::Window* window);
        ~VulkanGraphicContext();

        void beginFrame() override;
        void endFrame() override;
        void presentFrame() override;

    private:

        DeviceInfo          m_deviceInfo;
        VulkanDeviceCaps    m_deviceCaps;

        const std::string s_vulkanApplicationName = "VulkanGraphicContext";

        bool initialize() override;
        void destroy() override;

        bool createInstance();
        bool createDevice();

    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace vk
} //namespace renderer
} //namespace v3d
#endif //VULKAN_RENDER
