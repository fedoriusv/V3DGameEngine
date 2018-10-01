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

        struct AttachmentDesc
        {
            renderer::ImageFormat _format;
            TextureSamples        _samples;
            RenderTargetLoadOp    _loadOp;
            RenderTargetStoreOp   _storeOp;
        };

        ~RenderTarget();
        RenderTarget(const RenderTarget &) = delete;

        bool attachColorTexture(Texture2D* colorTexture, RenderTargetLoadOp loadOp = RenderTargetLoadOp::LoadOp_Clear, RenderTargetStoreOp storeOp = RenderTargetStoreOp::StoreOp_Store);
        bool attachDepthStencilTexture(Texture2D* depthStencilTexture, RenderTargetLoadOp loadOp = RenderTargetLoadOp::LoadOp_DontCare, RenderTargetStoreOp storeOp = RenderTargetStoreOp::StoreOp_DontCare);

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

} //namespace v3d
