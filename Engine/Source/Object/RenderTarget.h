#pragma once

#include "Common.h"
#include "CommandList.h"
#include "ImageFormats.h"
#include "TextureProperties.h"
#include "Utils/Observable.h"

namespace v3d
{
namespace renderer
{
    class Framebuffer;
    class RenderPass;
    class RenderPassManager;
    class Texture2D;

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    class RenderTarget : public Object, public utils::Observer //ref counter
    {
    public:

        ~RenderTarget();
        RenderTarget(const RenderTarget &) = delete;

        bool setColorTexture(Texture2D* colorTexture, 
            RenderTargetLoadOp loadOp = RenderTargetLoadOp::LoadOp_Clear, RenderTargetStoreOp storeOp = RenderTargetStoreOp::StoreOp_Store);

        bool setDepthStencilTexture(Texture2D* depthStencilTexture, 
            RenderTargetLoadOp depthLoadOp = RenderTargetLoadOp::LoadOp_DontCare, RenderTargetStoreOp DepthStoreOp = RenderTargetStoreOp::StoreOp_DontCare, 
            RenderTargetLoadOp stencilLoadOp = RenderTargetLoadOp::LoadOp_DontCare, RenderTargetStoreOp stencilStoreOp = RenderTargetStoreOp::StoreOp_DontCare);

        Texture2D* getColorTexture(u32 attachment) const;
        Texture2D* getDepthStencilTexture() const;

    private:

        RenderTarget(renderer::CommandList& cmdList, const core::Dimension2D& size);

        void makeRenderTarget();
        void handleNotify(utils::Observable* ob) override;

        renderer::CommandList&  m_cmdList;
        core::Dimension2D       m_size;

        std::vector<std::pair<Texture2D*, renderer::AttachmentDescription>> m_colorTextures;
        std::pair<Texture2D*, renderer::AttachmentDescription>              m_depthStencilTexture;

        renderer::Framebuffer* m_framebuffer;
        renderer::RenderPass*  m_renderpass;

        friend renderer::CommandList;

        static renderer::RenderPassManager* s_renderpassManager;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    class Backbuffer : public Object //ref couter,
    {
    public:

        ~Backbuffer() {};
        Backbuffer(const Backbuffer &) = delete;

    private:

        Backbuffer(renderer::CommandList& cmdList, SwapchainTexture* texture);

        renderer::CommandList&  m_cmdList;
        SwapchainTexture*       m_texture;

        friend renderer::CommandList;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace renderer
} //namespace v3d
