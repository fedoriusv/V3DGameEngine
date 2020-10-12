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
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    class Texture2D;
    class TextureCube;
    class Backbuffer;

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief RenderTargetState class. Game side
    */
    class RenderTargetState : public Object
    {
    public:

        RenderTargetState() = delete;
        RenderTargetState(const RenderTargetState&) = delete;
        ~RenderTargetState();

        /**
        * @brief ColorOpState struct
        */
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

        /**
        * @brief DepthOpState struct
        */
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

        /**
        * @brief StencilOpState struct
        */
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

        /**
        * @brief TransitionState struct
        */
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


        /**
        * @brief setColorTexture method. Used to adding a color attachment to a render target
        *
        * @param u32 index [required]. Attaches the texture to index slot in a shader
        * @param Texture2D* colorTexture [required]
        * @param RenderTargetLoadOp loadOp [optional]
        * @param RenderTargetStoreOp storeOp [optional]
        * @param const core::Vector4D& clearColor [optional]
        * 
        * @return true if compatibility is successed
        */
        bool setColorTexture(u32 index, Texture2D* colorTexture,
            RenderTargetLoadOp loadOp = RenderTargetLoadOp::LoadOp_Clear, RenderTargetStoreOp storeOp = RenderTargetStoreOp::StoreOp_Store,
            const core::Vector4D& clearColor = core::Vector4D(0.f));

        /**
        * @brief setColorTexture method. Used to adding a backbuffer to a render target
        *
        * @param u32 index [required]. Attaches the backbuffer to index slot in a shader
        * @param Backbuffer* swapchainTexture [required]
        * @param RenderTargetLoadOp loadOp [optional]
        * @param RenderTargetStoreOp storeOp [optional]
        * @param const core::Vector4D& clearColor [optional]
        *
        * @return true if compatibility is successed
        */
        bool setColorTexture(u32 index, Backbuffer* swapchainTexture,
            RenderTargetLoadOp loadOp = RenderTargetLoadOp::LoadOp_Clear, RenderTargetStoreOp storeOp = RenderTargetStoreOp::StoreOp_Store,
            const core::Vector4D& clearColor = core::Vector4D(0.f));

        /**
        * @brief setColorTexture method. Used to adding a color attachment of cubemap to render target
        *
        * @param u32 index [required]. Attaches the cubemap texture to index slot in a shader
        * @param Texture2D* colorCubeTexture [required]
        * @param s32 layer [required]
        * @param RenderTargetLoadOp loadOp [optional]
        * @param RenderTargetStoreOp storeOp [optional]
        * @param const core::Vector4D& clearColor [optional]
        *
        * @return true if compatibility is successed
        */
        bool setColorTexture(u32 index, TextureCube* colorCubeTexture, s32 layer,
            RenderTargetLoadOp loadOp = RenderTargetLoadOp::LoadOp_Clear, RenderTargetStoreOp storeOp = RenderTargetStoreOp::StoreOp_Store,
            const core::Vector4D& clearColor = core::Vector4D(0.f));

        /**
        * @brief setDepthStencilTexture method. Used to adding a depth-stencil attachment to a render target
        *
        * @param Texture2D* depthStencilTexture [required]
        * @param RenderTargetLoadOp depthLoadOp [optional]
        * @param RenderTargetStoreOp depthStoreOp [optional]
        * @param f32 clearDepth [optional]
        * @param RenderTargetLoadOp stencilLoadOp [optional]
        * @param RenderTargetStoreOp stencilStoreOp [optional]
        * @param f32 clearStencil [optional]
        * 
        * @return true if compatibility is successed
        */
        bool setDepthStencilTexture(Texture2D* depthStencilTexture, 
            RenderTargetLoadOp depthLoadOp = RenderTargetLoadOp::LoadOp_DontCare, RenderTargetStoreOp depthStoreOp = RenderTargetStoreOp::StoreOp_DontCare,
            f32 clearDepth = 1.0f,
            RenderTargetLoadOp stencilLoadOp = RenderTargetLoadOp::LoadOp_DontCare, RenderTargetStoreOp stencilStoreOp = RenderTargetStoreOp::StoreOp_DontCare,
            u32 clearStencil = 0);

        /**
        * @brief setDepthStencilTexture method. Used to adding a depth-stencil attachment of cubemap to a render target
        *
        * @param TextureCube* depthStencilCubeTexture [required]
        * @param s32 layer [required]
        * @param RenderTargetLoadOp depthLoadOp [optional]
        * @param RenderTargetStoreOp depthStoreOp [optional]
        * @param f32 clearDepth [optional]
        * @param RenderTargetLoadOp stencilLoadOp [optional]
        * @param RenderTargetStoreOp stencilStoreOp [optional]
        * @param f32 clearStencil [optional]
        *
        * @return true if compatibility is successed
        */
        bool setDepthStencilTexture(TextureCube* depthStencilCubeTexture, s32 layer,
            RenderTargetLoadOp depthLoadOp = RenderTargetLoadOp::LoadOp_DontCare, RenderTargetStoreOp depthStoreOp = RenderTargetStoreOp::StoreOp_DontCare,
            f32 clearDepth = 1.0f,
            RenderTargetLoadOp stencilLoadOp = RenderTargetLoadOp::LoadOp_DontCare, RenderTargetStoreOp stencilStoreOp = RenderTargetStoreOp::StoreOp_DontCare,
            u32 clearStencil = 0);

        /**
        * @brief setColorTexture method. Used to adding a color attachment to a render target
        *
        * @param u32 index [required]. Attaches the texture to index slot in a shader
        * @param Texture2D* colorTexture [required]
        * @param const ColorOpState& colorOpState [required]
        * @param const TransitionState& tansitionState [required]
        * @see TransitionState
        *
        * @return true if compatibility is successed
        */
        bool setColorTexture(u32 index, Texture2D* colorTexture, 
            const ColorOpState& colorOpState, const TransitionState& tansitionState);

        /**
        * @brief setColorTexture method. Used to adding a backbuffer to a render target
        *
        * @param u32 index [required]. Attaches the backbuffer to index slot in a shader
        * @param Backbuffer* swapchainTexture [required]
        * @param const ColorOpState& colorOpState [required]
        * @param const TransitionState& tansitionState [required]
        * @see TransitionState
        *
        * @return true if compatibility is successed
        */
        bool setColorTexture(u32 index, Backbuffer* swapchainTexture,
            const ColorOpState& colorOpState, const TransitionState& tansitionState);

        /**
        * @brief setColorTexture method. Used to adding a color attachment of cubemap layer to a render target
        *
        * @param u32 index [required]. Attaches the texture to index slot in a shader
        * @param TextureCube* colorCubeTexture [required]
        * @param s32 layer [required]
        * @param const ColorOpState& colorOpState [required]
        * @param const TransitionState& tansitionState [required]
        * @see TransitionState
        *
        * @return true if compatibility is successed
        */
        bool setColorTexture(u32 index, TextureCube* colorCubeTexture, s32 layer,
            const ColorOpState& colorOpState, const TransitionState& tansitionState);

        /**
        * @brief setDepthStencilTexture method. Used to adding a depth-stencil attachment to a render target
        *
        * @param Texture2D* depthStencilTexture [required]
        * @param const DepthOpState& depthOpState [required]
        * @param const StencilOpState& stencilOpState [required]
        * @param const TransitionState& tansitionState [required]
        * @see TransitionState
        *
        * @return true if compatibility is successed
        */
        bool setDepthStencilTexture(Texture2D* depthStencilTexture,
            const DepthOpState& depthOpState, const StencilOpState& stencilOpState, const TransitionState& tansitionState);

        /**
        * @brief setDepthStencilTexture method. Used to adding a depth-stencil attachment of cubemap layer to a render target
        *
        * @param Texture2D* depthStencilTexture [required]
        * @param s32 layer [required]
        * @param const DepthOpState& depthOpState [required]
        * @param const StencilOpState& stencilOpState [required]
        * @param const TransitionState& tansitionState [required]
        * @see TransitionState
        *
        * @return true if compatibility is successed
        */
        bool setDepthStencilTexture(TextureCube* depthStencilCubeTexture, s32 layer,
            const DepthOpState& depthOpState, const StencilOpState& stencilOpState, const TransitionState& tansitionState);

        template<class TTexture>
        TTexture* getColorTexture(u32 index) const
        {
            ASSERT(index < m_colorTextures.size(), "out of range");

            static_assert(std::is_base_of<Texture, TTexture>(), "wrong type");
            auto attachment = m_colorTextures.find(index);
            if (attachment == m_colorTextures.cend())
            {
                return nullptr;
            }

            return static_cast<TTexture*>(std::get<0>((*attachment).second));
        }
        
        template<class TTexture>
        TTexture* getDepthStencilTexture() const
        {
            static_assert(std::is_base_of<Texture, TTexture>(), "wrong type");
            return static_cast<TTexture*>(std::get<0>(m_depthStencilTexture));
        }

        const core::Dimension2D& getDimension() const;

        u32 getColorTextureCount() const;
        bool hasDepthStencilTexture() const;

    private:

        /**
        * @brief RenderTargetState constructor. Used for creating a render target
        * Private method. Use createObject interface inside CommandList class to call.
        *
        * @param const core::Dimension2D& size [required]
        * @param const std::string& name [optional]
        */
        RenderTargetState(CommandList& cmdList, const core::Dimension2D& size, [[maybe_unused]] const std::string& name = "") noexcept;

        void extractRenderTargetInfo(RenderPassDescription& renderpassDesc, std::vector<Image*>& attachments, Framebuffer::ClearValueInfo& clearInfo) const;
        void destroyFramebuffers(const std::vector<Framebuffer*>& framebuffers);
        void destroyRenderPasses(const std::vector<RenderPass*>& renderPasses);

        bool checkCompatibility(Texture* texture, AttachmentDescription& desc);

        friend CommandList;
        CommandList& m_cmdList;

        core::Dimension2D m_size;

        std::map<u32, std::tuple<Texture*, renderer::AttachmentDescription, core::Vector4D>> m_colorTextures;
        std::tuple<Texture*, renderer::AttachmentDescription, f32, u32>                      m_depthStencilTexture;

        ObjectTracker<Framebuffer>  m_trackerFramebuffer;
        ObjectTracker<RenderPass>   m_trackerRenderpass;
    };

     /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief Backbuffer class. Wraps swapchain images
    */
    class Backbuffer : public Object
    {
    public:

        Backbuffer() = delete;
        Backbuffer(const Backbuffer&) = delete;
        ~Backbuffer();

        const core::Dimension2D& getDimension() const;
        renderer::Format         getFormat() const;

        void read(const core::Dimension2D& offset, const core::Dimension2D& size, void* const data);
        void clear(const core::Vector4D& color);

    private:

        /**
        * @brief Backbuffer constructor. Used for creating a backbuffer.
        * Private method. Creates inside CommandList object.
        */
        Backbuffer(renderer::CommandList& cmdList) noexcept;

        friend CommandList;
        CommandList&  m_cmdList;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace renderer
} //namespace v3d
