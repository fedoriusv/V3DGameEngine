#pragma once

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

        VulkanSwapchain();
        ~VulkanSwapchain();

        bool create();
        void destroy();

        void present();
        void acquireImage();

    private:

    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace vk
} //namespace renderer
} //namespace v3d
