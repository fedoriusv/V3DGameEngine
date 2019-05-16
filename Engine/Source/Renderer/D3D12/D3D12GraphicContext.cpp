#include "D3D12GraphicContext.h"

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

#if (D3D_VERSION_MAJOR == 12 && D3D_VERSION_MINOR == 0)
D3D_FEATURE_LEVEL D3DGraphicContext::s_featureLevel = D3D_FEATURE_LEVEL_12_0;
#elif (D3D_VERSION_MAJOR == 12 && D3D_VERSION_MINOR == 1)
D3D_FEATURE_LEVEL D3DGraphicContext::s_featureLevel = D3D_FEATURE_LEVEL_12_1;
#else
#   error "DirectX version not supported"
#endif 

#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")

D3DGraphicContext::D3DGraphicContext(const platform::Window* window) noexcept
    : m_factory(nullptr)
    , m_adapter(nullptr)
    , m_device(nullptr)
    , m_commandQueue(nullptr)
#if D3D_DEBUG_LAYERS
    , m_debugController(nullptr)
#endif //D3D_DEBUG_LAYERS

    , m_swapchain(nullptr)
    , m_window(window)
{
    LOG_DEBUG("D3DGraphicContext::D3DGraphicContext constructor %llx", this);

    m_renderType = RenderType::D3DRender;
}

D3DGraphicContext::~D3DGraphicContext()
{
    LOG_DEBUG("D3DGraphicContext::D3DGraphicContext destructor %llx", this);

    ASSERT(!m_swapchain, "not nullptr");

    ASSERT(!m_commandQueue, "not nullptr");
    ASSERT(!m_device, "not nullptr");

    //?
    ASSERT(!m_adapter, "not nullptr");
    ASSERT(!m_factory, "not nullptr");

#if D3D_DEBUG_LAYERS
    ASSERT(!m_debugController, "not nullptr");
#endif //D3D_DEBUG_LAYERS
}

void D3DGraphicContext::beginFrame()
{
    u32 indexFrame = m_swapchain->acquireImage();
#if D3D_DEBUG
    LOG_DEBUG("D3DGraphicContext::beginFrame %d, index %d", m_frameCounter, indexFrame);
#endif //D3D_DEBUG
}

void D3DGraphicContext::endFrame()
{
#if D3D_DEBUG
    LOG_DEBUG("D3DGraphicContext::endFrame %d", m_frameCounter);
#endif //D3D_DEBUG
}

void D3DGraphicContext::presentFrame()
{
#if D3D_DEBUG
    LOG_DEBUG("D3DGraphicContext::presentFrame %d", m_frameCounter);
#endif //D3D_DEBUG

    m_swapchain->present();

    ++m_frameCounter;
}

void D3DGraphicContext::submit(bool wait)
{
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
}

void D3DGraphicContext::setScissor(const core::Rect32& scissor)
{
}

void D3DGraphicContext::setRenderTarget(const RenderPass::RenderPassInfo* renderpassInfo, const Framebuffer::FramebufferInfo* framebufferInfo)
{
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
    return nullptr;
}

void D3DGraphicContext::removeBuffer(Buffer* buffer)
{
}

void D3DGraphicContext::removeSampler(Sampler* sampler)
{
}

const DeviceCaps* D3DGraphicContext::getDeviceCaps() const
{
    return nullptr;
}

bool D3DGraphicContext::initialize()
{
    UINT dxgiFactoryFlags = 0;

#if D3D_DEBUG_LAYERS
    if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&m_debugController))))
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
            LOG_ERROR("D3DGraphicContext::initialize CreateDXGIFactory2 is failed. Error %s", StringError(result).c_str());
            return false;
        }

        D3DGraphicContext::GetHardwareAdapter(m_factory, &m_adapter);

        result = D3D12CreateDevice(m_adapter, D3DGraphicContext::s_featureLevel, IID_PPV_ARGS(&m_device));
        if (FAILED(result))
        {
            LOG_ERROR("D3DGraphicContext::initialize D3D12CreateDevice is failed. Error %s", StringError(result).c_str());
            D3DGraphicContext::destroy();

            return false;
        }
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
            LOG_ERROR("D3DGraphicContext::initialize CreateCommandQueue is failed. Error %s", StringError(result).c_str());
            D3DGraphicContext::destroy();

            return false;
        }
    }

    {
        D3DSwapchain::SwapchainConfig config;
        config._window = m_window->getWindowHandle();
        config._size = m_window->getSize();
        config._countSwapchainImages = 3;
        config._vsync = false;

        m_swapchain = new D3DSwapchain(m_factory, m_device, m_commandQueue);
        if (!m_swapchain->create(config))
        {
            LOG_ERROR("D3DGraphicContext::initialize D3DSwapchain::create  is failed");
            D3DGraphicContext::destroy();

            return false;
        }
    }
    
    return true;
}

void D3DGraphicContext::destroy()
{
    if (m_swapchain)
    {
        m_swapchain->destroy();

        delete m_swapchain;
        m_swapchain = nullptr;
    }

    if (m_commandQueue)
    {
        m_commandQueue->Release();
        m_commandQueue = nullptr;
    }

    if (m_device)
    {

        m_device->Release();
        m_device = nullptr;
    }

    //TODO check
    //m_hardwareAdapter
    //m_factory

#if D3D_DEBUG_LAYERS
    if (m_debugController)
    {
        m_debugController->Release();
        m_debugController = nullptr;
    }
#endif //D3D_DEBUG_LAYERS
}

void D3DGraphicContext::clearBackbuffer(const core::Vector4D& color)
{
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
void D3DGraphicContext::GetHardwareAdapter(IDXGIFactory2* pFactory, IDXGIAdapter1** ppAdapter)
{
    Microsoft::WRL::ComPtr<IDXGIAdapter1> adapter;
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
        HRESULT result = D3D12CreateDevice(adapter.Get(), D3DGraphicContext::s_featureLevel, _uuidof(ID3D12Device), nullptr);
        if (SUCCEEDED(result))
        {
            break;
        }
    }

    *ppAdapter = adapter.Detach();
}


} //namespace d3d12
} //namespace renderer
} //namespace v3d

#endif //D3D_RENDER
