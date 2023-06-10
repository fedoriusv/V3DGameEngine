#pragma once

#include "Common.h"
#include "Object.h"
#include "Utils/Thread.h"

#include "TextureProperties.h"

#include "Core/RenderPass.h"
#include "Core/Framebuffer.h"
#include "Core/Pipeline.h"

namespace v3d
{
namespace renderer
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    class Context;
    class CommandList;

    class Texture;
    class Texture2D;
    class Backbuffer;
    class RenderTargetState;
    class GraphicsPipelineState;
    class ComputePipelineState;
    class QueryRequest;

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief Command base class
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
    * @brief CommandList class
    */
    class CommandList final
    {
    public:

        enum class CommandListType
        {
            ImmediateCommandList,
            DelayedCommandList,
            ThreadCommandList
        };

        CommandList() = delete;
        CommandList(const CommandList&) = delete;
        CommandList& operator=(const CommandList&) = delete;

        CommandList(Context* context, CommandListType type) noexcept;
        ~CommandList();

        void flushCommands();

        void submitCommands(bool wait = false);
        void beginFrame();
        void endFrame();
        void presentFrame();

        void draw(const StreamBufferDescription& desc, u32 firstVertex, u32 countVertex, u32 countInstance);
        void drawIndexed(const StreamBufferDescription& desc, u32 firstIndex, u32 countIndex, u32 countInstance);

        void dispatchCompute(const math::Dimension3D& groups = { 1, 1, 1});

        void pushCommand(Command* cmd);
        void pushReadbackCall(Object* caller, std::function<void(Object*)>&& readback);

        void clearBackbuffer(const math::Vector4D& color);

        void setRenderTarget(RenderTargetState* rendertarget);
        void setPipelineState(GraphicsPipelineState* pipeline);
        void setPipelineState(ComputePipelineState* pipeline);

        void setViewport(const math::Rect32& viewport, const math::Vector2D& depth = {0.0f, 1.0f});
        void setScissor(const math::Rect32& scissor);

        void transition(const TextureView& texture, TransitionOp state);

        void generateMipmaps(Texture2D* texture, TransitionOp state);

        Context* getContext() const;
        bool isThreaded() const;
        bool isImmediate() const;

        Backbuffer* getBackbuffer() const;

        template<class TObject, class ... Args>
        TObject* createObject(Args&& ... args)
        {
            static_assert(std::is_base_of<Object, TObject>(), "CommandList::createObject wrong type");
            return new TObject(*this, std::forward<Args>(args)...);
        }

        struct ContextStates
        {
            math::Rect32    _viewportColor;
            math::Vector2D  _viewportDepth;
            math::Rect32    _scissor;
        };


    private:

        enum PendingFlushMask
        {
            PendingFlush_UpdateRenderTarget = 0x1,
            PendingFlush_UpdateContextState = 0x2,
            PendingFlush_UpdateGraphicsPipeline = 0x4,
            PendingFlush_UpdateComputePipeline = 0x8,
            PendingFlush_UpdateTransitions = 0x10
        };
        typedef u16 PendingFlushMaskFlags;

        PendingFlushMaskFlags flushPendingCommands(PendingFlushMaskFlags pendingFlushMask);

        struct RenderTargetPendingState
        {
            RenderPass::RenderPassInfo   _renderpassInfo;
            Framebuffer::FramebufferInfo _framebufferInfo;
        };

        struct PipelinePendingState
        {
            Pipeline::PipelineGraphicInfo _pipelineGraphicInfo;
            Pipeline::PipelineComputeInfo _pipelineComputeInfo;
        };

        struct TransitionPendingState
        {
            std::multimap<TransitionOp, TextureView> _transitions;
        };

        void executeCommands();
        void dispatchReadbackCalls();
        static void threadedCommandsCallback(CommandList* cmdList);

        std::queue<Command*>        m_commandList;
        std::queue<std::tuple<Object*, std::function<void(Object*)>>> m_readbackCalls;

        Context*                    m_context;
        const CommandListType       m_commandListType;
        utils::Thread*              m_thread;
        std::binary_semaphore       m_writeSemaphore;

        ContextStates               m_pendingStates;
        RenderTargetPendingState    m_pendingRenderTargetInfo;
        PipelinePendingState        m_pendingPipelineStateInfo;
        TransitionPendingState      m_pendingTransitions;

        PendingFlushMaskFlags       m_pendingFlushMask;

        Backbuffer*                 m_swapchainTexture;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace renderer
} //namespace v3d
