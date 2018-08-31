#pragma once

#include "Context.h"
#include "Utils/NonCopyable.h"

namespace v3d
{
namespace renderer
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    class CommandList;

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

        Context* getContext() const;
        bool     isThreaded() const;

        //....

    private:

        void pushCommand(Command* cmd);
        void executeCommands();

        std::queue<Command*> m_commandList;

        Context* m_context;
        CommandListType m_commandListType;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace renderer
} //namespace v3d
