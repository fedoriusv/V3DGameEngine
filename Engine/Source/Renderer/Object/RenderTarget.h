#pragma once

#include "Common.h"
#include "Object.h"
#include "Utils/Observable.h"
#include "Renderer/Formats.h"
#include "Renderer/TextureProperties.h"
#include "Renderer/ObjectTracker.h"
#include "Renderer/CommandList.h"

namespace v3d
{
namespace renderer
{
    class Texture2D;
    class Backbuffer;

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * RenderTarget class. Game side
    */
    class RenderTarget : public Object
    {
    public:

        RenderTarget() = delete;
        RenderTarget(const RenderTarget&) = delete;
        ~RenderTarget();

        bool setColorTexture(u32 index, Texture2D* colorTexture,
            RenderTargetLoadOp loadOp = RenderTargetLoadOp::LoadOp_Clear, RenderTargetStoreOp storeOp = RenderTargetStoreOp::StoreOp_Store,
            const core::Vector4D& clearColor = core::Vector4D(0.f));

        bool setColorTexture(u32 index, Backbuffer* swapchainTexture,
            RenderTargetLoadOp loadOp = RenderTargetLoadOp::LoadOp_Clear, RenderTargetStoreOp storeOp = RenderTargetStoreOp::StoreOp_Store,
            const core::Vector4D& clearColor = core::Vector4D(0.f));

        bool setDepthStencilTexture(Texture2D* depthStencilTexture, 
            RenderTargetLoadOp depthLoadOp = RenderTargetLoadOp::LoadOp_DontCare, RenderTargetStoreOp depthStoreOp = RenderTargetStoreOp::StoreOp_DontCare,
            f32 clearDepth = 0.0f,
            RenderTargetLoadOp stencilLoadOp = RenderTargetLoadOp::LoadOp_DontCare, RenderTargetStoreOp stencilStoreOp = RenderTargetStoreOp::StoreOp_DontCare,
            u32 clearStencil = 0);

        Texture2D* getColorTexture(u32 index) const;
        Texture2D* getDepthStencilTexture() const;
        const core::Dimension2D& getDimension() const;

        u32 getColorTextureCount() const;
        bool hasDepthStencilTexture() const;

    private:

        RenderTarget(renderer::CommandList& cmdList, const core::Dimension2D& size) noexcept;

        void extractRenderTargetInfo(RenderPass::RenderPassInfo& renderpassInfo, std::vector<Image*>& attachments, RenderPass::ClearValueInfo& clearInfo) const;
        void destroyFramebuffers(const std::vector<Framebuffer*>& framebuffers);
        void destroyRenderPasses(const std::vector<RenderPass*>& renderPasses);

        friend CommandList;
        CommandList& m_cmdList;

        core::Dimension2D m_size;

        std::map<u32, std::tuple<Texture2D*, renderer::AttachmentDescription, core::Vector4D>>   m_colorTextures;
        std::tuple<Texture2D*, renderer::AttachmentDescription, f32, u32>                        m_depthStencilTexture;

        ObjectTracker<Framebuffer>  m_trackerFramebuffer;
        ObjectTracker<RenderPass>   m_trackerRenderpass;
    };

     /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * Backbuffer class
    */
    class Backbuffer : public Object
    {
    public:

        ~Backbuffer();
        Backbuffer(const Backbuffer&) = delete;

        const core::Dimension2D& getDimension() const;
        renderer::Format         getFormat() const;

        void read(const core::Dimension2D& offset, const core::Dimension2D& size, void* const data);
        void clear(const core::Vector4D& color);

    private:

        Backbuffer(renderer::CommandList& cmdList) noexcept;

        friend CommandList;
        CommandList&  m_cmdList;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace renderer
} //namespace v3d
