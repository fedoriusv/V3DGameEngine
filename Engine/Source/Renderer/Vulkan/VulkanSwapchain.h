#pragma once
#include "Common.h"
#include "Platform/Window.h"

#ifdef VULKAN_RENDER
#include "VulkanWrapper.h"
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

        VulkanSwapchain(const struct DeviceInfo* info, VkSurfaceKHR surface);
        ~VulkanSwapchain();

        bool create();
        void destroy();

        void present();
        void acquireImage();

    private:

        friend class VulkanGraphicContext;

        static VkSurfaceKHR createSurface(VkInstance vkInstance,  NativeInstance hInstance, NativeWindows hWnd);
        static void detroySurface(VkInstance vkInstance, VkSurfaceKHR surface);

        bool createSwapchain();

        const DeviceInfo* m_deviceInfo;

        VkSurfaceKHR m_surface;
        VkSurfaceFormatKHR m_surfaceFormat;

        VkSwapchainKHR m_swapchain;

    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace vk
} //namespace renderer
} //namespace v3d
#endif //VULKAN_RENDER
