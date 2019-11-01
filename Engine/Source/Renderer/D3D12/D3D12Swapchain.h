#pragma once

#include "Common.h"
#include "Platform/Window.h"

#ifdef D3D_RENDER
#   include "D3D12Configuration.h"

namespace v3d
{
namespace renderer
{
namespace d3d12
{
    class D3DImage;

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * D3DSwapchain final class. DirectX Render side
    */
    class D3DSwapchain final
    {
    public:

        struct SwapchainConfig
        {
            NativeWindows       _window               = NULL;
            core::Dimension2D   _size;
            u32                 _countSwapchainImages = 0;
            bool                _vsync                = false;
            bool                _fullscreen           = false;
        };

        explicit D3DSwapchain(IDXGIFactory4* factory, ID3D12Device* device, ID3D12CommandQueue* cmdQueue) noexcept;
        ~D3DSwapchain();

        bool create(const SwapchainConfig& config);
        void destroy();

        void present();
        u32  acquireImage();

        bool resize(const SwapchainConfig& config);

        D3DImage* getSwapchainImage() const;

        bool vsync() const;

    private:

        IDXGIFactory4* m_factory;
        ID3D12Device* m_device;
        ID3D12CommandQueue* m_commandQueue;

        IDXGISwapChain3* m_swapChain;
        ID3D12DescriptorHeap* m_descriptorHeap;

        std::vector<D3DImage*> m_renderTargets;

        u32 m_frameIndex;
        u32 m_syncInterval;

        friend class D3DGraphicContext;

    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace d3d12
} //namespace renderer
} //namespace v3d
#endif //VULKAN_RENDER
