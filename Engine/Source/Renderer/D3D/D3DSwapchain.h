#pragma once

#include "Common.h"
#include "Platform/Window.h"

#ifdef D3D_RENDER
namespace v3d
{
namespace renderer
{
namespace d3d
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * D3DSwapchain final class. DirectX Render side
    */
    class D3DSwapchain final
    {
    public:

        struct SwapchainConfig
        {
            core::Dimension2D   _size;
            u32                 _countSwapchainImages = 0;
            bool                _vsync                = false;
        };

        D3DSwapchain(const struct DeviceInfo* info);
        ~D3DSwapchain();

        bool create(const SwapchainConfig& config);
        void destroy();

        void present();
        u32  acquireImage();

        bool recteate(const SwapchainConfig& config);

    private:

        friend class D3DGraphicContext;

    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace vk
} //namespace renderer
} //namespace v3d
#endif //VULKAN_RENDER
