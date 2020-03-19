#include "D3D12GraphicContext.h"

#include "Utils/Logger.h"

#ifdef D3D_RENDER
#   include <wrl.h>
#   include "D3D12Debug.h"
#   include "D3D12Wrapper.h"

#   include "D3D12Image.h"
#   include "D3D12Buffer.h"

namespace v3d
{
namespace renderer
{
namespace d3d12
{

#if (D3D_VERSION_MAJOR == 12 && D3D_VERSION_MINOR == 0)
D3D_FEATURE_LEVEL D3DGraphicContext::s_featureLevel = D3D_FEATURE_LEVEL_12_0;
#elif (D3D_VERSION_MAJOR == 12 && D3D_VERSION_MINOR == 1)
D3D_FEATURE_LEVEL D3DGraphicContext::s_featureLevel = D3D_FEATURE_LEVEL_12_1;
#else
#   error "DirectX version not supported"
#endif 

#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3d12.lib")

D3DGraphicContext::D3DGraphicContext(const platform::Window* window) noexcept
    : m_factory(nullptr)
    , m_adapter(nullptr)
    , m_device(nullptr)
#if D3D_DEBUG_LAYERS
    , m_debugController(nullptr)
#endif //D3D_DEBUG_LAYERS
    , m_commandQueue(nullptr)

    , m_swapchain(nullptr)
    , m_window(window)

    , m_commandListManager(nullptr)
{
    LOG_DEBUG("D3DGraphicContext::D3DGraphicContext constructor %llx", this);

    m_renderType = RenderType::DirectXRender;
    memset(&m_currentState, 0, sizeof(RenderState));
}

D3DGraphicContext::~D3DGraphicContext()
{
    LOG_DEBUG("D3DGraphicContext::D3DGraphicContext destructor %llx", this);
    
    ASSERT(!m_commandListManager, "not nullptr");

    ASSERT(!m_commandQueue, "not nullptr");

    ASSERT(!m_swapchain, "not nullptr");

    ASSERT(!m_device, "not nullptr");
    ASSERT(!m_adapter, "not nullptr");
    ASSERT(!m_factory, "not nullptr");
#if D3D_DEBUG_LAYERS
    ASSERT(!m_debugController, "not nullptr");
#endif //D3D_DEBUG_LAYERS
}

void D3DGraphicContext::beginFrame()
{
    u32 indexFrame = m_swapchain->acquireImage();

    m_commandListManager->sync(indexFrame, m_swapchain->vsync());
    m_commandListManager->update();

#if D3D_DEBUG
    LOG_DEBUG("D3DGraphicContext::beginFrame %d, index %d", m_frameCounter, indexFrame);
#endif //D3D_DEBUG

    ASSERT(!m_currentState.commandList(), "not nullptr");
    m_currentState._commandList = m_commandListManager->acquireCommandList(D3DCommandList::Type::Direct);
    m_currentState.commandList()->prepare();

    m_currentState.commandList()->transition(m_swapchain->getSwapchainImage(), D3D12_RESOURCE_STATE_RENDER_TARGET);
}

void D3DGraphicContext::endFrame()
{
#if D3D_DEBUG
    LOG_DEBUG("D3DGraphicContext::endFrame %d", m_frameCounter);
#endif //D3D_DEBUG

    ASSERT(m_currentState.commandList(), "nullptr");
    m_currentState.commandList()->transition(m_swapchain->getSwapchainImage(), D3D12_RESOURCE_STATE_PRESENT);
    m_currentState.commandList()->close();

    m_commandListManager->execute(m_currentState.commandList(), false);
    m_currentState.reset();
}

void D3DGraphicContext::presentFrame()
{
#if D3D_DEBUG
    LOG_DEBUG("D3DGraphicContext::presentFrame %d", m_frameCounter);
#endif //D3D_DEBUG

    m_swapchain->present();

    ASSERT(!m_currentState.commandList(), "not nullptr");
    //m_commandListManager->update();
    ++m_frameCounter;
}

void D3DGraphicContext::submit(bool wait)
{
    ASSERT(m_currentState.commandList(), "nullptr");

    m_currentState.commandList()->close();
    m_commandListManager->execute(m_currentState.commandList(), wait);
    m_currentState.reset();
}

void D3DGraphicContext::draw(const StreamBufferDescription& desc, u32 firstVertex, u32 vertexCount, u32 firstInstance, u32 instanceCount)
{
}

void D3DGraphicContext::drawIndexed(const StreamBufferDescription& desc, u32 firstIndex, u32 indexCount, u32 firstInstance, u32 instanceCount)
{
}

void D3DGraphicContext::bindImage(const Shader* shader, u32 bindIndex, const Image* image)
{
}

void D3DGraphicContext::bindSampler(const Shader* shader, u32 bindIndex, const Sampler::SamplerInfo* samplerInfo)
{
}

void D3DGraphicContext::bindSampledImage(const Shader* shader, u32 bindIndex, const Image* image, const Sampler::SamplerInfo* samplerInfo)
{
}

void D3DGraphicContext::bindUniformsBuffer(const Shader* shader, u32 bindIndex, u32 offset, u32 size, const void* data)
{
}

void D3DGraphicContext::transitionImages(const std::vector<Image*>& images, TransitionOp transition, s32 layer)
{
}

void D3DGraphicContext::setViewport(const core::Rect32& viewport, const core::Vector2D& depth)
{
    D3D12_VIEWPORT dxViewport =
    {
        static_cast<f32>(viewport.getLeftX()),
        static_cast<f32>(viewport.getTopY()),
        static_cast<f32>(viewport.getRightX()),
        static_cast<f32>(viewport.getBottomY()),
        depth.x,
        depth.y
    };

    static_cast<D3DGraphicsCommandList*>(D3DGraphicContext::getCurrentCommandList())->setViewport({ dxViewport });
}

void D3DGraphicContext::setScissor(const core::Rect32& scissor)
{
    D3D12_RECT dxScissor =
    {
        scissor.getLeftX(), scissor.getTopY(), scissor.getRightX(), scissor.getBottomY()
    };

    static_cast<D3DGraphicsCommandList*>(D3DGraphicContext::getCurrentCommandList())->setScissor({ dxScissor });
}

void D3DGraphicContext::setRenderTarget(const RenderPass::RenderPassInfo* renderpassInfo, const Framebuffer::FramebufferInfo* framebufferInfo)
{
    LOG_DEBUG("D3DGraphicContext::setRenderTarget");
    ASSERT(renderpassInfo && framebufferInfo, "nullptr");

    //D3DCommandList* commandList = getCurrentCommandList();
    //commandList->OMSetRenderTargets(1, &rtvHandle, FALSE, nullptr);
}

void D3DGraphicContext::removeFramebuffer(Framebuffer* framebuffer)
{
}

void D3DGraphicContext::removeRenderPass(RenderPass* renderpass)
{
}

void D3DGraphicContext::invalidateRenderPass()
{
}

void D3DGraphicContext::setPipeline(const Pipeline::PipelineGraphicInfo* pipelineInfo)
{
}

void D3DGraphicContext::removePipeline(Pipeline* pipeline)
{
}

Image* D3DGraphicContext::createImage(TextureTarget target, renderer::Format format, const core::Dimension3D& dimension, u32 layers, u32 mipmapLevel, TextureUsageFlags flags)
{
    return nullptr;
}

Image* D3DGraphicContext::createImage(renderer::Format format, const core::Dimension3D& dimension, TextureSamples samples, TextureUsageFlags flags)
{
    return nullptr;
}

void D3DGraphicContext::removeImage(Image* image)
{
}

Buffer* D3DGraphicContext::createBuffer(Buffer::BufferType type, u16 usageFlag, u64 size)
{
#if D3D_DEBUG
    LOG_DEBUG("VulkanGraphicContext::createBuffer");
#endif //D3D_DEBUG
    if (type == Buffer::BufferType::BufferType_VertexBuffer || type == Buffer::BufferType::BufferType_IndexBuffer || type == Buffer::BufferType::BufferType_UniformBuffer)
    {
        return new D3DBuffer(m_device, type, usageFlag, size);
    }

    ASSERT(false, "not supported");
    return nullptr;
}

void D3DGraphicContext::removeBuffer(Buffer* buffer)
{
    D3DBuffer* dxBuffer = static_cast<D3DBuffer*>(buffer);
    //TODO remove
}

void D3DGraphicContext::removeSampler(Sampler* sampler)
{
}

const DeviceCaps* D3DGraphicContext::getDeviceCaps() const
{
    return nullptr;
}

D3DCommandList* D3DGraphicContext::getCurrentCommandList() const
{
    ASSERT(m_currentState._commandList, "nullptr");
    return m_currentState._commandList;
}

bool D3DGraphicContext::initialize()
{
    UINT dxgiFactoryFlags = 0;

#if D3D_DEBUG_LAYERS
    if (SUCCEEDED(D3DWrapper::GetDebugInterface(IID_PPV_ARGS(&m_debugController))))
    {
        m_debugController->EnableDebugLayer();

        // Enable additional debug layers.
        dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
    }
#endif //D3D_DEBUG_LAYERS

    {
        HRESULT result = CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&m_factory));
        if (FAILED(result))
        {
            LOG_ERROR("D3DGraphicContext::initialize CreateDXGIFactory2 is failed. Error %s", D3DDebug::stringError(result).c_str());
            return false;
        }

        D3DGraphicContext::getHardwareAdapter(m_factory, &m_adapter);
    }

    {
        HRESULT result = D3DWrapper::CreateDevice(m_adapter, D3DGraphicContext::s_featureLevel, IID_PPV_ARGS(&m_device));
        if (FAILED(result))
        {
            LOG_ERROR("D3DGraphicContext::initialize D3D12CreateDevice is failed. Error %s", D3DDebug::stringError(result).c_str());
            D3DGraphicContext::destroy();

            return false;
        }

#if D3D_DEBUG
        D3DDebug::getInstance()->attachDevice(m_device, D3D12_DEBUG_FEATURE_NONE);
#endif
    }

    {
        // Describe and create the command queue.
        D3D12_COMMAND_QUEUE_DESC queueDesc = {};
        queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
        queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
        queueDesc.Priority = 0;
        queueDesc.NodeMask = 0;

        HRESULT result = m_device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_commandQueue));
        if (FAILED(result))
        {
            LOG_ERROR("D3DGraphicContext::initialize CreateCommandQueue is failed. Error %s", D3DDebug::stringError(result).c_str());
            D3DGraphicContext::destroy();

            return false;
        }
#if D3D_DEBUG
        m_commandQueue->SetName(L"PresentCommandQueue");
#endif
    }

    {
        D3DSwapchain::SwapchainConfig config;
        config._window = m_window->getWindowHandle();
        config._size = m_window->getSize();
        config._countSwapchainImages = 3;
        config._vsync = false;
        config._fullscreen = false;

        m_swapchain = new D3DSwapchain(m_factory, m_device, m_commandQueue);

        if (!m_swapchain->create(config))
        {
            LOG_ERROR("D3DGraphicContext::initialize D3DSwapchain::create is failed");
            D3DGraphicContext::destroy();

            return false;
        }

        m_commandListManager = new D3DCommandListManager(m_device, m_commandQueue, config._countSwapchainImages);
    }

#if D3D_DEBUG
    D3DDebug::getInstance()->report(D3D12_RLDO_SUMMARY | D3D12_RLDO_IGNORE_INTERNAL);
#endif

    return true;
}

void D3DGraphicContext::destroy()
{
#if D3D_DEBUG
    D3DDebug::getInstance()->report(D3D12_RLDO_SUMMARY | D3D12_RLDO_IGNORE_INTERNAL);
#endif

    if (m_commandListManager)
    {
        m_commandListManager->waitAndClear();

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

#if D3D_DEBUG
    D3DDebug::getInstance()->freeInstance();
#endif
    SAFE_DELETE(m_device);

    SAFE_DELETE(m_adapter);
    SAFE_DELETE(m_factory);

#if D3D_DEBUG_LAYERS
    if (m_debugController)
    {
        SAFE_DELETE(m_debugController);
    }
#endif //D3D_DEBUG_LAYERS
}

void D3DGraphicContext::clearBackbuffer(const core::Vector4D& color)
{
    m_swapchain->getSwapchainImage()->clear(this, color);
}

Framebuffer* D3DGraphicContext::createFramebuffer(const std::vector<Image*>& images, const core::Dimension2D& size)
{
    return nullptr;
}

RenderPass* D3DGraphicContext::createRenderPass(const RenderPassDescription* renderpassDesc)
{
    return nullptr;
}

Pipeline* D3DGraphicContext::createPipeline(Pipeline::PipelineType type)
{
    return nullptr;
}

Sampler* D3DGraphicContext::createSampler()
{
    return nullptr;
}

// Helper function for acquiring the first available hardware adapter that supports Direct3D 12.
// If no such adapter can be found, *ppAdapter will be set to nullptr.
_Use_decl_annotations_
void D3DGraphicContext::getHardwareAdapter(IDXGIFactory2* pFactory, IDXGIAdapter1** ppAdapter)
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

    ASSERT(SUCCEEDED(adapter->QueryInterface(ppAdapter)), "false");
    adapter->Release();
}


} //namespace d3d12
} //namespace renderer
} //namespace v3d

#endif //D3D_RENDER
