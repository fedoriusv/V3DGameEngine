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
    class VulkanImage;

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * VulkanSwapchain final class. Vulkan Render side
    */
    class VulkanSwapchain final
    {
    public:

        struct SwapchainConfig
        {
            core::Dimension2D   _size;
            u32                 _countSwapchainImages = 0;
            bool                _vsync                = false;
        };

        VulkanSwapchain(const struct DeviceInfo* info, VkSurfaceKHR surface);
        ~VulkanSwapchain();

        bool create(const SwapchainConfig& config);
        void destroy();

        void present(VkQueue queue, const std::vector<VkSemaphore>& waitSemaphores);
        u32  acquireImage();

        bool recteateSwapchain(const SwapchainConfig& config);

        VulkanImage* getBackbuffer() const;
        VulkanImage* getSwapchainImage(u32 index) const;
        u32 getSwapchainImageCount() const;

    private:

        friend class VulkanGraphicContext;

        static VkSurfaceKHR createSurface(VkInstance vkInstance,  NativeInstance hInstance, NativeWindows hWnd);
        static void detroySurface(VkInstance vkInstance, VkSurfaceKHR surface);

        bool createSwapchain(const SwapchainConfig& config);
        bool createSwapchainImages(const SwapchainConfig& config);

        const DeviceInfo* m_deviceInfo;

        VkSurfaceKHR m_surface;
        VkSurfaceCapabilitiesKHR m_surfaceCaps;
        VkSurfaceFormatKHR m_surfaceFormat;

        VkSwapchainKHR m_swapchain;
        std::vector<VulkanImage*> m_swapBuffers;

        u32 m_currentImageIndex;
        u32 m_currentSemaphoreIndex;
        std::vector<VkSemaphore> m_acquireSemaphore;
        std::vector<VkFence> m_acquireFence;

    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace vk
} //namespace renderer
} //namespace v3d
#endif //VULKAN_RENDER
