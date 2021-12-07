#pragma once

#include "Renderer/Context.h"

#ifdef D3D_RENDER
#include "D3DConfiguration.h"
#include "D3DResource.h"
#include "D3DSwapchain.h"
#include "D3DCommandListManager.h"
#include "D3DDescriptorHeap.h"
#include "D3DDescriptorSet.h"

namespace v3d
{
namespace renderer
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    class PipelineManager;
    class RenderPassManager;
    class FramebufferManager;

    /////////////////////////////////////////////////////////////////////////////////////////////////////

namespace dx3d
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    class D3DRenderTarget;
    class D3DPipelineState;
    class D3DRootSignatureManager;
    class D3DDescriptorHeapManager;
    class D3DConstantBufferManager;
    class D3DDebugLayerMessageCallback;
    class D3DMemoryHeapAllocator;

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

        void dispatchCompute(const core::Dimension3D& groups) override;

        void bindImage(const Shader* shader, u32 bindIndex, const Image* image, s32 layer = k_generalLayer, s32 mip = k_allMipmapsLevels) override;
        void bindSampler(const Shader* shader, u32 bindIndex, const Sampler::SamplerInfo* samplerInfo) override;
        void bindSampledImage(const Shader* shader, u32 bindIndex, const Image* image, const Sampler::SamplerInfo* samplerInfo, s32 layer = k_generalLayer, s32 mip = k_allMipmapsLevels) override;
        void bindUniformsBuffer(const Shader* shader, u32 bindIndex, u32 offset, u32 size, const void* data) override;
        void bindStorageImage(const Shader* shader, u32 bindIndex, const Image* image, s32 layer = k_generalLayer, s32 mip = k_allMipmapsLevels) override;

        void transitionImages(std::vector<std::tuple<const Image*, Image::Subresource>>& images, TransitionOp transition) override;

        void setViewport(const core::Rect32& viewport, const core::Vector2D& depth = { 0.0f, 1.0f }) override;
        void setScissor(const core::Rect32& scissor) override;

        void setRenderTarget(const RenderPass::RenderPassInfo* renderpassInfo, const Framebuffer::FramebufferInfo* framebufferInfo) override;
        void removeFramebuffer(Framebuffer* framebuffer) override;
        void removeRenderPass(RenderPass* renderpass) override;
        void invalidateRenderPass() override;

        void setPipeline(const Pipeline::PipelineGraphicInfo* pipelineInfo) override;
        void setPipeline(const Pipeline::PipelineComputeInfo* pipelineInfo) override;
        void removePipeline(Pipeline* pipeline) override;

        Image* createImage(TextureTarget target, Format format, const core::Dimension3D& dimension, u32 layers, u32 mipmapLevel, TextureUsageFlags flags, [[maybe_unused]] const std::string& name = "") override;
        Image* createImage(TextureTarget target, Format format, const core::Dimension3D& dimension, u32 layers, TextureSamples samples, TextureUsageFlags flags, [[maybe_unused]] const std::string& name = "") override;
        void removeImage(Image* image) override;
        void removeSampler(Sampler* sampler) override;

        Buffer* createBuffer(Buffer::BufferType type, u16 usageFlag, u64 size, [[maybe_unused]] const std::string& name = "") override;
        void removeBuffer(Buffer* buffer) override;

        void generateMipmaps(Image* image, u32 layer, TransitionOp state) override;

        const DeviceCaps* getDeviceCaps() const override;

        D3DCommandList* getOrAcquireCurrentCommandList(D3DCommandList::Type type = D3DCommandList::Direct);
        D3DCommandListManager* getCommandListManager() const;

        D3DResourceDeleter& getResourceDeleter();

    private:

        const std::string s_D3DApplicationName = "D3DGraphicContext";

        bool initialize() override;
        void destroy() override;

        void clearBackbuffer(const core::Vector4D & color) override;

        Framebuffer* createFramebuffer(const std::vector<Image*>& images, const core::Dimension2D& size) override;
        RenderPass* createRenderPass(const RenderPassDescription* renderpassDesc) override;
        Pipeline* createPipeline(Pipeline::PipelineType type) override;
        Sampler* createSampler(const SamplerDescription& desc) override;

        bool perpareDraw(D3DGraphicsCommandList* cmdList);
        bool perpareCompute(D3DGraphicsCommandList* cmdList);

        void clearRenderTargets(D3DGraphicsCommandList* cmdList, D3DRenderTarget* target, const Framebuffer::ClearValueInfo& clearInfo);

        static void switchRenderTargetTransitionToWrite(D3DGraphicsCommandList* cmdList, D3DRenderTarget* target);
        static void switchRenderTargetTransitionToFinal(D3DGraphicsCommandList* cmdList, D3DRenderTarget* target);

        IDXGIAdapter1*      m_adapter;
        ID3D12Device4*      m_device;
#ifdef PLATFORM_WINDOWS
        IDXGIFactory4*      m_factory;
#   if D3D_DEBUG_LAYERS
        ID3D12Debug*        m_debugController;
#   endif //D3D_DEBUG_LAYERS
#   if D3D_DEBUG_LAYERS_CALLBACK
        D3DDebugLayerMessageCallback* m_debugMessageCallback;
#   endif //D3D_DEBUG_LAYERS_CALLBACK
#endif //PLATFORM_WINDOWS

        ID3D12CommandQueue* m_commandQueue;

        D3DMemoryHeapAllocator* m_heapAllocator;
        D3DDescriptorHeapManager* m_descriptorHeapManager;

        D3DSwapchain* m_swapchain;
        const platform::Window* const m_window;

        D3DCommandListManager* m_commandListManager;

        PipelineManager* m_pipelineManager;
        SamplerManager* m_samplerManager;
        std::tuple<RenderPassManager*, FramebufferManager*> m_renderTargetManager;

        D3DRootSignatureManager* m_rootSignatureManager;
        D3DConstantBufferManager* m_constantBufferManager;

        D3DResourceDeleter m_delayedDeleter;

        struct RenderState
        {
            RenderState() noexcept
            {
                reset();
            }

            D3DGraphicsCommandList* commandList()
            {
                return _commandList;
            }

            void setCommandList(D3DGraphicsCommandList* cmdList)
            {
                _commandList = cmdList;
            }

            void reset()
            {
                _commandList = nullptr;

                _pipeline = nullptr;
                _renderTarget = nullptr;

                _bufferDesc.clear();
                _clearInfo.clear();
            }

            D3DPipelineState* _pipeline;
            D3DRenderTarget* _renderTarget;
            StreamBufferDescription _bufferDesc;

            Framebuffer::ClearValueInfo _clearInfo;

        private:

            D3DGraphicsCommandList* _commandList;

        };
        RenderState m_currentState;
        RenderState m_boundState;
        D3DDescriptorSetState* m_descriptorState;

public:

        static D3D_FEATURE_LEVEL s_featureLevel;
        static bool s_supportExerimentalShaderModelFeature;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace dx3d
} //namespace renderer
} //namespace v3d
#endif //D3D_RENDER
