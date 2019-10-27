#pragma once

#include "Renderer/Context.h"

#ifdef D3D_RENDER
#   include "D3D12Configuration.h"
#   include "D3D12Swapchain.h"
#   include "D3D12CommandListManager.h"

namespace v3d
{
namespace renderer
{
namespace d3d12
{
     /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * D3DGraphicContext final class. DirectX Render side
    */
    class D3DGraphicContext final : public Context
    {
    public:

        explicit D3DGraphicContext(const platform::Window* window) noexcept;
        ~D3DGraphicContext();

        void beginFrame() override;
        void endFrame() override;
        void presentFrame() override;
        void submit(bool wait = false) override;

        void draw(const StreamBufferDescription& desc, u32 firstVertex, u32 vertexCount, u32 firstInstance, u32 instanceCount) override;
        void drawIndexed(const StreamBufferDescription& desc, u32 firstIndex, u32 indexCount, u32 firstInstance, u32 instanceCount) override;

        void bindImage(const Shader* shader, u32 bindIndex, const Image* image) override;
        void bindSampler(const Shader* shader, u32 bindIndex, const Sampler::SamplerInfo* samplerInfo) override;
        void bindSampledImage(const Shader* shader, u32 bindIndex, const Image* image, const Sampler::SamplerInfo* samplerInfo) override;
        void bindUniformsBuffer(const Shader* shader, u32 bindIndex, u32 offset, u32 size, const void* data) override;

        void transitionImages(const std::vector<Image*>& images, TransitionOp transition, s32 layer = -1) override;

        void setViewport(const core::Rect32& viewport, const core::Vector2D& depth = { 0.0f, 1.0f }) override;
        void setScissor(const core::Rect32& scissor) override;

        void setRenderTarget(const RenderPass::RenderPassInfo* renderpassInfo, const Framebuffer::FramebufferInfo* framebufferInfo) override;
        void removeFramebuffer(Framebuffer* framebuffer) override;
        void removeRenderPass(RenderPass* renderpass) override;
        void invalidateRenderPass() override;

        void setPipeline(const Pipeline::PipelineGraphicInfo* pipelineInfo) override;
        void removePipeline(Pipeline* pipeline) override;

        Image* createImage(TextureTarget target, renderer::Format format, const core::Dimension3D& dimension, u32 layers, u32 mipmapLevel, TextureUsageFlags flags) override;
        Image* createImage(renderer::Format format, const core::Dimension3D& dimension, TextureSamples samples, TextureUsageFlags flags) override;
        void removeImage(Image* image) override;

        Buffer* createBuffer(Buffer::BufferType type, u16 usageFlag, u64 size) override;
        void removeBuffer(Buffer* buffer) override;

        void removeSampler(Sampler* sampler) override;

        const DeviceCaps* getDeviceCaps() const override;

        D3DCommandList* getCurrentCommandList() const;

    private:

        const std::string s_D3DApplicationName = "D3DGraphicContext";

        bool initialize() override;
        void destroy() override;

        void clearBackbuffer(const core::Vector4D & color) override;

        Framebuffer* createFramebuffer(const std::vector<Image*>& images, const core::Dimension2D& size) override;
        RenderPass* createRenderPass(const RenderPassDescription* renderpassDesc) override;
        Pipeline* createPipeline(Pipeline::PipelineType type) override;
        Sampler* createSampler() override;

        ComPtr<IDXGIFactory4>      m_factory;
        ComPtr<IDXGIAdapter1>      m_adapter;
        ComPtr<ID3D12Device>       m_device;
#if D3D_DEBUG_LAYERS
        ComPtr<ID3D12Debug>        m_debugController;
#endif //D3D_DEBUG_LAYERS

        D3DSwapchain*                 m_swapchain;
        const platform::Window* const m_window;


        D3DCommandListManager* m_commandListManager;


        struct RenderState
        {
            D3DGraphicsCommandList* _commandList;
        };
        RenderState m_currentState;

        static D3D_FEATURE_LEVEL s_featureLevel;

        static void getHardwareAdapter(_In_ IDXGIFactory2* pFactory, _Outptr_result_maybenull_ IDXGIAdapter1** ppAdapter);
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace d3d12
} //namespace renderer
} //namespace v3d
#endif //D3D_RENDER
