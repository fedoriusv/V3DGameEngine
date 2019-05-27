#include "D3D12Swapchain.h"

#include "Utils/Logger.h"

#ifdef D3D_RENDER
#   include <wrl.h>
#   include "D3D12Debug.h"

namespace v3d
{
namespace renderer
{
namespace d3d12
{

D3DSwapchain::D3DSwapchain(IDXGIFactory4* factory, ID3D12Device* device, ID3D12CommandQueue* commandQueue)
    : m_factory(factory)
    , m_device(device)
    , m_commandQueue(commandQueue)

    , m_swapChain(nullptr)
    , m_descriptorHeap(nullptr)

    , m_frameIndex(0U)
    , m_syncInterval(1)
{
    LOG_DEBUG("D3DSwapchain::D3DSwapchain constructor %llx", this);
}

D3DSwapchain::~D3DSwapchain()
{
    LOG_DEBUG("D3DSwapchain::~D3DSwapchain destructor %llx", this);

    ASSERT(!m_descriptorHeap, "not nullptr");
    ASSERT(!m_swapChain, "not nullptr");
}

bool D3DSwapchain::create(const SwapchainConfig& config)
{
    {
        // Describe and create the swap chain.
        DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
        swapChainDesc.Flags = 0;
        swapChainDesc.BufferCount = config._countSwapchainImages;
        swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        swapChainDesc.Width = config._size.width;
        swapChainDesc.Height = config._size.height;
        swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
        swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
        swapChainDesc.Scaling = DXGI_SCALING_NONE;
        swapChainDesc.SampleDesc.Count = 1;
        swapChainDesc.SampleDesc.Quality = 0;
        swapChainDesc.Stereo = FALSE;

        IDXGISwapChain1* swapChain;
        HRESULT result = m_factory->CreateSwapChainForHwnd(m_commandQueue, config._window, &swapChainDesc, nullptr, nullptr, &swapChain);
        if (FAILED(result))
        {
            LOG_ERROR("D3DSwapchain::create CreateSwapChainForHwnd is failed. Error %s", StringError(result).c_str());
            return false;
        }
        m_swapChain = static_cast<IDXGISwapChain3*>(swapChain);
    }

    {
        HRESULT result = m_factory->MakeWindowAssociation(config._window, DXGI_MWA_NO_ALT_ENTER);
        if (FAILED(result))
        {
            LOG_ERROR("D3DSwapchain::create MakeWindowAssociation is failed. Error %s", StringError(result).c_str());
            return false;
        }
    }

    {
        // Create descriptor heaps.
        // Describe and create a render target view (RTV) descriptor heap.
        D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
        rtvHeapDesc.NumDescriptors = config._countSwapchainImages;
        rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
        rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
        rtvHeapDesc.NodeMask = 0;
        HRESULT result = m_device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&m_descriptorHeap));
        if (FAILED(result))
        {
            LOG_ERROR("D3DSwapchain::create CreateDescriptorHeap is failed. Error %s", StringError(result).c_str());
            return false;
        }
    }

    {
        // Create frame resources.
        u32 rtvDescriptorSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
        CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_descriptorHeap->GetCPUDescriptorHandleForHeapStart());

        m_renderTargets.resize(config._countSwapchainImages, nullptr);

        // Create a RTV for each frame.
        for (u32 n = 0; n < config._countSwapchainImages; n++)
        {
            HRESULT result = m_swapChain->GetBuffer(n, IID_PPV_ARGS(&m_renderTargets[n]));
            if (FAILED(result))
            {
                LOG_ERROR("D3DSwapchain::create GetBuffer is failed. Error %s", StringError(result).c_str());
                return false;
            }

            m_device->CreateRenderTargetView(m_renderTargets[n], nullptr, rtvHandle);
            rtvHandle.Offset(1, rtvDescriptorSize);
        }
    }

    m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();

    return true;
}

void D3DSwapchain::destroy()
{
    if (m_descriptorHeap)
    {
        m_descriptorHeap->Release();
        m_descriptorHeap = nullptr;
    }

    if (m_swapChain)
    {
        m_swapChain->Release();
        m_swapChain = nullptr;
    }
}

void D3DSwapchain::present()
{
    HRESULT result = m_swapChain->Present(m_syncInterval, 0);
    if (FAILED(result))
    {
        LOG_ERROR("D3DSwapchain::present Present is failed");
        ASSERT(false, "present is failed");
    }
}

u32 D3DSwapchain::acquireImage()
{
    m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();
    return m_frameIndex;
}

bool D3DSwapchain::recteate(const SwapchainConfig& config)
{
    D3DSwapchain::destroy();

    if (!D3DSwapchain::create(config))
    {
        return false;
    }

    return true;
}

} //namespace d3d12
} //namespace renderer
} //namespace v3d
#endif //D3D_RENDER
