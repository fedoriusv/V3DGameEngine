#pragma once

#include "Renderer/Swapchain.h"

#ifdef VULKAN_RENDER
#   include "VulkanWrapper.h"

namespace v3d
{
namespace platform
{
    class Window;
} //namespace platform
namespace renderer
{
namespace vk
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief SWAPCHAIN_ON_ADVANCE feature
    */
#   define SWAPCHAIN_ON_ADVANCE 0

    class VulkanDevice;
    class VulkanCmdList;
    class VulkanResource;
    class VulkanImage;
    class VulkanSemaphore;
    class VulkanSemaphoreManager;

    /**
    *  @brief VulkanSwapchain final class. Vulkan Render side
    */
    class VulkanSwapchain final : public Swapchain
    {
    public:

        void beginFrame() override;
        void endFrame()override;

        void presentFrame() override;

        VulkanImage* getCurrentSwapchainImage() const;
        VulkanImage* getSwapchainImage(u32 index) const;
        u32 getSwapchainImageCount() const;
        u32 currentSwapchainIndex();

        VulkanSemaphore* getAcquiredSemaphore(u32 index) const;
        VulkanSemaphore* getCurrentAcquiredSemaphore() const;

        VkSurfaceTransformFlagBitsKHR getTransformFlag() const;

    public:

        explicit VulkanSwapchain(VulkanDevice* device, VulkanSemaphoreManager* semaphoreManager) noexcept;
        ~VulkanSwapchain();

        bool create(platform::Window* window, const SwapchainParams& params, VkSwapchainKHR oldSwapchain = VK_NULL_HANDLE);
        bool recteate(platform::Window* window, const SwapchainParams& params);
        void destroy();

        void present(VkQueue queue, const std::vector<VulkanSemaphore*>& waitSemaphores);
        u32  acquireImage();

        void attachResource(VulkanResource* resource, const std::function<bool(VulkanResource*)>& recreator);
        void attachCmdList(VulkanCmdList* cmdlist);

    private:

        friend VulkanCmdList;

        static VkSurfaceKHR createSurface(VkInstance vkInstance, NativeInstance hInstance, NativeWindows hWnd, const math::Dimension2D& size);

        bool createSwapchain(const SwapchainParams& params, const VkSurfaceFormatKHR& surfaceFormat, VkSwapchainKHR oldSwapchain = VK_NULL_HANDLE);
        bool createSwapchainImages(const SwapchainParams& params, const VkSurfaceFormatKHR& surfaceFormat, TextureUsageFlags flags);

        VulkanDevice&                       m_device;
        platform::Window*                   m_window;
        SwapchainParams                     m_params;
        std::recursive_mutex                m_mutex;

        VkSurfaceKHR                        m_surface;
        VkSurfaceCapabilitiesKHR            m_surfaceCapabilities;
        VkSwapchainKHR                      m_swapchain;
        std::vector<VulkanImage*>           m_swapchainImages;

        u32                                 m_currentImageIndex;

        VulkanSemaphoreManager* const       m_semaphoreManager;
        std::tuple<u32, VulkanSemaphore*>   m_presentInfo;
        u32                                 m_currentSemaphoreIndex;
        std::vector<VulkanSemaphore*>       m_acquiredSemaphores;

        std::vector<VulkanCmdList*>         m_cmdLists;

        void recreateAttachedResources();
        std::vector<std::tuple<VulkanResource*, const std::function<bool(VulkanResource*)>>> m_swapchainResources;

        bool m_insideFrame;
        bool m_ready;
    };

    inline VulkanImage* VulkanSwapchain::getSwapchainImage(u32 index) const
    {
        ASSERT(index < m_swapchainImages.size(), "invalid index");
        return m_swapchainImages[index];
    }

    inline VulkanImage* VulkanSwapchain::getCurrentSwapchainImage() const
    {
        ASSERT(m_currentImageIndex >= 0, "invalid index");
        return m_swapchainImages[m_currentImageIndex];
    }

    inline VulkanSemaphore* VulkanSwapchain::getAcquiredSemaphore(u32 index) const
    {
        ASSERT(index < m_acquiredSemaphores.size(), "invalid index");
        return m_acquiredSemaphores[index];
    }

    inline VulkanSemaphore* VulkanSwapchain::getCurrentAcquiredSemaphore() const
    {
        ASSERT(m_currentSemaphoreIndex >= 0, "invalid index");
        return m_acquiredSemaphores[m_currentSemaphoreIndex];
    }

    inline u32 VulkanSwapchain::getSwapchainImageCount() const
    {
        return static_cast<u32>(m_swapchainImages.size());
    }

    inline u32 VulkanSwapchain::currentSwapchainIndex()
    {
        return m_currentImageIndex;
    }

    inline VkSurfaceTransformFlagBitsKHR VulkanSwapchain::getTransformFlag() const
    {
        return m_surfaceCapabilities.currentTransform;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace vk
} //namespace renderer
} //namespace v3d
#endif //VULKAN_RENDER
