#pragma once

#include "Common.h"
#include "Object/Object.h"
#include "Utils/NonCopyable.h"
#include "RenderPass.h"
#include "Image.h"

namespace v3d
{
namespace renderer
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    class SwapchainTexture;
    class RenderTarget;
    class Backbuffer;
    class CommandList;
    class Context;

    struct ContextStates
    {
        core::Rect32 _viewport;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    class Command
    {
    public:

        Command();
        virtual ~Command();

        void* operator new (size_t size) noexcept;
        void operator delete(void* memory) noexcept;

        virtual void execute(const CommandList& cmdList) = 0;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    class CommandList final : public utils::NonCopyable
    {
    public:

        enum class CommandListType
        {
            ImmediateCommandList,
            DelayedCommandList,
            ThreadCommandList
        };

        CommandList(Context* context, CommandListType type);
        ~CommandList();

        void flushCommands();

        void beginFrame();
        void endFrame();
        void presentFrame();

        void pushCommand(Command* cmd);

        void clearBackbuffer(const core::Vector4D& color);

        void setRenderTarget(RenderTarget* rendertarget);
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

    private:

        struct RenderTargetInfo
        {
            std::vector<Image*> _attachments;
            RenderPassInfo      _renderpassInfo;
            ClearValueInfo      _clearInfo;
        };

        void executeCommands();

        void flushPendingCommands();

        std::queue<Command*> m_commandList;

        Context* m_context;
        CommandListType m_commandListType;

        ContextStates       m_pendingStates;
        bool                m_statesNeedUpdate;

        RenderTargetInfo    m_pendingRenderTargetInfo;
        bool                m_renderTargetNeedUpdate;

        //
        SwapchainTexture* m_swapchainTexture;
        Backbuffer*       m_backbuffer;
        //
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace renderer
} //namespace v3d
