#pragma once

#include "Context.h"
#include "TextureProperties.h"

namespace v3d
{
namespace renderer
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * EmptyContext class
    */
    class EmptyContext final : public Context
    {
    public:

        EmptyContext() noexcept;
        ~EmptyContext();

        void beginFrame() override;
        void endFrame() override;
        void presentFrame() override;
        void submit(bool wait = false) override;

        void draw(StreamBufferDescription& desc, u32 firstVertex, u32 vertexCount, u32 firstInstance, u32 instanceCount) override;
        void drawIndexed(StreamBufferDescription& desc, u32 firstIndex, u32 indexCount, u32 firstInstance, u32 instanceCount) override;

        void bindTexture(const Shader* shader, u32 bindIndex, const Image* image) override;
        void bindUniformsBuffer(const Shader* shader, u32 bindIndex, u32 offset, u32 size, const void* data) override;

        //void bindVertexBuffers(const std::vector<Buffer*>& buffer, const std::vector<u64>& offsets) override;

        void setViewport(const core::Rect32& viewport, const core::Vector2D& depth = { 0.0f, 1.0f }) override;
        void setScissor(const core::Rect32& scissor) override;

        void setRenderTarget(const RenderPass::RenderPassInfo* renderpassInfo, const std::vector<Image*>& attachments, const RenderPass::ClearValueInfo* clearInfo, 
            const std::tuple<ObjectTracker<RenderPass>*, ObjectTracker<Framebuffer>*>& trackers) override;
        //void removeRenderTarget(const RenderPass::RenderPassInfo * renderpassInfo, const std::vector<Image*>& attachments, const RenderPass::ClearValueInfo * clearInfo) override;
        void removeFramebuffer(Framebuffer* framebuffer) override;
        void removeRenderPass(RenderPass* renderpass) override;
        void invalidateRenderPass() override;

        void setPipeline(const Pipeline::PipelineGraphicInfo* pipelineInfo, ObjectTracker<Pipeline>* tracker) override;
        void removePipeline(Pipeline* pipeline) override;

        Image* createImage(TextureTarget target, renderer::Format format, const core::Dimension3D& dimension, u32 mipmapLevel, TextureUsageFlags flags) const override;
        //Image* createAttachmentImage(renderer::Format format, const core::Dimension3D& dimension, TextureSamples samples, TextureUsageFlags flags) const override;
        void removeImage(Image* image) override;

        Buffer* createBuffer(Buffer::BufferType type, u16 usageFlag, u64 size) override;
        void removeBuffer(Buffer* buffer) override;

        const DeviceCaps* getDeviceCaps() const override;

    private:

        virtual void clearBackbuffer(const core::Vector4D & color) override;

        Framebuffer* createFramebuffer(const std::vector<Image*>& attachments, const core::Dimension2D& size) override;
        RenderPass* createRenderPass(const RenderPass::RenderPassInfo* renderpassInfo) override;
        Pipeline* createPipeline(Pipeline::PipelineType type) override;
        Sampler* createSampler() override;

        bool initialize() override;
        void destroy() override;

    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace renderer
} //namespace v3d
