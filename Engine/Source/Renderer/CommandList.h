#pragma once

#include "Common.h"
#include "Utils/NonCopyable.h"
#include "Object/Object.h"
#include "RenderPass.h"
#include "Pipeline.h"
#include "Image.h"
#include "Framebuffer.h"

namespace v3d
{
namespace renderer
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    class SwapchainTexture;
    class RenderTarget;
    class GraphicsPipelineState;
    class Backbuffer;
    class CommandList;
    class Context;

    struct ContextStates
    {
        core::Rect32 _viewport;
    };

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

        void beginFrame();
        void endFrame();
        void presentFrame();

        void pushCommand(Command* cmd);

        void clearBackbuffer(const core::Vector4D& color);

        void setRenderTarget(RenderTarget* rendertarget);
        void setPipelineState(GraphicsPipelineState* pipeline);
        void setViewport(const core::Rect32& viewport);

        Context* getContext() const;
        bool isThreaded() const;
        bool isImmediate() const;

        //....

        template<class T, class ... Args>
        T* createObject(Args ... args)
        {
            static_assert(std::is_base_of<Object, T>());
            return new T(*this, (args)...);
        }

        enum PendingFlushMask
        {
            PendingFlush_UpdateRenderTarget = 0x1,
            PendingFlush_UpdateContextState = 0x2,
            PendingFlush_UpdateGraphicsPipeline = 0x4,
        };

        void flushPendingCommands(u32 pendingFlushMask);

    private:

        struct RenderTargetInfo
        {
            std::vector<Image*>         _attachments;
            RenderPass::RenderPassInfo  _renderpassInfo;
            RenderPass::ClearValueInfo  _clearInfo;
        };

        struct PipelineStateInfo
        {
            Pipeline::PipelineGraphicInfo    _pipelineInfo;
            ObjectTracker<Pipeline>*         _tracker;
        };

        void executeCommands();


        std::queue<Command*>        m_commandList;

        Context*                    m_context;
        CommandListType             m_commandListType;

        ContextStates               m_pendingStates;
        RenderTargetInfo            m_pendingRenderTargetInfo;
        PipelineStateInfo           m_pendingPipelineStateInfo;

        u32                         m_pendingFlushMask;

        //
        SwapchainTexture*           m_swapchainTexture;
        Backbuffer*                 m_backbuffer;
        //
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace renderer
} //namespace v3d
