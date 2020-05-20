#include "D3DSwapchain.h"

#include "Utils/Logger.h"

#ifdef D3D_RENDER
#include "D3DDebug.h"
#include "D3DImage.h"
#include "D3DDescriptorHeap.h"

namespace v3d
{
namespace renderer
{
namespace dx3d
{

D3DSwapchain::D3DSwapchain(IDXGIFactory4* factory, ID3D12Device* device, ID3D12CommandQueue* cmdQueue, D3DDescriptorHeapManager* heapMgr) noexcept
    : m_factory(factory)
    , m_device(device)
    , m_commandQueue(cmdQueue)
    , m_heapManager(heapMgr)

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
    Format format = Format::Format_R8G8B8A8_UNorm;
    m_syncInterval = config._vsync ? 1 : 0;

    {
        // Describe and create the swap chain.
        DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
        swapChainDesc.Flags = 0;
        swapChainDesc.BufferCount = config._countSwapchainImages;
        swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        swapChainDesc.Width = config._size.width;
        swapChainDesc.Height = config._size.height;
        swapChainDesc.Format = D3DImage::convertImageFormatToD3DFormat(format);
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
            LOG_ERROR("D3DSwapchain::create CreateSwapChainForHwnd is failed. Error %s", D3DDebug::stringError(result).c_str());
            return false;
        }

        swapChain->QueryInterface(&m_swapChain);
        swapChain->Release();
        ASSERT(m_swapChain, "nullptr");
    }

    {
        HRESULT result = m_factory->MakeWindowAssociation(config._window, DXGI_MWA_NO_ALT_ENTER);
        if (FAILED(result))
        {
            LOG_ERROR("D3DSwapchain::create MakeWindowAssociation is failed. Error %s", D3DDebug::stringError(result).c_str());
            D3DSwapchain::destroy();

            return false;
        }
    }

    {
        m_descriptorHeap = m_heapManager->allocateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_RTV, config._countSwapchainImages, D3D12_DESCRIPTOR_HEAP_FLAG_NONE);
        ASSERT(m_descriptorHeap, "nullptr");

        CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_descriptorHeap->getCPUHandle());
        m_renderTargets.reserve(config._countSwapchainImages);

        for (u32 index = 0; index < config._countSwapchainImages; index++)
        {
            ID3D12Resource* swapchainImage;
            HRESULT result = m_swapChain->GetBuffer(index, IID_PPV_ARGS(&swapchainImage));
            if (FAILED(result))
            {
                LOG_ERROR("D3DSwapchain::create GetBuffer is failed. Error %s", D3DDebug::stringError(result).c_str());
                D3DSwapchain::destroy();

                return false;
            }

            m_device->CreateRenderTargetView(swapchainImage, nullptr, rtvHandle);
            rtvHandle.Offset(1, m_descriptorHeap->getIncrement());

            CD3DX12_CPU_DESCRIPTOR_HANDLE imageHandle(m_descriptorHeap->getCPUHandle(), index, m_descriptorHeap->getIncrement());
            D3DImage* image = new D3DImage(m_device, format, config._size.width, config._size.height, 1,
                TextureUsage::TextureUsage_Attachment | TextureUsage::TextureUsage_Sampled | TextureUsage::TextureUsage_Read, "SwapchainImage_" + std::to_string(index));
            if (!image->create(swapchainImage, imageHandle))
            {
                LOG_ERROR("D3DSwapchain::create swapimage is failed");
                D3DSwapchain::destroy();

                return false;
            }

            m_renderTargets.push_back(image);
        }
    }

    m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();

    return true;
}

void D3DSwapchain::destroy()
{
    for (auto& image : m_renderTargets)
    {
        image->destroy();
        delete image;
    }
    m_renderTargets.clear();
    m_heapManager->deallocDescriptorHeap(m_descriptorHeap);
    m_descriptorHeap = nullptr;

#if D3D_DEBUG
    D3DDebug::getInstance()->report(D3D12_RLDO_SUMMARY | D3D12_RLDO_IGNORE_INTERNAL);
#endif
    SAFE_DELETE(m_swapChain);
}

void D3DSwapchain::present()
{
    static DXGI_PRESENT_PARAMETERS presentParameters = {};
    presentParameters.DirtyRectsCount = 0;
    presentParameters.pDirtyRects = nullptr;
    presentParameters.pScrollRect = nullptr;
    presentParameters.pScrollOffset = nullptr;

    HRESULT result = m_swapChain->Present1(m_syncInterval, 0, &presentParameters);
    if (FAILED(result))
    {
        LOG_ERROR("D3DSwapchain::present Present is failed %s", D3DDebug::stringError(result).c_str());
        ASSERT(false, "present is failed");
    }
}

u32 D3DSwapchain::acquireImage()
{
    m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();
    return m_frameIndex;
}

bool D3DSwapchain::resize(const SwapchainConfig& config)
{
    ASSERT(false, "not impl");
    return false;
}

D3DImage* D3DSwapchain::getSwapchainImage() const
{
    ASSERT(m_renderTargets[m_frameIndex], "nullptr");
    return m_renderTargets[m_frameIndex];
}

bool D3DSwapchain::vsync() const
{
    return m_syncInterval > 0;
}

} //namespace dx3d
} //namespace renderer
} //namespace v3d
#endif //D3D_RENDER
