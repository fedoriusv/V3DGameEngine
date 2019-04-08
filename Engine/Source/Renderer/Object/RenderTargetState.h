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
    * RenderTargetState class. Game side
    */
    class RenderTargetState : public Object
    {
    public:

        RenderTargetState() = delete;
        RenderTargetState(const RenderTargetState&) = delete;
        ~RenderTargetState();

        struct ColorOpState
        {
            ColorOpState()
                : _loadOp(RenderTargetLoadOp::LoadOp_Clear)
                , _storeOp(RenderTargetStoreOp::StoreOp_Store)
                , _clearColor(core::Vector4D(0.f))
            {
            }
            
            ColorOpState(RenderTargetLoadOp loadOp, RenderTargetStoreOp storeOp, const core::Vector4D& clearColor)
                : _loadOp(loadOp)
                , _storeOp(storeOp)
                , _clearColor(clearColor)
            {
            }

            RenderTargetLoadOp      _loadOp;
            RenderTargetStoreOp     _storeOp;
            const core::Vector4D    _clearColor;
        };

        struct DepthOpState
        {
            DepthOpState()
                : _loadOp(RenderTargetLoadOp::LoadOp_Clear)
                , _storeOp(RenderTargetStoreOp::StoreOp_DontCare)
                , _clearDepth(1.0f)
            {
            }

            DepthOpState(RenderTargetLoadOp loadOp, RenderTargetStoreOp storeOp, f32 clearDepth = 1.0f)
                : _loadOp(loadOp)
                , _storeOp(storeOp)
                , _clearDepth(clearDepth)
            {
            }

            RenderTargetLoadOp   _loadOp;
            RenderTargetStoreOp  _storeOp;
            f32                  _clearDepth;
        };

        struct StencilOpState
        {
            StencilOpState()
                : _loadOp(RenderTargetLoadOp::LoadOp_Clear)
                , _storeOp(RenderTargetStoreOp::StoreOp_DontCare)
                , _clearStencil(0U)
            {
            }

            StencilOpState(RenderTargetLoadOp loadOp, RenderTargetStoreOp storeOp, u32 clearStencil = 0U)
                : _loadOp(loadOp)
                , _storeOp(storeOp)
                , _clearStencil(clearStencil)
            {
            }

            RenderTargetLoadOp   _loadOp;
            RenderTargetStoreOp  _storeOp;
            u32                  _clearStencil;
        };

        struct TransitionState
        {
            TransitionState(TransitionOp initialState, TransitionOp finalState)
                : _initialState(initialState)
                , _finalState(finalState)
            {
            }

            TransitionOp _initialState;
            TransitionOp _finalState;
        };


        bool setColorTexture(u32 index, Texture2D* colorTexture,
            RenderTargetLoadOp loadOp = RenderTargetLoadOp::LoadOp_Clear, RenderTargetStoreOp storeOp = RenderTargetStoreOp::StoreOp_Store,
            const core::Vector4D& clearColor = core::Vector4D(0.f));

        bool setColorTexture(u32 index, Backbuffer* swapchainTexture,
            RenderTargetLoadOp loadOp = RenderTargetLoadOp::LoadOp_Clear, RenderTargetStoreOp storeOp = RenderTargetStoreOp::StoreOp_Store,
            const core::Vector4D& clearColor = core::Vector4D(0.f));

        bool setDepthStencilTexture(Texture2D* depthStencilTexture, 
            RenderTargetLoadOp depthLoadOp = RenderTargetLoadOp::LoadOp_DontCare, RenderTargetStoreOp depthStoreOp = RenderTargetStoreOp::StoreOp_DontCare,
            f32 clearDepth = 1.0f,
            RenderTargetLoadOp stencilLoadOp = RenderTargetLoadOp::LoadOp_DontCare, RenderTargetStoreOp stencilStoreOp = RenderTargetStoreOp::StoreOp_DontCare,
            u32 clearStencil = 0);

        bool setColorTexture(u32 index, Texture2D* colorTexture, 
            const ColorOpState& colorOpState, const TransitionState& tansitionState);

        bool setColorTexture(u32 index, Backbuffer* swapchainTexture,
            const ColorOpState& colorOpState, const TransitionState& tansitionState);

        bool setDepthStencilTexture(Texture2D* depthStencilTexture,
            const DepthOpState& depthOpState, const StencilOpState& stencilOpState, const TransitionState& tansitionState);

        Texture2D* getColorTexture(u32 index) const;
        Texture2D* getDepthStencilTexture() const;
        const core::Dimension2D& getDimension() const;

        u32 getColorTextureCount() const;
        bool hasDepthStencilTexture() const;

    private:

        RenderTargetState(renderer::CommandList& cmdList, const core::Dimension2D& size) noexcept;

        void extractRenderTargetInfo(RenderPassDescription& renderpassDesc, std::vector<Image*>& attachments, Framebuffer::ClearValueInfo& clearInfo) const;
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
