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
            NativeWindows       _window               = NULL;
        };

        D3DSwapchain(IDXGIFactory4* factory, ID3D12Device* device, ID3D12CommandQueue* commandQueue);
        ~D3DSwapchain();

        bool create(const SwapchainConfig& config);
        void destroy();

        void present();
        u32  acquireImage();

        bool recteate(const SwapchainConfig& config);

    private:

        u32 m_frameIndex;
        u32 m_syncInterval;

        IDXGIFactory4* m_factory;
        ID3D12Device* m_device;
        ID3D12CommandQueue* m_commandQueue;


        IDXGISwapChain3* m_swapChain;
        ID3D12DescriptorHeap* m_descriptorHeap;

        std::vector<ID3D12Resource*> m_renderTargets;

        friend class D3DGraphicContext;

    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace vk
} //namespace renderer
} //namespace v3d12
#endif //VULKAN_RENDER
