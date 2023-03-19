#include "D3DGraphicContext.h"

#include "Renderer/Shader.h"
#include "Utils/Logger.h"
#include "Utils/Profiler.h"

#ifdef D3D_RENDER
#include "D3DDebug.h"
#include "D3DWrapper.h"
#include "D3DImage.h"
#include "D3DSampler.h"
#include "D3DBuffer.h"
#include "D3DDeviceCaps.h"
#include "D3DGraphicPipelineState.h"
#include "D3DComputePipelineState.h"
#include "D3DRootSignature.h"
#include "D3DRenderTarget.h"
#include "D3DDescriptorHeap.h"
#include "D3DConstantBuffer.h"
#include "D3DMemoryHeap.h"
#include "D3DQueryHeap.h"

#include "Renderer/Core/RenderFrameProfiler.h"

namespace v3d
{
namespace renderer
{
namespace dx3d
{

#if (D3D_VERSION_MAJOR == 12 && D3D_VERSION_MINOR == 0)
D3D_FEATURE_LEVEL D3DGraphicContext::s_featureLevel = D3D_FEATURE_LEVEL_12_0;
#elif (D3D_VERSION_MAJOR == 12 && D3D_VERSION_MINOR == 1)
D3D_FEATURE_LEVEL D3DGraphicContext::s_featureLevel = D3D_FEATURE_LEVEL_12_1;
#else
#   error "DirectX version is not supported"
#endif 

#if defined(PLATFORM_WINDOWS)
#   pragma comment(lib, "dxgi.lib")
#   pragma comment(lib, "d3d12.lib")
#endif //PLATFORM

bool D3DGraphicContext::s_supportExerimentalShaderModelFeature = true;

D3DGraphicContext::D3DGraphicContext(const platform::Window* window) noexcept
    : m_adapter(nullptr)
    , m_device(nullptr)
#ifdef PLATFORM_WINDOWS
    , m_factory(nullptr)
#   if D3D_DEBUG_LAYERS
    , m_debugController(nullptr)
#   endif //D3D_DEBUG_LAYERS
#   if D3D_DEBUG_LAYERS_CALLBACK
    , m_debugMessageCallback(nullptr)
#   endif //D3D_DEBUG_LAYERS_CALLBACK
#endif //PLATFORM_WINDOWS

    , m_commandQueue(nullptr)

    , m_heapAllocator(nullptr)
    , m_descriptorHeapManager(nullptr)
    , m_renderQueryManager(nullptr)

    , m_swapchain(nullptr)
    , m_window(window)

    , m_commandListManager(nullptr)
    , m_pipelineManager(nullptr)
    , m_samplerManager(nullptr)
    , m_constantBufferManager(nullptr)

    , m_rootSignatureManager(nullptr)
    , m_descriptorState(nullptr)
{
    LOG_DEBUG("D3DGraphicContext::D3DGraphicContext constructor %llx", this);
    m_renderType = RenderType::DirectXRender;

    m_currentState.reset();
    m_boundState.reset();
}

D3DGraphicContext::~D3DGraphicContext()
{
    LOG_DEBUG("D3DGraphicContext::D3DGraphicContext destructor %llx", this);
    
    ASSERT(!m_descriptorState, "not nullptr");
    ASSERT(!m_rootSignatureManager, "not nullptr");
    ASSERT(!m_renderQueryManager, "not nullptr");

    ASSERT(!m_samplerManager, "not nullptr");
    ASSERT(!m_pipelineManager, "not nullptr");
    ASSERT(!m_commandListManager, "not nullptr");
    ASSERT(!m_constantBufferManager, "not nullptr");

    ASSERT(!m_commandQueue, "not nullptr");

    ASSERT(!m_swapchain, "not nullptr");
    ASSERT(!m_descriptorHeapManager, "not nullptr");
    ASSERT(!m_heapAllocator, "not nullptr");

    ASSERT(!m_device, "not nullptr");
#ifdef PLATFORM_WINDOWS
    ASSERT(!m_adapter, "not nullptr");
    ASSERT(!m_factory, "not nullptr");
#   if D3D_DEBUG_LAYERS
    ASSERT(!m_debugController, "not nullptr");
#   endif //D3D_DEBUG_LAYERS

#   if D3D_DEBUG_LAYERS_CALLBACK
    ASSERT(!m_debugMessageCallback, "not nullptr");
#   endif //D3D_DEBUG_LAYERS_CALLBACK
#endif //PLATFORM_WINDOWS
}

bool D3DGraphicContext::initialize()
{
#if defined(PLATFORM_WINDOWS)
    UINT dxgiFactoryFlags = 0;
#if D3D_DEBUG_LAYERS
    if (SUCCEEDED(D3DWrapper::GetDebugInterface(DX_IID_PPV_ARGS(&m_debugController))))
    {
        m_debugController->EnableDebugLayer();

        // Enable additional debug layers.
        dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
    }
#endif //D3D_DEBUG_LAYERS

    {
        HRESULT result = CreateDXGIFactory2(dxgiFactoryFlags, DX_IID_PPV_ARGS(&m_factory));
        if (FAILED(result))
        {
            LOG_ERROR("D3DGraphicContext::initialize CreateDXGIFactory2 is failed. Error %s", D3DDebug::stringError(result).c_str());
            return false;
        }

        // Helper function for acquiring the first available hardware adapter that supports Direct3D 12.
        // If no such adapter can be found, *ppAdapter will be set to nullptr.
        auto getHardwareAdapter = [](IDXGIFactory2* pFactory, IDXGIAdapter1** ppAdapter) -> HRESULT
        {
            IDXGIAdapter1* adapter = nullptr;
            *ppAdapter = nullptr;

            for (UINT adapterIndex = 0; DXGI_ERROR_NOT_FOUND != pFactory->EnumAdapters1(adapterIndex, &adapter); ++adapterIndex)
            {
                DXGI_ADAPTER_DESC1 desc;
                adapter->GetDesc1(&desc);

                if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
                {
                    // Don't select the Basic Render Driver adapter.
                    // If you want a software adapter, pass in "/warp" on the command line.
                    continue;
                }

                // Check to see if the adapter supports Direct3D 12, but don't create the
                // actual device yet.
                HRESULT result = D3DWrapper::CreateDevice(adapter, D3DGraphicContext::s_featureLevel, _uuidof(ID3D12Device), nullptr);
                if (SUCCEEDED(result))
                {
                    break;
                }
            }

            HRESULT result = adapter->QueryInterface(ppAdapter);
            adapter->Release();

            return result;
        };
        
        if (HRESULT result = getHardwareAdapter(m_factory, &m_adapter); FAILED(result))
        {
            LOG_ERROR("D3DGraphicContext::initialize getHardwareAdapter is failed. Error %d", D3DDebug::stringError(result).c_str());
            return false;
        }
    }

    {
        std::vector<UUID> experimentalFeatures;
        if (D3DGraphicContext::s_supportExerimentalShaderModelFeature)
        {
            experimentalFeatures.push_back(D3D12ExperimentalShaderModels);
        }

        if (!experimentalFeatures.empty())
        {
            HRESULT result = D3DWrapper::EnableExperimentalFeatures(static_cast<UINT>(experimentalFeatures.size()), experimentalFeatures.data(), nullptr, nullptr);
            ASSERT(SUCCEEDED(result), "failed");
        }
    }

    {
        HRESULT result = D3DWrapper::CreateDevice(m_adapter, D3DGraphicContext::s_featureLevel, DX_IID_PPV_ARGS(&m_device));
        if (FAILED(result))
        {
            LOG_ERROR("D3DGraphicContext::initialize D3D12CreateDevice is failed. Error %s", D3DDebug::stringError(result).c_str());
            D3DGraphicContext::destroy();

            return false;
        }

#if D3D_DEBUG_LIVE_REPORT
        D3DDebug::getLazyInstance()->attachDevice(m_device, D3D12_DEBUG_FEATURE_NONE);
#endif //D3D_DEBUG_LIVE_REPORT

#if D3D_DEBUG_LAYERS_CALLBACK
        m_debugMessageCallback = new D3DDebugLayerMessageCallback(m_device);
        m_debugMessageCallback->registerMessageCallback(D3DDebugLayerMessageCallback::debugLayersMessageCallback, D3D12_MESSAGE_CALLBACK_IGNORE_FILTERS, this);
#endif //D3D_DEBUG_LAYERS_CALLBACK
    }
#elif defined(PLATFORM_XBOX)
    {
        D3D12XBOX_CREATE_DEVICE_PARAMETERS params = {};
        params.Version = D3D12_SDK_VERSION;
        params.ProcessDebugFlags = D3D12XBOX_PROCESS_DEBUG_FLAG_NONE;
        params.GraphicsCommandQueueRingSizeBytes = static_cast<UINT>(D3D12XBOX_DEFAULT_SIZE_BYTES);
        params.pOffchipTessellationBuffer = D3D12_GPU_VIRTUAL_ADDRESS_NULL;
        params.GraphicsScratchMemorySizeBytes = static_cast<UINT>(D3D12XBOX_DEFAULT_SIZE_BYTES);
        params.ComputeScratchMemorySizeBytes = static_cast<UINT>(D3D12XBOX_DEFAULT_SIZE_BYTES);
        params.DisableGeometryShaderAllocations = TRUE;
        params.DisableTessellationShaderAllocations = TRUE;
        params.DisableDXR = TRUE;
        params.DisableAutomaticDPBBBreakBatchEvents = FALSE;
        params.pDXRStackBuffer = D3D12_GPU_VIRTUAL_ADDRESS_NULL;
        params.DXRStackBufferOverrideSizeBytes = 0;
        params.CreateDeviceFlags = D3D12XBOX_CREATE_DEVICE_FLAG_NONE;
        params.AutoHDRPaperWhiteLevelNits = 0;
        params.DisableAutomaticCommandSegmentChaining = FALSE;

#if D3D_DEBUG_LAYERS
        params.ProcessDebugFlags = D3D12_PROCESS_DEBUG_FLAG_DEBUG_LAYER_ENABLED;
#endif

        HRESULT result = D3DWrapper::CreateDevice(nullptr, &params, DX_IID_PPV_ARGS(&m_device));
        if (FAILED(result))
        {
            LOG_ERROR("D3DGraphicContext::initialize D3D12CreateDevice is failed. Error %s", D3DDebug::stringError(result).c_str());
            D3DGraphicContext::destroy();

            return false;
        }
        m_device->SetName(L"DeviceResources");
    }

    {
        IDXGIDevice1* dxgiDevice = nullptr;
        HRESULT result = m_device->QueryInterface(DX_IID_PPV_ARGS(&dxgiDevice));
        m_device->Release();
        ASSERT(SUCCEEDED(result) && dxgiDevice, "nullptr");

        IDXGIAdapter* dxgiAdapter;
        dxgiDevice->GetAdapter(&dxgiAdapter);
        m_adapter = static_cast<IDXGIAdapter1*>(dxgiAdapter);
        ASSERT(m_adapter, "nullptr");
    }
#endif //PLATFORM

    D3DDeviceCaps* caps = D3DDeviceCaps::getLazyInstance();
    caps->initialize(m_adapter, m_device);

    {
        // Describe and create the command queue.
        D3D12_COMMAND_QUEUE_DESC queueDesc = {};
        queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
        queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
        queueDesc.Priority = 0;
        queueDesc.NodeMask = 0;

        HRESULT result = m_device->CreateCommandQueue(&queueDesc, DX_IID_PPV_ARGS(&m_commandQueue));
        if (FAILED(result))
        {
            LOG_ERROR("D3DGraphicContext::initialize CreateCommandQueue is failed. Error %s", D3DDebug::stringError(result).c_str());
            D3DGraphicContext::destroy();

            return false;
        }
#if D3D_DEBUG
        m_commandQueue->SetName(L"PresentCommandQueue");
#endif //D3D_DEBUG
    }

    m_heapAllocator = new D3DSimpleHeapAllocator(m_device);
    m_descriptorHeapManager = new D3DDescriptorHeapManager(m_device);
    m_renderQueryManager = new D3DRenderQueryManager(m_device, m_heapAllocator);
    {
        D3DSwapchain::SwapchainConfig config;
        config._window = m_window->getWindowHandle();
        config._size = m_window->getSize();
        config._countSwapchainImages = 3;
        config._vsync = false;
        config._fullscreen = m_window->isFullscreen();
#if defined(PLATFORM_WINDOWS)
        m_swapchain = new D3DSwapchainWindows(m_factory, m_device, m_commandQueue, m_descriptorHeapManager);
#elif defined(PLATFORM_XBOX)
        m_swapchain = new D3DSwapchainXBOX(m_adapter, m_device, m_commandQueue, m_descriptorHeapManager);
#endif //PLATFORM
        if (!m_swapchain->create(config))
        {
            LOG_ERROR("D3DGraphicContext::initialize D3DSwapchain::create is failed");
            D3DGraphicContext::destroy();

            return false;
        }
        m_backufferDescription._size = config._size;
        m_backufferDescription._format = m_swapchain->getSwapchainImage()->getOriginFormat();

        m_commandListManager = new D3DCommandListManager(m_device, m_commandQueue, config._countSwapchainImages);
    }

    m_pipelineManager = new PipelineManager(this);
    m_samplerManager = new SamplerManager(this);
    m_renderTargetManager = std::make_tuple(new RenderPassManager(this), new FramebufferManager(this));

    m_rootSignatureManager = new D3DRootSignatureManager(m_device);
    m_constantBufferManager = new D3DConstantBufferManager(m_device, m_heapAllocator);

    m_descriptorState = new D3DDescriptorSetState(m_device, m_descriptorHeapManager);
#if defined(PLATFORM_WINDOWS) && D3D_DEBUG_LIVE_REPORT
    D3DDebug::getInstance()->report(D3D12_RLDO_SUMMARY | D3D12_RLDO_IGNORE_INTERNAL);
#endif //D3D_DEBUG

#if FRAME_PROFILER_ENABLE
    m_CPUProfiler = new RenderFrameProfiler(
        {
            RenderFrameProfiler::FrameCounter::FrameTime,
            RenderFrameProfiler::FrameCounter::SetTarget,
            RenderFrameProfiler::FrameCounter::SetPipeline,
            RenderFrameProfiler::FrameCounter::SetStates,
            RenderFrameProfiler::FrameCounter::BindResources,
            RenderFrameProfiler::FrameCounter::DrawCalls,
            RenderFrameProfiler::FrameCounter::QueryCommands,
            RenderFrameProfiler::FrameCounter::Submit,
            RenderFrameProfiler::FrameCounter::Present,
            RenderFrameProfiler::FrameCounter::UpdateSubmitResorces,
        },
        {
        });
    m_frameProfiler.attach(m_CPUProfiler);
#endif //FRAME_PROFILER_ENABLE
    return true;
}

void D3DGraphicContext::destroy()
{
    LOG_DEBUG("D3DGraphicContext::destroy");

#if defined(PLATFORM_WINDOWS) && D3D_DEBUG_LIVE_REPORT
    D3DDebug::getInstance()->report(D3D12_RLDO_SUMMARY | D3D12_RLDO_IGNORE_INTERNAL);
#endif

    if (m_commandListManager)
    {
        m_commandListManager->waitAndClear();
    }

    if (m_constantBufferManager)
    {
        m_constantBufferManager->updateStatus();
        m_constantBufferManager->destroyConstantBuffers();

        delete m_constantBufferManager;
        m_constantBufferManager = nullptr;
    }

    if (m_renderQueryManager)
    {
        m_renderQueryManager;
        delete m_renderQueryManager;
    }

    if (m_descriptorState)
    {
        m_descriptorState->updateStatus();
        delete m_descriptorState;
        m_descriptorState = nullptr;
    }
    m_delayedDeleter.update(true);

    //FramebufferManager
    if (std::get<1>(m_renderTargetManager))
    {
        delete std::get<1>(m_renderTargetManager);
    }

    //RenderPassManager
    if (std::get<0>(m_renderTargetManager))
    {
        delete std::get<0>(m_renderTargetManager);
    }

    if (m_samplerManager)
    {
        delete m_samplerManager;
        m_samplerManager = nullptr;
    }

    if (m_pipelineManager)
    {
        delete m_pipelineManager;
        m_pipelineManager = nullptr;
    }

    if (m_rootSignatureManager)
    {
        m_rootSignatureManager->removeAllRootSignatures();

        delete m_rootSignatureManager;
        m_rootSignatureManager = nullptr;
    }

    if (m_commandListManager)
    {
        delete m_commandListManager;
        m_commandListManager = nullptr;
    }

    if (m_swapchain)
    {
        m_swapchain->destroy();

        delete m_swapchain;
        m_swapchain = nullptr;
    }
    SAFE_DELETE(m_commandQueue);

    if (m_descriptorHeapManager)
    {
        delete m_descriptorHeapManager;
        m_descriptorHeapManager = nullptr;
    }

    if (m_heapAllocator)
    {
        delete m_heapAllocator;
        m_heapAllocator = nullptr;
    }

#if defined(PLATFORM_WINDOWS)
#   if D3D_DEBUG_LAYERS_CALLBACK
    if (m_debugMessageCallback)
    {
        m_debugMessageCallback->unregisterMessageCallback();
        delete m_debugMessageCallback;
        m_debugMessageCallback = nullptr;
    }
#   endif //D3D_DEBUG_LAYERS_CALLBACK

#if defined(PLATFORM_WINDOWS) && D3D_DEBUG_LIVE_REPORT
    D3DDebug::getInstance()->report(D3D12_RLDO_DETAIL | D3D12_RLDO_IGNORE_INTERNAL);
    D3DDebug::getInstance()->freeInstance();
#   endif
#endif //PLATFORM_WINDOWS

    SAFE_DELETE(m_device);

#if defined(PLATFORM_WINDOWS)
    SAFE_DELETE(m_adapter);
    SAFE_DELETE(m_factory);

#   if D3D_DEBUG_LAYERS
    if (m_debugController)
    {
        SAFE_DELETE(m_debugController);
    }
#   endif //D3D_DEBUG_LAYERS
#endif //PLATFORM_WINDOWS
}

void D3DGraphicContext::beginFrame()
{
#if FRAME_PROFILER_ENABLE
    m_frameProfiler.update();

    RenderFrameProfiler::StackProfiler stackFrameProfiler(m_CPUProfiler, RenderFrameProfiler::FrameCounter::FrameTime);
#endif //FRAME_PROFILER_ENABLE

    u32 indexFrame = m_swapchain->acquireImage();

    m_commandListManager->sync(indexFrame, m_swapchain->vsync());
    m_commandListManager->update();

#if D3D_DEBUG
    LOG_DEBUG("D3DGraphicContext::beginFrame %d, index %d", m_frameCounter, indexFrame);
#endif //D3D_DEBUG

   ASSERT(!m_currentState.commandList(), "not nullptr");
   D3DGraphicContext::getOrAcquireCurrentCommandList();
}

void D3DGraphicContext::endFrame()
{
#if D3D_DEBUG
    LOG_DEBUG("D3DGraphicContext::endFrame %d", m_frameCounter);
#endif //D3D_DEBUG

#if FRAME_PROFILER_ENABLE
    RenderFrameProfiler::StackProfiler stackFrameProfiler(m_CPUProfiler, RenderFrameProfiler::FrameCounter::FrameTime);
#endif //FRAME_PROFILER_ENABLE
}

void D3DGraphicContext::presentFrame()
{
#if D3D_DEBUG
    LOG_DEBUG("D3DGraphicContext::presentFrame %d", m_frameCounter);
#endif //D3D_DEBUG

#if FRAME_PROFILER_ENABLE
    RenderFrameProfiler::StackProfiler stackFrameProfiler(m_CPUProfiler, RenderFrameProfiler::FrameCounter::FrameTime);
    RenderFrameProfiler::StackProfiler stackProfiler(m_CPUProfiler, RenderFrameProfiler::FrameCounter::Present);
#endif //FRAME_PROFILER_ENABLE

    if (D3DGraphicsCommandList* cmdList = m_currentState.commandList(); cmdList)
    {
        m_renderQueryManager->resolve(cmdList);

        if (m_boundState._renderTarget)
        {
            D3DGraphicsCommandList::switchRenderTargetTransitionToFinal(cmdList, m_boundState._renderTarget);
        }
        cmdList->close();
        m_commandListManager->execute(cmdList, false);
        m_currentState.setCommandList(nullptr);
    }

    m_swapchain->present();

    m_boundState.reset();
    m_descriptorState->updateStatus();
    ASSERT(!m_currentState.commandList(), "not nullptr");
    m_constantBufferManager->updateStatus();
    m_renderQueryManager->update();
    m_delayedDeleter.update(false);

    ++m_frameCounter;
}

void D3DGraphicContext::submit(bool wait)
{
#if D3D_DEBUG
    LOG_DEBUG("D3DGraphicContext::submit");
#endif //D3D_DEBUG

#if FRAME_PROFILER_ENABLE
    RenderFrameProfiler::StackProfiler stackFrameProfiler(m_CPUProfiler, RenderFrameProfiler::FrameCounter::FrameTime);
    RenderFrameProfiler::StackProfiler stackProfiler(m_CPUProfiler, RenderFrameProfiler::FrameCounter::Submit);
#endif //FRAME_PROFILER_ENABLE

    if (m_currentState.commandList())
    {
        D3DGraphicsCommandList* cmdList = m_currentState.commandList();

        m_renderQueryManager->resolve(cmdList);

        cmdList->close();
        m_commandListManager->execute(cmdList, wait);
        m_currentState.setCommandList(nullptr);
    }
}

void D3DGraphicContext::beginQuery(const Query* query, u32 id, const std::string& tag)
{
#if D3D_DEBUG
    LOG_DEBUG("D3DGraphicContext::beginQuery %llx, %d", query, id);
#endif //D3D_DEBUG
    ASSERT(query, "nullptr");
    const D3DRenderQuery* d3dQuery = static_cast<const D3DRenderQuery*>(query);
    D3DQueryHeap* heap = m_renderQueryManager->acquireQueryHeap(d3dQuery->getType());

    D3DGraphicsCommandList* cmdList = static_cast<D3DGraphicsCommandList*>(D3DGraphicContext::getOrAcquireCurrentCommandList());
    ASSERT(m_currentState.commandList(), "nullptr");
    cmdList->beginQuery(heap, id);

    m_renderQueryManager->bind(const_cast<D3DRenderQuery*>(d3dQuery), heap, 0, d3dQuery->getCount());
}

void D3DGraphicContext::endQuery(const Query* query, u32 id)
{
#if D3D_DEBUG
    LOG_DEBUG("D3DGraphicContext::endQuery %llx, %d", query, id);
#endif //D3D_DEBUG
    ASSERT(query, "nullptr");
    const D3DRenderQuery* d3dQuery = static_cast<const D3DRenderQuery*>(query);
    D3DQueryHeap* heap = m_renderQueryManager->find(const_cast<D3DRenderQuery*>(d3dQuery));
    ASSERT(heap, "not found");

    D3DGraphicsCommandList* cmdList = m_currentState.commandList();
    ASSERT(m_currentState.commandList(), "nullptr");
    cmdList->endQuery(heap, id);
}

void D3DGraphicContext::timestampQuery(const Query* query, u32 id, const std::string& tag)
{
#if D3D_DEBUG
    LOG_DEBUG("D3DGraphicContext::timestampQuery %llx, %d", query, id);
#endif //D3D_DEBUG
    ASSERT(query, "nullptr");
    const D3DRenderQuery* d3dQuery = static_cast<const D3DRenderQuery*>(query);
    D3DQueryHeap* heap = m_renderQueryManager->acquireQueryHeap(d3dQuery->getType());

    D3DGraphicsCommandList* cmdList = static_cast<D3DGraphicsCommandList*>(D3DGraphicContext::getOrAcquireCurrentCommandList());
    ASSERT(m_currentState.commandList(), "nullptr");
    cmdList->endQuery(heap, id);

    m_renderQueryManager->bind(const_cast<D3DRenderQuery*>(d3dQuery), heap, 0, d3dQuery->getCount());
}

void D3DGraphicContext::draw(const StreamBufferDescription& desc, u32 firstVertex, u32 vertexCount, u32 firstInstance, u32 instanceCount)
{
#if D3D_DEBUG
    LOG_DEBUG("D3DGraphicContext::draw");
#endif //D3D_DEBUG

#if FRAME_PROFILER_ENABLE
    RenderFrameProfiler::StackProfiler stackFrameProfiler(m_CPUProfiler, RenderFrameProfiler::FrameCounter::FrameTime);
    RenderFrameProfiler::StackProfiler stackProfiler(m_CPUProfiler, RenderFrameProfiler::FrameCounter::DrawCalls);
#endif //FRAME_PROFILER_ENABLE

    D3DGraphicsCommandList* cmdList = static_cast<D3DGraphicsCommandList*>(D3DGraphicContext::getOrAcquireCurrentCommandList());
    ASSERT(m_currentState.commandList(), "nullptr");

    if (perpareDraw(cmdList))
    {
        if (m_boundState._bufferDesc != desc)
        {
            ASSERT(m_currentState._pipeline && m_currentState._pipeline->getType() == Pipeline::PipelineType::PipelineType_Graphic, "wrong");
            D3DGraphicPipelineState* dxPipeline = static_cast<D3DGraphicPipelineState*>(m_currentState._pipeline);

            cmdList->setVertexState(0, dxPipeline->getBuffersStrides(), desc._vertices);
            m_boundState._bufferDesc = desc;
        }

        cmdList->draw(vertexCount, instanceCount, firstVertex, firstInstance);
    }
}

void D3DGraphicContext::drawIndexed(const StreamBufferDescription& desc, u32 firstIndex, u32 indexCount, u32 firstInstance, u32 instanceCount)
{
#if D3D_DEBUG
    LOG_DEBUG("D3DGraphicContext::drawIndexed");
#endif //D3D_DEBUG

#if FRAME_PROFILER_ENABLE
    RenderFrameProfiler::StackProfiler stackFrameProfiler(m_CPUProfiler, RenderFrameProfiler::FrameCounter::FrameTime);
    RenderFrameProfiler::StackProfiler stackProfiler(m_CPUProfiler, RenderFrameProfiler::FrameCounter::DrawCalls);
#endif //FRAME_PROFILER_ENABLE

    D3DGraphicsCommandList* cmdList = static_cast<D3DGraphicsCommandList*>(D3DGraphicContext::getOrAcquireCurrentCommandList());
    ASSERT(m_currentState.commandList(), "nullptr");

    if (perpareDraw(cmdList))
    {
        if (m_boundState._bufferDesc != desc)
        {
            ASSERT(m_currentState._pipeline && m_currentState._pipeline->getType() == Pipeline::PipelineType::PipelineType_Graphic, "wrong");
            D3DGraphicPipelineState* dxPipeline = static_cast<D3DGraphicPipelineState*>(m_currentState._pipeline);

            cmdList->setIndexState(desc._indices, desc._indexType == IndexType_32 ? DXGI_FORMAT_R32_UINT : DXGI_FORMAT_R16_UINT);
            cmdList->setVertexState(0, dxPipeline->getBuffersStrides(), desc._vertices);

            m_boundState._bufferDesc = desc;
        }

        cmdList->drawIndexed(indexCount, instanceCount, firstIndex, 0, firstInstance);
    }
}

void D3DGraphicContext::dispatchCompute(const core::Dimension3D& groups)
{
#if D3D_DEBUG
    LOG_DEBUG("D3DGraphicContext::dispatchCompute");
#endif //D3D_DEBUG

#if FRAME_PROFILER_ENABLE
    RenderFrameProfiler::StackProfiler stackFrameProfiler(m_CPUProfiler, RenderFrameProfiler::FrameCounter::FrameTime);
    RenderFrameProfiler::StackProfiler stackProfiler(m_CPUProfiler, RenderFrameProfiler::FrameCounter::ComputeCalls);
#endif //FRAME_PROFILER_ENABLE

    ASSERT(m_currentState.commandList(), "nullptr");
    D3DGraphicsCommandList* cmdList = m_currentState.commandList();

    if (perpareCompute(cmdList))
    {
        cmdList->dispatch(groups);
    }
}

void D3DGraphicContext::bindImage(const Shader* shader, u32 bindIndex, const Image* image, s32 layer, s32 mip)
{
#if D3D_DEBUG
    LOG_DEBUG("D3DGraphicContext::bindImage");
#endif //D3D_DEBUG

#if FRAME_PROFILER_ENABLE
    RenderFrameProfiler::StackProfiler stackFrameProfiler(m_CPUProfiler, RenderFrameProfiler::FrameCounter::FrameTime);
    RenderFrameProfiler::StackProfiler stackProfiler(m_CPUProfiler, RenderFrameProfiler::FrameCounter::BindResources);
#endif //FRAME_PROFILER_ENABLE

    ASSERT(bindIndex < shader->getReflectionInfo()._images.size(), "range out");
    u32 space = shader->getReflectionInfo()._images[bindIndex]._set;
    u32 binding = shader->getReflectionInfo()._images[bindIndex]._binding;
    u32 array = shader->getReflectionInfo()._images[bindIndex]._array;

    ASSERT(image, "nullptr");
    D3DImage* dxImage = const_cast<D3DImage*>(static_cast<const D3DImage*>(image));

    m_descriptorState->bindDescriptor<D3DImage, false, const Image::Subresource&>(space, binding, array, dxImage, D3DImage::makeD3DImageSubresource(dxImage, layer, mip));
}

void D3DGraphicContext::bindSampler(const Shader* shader, u32 bindIndex, const Sampler::SamplerInfo* samplerInfo)
{
#if D3D_DEBUG
    LOG_DEBUG("D3DGraphicContext::bindSampler");
#endif //D3D_DEBUG

#if FRAME_PROFILER_ENABLE
    RenderFrameProfiler::StackProfiler stackFrameProfiler(m_CPUProfiler, RenderFrameProfiler::FrameCounter::FrameTime);
    RenderFrameProfiler::StackProfiler stackProfiler(m_CPUProfiler, RenderFrameProfiler::FrameCounter::BindResources);
#endif //FRAME_PROFILER_ENABLE

    ASSERT(bindIndex < shader->getReflectionInfo()._samplers.size(), "range out");
    u32 space = shader->getReflectionInfo()._samplers[bindIndex]._set;
    u32 binding = shader->getReflectionInfo()._samplers[bindIndex]._binding;

    ASSERT(samplerInfo, "nullptr");
    D3DSampler* dxSampler = static_cast<D3DSampler*>(m_samplerManager->acquireSampler(samplerInfo->_desc));

    m_descriptorState->bindDescriptor<D3DSampler, false>(space, binding, 1, dxSampler);
}

void D3DGraphicContext::bindSampledImage(const Shader* shader, u32 bindIndex, const Image* image, const Sampler::SamplerInfo* samplerInfo, s32 layer, s32 mip)
{
#if D3D_DEBUG
    LOG_DEBUG("D3DGraphicContext::bindSampledImage");
#endif //D3D_DEBUG

#if FRAME_PROFILER_ENABLE
    RenderFrameProfiler::StackProfiler stackFrameProfiler(m_CPUProfiler, RenderFrameProfiler::FrameCounter::FrameTime);
    RenderFrameProfiler::StackProfiler stackProfiler(m_CPUProfiler, RenderFrameProfiler::FrameCounter::BindResources);
#endif //FRAME_PROFILER_ENABLE

    ASSERT(false, "unsupported");
}

void D3DGraphicContext::bindUniformsBuffer(const Shader* shader, u32 bindIndex, u32 offset, u32 size, const void* data)
{
#if D3D_DEBUG
    LOG_DEBUG("D3DGraphicContext::bindUniformsBuffer");
#endif //D3D_DEBUG

#if FRAME_PROFILER_ENABLE
    RenderFrameProfiler::StackProfiler stackFrameProfiler(m_CPUProfiler, RenderFrameProfiler::FrameCounter::FrameTime);
    RenderFrameProfiler::StackProfiler stackProfiler(m_CPUProfiler, RenderFrameProfiler::FrameCounter::BindResources);
#endif //FRAME_PROFILER_ENABLE

    ASSERT(bindIndex < shader->getReflectionInfo()._uniformBuffers.size(), "range out");
    u32 space = shader->getReflectionInfo()._uniformBuffers[bindIndex]._set;
    u32 binding = shader->getReflectionInfo()._uniformBuffers[bindIndex]._binding;
    u32 array = shader->getReflectionInfo()._uniformBuffers[bindIndex]._array;

    D3DBuffer* constantBuffer = m_constantBufferManager->acquireConstanBuffer(size);
    ASSERT(constantBuffer, "nulllptr");
    constantBuffer->write(this, offset, size, data);

    m_descriptorState->bindDescriptor<D3DBuffer, false>(space, binding, array, constantBuffer, 0, size);
}

void D3DGraphicContext::bindStorageImage(const Shader* shader, u32 bindIndex, const Image* image, s32 layer, s32 mip)
{
#if D3D_DEBUG
    LOG_DEBUG("D3DGraphicContext::bindStorageImage");
#endif //D3D_DEBUG

#if FRAME_PROFILER_ENABLE
    RenderFrameProfiler::StackProfiler stackFrameProfiler(m_CPUProfiler, RenderFrameProfiler::FrameCounter::FrameTime);
    RenderFrameProfiler::StackProfiler stackProfiler(m_CPUProfiler, RenderFrameProfiler::FrameCounter::BindResources);
#endif //FRAME_PROFILER_ENABLE

    ASSERT(bindIndex < shader->getReflectionInfo()._storageImages.size(), "range out");
    u32 space = shader->getReflectionInfo()._storageImages[bindIndex]._set;
    u32 binding = shader->getReflectionInfo()._storageImages[bindIndex]._binding;
    u32 array = shader->getReflectionInfo()._storageImages[bindIndex]._array;

    ASSERT(image, "nullptr");
    D3DImage* dxImage = const_cast<D3DImage*>(static_cast<const D3DImage*>(image));

    m_descriptorState->bindDescriptor<D3DImage, true, const Image::Subresource&>(space, binding, array, dxImage, D3DImage::makeD3DImageSubresource(dxImage, layer, mip));
}

void D3DGraphicContext::transitionImages(std::vector<std::tuple<const Image*, Image::Subresource>>& images, TransitionOp transition)
{
#if FRAME_PROFILER_ENABLE
    RenderFrameProfiler::StackProfiler stackFrameProfiler(m_CPUProfiler, RenderFrameProfiler::FrameCounter::FrameTime);
    RenderFrameProfiler::StackProfiler stackProfiler(m_CPUProfiler, RenderFrameProfiler::FrameCounter::Transitions);
#endif //FRAME_PROFILER_ENABLE

    D3DGraphicsCommandList* cmdList = static_cast<D3DGraphicsCommandList*>(D3DGraphicContext::getOrAcquireCurrentCommandList());
    for (auto& img : images)
    {
        D3DImage* dxImage = const_cast<D3DImage*>(static_cast<const D3DImage*>(std::get<0>(img)));
        const Image::Subresource& subresource = std::get<1>(img);
        D3D12_RESOURCE_STATES dxState = D3DRenderState::convertTransitionToD3D(transition);

        cmdList->transition(dxImage, subresource, dxState);
    }
}

void D3DGraphicContext::setViewport(const core::Rect32& viewport, const core::Vector2D& depth)
{
#if D3D_DEBUG
    LOG_DEBUG("D3DGraphicContext::setViewport");
#endif //D3D_DEBUG

#if FRAME_PROFILER_ENABLE
    RenderFrameProfiler::StackProfiler stackFrameProfiler(m_CPUProfiler, RenderFrameProfiler::FrameCounter::FrameTime);
    RenderFrameProfiler::StackProfiler stackProfiler(m_CPUProfiler, RenderFrameProfiler::FrameCounter::SetStates);
#endif //FRAME_PROFILER_ENABLE

    D3D12_VIEWPORT dxViewport =
    {
        static_cast<f32>(viewport.getLeftX()),
        static_cast<f32>(viewport.getTopY()),
        static_cast<f32>(viewport.getRightX()),
        static_cast<f32>(viewport.getBottomY()),
        depth.x,
        depth.y
    };

    D3DGraphicsCommandList* cmdList = static_cast<D3DGraphicsCommandList*>(D3DGraphicContext::getOrAcquireCurrentCommandList());
    cmdList->setViewport({ dxViewport });
}

void D3DGraphicContext::setScissor(const core::Rect32& scissor)
{
#if D3D_DEBUG
    LOG_DEBUG("D3DGraphicContext::setScissor");
#endif //D3D_DEBUG

#if FRAME_PROFILER_ENABLE
    RenderFrameProfiler::StackProfiler stackFrameProfiler(m_CPUProfiler, RenderFrameProfiler::FrameCounter::FrameTime);
    RenderFrameProfiler::StackProfiler stackProfiler(m_CPUProfiler, RenderFrameProfiler::FrameCounter::SetStates);
#endif //FRAME_PROFILER_ENABLE

    D3D12_RECT dxScissor =
    {
        scissor.getLeftX(), scissor.getTopY(), scissor.getRightX(), scissor.getBottomY()
    };

    D3DGraphicsCommandList* cmdList = static_cast<D3DGraphicsCommandList*>(D3DGraphicContext::getOrAcquireCurrentCommandList());
    cmdList->setScissor({ dxScissor });
}

void D3DGraphicContext::setRenderTarget(const RenderPass::RenderPassInfo* renderpassInfo, const Framebuffer::FramebufferInfo* framebufferInfo)
{
#if D3D_DEBUG
    LOG_DEBUG("D3DGraphicContext::setRenderTarget");
#endif //D3D_DEBUG

#if FRAME_PROFILER_ENABLE
    RenderFrameProfiler::StackProfiler stackFrameProfiler(m_CPUProfiler, RenderFrameProfiler::FrameCounter::FrameTime);
    RenderFrameProfiler::StackProfiler stackProfiler(m_CPUProfiler, RenderFrameProfiler::FrameCounter::SetTarget);
#endif //FRAME_PROFILER_ENABLE
    ASSERT(renderpassInfo && framebufferInfo, "nullptr");

    D3DRenderState* renderState = static_cast<D3DRenderState*>(std::get<0>(m_renderTargetManager)->acquireRenderPass(renderpassInfo->_desc));
    ASSERT(renderState, "nullptr");
    renderpassInfo->_tracker->attach(renderState);

    Framebuffer* renderTarget = nullptr;
    bool swapchainPresent = std::find(framebufferInfo->_images.cbegin(), framebufferInfo->_images.cend(), nullptr) != framebufferInfo->_images.cend();
    if (swapchainPresent)
    {
        std::vector<Image*> images;
        images.reserve(framebufferInfo->_images.size());

        Image* swapchainImage = m_swapchain->getSwapchainImage();
        for (auto iter = framebufferInfo->_images.begin(); iter < framebufferInfo->_images.end(); ++iter)
        {
            if (*iter == nullptr)
            {
                images.push_back(swapchainImage);
                continue;
            }
            images.push_back(*iter);
        }

        std::tie(renderTarget, std::ignore) = std::get<1>(m_renderTargetManager)->acquireFramebuffer(renderState, images, {});
    }
    else
    {
        std::tie(renderTarget, std::ignore) = std::get<1>(m_renderTargetManager)->acquireFramebuffer(renderState, framebufferInfo->_images, {});
        ASSERT(renderTarget, "nullptr");
    }

    D3DRenderTarget* dxRenderTarget = static_cast<D3DRenderTarget*>(renderTarget);
    framebufferInfo->_tracker->attach(dxRenderTarget);

    m_currentState._renderTarget = dxRenderTarget;
    m_currentState._clearInfo = framebufferInfo->_clearInfo;
}

void D3DGraphicContext::removeFramebuffer(Framebuffer* framebuffer)
{
#if D3D_DEBUG
    LOG_DEBUG("D3DGraphicContext::removeFramebuffer");
#endif //D3D_DEBUG

#if FRAME_PROFILER_ENABLE
    RenderFrameProfiler::StackProfiler stackFrameProfiler(m_CPUProfiler, RenderFrameProfiler::FrameCounter::FrameTime);
    RenderFrameProfiler::StackProfiler stackProfiler(m_CPUProfiler, RenderFrameProfiler::FrameCounter::RemoveResources);
#endif //FRAME_PROFILER_ENABLE
    ASSERT(framebuffer, "nullptr");

    D3DRenderTarget* dxRenderTarget = static_cast<D3DRenderTarget*>(framebuffer);
    FramebufferManager* framebufferMgr = std::get<1>(m_renderTargetManager);
    if (dxRenderTarget->isUsed())
    {
        m_delayedDeleter.requestToDelete(dxRenderTarget, [framebufferMgr, framebuffer]() -> void
            {
                framebufferMgr->removeFramebuffer(framebuffer);
            });
    }
    else
    {
        framebufferMgr->removeFramebuffer(framebuffer);
    }
}

void D3DGraphicContext::removeRenderPass(RenderPass* renderpass)
{
#if D3D_DEBUG
    LOG_DEBUG("D3DGraphicContext::removeRenderPass");
#endif //D3D_DEBUG

#if FRAME_PROFILER_ENABLE
    RenderFrameProfiler::StackProfiler stackFrameProfiler(m_CPUProfiler, RenderFrameProfiler::FrameCounter::FrameTime);
    RenderFrameProfiler::StackProfiler stackProfiler(m_CPUProfiler, RenderFrameProfiler::FrameCounter::RemoveResources);
#endif //FRAME_PROFILER_ENABLE

    ASSERT(renderpass, "nullptr");
    D3DRenderState* dxRenderState = static_cast<D3DRenderState*>(renderpass);
    RenderPassManager* renderpassMgr = std::get<0>(m_renderTargetManager);
    if (dxRenderState->isUsed())
    {
        m_delayedDeleter.requestToDelete(dxRenderState, [renderpassMgr, renderpass]() -> void
            {
                renderpassMgr->removeRenderPass(renderpass);
            });
    }
    else
    {
        renderpassMgr->removeRenderPass(renderpass);
    }
}

void D3DGraphicContext::invalidateRenderTarget()
{
#if FRAME_PROFILER_ENABLE
    RenderFrameProfiler::StackProfiler stackFrameProfiler(m_CPUProfiler, RenderFrameProfiler::FrameCounter::FrameTime);
    RenderFrameProfiler::StackProfiler stackProfiler(m_CPUProfiler, RenderFrameProfiler::FrameCounter::SetTarget);
#endif //FRAME_PROFILER_ENABLE

    m_currentState._renderTarget = nullptr;
}

void D3DGraphicContext::setPipeline(const Pipeline::PipelineGraphicInfo* pipelineInfo)
{
    ASSERT(pipelineInfo, "nullptr");

#if FRAME_PROFILER_ENABLE
    RenderFrameProfiler::StackProfiler stackFrameProfiler(m_CPUProfiler, RenderFrameProfiler::FrameCounter::FrameTime);
    RenderFrameProfiler::StackProfiler stackProfiler(m_CPUProfiler, RenderFrameProfiler::FrameCounter::SetPipeline);
#endif //FRAME_PROFILER_ENABLE

    Pipeline* pipeline = m_pipelineManager->acquireGraphicPipeline(*pipelineInfo);
    ASSERT(pipeline, "nullptr");
#if D3D_DEBUG
    LOG_DEBUG("D3DGraphicContext::setPipeline %llx", pipeline);
#endif //D3D_DEBUG
    pipelineInfo->_tracker->attach(pipeline);

    D3DGraphicPipelineState* dxPipeline = static_cast<D3DGraphicPipelineState*>(pipeline);
    m_currentState._pipeline = dxPipeline;
}

void D3DGraphicContext::setPipeline(const Pipeline::PipelineComputeInfo* pipelineInfo)
{
    ASSERT(pipelineInfo, "nullptr");

#if FRAME_PROFILER_ENABLE
    RenderFrameProfiler::StackProfiler stackFrameProfiler(m_CPUProfiler, RenderFrameProfiler::FrameCounter::FrameTime);
    RenderFrameProfiler::StackProfiler stackProfiler(m_CPUProfiler, RenderFrameProfiler::FrameCounter::SetPipeline);
#endif //FRAME_PROFILER_ENABLE

    Pipeline* pipeline = m_pipelineManager->acquireComputePipeline(*pipelineInfo);
    ASSERT(pipeline, "nullptr");
#if D3D_DEBUG
    LOG_DEBUG("D3DGraphicContext::setPipeline %llx", pipeline);
#endif //D3D_DEBUG
    pipelineInfo->_tracker->attach(pipeline);

    D3DComputePipelineState* dxPipeline = static_cast<D3DComputePipelineState*>(pipeline);
    m_currentState._pipeline = dxPipeline;
}

void D3DGraphicContext::removePipeline(Pipeline* pipeline)
{
#if D3D_DEBUG
    LOG_DEBUG("D3DGraphicContext::removePipeline");
#endif //D3D_DEBUG

#if FRAME_PROFILER_ENABLE
    RenderFrameProfiler::StackProfiler stackFrameProfiler(m_CPUProfiler, RenderFrameProfiler::FrameCounter::FrameTime);
    RenderFrameProfiler::StackProfiler stackProfiler(m_CPUProfiler, RenderFrameProfiler::FrameCounter::RemoveResources);
#endif //FRAME_PROFILER_ENABLE

    ASSERT(pipeline, "nullptr");
    D3DPipelineState* dxPipeline = static_cast<D3DPipelineState*>(pipeline);
    if (dxPipeline->isUsed())
    {
        m_delayedDeleter.requestToDelete(dxPipeline, [this, pipeline]() -> void
            {
                m_pipelineManager->removePipeline(pipeline);
            });
    }
    else
    {
        m_pipelineManager->removePipeline(pipeline);
    }
}

Query* D3DGraphicContext::createQuery(QueryType type, u32 count, const Query::QueryRespose& callback, const std::string& name)
{
#if D3D_DEBUG
    LOG_DEBUG("D3DGraphicContext::createQuery");
#endif //D3D_DEBUG

#if FRAME_PROFILER_ENABLE
    RenderFrameProfiler::StackProfiler stackFrameProfiler(m_CPUProfiler, RenderFrameProfiler::FrameCounter::FrameTime);
    RenderFrameProfiler::StackProfiler stackProfiler(m_CPUProfiler, RenderFrameProfiler::FrameCounter::CreateResources);
#endif //FRAME_PROFILER_ENABLE

    D3DRenderQuery* d3dQuery = new D3DRenderQuery(m_device, type, count, callback, name, m_heapAllocator);
    if (!d3dQuery->create())
    {
        LOG_DEBUG("D3DGraphicContext::createQuery can't create query");
        delete d3dQuery;

    return nullptr;
}

    return d3dQuery;
}

void D3DGraphicContext::removeQuery(Query* query)
{
#if D3D_DEBUG
    LOG_DEBUG("D3DGraphicContext::removeQuery");
#endif //D3D_DEBUG

#if FRAME_PROFILER_ENABLE
    RenderFrameProfiler::StackProfiler stackFrameProfiler(m_CPUProfiler, RenderFrameProfiler::FrameCounter::FrameTime);
    RenderFrameProfiler::StackProfiler stackProfiler(m_CPUProfiler, RenderFrameProfiler::FrameCounter::RemoveResources);
#endif //FRAME_PROFILER_ENABLE
    
    ASSERT(query, "nullptr");
    D3DRenderQuery* dxQuery = static_cast<D3DRenderQuery*>(query);
    if (dxQuery->isUsed())
    {
        m_delayedDeleter.requestToDelete(dxQuery, [this, dxQuery]() -> void
            {
                dxQuery->notifyObservers();

                dxQuery->destroy();
                delete dxQuery;

            });
    }
    else
    {
        dxQuery->notifyObservers();

        dxQuery->destroy();
        delete dxQuery;
    }
}

Image* D3DGraphicContext::createImage(TextureTarget target, Format format, const core::Dimension3D& dimension, u32 layers, u32 mipmapLevel, TextureUsageFlags flags, const std::string& name)
{
#if D3D_DEBUG
    LOG_DEBUG("D3DGraphicContext::createImage");
#endif //D3D_DEBUG

#if FRAME_PROFILER_ENABLE
    RenderFrameProfiler::StackProfiler stackFrameProfiler(m_CPUProfiler, RenderFrameProfiler::FrameCounter::FrameTime);
    RenderFrameProfiler::StackProfiler stackProfiler(m_CPUProfiler, RenderFrameProfiler::FrameCounter::CreateResources);
#endif //FRAME_PROFILER_ENABLE

    D3D12_RESOURCE_DIMENSION dxDimension = D3DImage::convertImageTargetToD3DDimension(target);

    return new D3DImage(m_device, dxDimension, format, dimension, layers, mipmapLevel, flags, name);
}

Image* D3DGraphicContext::createImage(TextureTarget target, Format format, const core::Dimension3D& dimension, u32 layers, TextureSamples samples, TextureUsageFlags flags, const std::string& name)
{
#if D3D_DEBUG
    LOG_DEBUG("D3DGraphicContext::createImage");
#endif //D3D_DEBUG

#if FRAME_PROFILER_ENABLE
    RenderFrameProfiler::StackProfiler stackFrameProfiler(m_CPUProfiler, RenderFrameProfiler::FrameCounter::FrameTime);
    RenderFrameProfiler::StackProfiler stackProfiler(m_CPUProfiler, RenderFrameProfiler::FrameCounter::CreateResources);
#endif //FRAME_PROFILER_ENABLE

    D3D12_RESOURCE_DIMENSION dxDimension = D3DImage::convertImageTargetToD3DDimension(target);
    u32 dxSamples = (samples > TextureSamples::TextureSamples_x1) ? 1 << (u32)samples : 1;
    ASSERT(dimension.depth == 1, "must be 1");

    return new D3DImage(m_device, format, dimension.width, dimension.height, layers, dxSamples, flags, name);
}

void D3DGraphicContext::removeImage(Image* image)
{
#if D3D_DEBUG
    LOG_DEBUG("D3DGraphicContext::removeImage");
#endif //D3D_DEBUG

#if FRAME_PROFILER_ENABLE
    RenderFrameProfiler::StackProfiler stackFrameProfiler(m_CPUProfiler, RenderFrameProfiler::FrameCounter::FrameTime);
    RenderFrameProfiler::StackProfiler stackProfiler(m_CPUProfiler, RenderFrameProfiler::FrameCounter::RemoveResources);
#endif //FRAME_PROFILER_ENABLE
    ASSERT(image, "nullptr");

    D3DImage* dxImage = static_cast<D3DImage*>(image);
    if (dxImage->isUsed())
    {
        m_delayedDeleter.requestToDelete(dxImage, [dxImage]() -> void
            {
                dxImage->notifyObservers();

                dxImage->destroy();
                delete dxImage;
            });
    }
    else
    {
        dxImage->notifyObservers();

        dxImage->destroy();
        delete dxImage;
    }
}

Buffer* D3DGraphicContext::createBuffer(Buffer::BufferType type, u16 usageFlag, u64 size, const std::string& name)
{
#if D3D_DEBUG
    LOG_DEBUG("D3DGraphicContext::createBuffer");
#endif //D3D_DEBUG

#if FRAME_PROFILER_ENABLE
    RenderFrameProfiler::StackProfiler stackFrameProfiler(m_CPUProfiler, RenderFrameProfiler::FrameCounter::FrameTime);
    RenderFrameProfiler::StackProfiler stackProfiler(m_CPUProfiler, RenderFrameProfiler::FrameCounter::CreateResources);
#endif //FRAME_PROFILER_ENABLE

    if (type == Buffer::BufferType::BufferType_VertexBuffer || type == Buffer::BufferType::BufferType_IndexBuffer || type == Buffer::BufferType::BufferType_UniformBuffer)
    {
        return new D3DBuffer(m_device, type, usageFlag, size, name, m_heapAllocator);
    }

    ASSERT(false, "not supported");
    return nullptr;
}

void D3DGraphicContext::removeBuffer(Buffer* buffer)
{
#if D3D_DEBUG
    LOG_DEBUG("D3DGraphicContext::removeBuffer");
#endif //D3D_DEBUG

#if FRAME_PROFILER_ENABLE
    RenderFrameProfiler::StackProfiler stackFrameProfiler(m_CPUProfiler, RenderFrameProfiler::FrameCounter::FrameTime);
    RenderFrameProfiler::StackProfiler stackProfiler(m_CPUProfiler, RenderFrameProfiler::FrameCounter::RemoveResources);
#endif //FRAME_PROFILER_ENABLE
    
    ASSERT(buffer, "nullptr");
    D3DBuffer* dxBuffer = static_cast<D3DBuffer*>(buffer);
    if (dxBuffer->isUsed())
    {
        m_delayedDeleter.requestToDelete(dxBuffer, [dxBuffer]() -> void
            {
                dxBuffer->notifyObservers();

                dxBuffer->destroy();
                delete dxBuffer;
            });
    }
    else
    {
        dxBuffer->notifyObservers();

        dxBuffer->destroy();
        delete dxBuffer;
    }
}

void D3DGraphicContext::removeSampler(Sampler* sampler)
{
#if D3D_DEBUG
    LOG_DEBUG("D3DGraphicContext::removeSampler");
#endif //D3D_DEBUG

#if FRAME_PROFILER_ENABLE
    RenderFrameProfiler::StackProfiler stackFrameProfiler(m_CPUProfiler, RenderFrameProfiler::FrameCounter::FrameTime);
    RenderFrameProfiler::StackProfiler stackProfiler(m_CPUProfiler, RenderFrameProfiler::FrameCounter::RemoveResources);
#endif //FRAME_PROFILER_ENABLE

    ASSERT(sampler, "nullptr");
    D3DSampler* dxSampler = static_cast<D3DSampler*>(sampler);
    if (dxSampler->isUsed())
    {
        m_delayedDeleter.requestToDelete(dxSampler, [this, dxSampler]() -> void
            {
                m_samplerManager->removeSampler(dxSampler);
            });
    }
    else
    {
        m_samplerManager->removeSampler(sampler);
    }

  
    ASSERT(false, "not impl");
}

const DeviceCaps* D3DGraphicContext::getDeviceCaps() const
{
    return D3DDeviceCaps::getInstance();
}

void D3DGraphicContext::generateMipmaps(Image* image, u32 layer, TransitionOp state)
{
    ASSERT(false, "not impl");
}

void D3DGraphicContext::clearRenderTarget(const std::vector<const Image*>& images, Framebuffer::ClearValueInfo& clearValues)
{
#if D3D_DEBUG
    LOG_DEBUG("D3DGraphicContext::clearRenderTarget");
#endif //D3D_DEBUG

#if FRAME_PROFILER_ENABLE
    RenderFrameProfiler::StackProfiler stackFrameProfiler(m_CPUProfiler, RenderFrameProfiler::FrameCounter::FrameTime);
    RenderFrameProfiler::StackProfiler stackProfiler(m_CPUProfiler, RenderFrameProfiler::FrameCounter::DrawCalls);
#endif //FRAME_PROFILER_ENABLE

    //TODO
}

Framebuffer* D3DGraphicContext::createFramebuffer(const std::vector<Image*>& images, const core::Dimension2D& size)
{
#if D3D_DEBUG
    LOG_DEBUG("D3DGraphicContext::createFramebuffer");
#endif //D3D_DEBUG

#if FRAME_PROFILER_ENABLE
    RenderFrameProfiler::StackProfiler stackFrameProfiler(m_CPUProfiler, RenderFrameProfiler::FrameCounter::FrameTime);
    RenderFrameProfiler::StackProfiler stackProfiler(m_CPUProfiler, RenderFrameProfiler::FrameCounter::CreateResources);
#endif //FRAME_PROFILER_ENABLE

    return new D3DRenderTarget(m_device, m_descriptorHeapManager, images);
}

RenderPass* D3DGraphicContext::createRenderPass(const RenderPassDescription* renderpassDesc)
{
#if D3D_DEBUG
    LOG_DEBUG("D3DGraphicContext::createRenderPass");
#endif //D3D_DEBUG

#if FRAME_PROFILER_ENABLE
    RenderFrameProfiler::StackProfiler stackFrameProfiler(m_CPUProfiler, RenderFrameProfiler::FrameCounter::FrameTime);
    RenderFrameProfiler::StackProfiler stackProfiler(m_CPUProfiler, RenderFrameProfiler::FrameCounter::CreateResources);
#endif //FRAME_PROFILER_ENABLE

    return new D3DRenderState(*renderpassDesc);
}

Pipeline* D3DGraphicContext::createPipeline(Pipeline::PipelineType type, const std::string& name)
{
#if D3D_DEBUG
    LOG_DEBUG("D3DGraphicContext::createPipeline");
#endif //D3D_DEBUG

#if FRAME_PROFILER_ENABLE
    RenderFrameProfiler::StackProfiler stackFrameProfiler(m_CPUProfiler, RenderFrameProfiler::FrameCounter::FrameTime);
    RenderFrameProfiler::StackProfiler stackProfiler(m_CPUProfiler, RenderFrameProfiler::FrameCounter::CreateResources);
#endif //FRAME_PROFILER_ENABLE

    if (type == Pipeline::PipelineType::PipelineType_Graphic)
    {
        return new D3DGraphicPipelineState(m_device, m_rootSignatureManager);
    }
    else if (type == Pipeline::PipelineType::PipelineType_Compute)
    {
        return new D3DComputePipelineState(m_device, m_rootSignatureManager);
    }

    ASSERT(false, "not impl");
    return nullptr;
}

Sampler* D3DGraphicContext::createSampler(const SamplerDescription& desc)
{
#if D3D_DEBUG
    LOG_DEBUG("D3DGraphicContext::createSampler");
#endif //D3D_DEBUG

#if FRAME_PROFILER_ENABLE
    RenderFrameProfiler::StackProfiler stackFrameProfiler(m_CPUProfiler, RenderFrameProfiler::FrameCounter::FrameTime);
    RenderFrameProfiler::StackProfiler stackProfiler(m_CPUProfiler, RenderFrameProfiler::FrameCounter::CreateResources);
#endif //FRAME_PROFILER_ENABLE

    return new D3DSampler(desc);
}

void D3DGraphicContext::clearBackbuffer(const core::Vector4D& color)
{
#if FRAME_PROFILER_ENABLE
    RenderFrameProfiler::StackProfiler stackFrameProfiler(m_CPUProfiler, RenderFrameProfiler::FrameCounter::FrameTime);
#endif //FRAME_PROFILER_ENABLE

    m_swapchain->getSwapchainImage()->clear(this, color);
}

bool D3DGraphicContext::perpareDraw(D3DGraphicsCommandList* cmdList)
{
    ASSERT(m_currentState._pipeline && m_currentState._pipeline->getType() == Pipeline::PipelineType::PipelineType_Graphic, "wrong");
    D3DGraphicPipelineState* dxPipeline = static_cast<D3DGraphicPipelineState*>(m_currentState._pipeline);

    if (m_boundState._pipeline != m_currentState._pipeline)
    {
        cmdList->setPipelineState(dxPipeline);
        m_boundState._pipeline = m_currentState._pipeline;
    }

    if (m_boundState._renderTarget != m_currentState._renderTarget)
    {
        //change layout
        if (m_boundState._renderTarget)
        {
            D3DGraphicsCommandList::switchRenderTargetTransitionToFinal(cmdList, m_boundState._renderTarget);
        }
        D3DGraphicsCommandList::switchRenderTargetTransitionToWrite(cmdList, m_currentState._renderTarget);

        cmdList->setRenderTarget(m_currentState._renderTarget);
        m_boundState._renderTarget = m_currentState._renderTarget;
        if (D3DDeviceCaps::getInstance()->supportMultiview && m_currentState._renderTarget->getDescription()._viewsMask)
        {
            cmdList->setViewInstanceMask(m_currentState._renderTarget->getDescription()._viewsMask);
        }

        D3DGraphicsCommandList::clearRenderTargets(cmdList, m_currentState._renderTarget, m_currentState._clearInfo);
    }

    m_descriptorState->updateDescriptorSets(cmdList, dxPipeline);
    m_descriptorState->invalidateDescriptorSetTable();

    return true;
}

bool D3DGraphicContext::perpareCompute(D3DGraphicsCommandList* cmdList)
{
    ASSERT(m_currentState._pipeline && m_currentState._pipeline->getType() == Pipeline::PipelineType::PipelineType_Compute, "wrong");
    D3DComputePipelineState* dxPipeline = static_cast<D3DComputePipelineState*>(m_currentState._pipeline);

    if (m_boundState._pipeline != m_currentState._pipeline)
    {
        cmdList->setPipelineState(dxPipeline);
        m_boundState._pipeline = m_currentState._pipeline;
    }

    m_descriptorState->updateDescriptorSets(cmdList, dxPipeline);
    m_descriptorState->invalidateDescriptorSetTable();

    return true;
}

D3DCommandList* D3DGraphicContext::getOrAcquireCurrentCommandList(D3DCommandList::Type type)
{
    if (!m_currentState.commandList())
    {
        m_currentState.setCommandList(m_commandListManager->acquireCommandList(type));
        m_currentState.commandList()->prepare();
            }

    ASSERT(m_currentState.commandList(), "nullptr");
    return m_currentState.commandList();
            }

D3DCommandListManager* D3DGraphicContext::getCommandListManager() const
            {
    return m_commandListManager;
            }

D3DResourceDeleter& D3DGraphicContext::getResourceDeleter()
            {
    return m_delayedDeleter;
}

} //namespace dx3d
} //namespace renderer
} //namespace v3d
#endif //D3D_RENDER
