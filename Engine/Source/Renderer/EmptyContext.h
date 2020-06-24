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

        Image* createImage(TextureTarget target, renderer::Format format, const core::Dimension3D& dimension, u32 layers, u32 mipmapLevel, TextureUsageFlags flags, [[maybe_unused]] const std::string& name = "") override;
        Image* createImage(renderer::Format format, const core::Dimension3D& dimension, TextureSamples samples, TextureUsageFlags flags, [[maybe_unused]] const std::string& name = "") override;
        void removeImage(Image* image) override;

        Buffer* createBuffer(Buffer::BufferType type, u16 usageFlag, u64 size, [[maybe_unused]] const std::string& name = "") override;
        void removeBuffer(Buffer* buffer) override;

        void removeSampler(Sampler* sampler) override;

        const DeviceCaps* getDeviceCaps() const override;

    private:

        virtual void clearBackbuffer(const core::Vector4D & color) override;

        Framebuffer* createFramebuffer(const std::vector<Image*>& attachments, const core::Dimension2D& size) override;
        RenderPass* createRenderPass(const RenderPassDescription* renderpassDesc) override;
        Pipeline* createPipeline(Pipeline::PipelineType type) override;
        Sampler* createSampler() override;

        bool initialize() override;
        void destroy() override;

    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace renderer
} //namespace v3d
