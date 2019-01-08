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
    /////////////////////////////////////////////////////////////////////////////////////////////////////
    
    class Texture2D;

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    class RenderTarget : public Object //ref counter
    {
    public:

        ~RenderTarget();
        RenderTarget(const RenderTarget &) = delete;

        bool setColorTexture(u32 index, Texture2D* colorTexture,
            RenderTargetLoadOp loadOp = RenderTargetLoadOp::LoadOp_Clear, RenderTargetStoreOp storeOp = RenderTargetStoreOp::StoreOp_Store,
            const core::Vector4D& clearColor = core::Vector4D(0.f));

        bool setDepthStencilTexture(Texture2D* depthStencilTexture, 
            RenderTargetLoadOp depthLoadOp = RenderTargetLoadOp::LoadOp_DontCare, RenderTargetStoreOp depthStoreOp = RenderTargetStoreOp::StoreOp_DontCare,
            f32 clearDepth = 0.0f,
            RenderTargetLoadOp stencilLoadOp = RenderTargetLoadOp::LoadOp_DontCare, RenderTargetStoreOp stencilStoreOp = RenderTargetStoreOp::StoreOp_DontCare,
            u32 clearStencil = 0);

        Texture2D* getColorTexture(u32 index) const;
        Texture2D* getDepthStencilTexture() const;

        u32        getColorTextureCount() const;
        bool       hasDepthStencilTexture() const;

    private:

        RenderTarget(renderer::CommandList& cmdList, const core::Dimension2D& size);

        renderer::CommandList&  m_cmdList;
        core::Dimension2D       m_size;

        std::map<u32, std::tuple<Texture2D*, renderer::AttachmentDescription, core::Vector4D>>   m_colorTextures;
        std::tuple<Texture2D*, renderer::AttachmentDescription, f32, u32>                        m_depthStencilTexture;

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

} //namespace renderer
} //namespace v3d
