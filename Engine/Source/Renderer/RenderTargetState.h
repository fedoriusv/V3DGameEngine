#pragma once

#include "Common.h"
#include "Object.h"
#include "Utils/Observable.h"

#include "TextureProperties.h"
#include "ObjectTracker.h"
#include "CommandList.h"

#include "Core/Framebuffer.h"

namespace v3d
{
namespace renderer
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    class Texture2D;
    class TextureCube;
    class Texture2DArray;
    class Backbuffer;
    class Framebuffer;
    class RenderPass;

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief RenderTargetState class. Game side
    */
    class RenderTargetState : public Object
    {
    public:

        /**
        * @brief RenderTargetState desctructor
        */
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

            DepthOpState(RenderTargetLoadOp loadOp, RenderTargetStoreOp storeOp, f32 clearDepth = 0.f)
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
        * @param TTexture* colorTexture [required]
        * @param RenderTargetLoadOp loadOp [optional]
        * @param RenderTargetStoreOp storeOp [optional]
        * @param const core::Vector4D& clearColor [optional]
        *
        * @return true if compatibility is successed
        */
        template<class TTexture>
        bool setColorTexture(u32 index, TTexture* colorTexture,
            RenderTargetLoadOp loadOp = RenderTargetLoadOp::LoadOp_Clear, RenderTargetStoreOp storeOp = RenderTargetStoreOp::StoreOp_Store,
            const core::Vector4D& clearColor = core::Vector4D(0.f));

        /**
        * @brief setColorTexture method. Used to adding a layer of color attachment to render target
        *
        * @param u32 index [required]. Attaches the cubemap texture to index slot in a shader
        * @param TTexture* colorTexture [required]
        * @param s32 layer [required]
        * @param RenderTargetLoadOp loadOp [optional]
        * @param RenderTargetStoreOp storeOp [optional]
        * @param const core::Vector4D& clearColor [optional]
        *
        * @return true if compatibility is successed
        */
        template<class TTexture>
        bool setColorTexture(u32 index, TTexture* colorTexture, s32 layer,
            RenderTargetLoadOp loadOp = RenderTargetLoadOp::LoadOp_Clear, RenderTargetStoreOp storeOp = RenderTargetStoreOp::StoreOp_Store,
            const core::Vector4D& clearColor = core::Vector4D(0.f));


        /**
        * @brief setDepthStencilTexture method. Used to adding a depth-stencil attachment to a render target
        *
        * @param TTexture* depthStencilTexture [required]
        * @param RenderTargetLoadOp depthLoadOp [optional]
        * @param RenderTargetStoreOp depthStoreOp [optional]
        * @param f32 clearDepth [optional]
        * @param RenderTargetLoadOp stencilLoadOp [optional]
        * @param RenderTargetStoreOp stencilStoreOp [optional]
        * @param f32 clearStencil [optional]
        * 
        * @return true if compatibility is successed
        */
        template<class TTexture>
        bool setDepthStencilTexture(TTexture* depthStencilTexture,
            RenderTargetLoadOp depthLoadOp = RenderTargetLoadOp::LoadOp_DontCare, RenderTargetStoreOp depthStoreOp = RenderTargetStoreOp::StoreOp_DontCare,
            f32 clearDepth = 0.f,
            RenderTargetLoadOp stencilLoadOp = RenderTargetLoadOp::LoadOp_DontCare, RenderTargetStoreOp stencilStoreOp = RenderTargetStoreOp::StoreOp_DontCare,
            u32 clearStencil = 0);

        /**
        * @brief setDepthStencilTexture method. Used to adding a layer of depth-stencil attachment to a render target
        *
        * @param TTexture* depthStencilTexture [required]
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
        template<class TTexture>
        bool setDepthStencilTexture(TTexture* depthStencilTexture, s32 layer,
            RenderTargetLoadOp depthLoadOp = RenderTargetLoadOp::LoadOp_DontCare, RenderTargetStoreOp depthStoreOp = RenderTargetStoreOp::StoreOp_DontCare,
            f32 clearDepth = 0.f,
            RenderTargetLoadOp stencilLoadOp = RenderTargetLoadOp::LoadOp_DontCare, RenderTargetStoreOp stencilStoreOp = RenderTargetStoreOp::StoreOp_DontCare,
            u32 clearStencil = 0);

        /**
        * @brief setColorTexture method. Used to adding a color attachment to a render target
        *
        * @param u32 index [required]. Attaches the texture to index slot in a shader
        * @param TTexture* colorTexture [required]
        * @param const ColorOpState& colorOpState [required]
        * @param const TransitionState& tansitionState [required]
        * @see TransitionState
        *
        * @return true if compatibility is successed
        */
        template<class TTexture>
        bool setColorTexture(u32 index, TTexture* colorTexture,
            const ColorOpState& colorOpState, const TransitionState& tansitionState);

        /**
        * @brief setColorTexture method. Used to adding a layer of color attachment to a render target
        *
        * @param u32 index [required]. Attaches the texture to index slot in a shader
        * @param TTexture* colorTexture [required]
        * @param s32 layer [required]
        * @param const ColorOpState& colorOpState [required]
        * @param const TransitionState& tansitionState [required]
        * @see TransitionState
        *
        * @return true if compatibility is successed
        */
        template<class TTexture>
        bool setColorTexture(u32 index, TTexture* colorTexture, s32 layer,
            const ColorOpState& colorOpState, const TransitionState& tansitionState);

        /**
        * @brief setDepthStencilTexture method. Used to adding a depth-stencil attachment to a render target
        *
        * @param TTexture* depthStencilTexture [required]
        * @param const DepthOpState& depthOpState [required]
        * @param const StencilOpState& stencilOpState [required]
        * @param const TransitionState& tansitionState [required]
        * @see TransitionState
        *
        * @return true if compatibility is successed
        */
        template<class TTexture>
        bool setDepthStencilTexture(TTexture* depthStencilTexture,
            const DepthOpState& depthOpState, const StencilOpState& stencilOpState, const TransitionState& tansitionState);

        /**
        * @brief setDepthStencilTexture method. Used to adding a layer of depth-stencil attachment to a render target
        *
        * @param TTexture* depthStencilTexture [required]
        * @param s32 layer [required]
        * @param const DepthOpState& depthOpState [required]
        * @param const StencilOpState& stencilOpState [required]
        * @param const TransitionState& tansitionState [required]
        * @see TransitionState
        *
        * @return true if compatibility is successed
        */
        template<class TTexture>
        bool setDepthStencilTexture(TTexture* depthStencilTexture, s32 layer,
            const DepthOpState& depthOpState, const StencilOpState& stencilOpState, const TransitionState& tansitionState);

        /**
        * @brief getColorTexture method
        *
        * @param u32 index [required]
        * @return color texture form index slot
        */
        template<class TTexture>
        TTexture* getColorTexture(u32 index) const;

        /**
        * @brief getDepthStencilTexture method
        *
        * @return depth-stencil texture
        */
        template<class TTexture>
        TTexture* getDepthStencilTexture() const;

        /**
        * @brief getDimension method. Size of texture
        * @return core::Dimension2D
        */
        const core::Dimension2D& getDimension() const;

        /**
        * @brief getColorTextureCount method
        * @return u32. Count of color attachment
        */
        u32 getColorTextureCount() const;

        /**
        * @brief hasDepthStencilTexture method
        * @return bool. Is depth/stencil attachment is presented
        */
        bool hasDepthStencilTexture() const;

    private:

        /**
        * @brief RenderTargetState constructor. Used for creating a render target
        * Private method. Use createObject interface inside CommandList class to call.
        *
        * @param const core::Dimension2D& size [required]
        * @param u32 viewsMask [optional]. Use 0 if only one view or mask by bits if multiview feature is supported.
        * @param const std::string& name [optional]
        */
        explicit RenderTargetState(CommandList& cmdList, const core::Dimension2D& size, u32 viewsMask = 0, [[maybe_unused]] const std::string& name = "") noexcept;

        RenderTargetState() = delete;
        RenderTargetState(const RenderTargetState&) = delete;

        bool setColorTexture_Impl(u32 index, Texture* colorTexture, RenderTargetLoadOp loadOp, RenderTargetStoreOp storeOp, const core::Vector4D& clearColor);
        bool setColorTexture_Impl(u32 index, Texture* colorTexture, u32 layer, RenderTargetLoadOp loadOp, RenderTargetStoreOp storeOp, const core::Vector4D& clearColor);
        bool setColorTexture_Impl(u32 index, Texture* colorTexture, const ColorOpState& colorOpState, const TransitionState& tansitionState);
        bool setColorTexture_Impl(u32 index, Texture* colorTexture, s32 layer, const ColorOpState& colorOpState, const TransitionState& tansitionState);

        bool setDepthStencilTexture_Impl(Texture* depthStencilTexture, RenderTargetLoadOp depthLoadOp, RenderTargetStoreOp depthStoreOp, f32 clearDepth, RenderTargetLoadOp stencilLoadOp, RenderTargetStoreOp stencilStoreOp, u32 clearStencil);
        bool setDepthStencilTexture_Impl(Texture* depthStencilTexture, u32 layer, RenderTargetLoadOp depthLoadOp, RenderTargetStoreOp depthStoreOp, f32 clearDepth, RenderTargetLoadOp stencilLoadOp, RenderTargetStoreOp stencilStoreOp, u32 clearStencil);
        bool setDepthStencilTexture_Impl(Texture* depthStencilTexture, const DepthOpState& depthOpState, const StencilOpState& stencilOpState, const TransitionState& tansitionState);
        bool setDepthStencilTexture_Impl(Texture* depthStencilTexture, s32 layer, const DepthOpState& depthOpState, const StencilOpState& stencilOpState, const TransitionState& tansitionState);

        void extractRenderTargetInfo(RenderPassDescription& renderpassDesc, std::vector<Image*>& attachments, Framebuffer::ClearValueInfo& clearInfo) const;
        void destroyFramebuffers(const std::vector<Framebuffer*>& framebuffers);
        void destroyRenderPasses(const std::vector<RenderPass*>& renderPasses);

        bool checkCompatibility(Texture* texture, AttachmentDescription& desc);

        friend CommandList;
        CommandList& m_cmdList;

        core::Dimension2D m_size;
        u32 m_viewsMask;

        std::map<u32, std::tuple<Texture*, renderer::AttachmentDescription, core::Vector4D>> m_colorTextures;
        std::tuple<Texture*, renderer::AttachmentDescription, f32, u32>                      m_depthStencilTexture;

        ObjectTracker<Framebuffer>  m_trackerFramebuffer;
        ObjectTracker<RenderPass>   m_trackerRenderpass;
        [[maybe_unused]] const std::string m_name;
    };


    template<class TTexture>
    inline bool RenderTargetState::setColorTexture(u32 index, TTexture* colorTexture, RenderTargetLoadOp loadOp, RenderTargetStoreOp storeOp, const core::Vector4D& clearColor)
    {
        static_assert(std::is_same<TTexture, Texture2D>() || std::is_same<TTexture, Backbuffer>(), "wrong type");
        return setColorTexture_Impl(index, colorTexture, loadOp, storeOp, clearColor);
    }

    template<class TTexture>
    inline bool RenderTargetState::setColorTexture(u32 index, TTexture* colorTexture, s32 layer, RenderTargetLoadOp loadOp, RenderTargetStoreOp storeOp, const core::Vector4D& clearColor)
    {
        static_assert(std::is_same<TTexture, Texture2DArray>() || std::is_same<TTexture, TextureCube>(), "wrong type");
        return setColorTexture_Impl(index, colorTexture, layer, loadOp, storeOp, clearColor);
    }

    template<class TTexture>
    inline bool RenderTargetState::setColorTexture(u32 index, TTexture* colorTexture, const ColorOpState& colorOpState, const TransitionState& tansitionState)
    {
        static_assert(std::is_same<TTexture, Texture2D>() || std::is_same<TTexture, Backbuffer>(), "wrong type");
        return setColorTexture_Impl(index, colorTexture, colorOpState, tansitionState);
    }

    template<class TTexture>
    inline bool RenderTargetState::setColorTexture(u32 index, TTexture* colorTexture, s32 layer, const ColorOpState& colorOpState, const TransitionState& tansitionState)
    {
        static_assert(std::is_same<TTexture, Texture2DArray>() || std::is_same<TTexture, TextureCube>(), "wrong type");
        return setColorTexture_Impl(index, colorTexture, layer, colorOpState, tansitionState);
    }

    template<class TTexture>
    inline bool RenderTargetState::setDepthStencilTexture(TTexture* depthStencilTexture, RenderTargetLoadOp depthLoadOp, RenderTargetStoreOp depthStoreOp, f32 clearDepth, RenderTargetLoadOp stencilLoadOp, RenderTargetStoreOp stencilStoreOp, u32 clearStencil)
    {
        static_assert(std::is_same<TTexture, Texture2D>(), "wrong type");
        return setDepthStencilTexture_Impl(depthStencilTexture, depthLoadOp, depthStoreOp, clearDepth, stencilLoadOp, stencilStoreOp, clearStencil);
    }

    template<class TTexture>
    inline bool RenderTargetState::setDepthStencilTexture(TTexture* depthStencilTexture, s32 layer, RenderTargetLoadOp depthLoadOp, RenderTargetStoreOp depthStoreOp, f32 clearDepth, RenderTargetLoadOp stencilLoadOp, RenderTargetStoreOp stencilStoreOp, u32 clearStencil)
    {
        static_assert(std::is_same<TTexture, Texture2DArray>() || std::is_same<TTexture, TextureCube>(), "wrong type");
        return setDepthStencilTexture_Impl(depthStencilTexture, layer, depthLoadOp, depthStoreOp, clearDepth, stencilLoadOp, stencilStoreOp, clearStencil);
    }

    template<class TTexture>
    inline bool RenderTargetState::setDepthStencilTexture(TTexture* depthStencilTexture, const DepthOpState& depthOpState, const StencilOpState& stencilOpState, const TransitionState& tansitionState)
    {
        static_assert(std::is_same<TTexture, Texture2D>() || std::is_same<TTexture, TextureCube>() || std::is_same<TTexture, Texture2DArray>(), "wrong type");
        return setDepthStencilTexture_Impl(depthStencilTexture, depthOpState, stencilOpState, tansitionState);
    }

    template<class TTexture>
    inline bool RenderTargetState::setDepthStencilTexture(TTexture* depthStencilTexture, s32 layer, const DepthOpState& depthOpState, const StencilOpState& stencilOpState, const TransitionState& tansitionState)
    {
        static_assert(std::is_same<TTexture, Texture2DArray>() || std::is_same<TTexture, TextureCube>(), "wrong type");
        return setDepthStencilTexture_Impl(depthStencilTexture, layer, depthOpState, stencilOpState, tansitionState);
    }

    template<class TTexture>
    TTexture* RenderTargetState::getColorTexture(u32 index) const
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
    inline TTexture* RenderTargetState::getDepthStencilTexture() const
    {
        static_assert(std::is_base_of<Texture, TTexture>(), "wrong type");
        return static_cast<TTexture*>(std::get<0>(m_depthStencilTexture));
    }

    inline const core::Dimension2D& RenderTargetState::getDimension() const
    {
        return m_size;
    }

    inline u32 RenderTargetState::getColorTextureCount() const
    {
        return static_cast<u32>(m_colorTextures.size());
    }

    inline bool RenderTargetState::hasDepthStencilTexture() const
    {
        return std::get<0>(m_depthStencilTexture) != nullptr;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace renderer
} //namespace v3d
