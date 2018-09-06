#pragma once

#include "Common.h"
#include "Platform/Window.h"
#include "VulkanWrapper.h"

#ifdef VULKAN_RENDER
namespace v3d
{
namespace renderer
{
namespace vk
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    class VulkanSwapchain final
    {
    public:

        struct SwapchainConfig
        {
            core::Dimension2D   _size;
            bool                _vsync;
        };

        VulkanSwapchain(const struct DeviceInfo* info, VkSurfaceKHR surface);
        ~VulkanSwapchain();

        bool create(const SwapchainConfig& config);
        void destroy();

        void present();
        void acquireImage();

        bool recteateSwapchain(const SwapchainConfig& config);

    private:

        friend class VulkanGraphicContext;

        static VkSurfaceKHR createSurface(VkInstance vkInstance,  NativeInstance hInstance, NativeWindows hWnd);
        static void detroySurface(VkInstance vkInstance, VkSurfaceKHR surface);

        bool createSwapchain(const SwapchainConfig& config);
        bool createSwapchainImages();

        const DeviceInfo* m_deviceInfo;

        VkSurfaceKHR m_surface;
        VkSurfaceCapabilitiesKHR m_surfaceCaps;
        VkSurfaceFormatKHR m_surfaceFormat;

        VkSwapchainKHR m_swapchain;

    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace vk
} //namespace renderer
} //namespace v3d
#endif //VULKAN_RENDER
