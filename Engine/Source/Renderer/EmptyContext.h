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

        virtual void clearBackbuffer(const core::Vector4D & color) override;

        void setViewport(const core::Rect32& viewport) override;

        Image* createImage(TextureTarget target, renderer::Format format, const core::Dimension3D& dimension, u32 mipmapLevel, s16 filter, TextureAnisotropic anisotropicLevel, TextureWrap wrap) const override;
        Image* createAttachmentImage(renderer::Format format, const core::Dimension3D& dimension, TextureSamples samples, s16 filter, TextureAnisotropic anisotropicLevel, TextureWrap wrap) const override;

        void setRenderTarget(const RenderPass::RenderPassInfo* renderpassInfo, const std::vector<Image*>& attachments, const RenderPass::ClearValueInfo* clearInfo) override;
        void removeRenderTarget(const RenderPass::RenderPassInfo * renderpassInfo, const std::vector<Image*>& attachments, const RenderPass::ClearValueInfo * clearInfo) override;

        void setPipeline(const GraphicsPipelineState::GraphicsPipelineStateDesc* pipelineInfo,
            const ShaderProgram::ShaderProgramInfo* programInfo, const RenderPass::RenderPassInfo* renderpassInfo) override;

        const DeviceCaps* getDeviceCaps() const override;

    private:

        Framebuffer* createFramebuffer(const std::vector<Image*>& attachments, const core::Dimension2D& size) override;
        RenderPass* createRenderPass(const RenderPass::RenderPassInfo* renderpassInfo) override;
        Pipeline* createPipeline(const GraphicsPipelineState::GraphicsPipelineStateDesc* pipelineInfo,
            const ShaderProgram::ShaderProgramInfo* programInfo, const RenderPass::RenderPassInfo* renderpassInfo) override;

        bool initialize() override;
        void destroy() override;

    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace renderer
} //namespace v3d
