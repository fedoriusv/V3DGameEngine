#pragma once

#include "Common.h"
#include "CommandList.h"
#include "ImageFormats.h"
#include "Object/TextureEnums.h"

namespace v3d
{
  /////////////////////////////////////////////////////////////////////////////////////////////////////

    class Framebuffer;
    class RenderPass;

    class RenderTarget : public Object //ref couter, 
    {
    public:

        ~RenderTarget();
        RenderTarget(const RenderTarget &) = delete;

        bool setColorTexture(Texture2D* colorTexture, RenderTargetLoadOp loadOp = RenderTargetLoadOp::LoadOp_Clear, RenderTargetStoreOp storeOp = RenderTargetStoreOp::StoreOp_Store);
        bool setDepthStencilTexture(Texture2D* depthStencilTexture, RenderTargetLoadOp loadOp = RenderTargetLoadOp::LoadOp_DontCare, RenderTargetStoreOp storeOp = RenderTargetStoreOp::StoreOp_DontCare);

        Texture2D* getColorTexture(u32 attachment) const;
        Texture2D* getDepthStencilTexture() const;

    private:

        RenderTarget(renderer::CommandList& cmdList, const core::Dimension2D& dimension);

        renderer::CommandList&  m_cmdList;
        core::Dimension2D       m_dimension;

        std::vector<std::pair<Texture2D*, AttachmentDesc>> m_colorTextures;
        std::pair<Texture2D*, AttachmentDesc>              m_depthStencilTexture;

        Framebuffer* m_framebuffer;
        RenderPass*  m_renderpass;

        friend renderer::CommandList;
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

} //namespace v3d
