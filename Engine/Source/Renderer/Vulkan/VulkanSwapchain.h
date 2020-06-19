#pragma once

#include "Common.h"
#include "Platform/Window.h"

#ifdef VULKAN_RENDER
#include "VulkanWrapper.h"
#include "VulkanDeviceCaps.h"

namespace v3d
{
namespace renderer
{
namespace vk
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////
    
    class VulkanResource;
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
            const platform::Window* _window               = nullptr;
            core::Dimension2D       _size;
            u32                     _countSwapchainImages = 0;
            bool                    _vsync                = false;
        };

        VulkanSwapchain(const struct DeviceInfo* info);
        ~VulkanSwapchain();

        bool create(const SwapchainConfig& config, VkSwapchainKHR oldSwapchain = VK_NULL_HANDLE);
        void destroy();

        void present(VkQueue queue, const std::vector<VkSemaphore>& waitSemaphores);
        u32  acquireImage();

        bool recteate(const SwapchainConfig& config);

        void attachResource(VulkanResource* resource, const std::function<bool(VulkanResource*)>& recreator);

        VulkanImage* getBackbuffer() const;
        VulkanImage* getSwapchainImage(u32 index) const;
        u32 getSwapchainImageCount() const;

        static u32 currentSwapchainIndex();

        template<typename Type>
        static bool correctViewByOrientation(const VulkanSwapchain* swapchain, Type& width, Type& height)
        {
            if (swapchain->getTransformFlag() == VK_SURFACE_TRANSFORM_ROTATE_90_BIT_KHR || swapchain->getTransformFlag() == VK_SURFACE_TRANSFORM_ROTATE_270_BIT_KHR)
            {
                std::swap(width, height);
                return true;
            }
            return false;
        }

        VkSurfaceTransformFlagBitsKHR getTransformFlag() const;

    private:

        static VkSurfaceKHR createSurface(VkInstance vkInstance,  NativeInstance hInstance, NativeWindows hWnd);

        bool createSwapchain(const SwapchainConfig& config, VkSwapchainKHR oldSwapchain = VK_NULL_HANDLE);
        bool createSwapchainImages(const SwapchainConfig& config);

        const DeviceInfo* m_deviceInfo;

        VkSurfaceKHR m_surface;
        VkSurfaceCapabilitiesKHR m_surfaceCaps;
        VkSurfaceFormatKHR m_surfaceFormat;

        VkSwapchainKHR m_swapchain;
        std::vector<VulkanImage*> m_swapBuffers;

        u32 m_currentSemaphoreIndex;
        static u32 s_currentImageIndex;
        std::vector<VkSemaphore> m_acquireSemaphore;
        std::vector<VkFence> m_acquireFence;

        void recreateAttachedResources();
        std::vector<std::tuple<VulkanResource*, const std::function<bool(VulkanResource*)>>> m_swapchainResources;

        SwapchainConfig m_config;

        bool m_ready;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace vk
} //namespace renderer
} //namespace v3d
#endif //VULKAN_RENDER
