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

D3DSwapchain::D3DSwapchain(ID3D12Device* device, ID3D12CommandQueue* cmdQueue, D3DDescriptorHeapManager* heapMgr) noexcept
    : m_device(device)
    , m_commandQueue(cmdQueue)
    , m_heapManager(heapMgr)

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
}

bool D3DSwapchain::resize(const SwapchainConfig& config)
{
    ASSERT(false, "not impl");
    return false;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////

#if defined(PLATFORM_WINDOWS)
D3DSwapchainWindows::D3DSwapchainWindows(IDXGIFactory4* factory, ID3D12Device* device, ID3D12CommandQueue* cmdQueue, D3DDescriptorHeapManager* heapMgr) noexcept
    : D3DSwapchain(device, cmdQueue, heapMgr)
    , m_factory(factory)
    , m_swapChain(nullptr)
{
    LOG_DEBUG("D3DSwapchainWindows::D3DSwapchainWindows constructor %llx", this);
}

D3DSwapchainWindows::~D3DSwapchainWindows()
{
    LOG_DEBUG("D3DSwapchainWindows::~D3DSwapchainWindows destructor %llx", this);

    ASSERT(!m_swapChain, "not nullptr");
}

bool D3DSwapchainWindows::create(const SwapchainConfig& config)
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
            D3DSwapchainWindows::destroy();

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
            HRESULT result = m_swapChain->GetBuffer(index, DX_IID_PPV_ARGS(&swapchainImage));
            if (FAILED(result))
            {
                LOG_ERROR("D3DSwapchain::create GetBuffer is failed. Error %s", D3DDebug::stringError(result).c_str());
                D3DSwapchainWindows::destroy();

                return false;
            }

            m_device->CreateRenderTargetView(swapchainImage, nullptr, rtvHandle);
            rtvHandle.Offset(1, m_descriptorHeap->getIncrement());

            CD3DX12_CPU_DESCRIPTOR_HANDLE imageHandle(m_descriptorHeap->getCPUHandle(), index, m_descriptorHeap->getIncrement());
            D3DImage* image = new D3DImage(m_device, format, config._size.width, config._size.height, 1U, 1U,
                TextureUsage::TextureUsage_Attachment | TextureUsage::TextureUsage_Sampled | TextureUsage::TextureUsage_Read, "SwapchainImage_" + std::to_string(index));
            if (!image->create(swapchainImage, imageHandle))
            {
                LOG_ERROR("D3DSwapchain::create swapimage is failed");
                D3DSwapchainWindows::destroy();

                return false;
            }

            m_renderTargets.push_back(image);
        }
    }

    m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();

    return true;
}

void D3DSwapchainWindows::destroy()
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



void D3DSwapchainWindows::present()
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

u32 D3DSwapchainWindows::acquireImage()
{
    m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();
    return m_frameIndex;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////

#elif defined(PLATFORM_XBOX)
D3DSwapchainXBOX::D3DSwapchainXBOX(IDXGIAdapter* adapter, ID3D12Device* device, ID3D12CommandQueue* cmdQueue, D3DDescriptorHeapManager* heapMgr) noexcept
    : D3DSwapchain(device, cmdQueue, heapMgr)
    , m_adapter(adapter)
    , m_frameToken(D3D12XBOX_FRAME_PIPELINE_TOKEN_NULL)
    , m_colorSpace(DXGI_COLOR_SPACE_RGB_FULL_G22_NONE_P709)
{
    LOG_DEBUG("D3DSwapchainXBOX::D3DSwapchainXBOX constructor %llx", this);
}

D3DSwapchainXBOX::~D3DSwapchainXBOX()
{
    LOG_DEBUG("D3DSwapchainXBOX::~D3DSwapchainXBOX destructor %llx", this);
}

bool D3DSwapchainXBOX::create(const SwapchainConfig& config)
{
    Format format = Format::Format_R8G8B8A8_UNorm;
    m_syncInterval = 1; //always vsync

    {
        m_descriptorHeap = m_heapManager->allocateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_RTV, config._countSwapchainImages, D3D12_DESCRIPTOR_HEAP_FLAG_NONE);
        ASSERT(m_descriptorHeap, "nullptr");

        CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_descriptorHeap->getCPUHandle());
        m_renderTargets.reserve(config._countSwapchainImages);


        CD3DX12_HEAP_PROPERTIES swapChainHeapProperties(D3D12_HEAP_TYPE_DEFAULT);

        D3D12_RESOURCE_DESC swapChainBufferDesc = CD3DX12_RESOURCE_DESC::Tex2D(D3DImage::convertImageFormatToD3DFormat(format), config._size.width, config._size.height, 1, 1);
        swapChainBufferDesc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;

        D3D12_CLEAR_VALUE swapChainOptimizedClearValue = {};
        swapChainOptimizedClearValue.Format = D3DImage::convertImageFormatToD3DFormat(format);

        for (u32 index = 0; index < config._countSwapchainImages; index++)
        {
            ID3D12Resource* swapchainImage = nullptr;
            HRESULT result = m_device->CreateCommittedResource(&swapChainHeapProperties, D3D12_HEAP_FLAG_ALLOW_DISPLAY, &swapChainBufferDesc, D3D12_RESOURCE_STATE_PRESENT, &swapChainOptimizedClearValue, DX_IID_PPV_ARGS(&swapchainImage));
            if (FAILED(result))
            {
                LOG_ERROR("D3DSwapchainXBOX::create CreateCommittedResource is failed. Error %s", D3DDebug::stringError(result).c_str());
                D3DSwapchainXBOX::destroy();

                return false;
            }

            m_device->CreateRenderTargetView(swapchainImage, nullptr, rtvHandle);
            rtvHandle.Offset(1, m_descriptorHeap->getIncrement());

            CD3DX12_CPU_DESCRIPTOR_HANDLE imageHandle(m_descriptorHeap->getCPUHandle(), index, m_descriptorHeap->getIncrement());
            D3DImage* image = new D3DImage(m_device, format, config._size.width, config._size.height, 1U, 1U, TextureUsage::TextureUsage_Attachment | TextureUsage::TextureUsage_Sampled | TextureUsage::TextureUsage_Read, "SwapchainImage_" + std::to_string(index));
            if (!image->create(swapchainImage, imageHandle))
            {
                LOG_ERROR("D3DSwapchainXBOX::create swapimage is failed");
                D3DSwapchainXBOX::destroy();

                return false;
            }

            m_renderTargets.push_back(image);
        }
    }

    {
        IDXGIOutput* dxgiOutput;
        ASSERT(SUCCEEDED(m_adapter->EnumOutputs(0, &dxgiOutput)) && dxgiOutput, "failed");

        u32 timeInterval = m_syncInterval > 1 ? D3D12XBOX_FRAME_INTERVAL_30_HZ : D3D12XBOX_FRAME_INTERVAL_60_HZ;
        HRESULT result = m_device->SetFrameIntervalX(dxgiOutput, timeInterval, static_cast<UINT>(m_renderTargets.size()) - 1, D3D12XBOX_FRAME_INTERVAL_FLAG_NONE);
        if (FAILED(result))
        {
            LOG_ERROR("D3DSwapchainXBOX::create SetFrameIntervalX is failed. Error %s", D3DDebug::stringError(result).c_str());
            D3DSwapchainXBOX::destroy();

            return false;
        }
    }

    {
        HRESULT result = m_device->ScheduleFrameEventX(D3D12XBOX_FRAME_EVENT_ORIGIN, 0U, nullptr, D3D12XBOX_SCHEDULE_FRAME_EVENT_FLAG_NONE);
        if (FAILED(result))
        {
            LOG_ERROR("D3DSwapchainXBOX::create ScheduleFrameEventX is failed. Error %s", D3DDebug::stringError(result).c_str());
            D3DSwapchainXBOX::destroy();

            return false;
        }
    }

    m_frameIndex = 0;

    return true;
}

void D3DSwapchainXBOX::destroy()
{
    for (auto& image : m_renderTargets)
    {
        image->destroy();
        delete image;
    }
    m_renderTargets.clear();

    if (m_descriptorHeap)
    {
        m_heapManager->deallocDescriptorHeap(m_descriptorHeap);
        m_descriptorHeap = nullptr;
    }
}

void D3DSwapchainXBOX::present()
{
    std::vector<ID3D12Resource*> dxResources;
    dxResources.push_back(m_renderTargets[m_frameIndex]->getResource());

    D3D12XBOX_PRESENT_PLANE_PARAMETERS planeParameters = {};
    planeParameters.Token = m_frameToken;
    planeParameters.ResourceCount = static_cast<UINT>(dxResources.size());
    planeParameters.ppResources = dxResources.data();
    planeParameters.pSrcViewRects = nullptr;
    planeParameters.pDestPlacementBase = nullptr;
    planeParameters.ColorSpace = m_colorSpace;
    planeParameters.ScaleFilter = D3D12XBOX_PRESENT_PLANE_SCALE_FILTER_DEFAULT;
    planeParameters.ExtendedDescCount = 0;
    planeParameters.pExtendedDescs = nullptr;
    planeParameters.Flags = D3D12XBOX_PRESENT_PLANE_FLAG_NONE;

    HRESULT result = m_commandQueue->PresentX(1, &planeParameters, nullptr);
    if (FAILED(result))
    {
        LOG_ERROR("D3DSwapchain::present PresentX is failed %s", D3DDebug::stringError(result).c_str());
        ASSERT(false, "present is failed");
    }
}

u32 D3DSwapchainXBOX::acquireImage()
{
    m_frameToken = D3D12XBOX_FRAME_PIPELINE_TOKEN_NULL;
    HRESULT result = m_device->WaitFrameEventX(D3D12XBOX_FRAME_EVENT_ORIGIN, INFINITE, nullptr, D3D12XBOX_WAIT_FRAME_EVENT_FLAG_NONE, &m_frameToken);
    if (FAILED(result))
    {
        LOG_ERROR("D3DSwapchainXBOX::acquireImage WaitFrameEventX is failed. Error %s", D3DDebug::stringError(result).c_str());
        D3DSwapchainXBOX::destroy();

        return false;
    }

    m_frameIndex = (m_frameIndex + 1) % m_renderTargets.size();
    return m_frameIndex;
}
#endif //PLATFORM
} //namespace dx3d
} //namespace renderer
} //namespace v3d
#endif //D3D_RENDER
