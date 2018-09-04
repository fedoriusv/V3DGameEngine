#pragma once

#include "Common.h"
#include "Utils/NonCopyable.h"

namespace v3d
{
namespace renderer
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

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

        void cmdBeginFrame();
        void cmdEndFrame();
        void cmdPresentFrame();


        //test
        void cmdClearColor(const core::Vector4D& color);

        void setViewport(const core::Rect32& viewport);

        Context* getContext() const;
        bool isThreaded() const;

        //....

    private:

        void cmdSetContextStates(const ContextStates& pendingStates);

        void pushCommand(Command* cmd);
        void executeCommands();

        std::queue<Command*> m_commandList;

        Context* m_context;
        CommandListType m_commandListType;

        ContextStates m_pendingStates;
        bool m_statesNeedUpdate;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace renderer
} //namespace v3d
