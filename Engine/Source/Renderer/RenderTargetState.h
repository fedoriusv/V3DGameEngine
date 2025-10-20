#pragma once

#include "Object.h"
#include "Render.h"
#include "Texture.h"
#include "ObjectTracker.h"

namespace v3d
{
namespace renderer
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    class Device;

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief AttachmentDesc struct. Uses inside a render target.
    * 4 bytes size
    */
    struct AttachmentDesc
    {
        AttachmentDesc() noexcept
        {
            static_assert(sizeof(AttachmentDesc) == 4, "wrong size");
            memset(this, 0, sizeof(AttachmentDesc));
        }

        Format                _format           : 8;
        RenderTargetLoadOp    _loadOp           : 2;
        RenderTargetStoreOp   _storeOp          : 2;
        RenderTargetLoadOp    _stencilLoadOp    : 2;
        RenderTargetStoreOp   _stencilStoreOp   : 2;
        TextureSamples        _samples          : 3;
        TransitionOp          _transition       : 3;
        TransitionOp          _finalTransition  : 3;
        u32                   _unused           : 7;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief RenderPassDesc struct. Uses inside a render pass.
    */
    struct RenderPassDesc final
    {
        RenderPassDesc() noexcept
        {
            static_assert(sizeof(RenderPassDesc) == sizeof(u32) + sizeof(_attachmentsDesc), "wrong size");
            memset(this, 0, sizeof(RenderPassDesc));
        }

        bool operator==(const RenderPassDesc& other) const
        {
            if (this == &other)
            {
                return true;
            }

            return memcmp(this, &other, sizeof(RenderPassDesc)) == 0;
        }

        [[nodiscard]] static u32 countActiveViews(u32 viewsMask)
        {
            u32 countActiveLayers = 0;
            for (u32 i = 0; i < std::numeric_limits<u16>::digits; ++i)
            {
                if ((viewsMask >> i) & 0x1)
                {
                    ++countActiveLayers;
                }

            }

            return countActiveLayers;
        }

        [[nodiscard]] static bool isActiveViewByIndex(u32 viewsMask, u32 index)
        {
            return (viewsMask >> index) & 0x1;
        }

        std::array<AttachmentDesc, k_maxColorAttachments + 1>   _attachmentsDesc;
        u32                                                     _countColorAttachment       : 4;
        u32                                                     _hasDepthStencilAttachment  : 1;
        u32                                                     _viewsMask                  : 16;
        u32                                                     _unused                     : 11;
    };

    ///////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief FramebufferDesc struct. Uses inside a render pass.
    */
    struct FramebufferDesc final
    {
        FramebufferDesc() noexcept
        {
            memset(this, 0, sizeof(FramebufferDesc));
        }

        std::array<TextureView, k_maxColorAttachments + 1> _imageViews;
        math::Dimension2D                                  _renderArea;
        std::array<color::Color, k_maxColorAttachments>    _clearColorValues;
        f32                                                _clearDepthValue;
        u32                                                _clearStencilValue : 16;
        u32                                                _viewsMask         : 16;
    };

    ///////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief RenderTargetState class. Game side
    */
    class RenderTargetState : public Object
    {
    public:

        /**
        * @brief ColorOpState struct
        */
        struct ColorOpState
        {
            ColorOpState() noexcept
                : _loadOp(RenderTargetLoadOp::LoadOp_Clear)
                , _storeOp(RenderTargetStoreOp::StoreOp_Store)
                , _clearColor(0.f, 0.f, 0.f, 0.f)
            {
            }
            
            ColorOpState(RenderTargetLoadOp loadOp, RenderTargetStoreOp storeOp, const color::Color& clearColor) noexcept
                : _loadOp(loadOp)
                , _storeOp(storeOp)
                , _clearColor(clearColor)
            {
            }

            RenderTargetLoadOp      _loadOp     : 16;
            RenderTargetStoreOp     _storeOp    : 16;
            const color::Color   _clearColor;
        };

        /**
        * @brief DepthOpState struct
        */
        struct DepthOpState
        {
            DepthOpState() noexcept
                : _loadOp(RenderTargetLoadOp::LoadOp_Clear)
                , _storeOp(RenderTargetStoreOp::StoreOp_DontCare)
                , _clearDepth(1.0f)
            {
            }

            DepthOpState(RenderTargetLoadOp loadOp, RenderTargetStoreOp storeOp, f32 clearDepth = 0.f) noexcept
                : _loadOp(loadOp)
                , _storeOp(storeOp)
                , _clearDepth(clearDepth)
            {
            }

            RenderTargetLoadOp   _loadOp        : 16;
            RenderTargetStoreOp  _storeOp       : 16;
            f32                  _clearDepth;
        };

        /**
        * @brief StencilOpState struct
        */
        struct StencilOpState
        {
            StencilOpState() noexcept
                : _loadOp(RenderTargetLoadOp::LoadOp_Clear)
                , _storeOp(RenderTargetStoreOp::StoreOp_DontCare)
                , _clearStencil(0U)
            {
            }

            StencilOpState(RenderTargetLoadOp loadOp, RenderTargetStoreOp storeOp, u32 clearStencil = 0U) noexcept
                : _loadOp(loadOp)
                , _storeOp(storeOp)
                , _clearStencil(clearStencil)
            {
            }

            RenderTargetLoadOp   _loadOp        : 16;
            RenderTargetStoreOp  _storeOp       : 16;
            u32                  _clearStencil;
        };

        /**
        * @brief TransitionState struct
        */
        struct TransitionState
        {
            TransitionState(TransitionOp state, TransitionOp finalState) noexcept
                : _state(state)
                , _finalState(finalState)
            {
            }

            TransitionOp _state      : 16;
            TransitionOp _finalState : 16;
        };

        /**
        * @brief setColorTexture method. Used to adding a color attachment to a render target
        *
        * @param u32 index [required]. Attaches the texture to index slot in a shader
        * @param const TextureView& colorTextureView [required]
        * @param RenderTargetLoadOp loadOp [optional]
        * @param RenderTargetStoreOp storeOp [optional]
        * @param const color::Color& clearColor [optional]
        *
        * @return true if compatibility is successed
        */
        bool setColorTexture(u32 index, const TextureView& colorTextureView,
            RenderTargetLoadOp loadOp = RenderTargetLoadOp::LoadOp_Clear, RenderTargetStoreOp storeOp = RenderTargetStoreOp::StoreOp_Store,
            const color::Color& clearColor = color::Color(0.f));

        /**
        * @brief setDepthStencilTexture method. Used to adding a depth-stencil attachment to a render target
        *
        * @param const TextureView& depthStencilTextureView [required]
        * @param RenderTargetLoadOp depthLoadOp [optional]
        * @param RenderTargetStoreOp depthStoreOp [optional]
        * @param f32 clearDepth [optional]
        * @param RenderTargetLoadOp stencilLoadOp [optional]
        * @param RenderTargetStoreOp stencilStoreOp [optional]
        * @param f32 clearStencil [optional]
        * 
        * @return true if compatibility is successed
        */
        bool setDepthStencilTexture(const TextureView& depthStencilTextureView,
            RenderTargetLoadOp depthLoadOp = RenderTargetLoadOp::LoadOp_DontCare, RenderTargetStoreOp depthStoreOp = RenderTargetStoreOp::StoreOp_DontCare,
            f32 clearDepth = 0.f,
            RenderTargetLoadOp stencilLoadOp = RenderTargetLoadOp::LoadOp_DontCare, RenderTargetStoreOp stencilStoreOp = RenderTargetStoreOp::StoreOp_DontCare,
            u32 clearStencil = 0);

        /**
        * @brief setColorTexture method. Used to adding a color attachment to a render target
        *
        * @param u32 index [required]. Attaches the texture to index slot in a shader
        * @param const TextureView& colorTextureView [required]
        * @param const ColorOpState& colorOpState [required]
        * @param const TransitionState& tansitionState [required]
        * @param const color::Color& clearColor [optional]
        * @see TransitionState
        *
        * @return true if compatibility is successed
        */
        bool setColorTexture(u32 index, const TextureView& colorTextureView,
            const ColorOpState& colorOpState, const TransitionState& tansitionState);

        /**
        * @brief setDepthStencilTexture method. Used to adding a depth-stencil attachment to a render target
        *
        * @param const TextureView& depthStencilTextureView [required]
        * @param const DepthOpState& depthOpState [required]
        * @param const StencilOpState& stencilOpState [required]
        * @param const TransitionState& tansitionState [required]
        * @see TransitionState
        *
        * @return true if compatibility is successed
        */
        bool setDepthStencilTexture(const TextureView& depthStencilTextureView,
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
        * @brief getRenderArea method. Size of texture
        * @return math::Dimension2D
        */
        const math::Dimension2D& getRenderArea() const;

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

    public:

        /**
        * @brief RenderTargetState constructor. Used for creating a render target
        * Private method. Use createObject interface inside CommandList class to call.
        *
        * @param const math::Dimension2D& size [required]
        * @param u32 viewsMask [optional]. Use 0 if only one view or mask by bits if multiview feature is supported.
        * @param const std::string& name [optional]
        */
        explicit RenderTargetState(Device* device, const math::Dimension2D& size, u32 countAttacment = 1U, u32 viewsMask = 0U, const std::string& name = "") noexcept;

        /**
        * @brief RenderTargetState desctructor
        */
        ~RenderTargetState();


        const RenderPassDesc& getRenderPassDesc() const;
        const FramebufferDesc& getFramebufferDesc() const;
        const std::string& getName() const;

    private:

        RenderTargetState() = delete;
        RenderTargetState(const RenderTargetState&) = delete;

        bool checkCompatibility(const TextureView& textureView, AttachmentDesc& desc);
        void destroyFramebuffers(const std::vector<Framebuffer*>& framebuffers);
        void destroyRenderPasses(const std::vector<RenderPass*>& renderPasses);

        Device* const               m_device;
        RenderPassDesc              m_renderpassDesc;
        FramebufferDesc             m_attachmentsDesc;
        const std::string           m_name;

    public:

        ObjectTracker<Framebuffer>  m_trackerFramebuffer;
        ObjectTracker<RenderPass>   m_trackerRenderpass;
    };

    template<class TTexture>
    inline TTexture* RenderTargetState::getColorTexture(u32 index) const
    {
        ASSERT(index < m_renderpassDesc._countColorAttachment, "out of range");

        static_assert(std::is_base_of<Texture, TTexture>(), "wrong type");
        return static_cast<TTexture*>(m_attachmentsDesc._imageViews[index]._texture);
    }

    template<class TTexture>
    inline TTexture* RenderTargetState::getDepthStencilTexture() const
    {
        static_assert(std::is_base_of<Texture, TTexture>(), "wrong type");
        return static_cast<TTexture*>(m_attachmentsDesc._imageViews.back()._texture);
    }

    inline const math::Dimension2D& RenderTargetState::getRenderArea() const
    {
        return m_attachmentsDesc._renderArea;
    }

    inline u32 RenderTargetState::getColorTextureCount() const
    {
        return m_renderpassDesc._countColorAttachment;
    }

    inline bool RenderTargetState::hasDepthStencilTexture() const
    {
        return m_renderpassDesc._hasDepthStencilAttachment;
    }

    inline const RenderPassDesc& RenderTargetState::getRenderPassDesc() const
    {
        return m_renderpassDesc;
    }
    
    inline const FramebufferDesc& RenderTargetState::getFramebufferDesc() const
    {
        return m_attachmentsDesc;
    }

    inline const std::string& RenderTargetState::getName() const
    {
        return m_name;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace renderer
} //namespace v3d
