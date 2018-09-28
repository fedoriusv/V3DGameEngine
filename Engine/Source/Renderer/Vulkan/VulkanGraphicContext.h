#pragma once

#include "Context.h"
#include "VulkanDeviceCaps.h"
#include "VulkanCommandBufferManager.h"

#include "Object/Texture.h"

#ifdef VULKAN_RENDER
#include "VulkanWrapper.h"

namespace v3d
{
namespace renderer
{
namespace vk
{
    class VulkanImage;

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    struct DeviceInfo
    {
        VkInstance          _instance;
        VkPhysicalDevice    _physicalDevice;
        u32                 _queueFamilyIndex;
        VkDevice            _device;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    class VulkanGraphicContext final : public Context
    {
    public:

        VulkanGraphicContext(const platform::Window* window);
        ~VulkanGraphicContext();

        //commands
        void beginFrame() override;
        void endFrame() override;
        void presentFrame() override;

        void clearBackbuffer(const core::Vector4D & color) override;

        //states
        void setViewport(const core::Rect32& viewport) override;

        Image* createImage(TextureTarget target, renderer::ImageFormat format, core::Dimension3D dimension, u32 mipLevels,
            s16 filter, TextureAnisotropic anisotropicLevel, TextureWrap wrap) const override;

        VulkanCommandBuffer* getCurrentBuffer(VulkanCommandBufferManager::CommandTargetType type) const;

        void transferImageLayout(VulkanImage* image, VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask, VkImageLayout layout) const;

    private:

        DeviceInfo          m_deviceInfo;
        VulkanDeviceCaps&   m_deviceCaps;

        const std::string s_vulkanApplicationName = "VulkanGraphicContext";

        bool initialize() override;
        void destroy() override;

        bool createInstance();
        bool createDevice();

        std::vector<VkQueue>    m_queueList;
        class VulkanSwapchain*  m_swapchain;

        class VulkanCommandBufferManager* m_drawCmdBufferManager;
        class VulkanCommandBuffer* m_currentDrawBuffer;

        const platform::Window* m_window;

        u64 m_frameCounter;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace vk
} //namespace renderer
} //namespace v3d
#endif //VULKAN_RENDER
