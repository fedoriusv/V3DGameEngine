#pragma once

#include "Common.h"
#include "Platform/Window.h"

#ifdef VULKAN_RENDER
#include "VulkanWrapper.h"
#include "VulkanDeviceCaps.h"

/**
* SWAPCHAIN_ON_ADVANCE feature
*/
#define SWAPCHAIN_ON_ADVANCE 1

namespace v3d
{
namespace renderer
{
namespace vk
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////
    
    class VulkanResource;
    class VulkanImage;
    class VulkanSemaphore;
    class VulkanSemaphoreManager;

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * VulkanSwapchain final class. Vulkan Render side
    */
    class VulkanSwapchain final
    {
    public:

        struct SwapchainConfig
        {
            platform::Window*       _window               = nullptr;
            core::Dimension2D       _size;
            u32                     _countSwapchainImages = 0;
            bool                    _vsync                = false;
            bool                    _forceSRGB            = false;
        };

        VulkanSwapchain(const struct DeviceInfo* info, VulkanSemaphoreManager* const semaphoreManager);
        ~VulkanSwapchain();

        bool create(const SwapchainConfig& config, VkSwapchainKHR oldSwapchain = VK_NULL_HANDLE);
        void destroy();

        void present(VkQueue queue, const std::vector<VulkanSemaphore*>& waitSemaphores);
        u32  acquireImage();

        bool recteate(const SwapchainConfig& config);

        void attachResource(VulkanResource* resource, const std::function<bool(VulkanResource*)>& recreator);

        VulkanImage* getBackbuffer() const;
        VulkanImage* getSwapchainImage(u32 index) const;
        u32 getSwapchainImageCount() const;
        VulkanSemaphore* getAcquireSemaphore(u32 index) const;

        static u32 currentSwapchainIndex();
        
        VkSurfaceTransformFlagBitsKHR getTransformFlag() const;

    private:

        static VkSurfaceKHR createSurface(VkInstance vkInstance,  NativeInstance hInstance, NativeWindows hWnd, [[maybe_unused]] const core::Dimension2D& size);

        bool createSwapchain(const SwapchainConfig& config, VkSwapchainKHR oldSwapchain = VK_NULL_HANDLE);
        bool createSwapchainImages(const SwapchainConfig& config);

        const DeviceInfo* m_deviceInfo;

        VkSurfaceKHR m_surface;
        VkSurfaceCapabilitiesKHR m_surfaceCaps;
        VkSurfaceFormatKHR m_surfaceFormat;

        VkSwapchainKHR m_swapchain;
        std::vector<VulkanImage*> m_swapBuffers;

        VulkanSemaphoreManager* const m_semaphoreManager;

        static u32 s_currentImageIndex;

        std::tuple<u32, VulkanSemaphore*> m_presentInfo;
        u32 m_currentSemaphoreIndex;
        std::vector<VulkanSemaphore*> m_acquireSemaphore;

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