#pragma once

#include "Common.h"
#include "Utils/NonCopyable.h"

#include "Object.h"

#include "RenderPass.h"
#include "Pipeline.h"
#include "Image.h"
#include "Framebuffer.h"

#include "BufferProperties.h"


namespace v3d
{
namespace renderer
{
    class Context;
    class CommandList;

    class Backbuffer;
    class RenderTarget;
    class GraphicsPipelineState;

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * Command base class
    */
    class Command
    {
    public:

        Command() noexcept;
        virtual ~Command();

        void* operator new (size_t size) noexcept;
        void operator delete(void* memory) noexcept;

        virtual void execute(const CommandList& cmdList) = 0;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * CommandList class
    */
    class CommandList final : public utils::NonCopyable
    {
    public:

        enum class CommandListType
        {
            ImmediateCommandList,
            DelayedCommandList,
            ThreadCommandList
        };

        CommandList(Context* context, CommandListType type) noexcept;
        ~CommandList();

        void flushCommands();

        void sumitCommands(bool wait = false);
        void beginFrame();
        void endFrame();
        void presentFrame();

        void draw(StreamBufferDescription& desc, u32 firstVertex, u32 countVertex, u32 countInstance);
        void drawIndexed(StreamBufferDescription& desc, u32 firstIndex, u32 countIndex, u32 countInstance);

        void pushCommand(Command* cmd);

        void clearBackbuffer(const core::Vector4D& color);

        void setRenderTarget(RenderTarget* rendertarget);
        void setPipelineState(GraphicsPipelineState* pipeline);
        void setViewport(const core::Rect32& viewport, const core::Vector2D& depth = {0.0f, 1.0f});
        void setScissor(const core::Rect32& scissor);

        Context* getContext() const;
        bool isThreaded() const;
        bool isImmediate() const;

        Backbuffer* getBackbuffer() const;

        template<class TObject, class ... Args>
        TObject* createObject(Args ... args)
        {
            static_assert(std::is_base_of<Object, TObject>());
            return new TObject(*this, (args)...);
        }

        enum PendingFlushMask
        {
            PendingFlush_UpdateRenderTarget = 0x1,
            PendingFlush_UpdateContextState = 0x2,
            PendingFlush_UpdateGraphicsPipeline = 0x4,
        };

        typedef u16 PendingFlushMaskFlags;

        PendingFlushMaskFlags flushPendingCommands(PendingFlushMaskFlags pendingFlushMask);

        struct ContextStates
        {
            core::Rect32    _viewportColor;
            core::Vector2D  _viewportDepth;
            core::Rect32    _scissor;
        };

    private:

        struct RenderTargetInfo
        {
            std::vector<Image*>         _attachments;
            RenderPass::RenderPassInfo  _renderpassInfo;
            RenderPass::ClearValueInfo  _clearInfo;

            ObjectTracker<RenderPass>*  _trackerRenderpass;
            ObjectTracker<Framebuffer>* _trackerFramebuffer;
        };

        struct PipelineStateInfo
        {
            Pipeline::PipelineGraphicInfo _pipelineInfo;
            ObjectTracker<Pipeline>*      _tracker;
        };

        void executeCommands();


        std::queue<Command*>        m_commandList;

        Context*                    m_context;
        CommandListType             m_commandListType;

        ContextStates               m_pendingStates;
        RenderTargetInfo            m_pendingRenderTargetInfo;
        PipelineStateInfo           m_pendingPipelineStateInfo;

        PendingFlushMaskFlags       m_pendingFlushMask;

        Backbuffer*                 m_swapchainTexture;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace renderer
} //namespace v3d
