#include "D3DGraphicContext.h"

#include "Utils/Logger.h"
#include "Renderer/Pipeline.h"
#include "Renderer/RenderPass.h"
#include "Renderer/Framebuffer.h"

#ifdef D3D_RENDER
#include <wrl.h>
#include "D3DDebug.h"
#include "D3DWrapper.h"
#include "D3DImage.h"
#include "D3DBuffer.h"
#include "D3DDeviceCaps.h"
#include "D3DPipelineState.h"
#include "D3DRootSignature.h"
#include "D3DRenderTarget.h"
#include "D3DDescriptorHeap.h"
#include "D3DConstantBuffer.h"

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
#error "DirectX version not supported"
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

    , m_descriptorHeapManager(nullptr)
    , m_swapchain(nullptr)
    , m_window(window)

    , m_commandListManager(nullptr)
    , m_pipelineManager(nullptr)
    , m_constantBufferManager(nullptr)

    , m_rootSignatureManager(nullptr)
{
    LOG_DEBUG("D3DGraphicContext::D3DGraphicContext constructor %llx", this);
    m_renderType = RenderType::DirectXRender;

    m_currentState.reset();
    m_boundState.reset();
}

D3DGraphicContext::~D3DGraphicContext()
{
    LOG_DEBUG("D3DGraphicContext::D3DGraphicContext destructor %llx", this);
    
    ASSERT(!m_rootSignatureManager, "not nullptr");

    ASSERT(!m_pipelineManager, "not nullptr");
    ASSERT(!m_commandListManager, "not nullptr");
    ASSERT(!m_constantBufferManager, "not nullptr");

    ASSERT(!m_commandQueue, "not nullptr");

    ASSERT(!m_swapchain, "not nullptr");
    ASSERT(!m_descriptorHeapManager, "not nullptr");

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

    //m_currentState.commandList()->transition(m_swapchain->getSwapchainImage(), D3D12_RESOURCE_STATE_RENDER_TARGET);
}

void D3DGraphicContext::endFrame()
{
#if D3D_DEBUG
    LOG_DEBUG("D3DGraphicContext::endFrame %d", m_frameCounter);
#endif //D3D_DEBUG

    ASSERT(m_currentState.commandList(), "nullptr");
    D3DGraphicsCommandList* cmdList = m_currentState.commandList();
    if (m_boundState._renderTarget)
    {
        D3DGraphicContext::switchRenderTargetTransitionToFinal(cmdList, m_boundState._renderTarget);
    }
    //cmdList->transition(m_swapchain->getSwapchainImage(), D3D12_RESOURCE_STATE_PRESENT);
    cmdList->close();

    m_commandListManager->execute(cmdList, false);
    m_currentState.reset();
}

void D3DGraphicContext::presentFrame()
{
#if D3D_DEBUG
    LOG_DEBUG("D3DGraphicContext::presentFrame %d", m_frameCounter);
#endif //D3D_DEBUG

    m_swapchain->present();

    m_boundState.reset();
    m_currentState.update(m_descriptorHeapManager);
    ASSERT(!m_currentState.commandList(), "not nullptr");
    m_constantBufferManager->updateStatus();
    //m_commandListManager->update();

    ++m_frameCounter;
}

void D3DGraphicContext::submit(bool wait)
{
    if (m_currentState.commandList())
    {
        D3DGraphicsCommandList* cmdList = m_currentState.commandList();
        cmdList->close();
        m_commandListManager->execute(cmdList, wait);
    }
    m_currentState.reset();
}

void D3DGraphicContext::draw(const StreamBufferDescription& desc, u32 firstVertex, u32 vertexCount, u32 firstInstance, u32 instanceCount)
{
#if D3D_DEBUG
    LOG_DEBUG("D3DGraphicContext::draw");
#endif //D3D_DEBUG
    ASSERT(m_currentState.commandList(), "nullptr");
    D3DGraphicsCommandList* cmdList = m_currentState.commandList();

    if (perpareDraw(cmdList))
    {
        if (m_boundState._bufferDesc != desc)
        {
            cmdList->setVertexState(0, m_currentState._pipeline->getBuffersStrides(), desc._vertices);
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
}

void D3DGraphicContext::bindImage(const Shader* shader, u32 bindIndex, const Image* image)
{
#if D3D_DEBUG
    LOG_DEBUG("D3DGraphicContext::bindImage");
#endif //D3D_DEBUG
}

void D3DGraphicContext::bindSampler(const Shader* shader, u32 bindIndex, const Sampler::SamplerInfo* samplerInfo)
{
#if D3D_DEBUG
    LOG_DEBUG("D3DGraphicContext::bindSampler");
#endif //D3D_DEBUG
}

void D3DGraphicContext::bindSampledImage(const Shader* shader, u32 bindIndex, const Image* image, const Sampler::SamplerInfo* samplerInfo)
{
#if D3D_DEBUG
    LOG_DEBUG("D3DGraphicContext::bindSampledImage");
    ASSERT(false, "unsupported");
#endif //D3D_DEBUG
}

void D3DGraphicContext::bindUniformsBuffer(const Shader* shader, u32 bindIndex, u32 offset, u32 size, const void* data)
{
#if D3D_DEBUG
    LOG_DEBUG("D3DGraphicContext::bindUniformsBuffer");
#endif //D3D_DEBUG

    D3DBuffer* constantBuffer = m_constantBufferManager->acquireConstanBuffer(size);
    ASSERT(constantBuffer, "nulllptr");
    constantBuffer->upload(this, offset, size, data);

    D3DGraphicsCommandList* cmdList = m_currentState.commandList();
    cmdList->setUsed(constantBuffer, 0);

    D3DDescriptor* descriptor = m_descriptorHeapManager->acquireDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE);
    ASSERT(descriptor, "nullptr");

    D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
    cbvDesc.BufferLocation = constantBuffer->getGPUAddress();
    cbvDesc.SizeInBytes = core::alignUp<UINT>(size, 256);

    CD3DX12_CPU_DESCRIPTOR_HANDLE cbvHandle(D3DDescriptor::createCPUDescriptorHandle(descriptor));
    m_device->CreateConstantBufferView(&cbvDesc, cbvHandle);

    m_currentState.bindDescriptor(descriptor, bindIndex);
}

void D3DGraphicContext::transitionImages(const std::vector<Image*>& images, TransitionOp transition, s32 layer)
{
    ASSERT(false, "not impl");
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

    D3DGraphicsCommandList* cmdList = m_currentState.commandList();
    cmdList->setViewport({ dxViewport });
}

void D3DGraphicContext::setScissor(const core::Rect32& scissor)
{
    D3D12_RECT dxScissor =
    {
        scissor.getLeftX(), scissor.getTopY(), scissor.getRightX(), scissor.getBottomY()
    };

    D3DGraphicsCommandList* cmdList = m_currentState.commandList();
    cmdList->setScissor({ dxScissor });
}

void D3DGraphicContext::setRenderTarget(const RenderPass::RenderPassInfo* renderpassInfo, const Framebuffer::FramebufferInfo* framebufferInfo)
{
    LOG_DEBUG("D3DGraphicContext::setRenderTarget");
    ASSERT(renderpassInfo && framebufferInfo, "nullptr");

    D3DRenderState* renderState = static_cast<D3DRenderState*>(std::get<0>(m_renderTargetManager)->acquireRenderPass(renderpassInfo->_value._desc));
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
    ASSERT(false, "not impl");
}

void D3DGraphicContext::removeRenderPass(RenderPass* renderpass)
{
    ASSERT(false, "not impl");
}

void D3DGraphicContext::invalidateRenderPass()
{
}

void D3DGraphicContext::setPipeline(const Pipeline::PipelineGraphicInfo* pipelineInfo)
{
    ASSERT(pipelineInfo, "nullptr");

    Pipeline* pipeline = m_pipelineManager->acquireGraphicPipeline(*pipelineInfo);
    ASSERT(pipeline, "nullptr");
#if D3D_DEBUG
    LOG_DEBUG("D3DGraphicContext::setPipeline %llx", pipeline);
#endif
    pipelineInfo->_tracker->attach(pipeline);

    D3DGraphicPipelineState* dxPipeline = static_cast<D3DGraphicPipelineState*>(pipeline);
    m_currentState._pipeline = dxPipeline;
}

void D3DGraphicContext::removePipeline(Pipeline* pipeline)
{
    ASSERT(false, "not impl");
}

Image* D3DGraphicContext::createImage(TextureTarget target, Format format, const core::Dimension3D& dimension, u32 layers, u32 mipmapLevel, TextureUsageFlags flags)
{
#if D3D_DEBUG
    LOG_DEBUG("D3DGraphicContext::createImage");
#endif //D3D_DEBUG
    D3D12_RESOURCE_DIMENSION dxDimension = D3DImage::convertImageTargetToD3DDimension(target);

    return new D3DImage(m_device, dxDimension, format, dimension, layers, mipmapLevel, flags);
}

Image* D3DGraphicContext::createImage(Format format, const core::Dimension3D& dimension, TextureSamples samples, TextureUsageFlags flags)
{
#if D3D_DEBUG
    LOG_DEBUG("D3DGraphicContext::createImage");
#endif //D3D_DEBUG
    u32 dxSamples = (samples > TextureSamples::TextureSamples_x1) ? 2 << (u32)samples : 1;
    ASSERT(dimension.depth == 1, "must be 1");

    return new D3DImage(m_device, format, dimension.width, dimension.height, dxSamples, flags);
}

void D3DGraphicContext::removeImage(Image* image)
{
    D3DImage* dxImage = static_cast<D3DImage*>(image);
    ASSERT(!dxImage->isUsed(), "still used");
    dxImage->notifyObservers();

    dxImage->destroy();
    delete dxImage;
}

Buffer* D3DGraphicContext::createBuffer(Buffer::BufferType type, u16 usageFlag, u64 size)
{
#if D3D_DEBUG
    LOG_DEBUG("D3DGraphicContext::createBuffer");
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
    ASSERT(!dxBuffer->isUsed(), "still used");
    dxBuffer->notifyObservers();

    dxBuffer->destroy();
    delete dxBuffer;
}

void D3DGraphicContext::removeSampler(Sampler* sampler)
{
    ASSERT(false, "not impl");
}

const DeviceCaps* D3DGraphicContext::getDeviceCaps() const
{
    return D3DDeviceCaps::getInstance();
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

    D3DDeviceCaps* caps = D3DDeviceCaps::getInstance();
    caps->initialize(m_device);

    m_descriptorHeapManager = new D3DDescriptorHeapManager(m_device);
    {
        D3DSwapchain::SwapchainConfig config;
        config._window = m_window->getWindowHandle();
        config._size = m_window->getSize();
        config._countSwapchainImages = 3;
        config._vsync = false;
        config._fullscreen = m_window->isFullscreen();

        m_swapchain = new D3DSwapchain(m_factory, m_device, m_commandQueue, m_descriptorHeapManager);
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
    m_renderTargetManager = std::make_tuple(new RenderPassManager(this), new FramebufferManager(this));

    m_rootSignatureManager = new D3DRootSignatureManager(m_device);
    m_constantBufferManager = new D3DConstantBufferManager(m_device);

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

    if (m_constantBufferManager)
    {
        delete m_constantBufferManager;
        m_constantBufferManager = nullptr;
    }

    //RenderPassManager
    if (std::get<0>(m_renderTargetManager))
    {
        delete std::get<0>(m_renderTargetManager);
    }

    //FramebufferManager
    if (std::get<1>(m_renderTargetManager))
    {
        delete std::get<1>(m_renderTargetManager);
    }

    if (m_pipelineManager)
    {
        delete m_pipelineManager;
        m_pipelineManager = nullptr;
    }

    if (m_rootSignatureManager)
    {
        delete m_rootSignatureManager;
        m_rootSignatureManager = nullptr;
    }

    if (m_commandListManager)
    {
        m_commandListManager->waitAndClear();

        delete m_commandListManager;
        m_commandListManager = nullptr;
    }

#if D3D_DEBUG
    D3DDebug::getInstance()->report(D3D12_RLDO_SUMMARY | D3D12_RLDO_IGNORE_INTERNAL);
#endif
    if (m_swapchain)
    {
        m_swapchain->destroy();

        delete m_swapchain;
        m_swapchain = nullptr;
    }

    if (m_descriptorHeapManager)
    {
        delete m_descriptorHeapManager;
        m_descriptorHeapManager = nullptr;
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
    return new D3DRenderTarget(m_device, m_descriptorHeapManager, images);
}

RenderPass* D3DGraphicContext::createRenderPass(const RenderPassDescription* renderpassDesc)
{
    return new D3DRenderState(*renderpassDesc);
}

Pipeline* D3DGraphicContext::createPipeline(Pipeline::PipelineType type)
{
    if (type == Pipeline::PipelineType::PipelineType_Graphic)
    {
        return new D3DGraphicPipelineState(m_device, m_rootSignatureManager);
    }
    
    ASSERT(false, "not impl");
    return nullptr;
}

Sampler* D3DGraphicContext::createSampler()
{
    return nullptr;
}

bool D3DGraphicContext::perpareDraw(D3DGraphicsCommandList* cmdList)
{
    if (m_boundState._pipeline != m_currentState._pipeline)
    {
        cmdList->setPipelineState(m_currentState._pipeline);
        m_boundState._pipeline = m_currentState._pipeline;
    }

    std::vector<ID3D12DescriptorHeap*> heaps(m_currentState._descriptorHeaps.cbegin(), m_currentState._descriptorHeaps.cend());
    cmdList->setDescriptorTables(heaps, m_currentState._descriptorsList);

    if (m_boundState._renderTarget != m_currentState._renderTarget)
    {
        //change layout
        if (m_boundState._renderTarget)
        {
            D3DGraphicContext::switchRenderTargetTransitionToFinal(cmdList, m_boundState._renderTarget);
        }
        D3DGraphicContext::switchRenderTargetTransitionToWrite(cmdList, m_currentState._renderTarget);

        cmdList->setRenderTarget(m_currentState._renderTarget);
        m_boundState._renderTarget = m_currentState._renderTarget;

        D3DGraphicContext::clearRenderTargets(cmdList, m_currentState._renderTarget, m_currentState._clearInfo);
    }

    m_currentState.resetDescriptorsState();
    return true;
}

void D3DGraphicContext::clearRenderTargets(D3DGraphicsCommandList* cmdList, D3DRenderTarget* target, const Framebuffer::ClearValueInfo& clearInfo)
{
    for (u32 i = 0; i < target->getImages().size(); ++i)
    {
        D3DImage* dxImage = static_cast<D3DImage*>(target->getImages()[i]);
        const AttachmentDescription& attachment = target->getDescription()._attachments[i];
        if (D3DImage::isColorFormat(dxImage->getFormat()))
        {
            switch (attachment._loadOp)
            {
            case RenderTargetLoadOp::LoadOp_DontCare:
            case RenderTargetLoadOp::LoadOp_Load:
                break;

            case RenderTargetLoadOp::LoadOp_Clear:
            {
                f32 color[4] = {};
                memcpy(&color, &clearInfo._color[i], sizeof(core::Vector4D));

                std::vector<D3D12_RECT> rect =
                {
                    { 0, 0, static_cast<LONG>(clearInfo._size.width), static_cast<LONG>(clearInfo._size.height) }
                };

                cmdList->clearRenderTarget(target->getColorDescHandles()[i], color, rect);
                cmdList->setUsed(target, 0);
            }
            break;

            default:
                ASSERT(false, "wrong op");
            }
        }
        else
        {
            enum DepthStencilClearFlag
            {
                DepthStencilClearFlag_None = 0x0,
                DepthStencilClearFlag_Depth = D3D12_CLEAR_FLAG_DEPTH,
                DepthStencilClearFlag_Stencil = D3D12_CLEAR_FLAG_STENCIL
            };

            u32 clearFlags = DepthStencilClearFlag::DepthStencilClearFlag_None;
            switch (attachment._loadOp)
            {
            case RenderTargetLoadOp::LoadOp_DontCare:
            case RenderTargetLoadOp::LoadOp_Load:
                break;

            case RenderTargetLoadOp::LoadOp_Clear:
            {
                clearFlags |= DepthStencilClearFlag::DepthStencilClearFlag_Depth;
            }
            break;

            default:
                ASSERT(false, "wrong op");
            }

            switch (attachment._stencilLoadOp)
            {
            case RenderTargetLoadOp::LoadOp_DontCare:
            case RenderTargetLoadOp::LoadOp_Load:
                break;

            case RenderTargetLoadOp::LoadOp_Clear:
            {
                clearFlags |= DepthStencilClearFlag::DepthStencilClearFlag_Stencil;
            }
            break;

            default:
                ASSERT(false, "wrong op");
            }

            if (clearFlags != DepthStencilClearFlag::DepthStencilClearFlag_None)
            {
                std::vector<D3D12_RECT> rect =
                {
                    { 0, 0, static_cast<LONG>(clearInfo._size.width), static_cast<LONG>(clearInfo._size.height) }
                };

                D3D12_CLEAR_FLAGS flag = (D3D12_CLEAR_FLAGS)clearFlags;
                cmdList->clearRenderTarget(target->getDepthStensilDescHandles(), clearInfo._depth, clearInfo._stencil, flag, rect);
                cmdList->setUsed(target, 0);
            }
        }
    }
}

void D3DGraphicContext::switchRenderTargetTransitionToWrite(D3DGraphicsCommandList* cmdList, D3DRenderTarget* target)
{
    for (u32 i = 0; i < target->getDescription()._countColorAttachments; ++i)
    {
        D3DImage* dxImage = static_cast<D3DImage*>(target->getImages()[i]);
        //const AttachmentDescription& attachment = target->getDescription()._attachments[i];

        cmdList->transition(dxImage, D3D12_RESOURCE_STATE_RENDER_TARGET);
    }

    if (target->getDescription()._hasDepthStencilAttahment)
    {
        D3DImage* dxImage = static_cast<D3DImage*>(target->getImages().back());
        //const AttachmentDescription& attachment = target->getDescription()._attachments.back();

        cmdList->transition(dxImage, D3D12_RESOURCE_STATE_DEPTH_WRITE);
    }
}

void D3DGraphicContext::switchRenderTargetTransitionToFinal(D3DGraphicsCommandList* cmdList, D3DRenderTarget* target)
{
    for (u32 i = 0; i < target->getDescription()._countColorAttachments; ++i)
    {
        D3DImage* dxImage = static_cast<D3DImage*>(target->getImages()[i]);
        const AttachmentDescription& attachment = target->getDescription()._attachments[i];
        switch (attachment._finalTransition)
        {
        case TransitionOp::TransitionOp_ColorAttachmet:
            cmdList->transition(dxImage, D3D12_RESOURCE_STATE_RENDER_TARGET);
            break;

        case TransitionOp::TransitionOp_ShaderRead:
            cmdList->transition(dxImage, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
            break;

        case TransitionOp::TransitionOp_Present:
            cmdList->transition(dxImage, D3D12_RESOURCE_STATE_PRESENT);
            break;

        default:
            ASSERT(false, "wrong transition");
        }
    }

    if (target->getDescription()._hasDepthStencilAttahment)
    {
        D3DImage* dxImage = static_cast<D3DImage*>(target->getImages().back());
        const AttachmentDescription& attachment = target->getDescription()._attachments.back();

        if (attachment._finalTransition == TransitionOp::TransitionOp_DepthStencilAttachmet)
        {
            cmdList->transition(dxImage, D3D12_RESOURCE_STATE_DEPTH_READ);
        }
    }
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


} //namespace dx3d
} //namespace renderer
} //namespace v3d
#endif //D3D_RENDER
