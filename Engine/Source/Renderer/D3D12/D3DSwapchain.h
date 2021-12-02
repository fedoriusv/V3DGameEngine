#pragma once

#include "Common.h"
#include "Platform/Window.h"

#ifdef D3D_RENDER
#include "D3DConfiguration.h"

namespace v3d
{
namespace renderer
{
namespace dx3d
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    class D3DImage;
    class D3DDescriptorHeap;
    class D3DDescriptorHeapManager;

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * D3DSwapchain class. DirectX Render side
    */
    class D3DSwapchain
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

        explicit D3DSwapchain(ID3D12Device* device, ID3D12CommandQueue* cmdQueue, D3DDescriptorHeapManager* heapManager) noexcept;
        virtual ~D3DSwapchain();

        virtual bool create(const SwapchainConfig& config) = 0;
        virtual void destroy() = 0;

        virtual void present() = 0;
        virtual u32 acquireImage() = 0;

        bool resize(const SwapchainConfig& config);

        D3DImage* getSwapchainImage() const;

        bool vsync() const;

    protected:

        ID3D12Device* const m_device;
        ID3D12CommandQueue* m_commandQueue;
        D3DDescriptorHeapManager* const m_heapManager;

        D3DDescriptorHeap* m_descriptorHeap;

        std::vector<D3DImage*> m_renderTargets;

        u32 m_frameIndex;
        u32 m_syncInterval;

        friend class D3DGraphicContext;

    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

#if defined(PLATFORM_WINDOWS)
    /**
    * D3DSwapchain Windows platform final class. DirectX Render side
    */
    class D3DSwapchainWindows final : public D3DSwapchain
    {
    public:

        explicit D3DSwapchainWindows(IDXGIFactory4* factory, ID3D12Device* device, ID3D12CommandQueue* cmdQueue, D3DDescriptorHeapManager* heapManager) noexcept;
        ~D3DSwapchainWindows();

        bool create(const SwapchainConfig& config) override;
        void destroy() override;

        void present() override;
        u32 acquireImage() override;

    private:

        IDXGIFactory4* const m_factory;
        IDXGISwapChain3* m_swapChain;
    };
#elif defined(PLATFORM_XBOX)
    /**
    * D3DSwapchain XBOX platform final class. DirectX Render side
    */
    class D3DSwapchainXBOX final : public D3DSwapchain
    {
    public:

        explicit D3DSwapchainXBOX(IDXGIAdapter* adapter, ID3D12Device* device, ID3D12CommandQueue* cmdQueue, D3DDescriptorHeapManager* heapManager) noexcept;
        ~D3DSwapchainXBOX();

        bool create(const SwapchainConfig& config) override;
        void destroy() override;

        void present() override;
        u32 acquireImage() override;

    private:

        IDXGIAdapter* m_adapter;
        D3D12XBOX_FRAME_PIPELINE_TOKEN m_frameToken;

        DXGI_COLOR_SPACE_TYPE m_colorSpace;
    };
#endif //PLATFORM

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace dx3d
} //namespace renderer
} //namespace v3d
#endif //VULKAN_RENDER
