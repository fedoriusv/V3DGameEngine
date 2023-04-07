#pragma once

#include "Context.h"

namespace v3d
{
namespace renderer
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief EmptyContext class
    */
    class EmptyContext final : public Context
    {
    public:

        EmptyContext() noexcept;
        ~EmptyContext();

        const DeviceCaps* getDeviceCaps() const override;

        void beginFrame() override;
        void endFrame() override;
        void presentFrame() override;
        void submit(bool wait = false) override;

        void beginQuery(const Query* query, u32 id, const std::string& tag = "") override;
        void endQuery(const Query* query, u32 id) override;
        void timestampQuery(const Query* query, u32 id, const std::string& tag = "") override;

        void draw(const StreamBufferDescription& desc, u32 firstVertex, u32 vertexCount, u32 firstInstance, u32 instanceCount) override;
        void drawIndexed(const StreamBufferDescription& desc, u32 firstIndex, u32 indexCount, u32 firstInstance, u32 instanceCount) override;

        void dispatchCompute(const math::Dimension3D& groups) override;

        void bindImage(const Shader* shader, u32 bindIndex, const Image* image, s32 layer = k_generalLayer, s32 mip = k_allMipmapsLevels) override;
        void bindSampler(const Shader* shader, u32 bindIndex, const Sampler::SamplerInfo* samplerInfo) override;
        void bindSampledImage(const Shader* shader, u32 bindIndex, const Image* image, const Sampler::SamplerInfo* samplerInfo, s32 layer = k_generalLayer, s32 mip = k_allMipmapsLevels) override;
        void bindUniformsBuffer(const Shader* shader, u32 bindIndex, u32 offset, u32 size, const void* data) override;
        void bindStorageImage(const Shader* shader, u32 bindIndex, const Image* image, s32 layer = k_generalLayer, s32 mip = k_allMipmapsLevels) override;

        void transitionImages(std::vector<std::tuple<const Image*, Image::Subresource>>& images, TransitionOp transition) override;

        void setViewport(const math::Rect32& viewport, const math::Vector2D& depth = { 0.0f, 1.0f }) override;
        void setScissor(const math::Rect32& scissor) override;

        void setRenderTarget(const RenderPass::RenderPassInfo* renderpassInfo, const Framebuffer::FramebufferInfo* framebufferInfo) override;
        void invalidateRenderTarget() override;

        void setPipeline(const Pipeline::PipelineGraphicInfo* pipelineInfo) override;
        void setPipeline(const Pipeline::PipelineComputeInfo* pipelineInfo) override;

        void clearRenderTarget(const std::vector<const Image*>& images, Framebuffer::ClearValueInfo& clearValues) override;

        [[nodiscard]] Image* createImage(TextureTarget target, Format format, const math::Dimension3D& dimension, u32 layers, u32 mipmapLevel, TextureUsageFlags flags, [[maybe_unused]] const std::string& name = "") override;
        [[nodiscard]] Image* createImage(TextureTarget target, Format format, const math::Dimension3D& dimension, u32 layers, TextureSamples samples, TextureUsageFlags flags, [[maybe_unused]] const std::string& name = "") override;
        void removeImage(Image* image) override;

        [[nodiscard]] Buffer* createBuffer(Buffer::BufferType type, u16 usageFlag, u64 size, [[maybe_unused]] const std::string& name = "") override;
        void removeBuffer(Buffer* buffer) override;

        [[nodiscard]] Sampler* createSampler(const SamplerDescription& desc) override;
        void removeSampler(Sampler* sampler) override;

        [[nodiscard]] Pipeline* createPipeline(Pipeline::PipelineType type, [[maybe_unused]] const std::string& name = "") override;
        void removePipeline(Pipeline* pipeline) override;

        [[nodiscard]] Framebuffer* createFramebuffer(const std::vector<Image*>& attachments, const math::Dimension2D& size) override;
        void removeFramebuffer(Framebuffer* framebuffer) override;

        [[nodiscard]] RenderPass* createRenderPass(const RenderPassDescription* renderpassDesc) override;
        void removeRenderPass(RenderPass* renderpass) override;

        [[nodiscard]] Query* createQuery(QueryType type, u32 size, const Query::QueryRespose& callback, [[maybe_unused]] const std::string& name = "") override;
        void removeQuery(Query* query) override;

        void clearBackbuffer(const math::Vector4D& color) override;
        void generateMipmaps(Image* image, u32 layer, TransitionOp state) override;

    private:

        bool initialize() override;
        void destroy() override;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace renderer
} //namespace v3d
