#include "D3DGraphicContext.h"

#include "Utils/Logger.h"

#ifdef D3D_RENDER
namespace v3d
{
namespace renderer
{
namespace d3d
{

D3DGraphicContext::D3DGraphicContext(const platform::Window* window) noexcept
{
}

D3DGraphicContext::~D3DGraphicContext()
{
}

void D3DGraphicContext::beginFrame()
{
}

void D3DGraphicContext::endFrame()
{
}

void D3DGraphicContext::presentFrame()
{
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
    /*IDXGIFactory4* factory;
    CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&factory));

    IDXGIAdapter> warpAdapter;
    factory->EnumWarpAdapter(IID_PPV_ARGS(&warpAdapter));


    D3D12CreateDevice(warpAdapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&m_device));*/
   
    return false;
}

void D3DGraphicContext::destroy()
{
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

void D3DGraphicContext::invalidateStates()
{
}

bool D3DGraphicContext::createInstance()
{
    return false;
}

bool D3DGraphicContext::createDevice()
{
    return false;
}

} //namespace d3d
} //namespace renderer
} //namespace v3d

#endif //D3D_RENDER
