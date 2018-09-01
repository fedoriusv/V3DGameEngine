#include "CommandList.h"
#include "Utils/Logger.h"

#include "Context.h"

namespace v3d
{
namespace renderer
{

class CommandBeginFrame : public Command
{
public:
    CommandBeginFrame() 
    {
        LOG_DEBUG("CommandBeginFrame constructor");
    };
    ~CommandBeginFrame() 
    {
        LOG_DEBUG("CommandBeginFrame destructor");
    };

    void execute(const CommandList& cmdList)
    {
        cmdList.getContext()->beginFrame();
    }
};

class CommandEndFrame : public Command
{
public:
    CommandEndFrame() 
    {
        LOG_DEBUG("CommandEndFrame constructor");
    };
    ~CommandEndFrame() 
    {
        LOG_DEBUG("CommandBeginFrame destructor");
    };

    void execute(const CommandList& cmdList)
    {
        cmdList.getContext()->endFrame();
    }
};

class CommandPresentFrame : public Command
{
public:
    CommandPresentFrame()
    {
        LOG_DEBUG("CommandPresentFrame constructor");
    };
    ~CommandPresentFrame()
    {
        LOG_DEBUG("CommandPresentFrame destructor");
    };

    void execute(const CommandList& cmdList)
    {
        cmdList.getContext()->presentFrame();
    }
};

CommandList::CommandList(Context* context, CommandListType type)
    : m_context(context)
    , m_commandListType(type)
{
}

CommandList::~CommandList()
{
}

void CommandList::flushCommands()
{
    if (m_commandListType == CommandListType::ImmediateCommandList)
    {
        return;
    }

    CommandList::executeCommands();
}

void CommandList::cmdBeginFrame()
{
    if (m_commandListType == CommandListType::ImmediateCommandList)
    {
        m_context->beginFrame();
    }
    else
    {
        CommandList::pushCommand(new CommandBeginFrame());
    }
}

void CommandList::cmdEndFrame()
{
    if (m_commandListType == CommandListType::ImmediateCommandList)
    {
        m_context->endFrame();
    }
    else
    {
        CommandList::pushCommand(new CommandEndFrame());
    }
 }

void CommandList::cmdPresentFrame()
{
    if (m_commandListType == CommandListType::ImmediateCommandList)
    {
        m_context->endFrame();
    }
    else
    {
        CommandList::pushCommand(new CommandPresentFrame());
    }
}

Context* CommandList::getContext() const
{
    return m_context;
}

bool CommandList::isThreaded() const
{
    return m_commandListType == CommandListType::ThreadCommandList;
}

void CommandList::pushCommand(Command* cmd)
{
    m_commandList.push(cmd);
}

void CommandList::executeCommands()
{
    while (!m_commandList.empty())
    {
        Command* cmd = m_commandList.front();
        m_commandList.pop();

        cmd->execute(*this);
        delete cmd;
    }
}

Command::Command()
{
    LOG_DEBUG("Command constructor");
}

Command::~Command()
{
    LOG_DEBUG("Command destructor");
}

void* Command::operator new(size_t size)
{
    //TODO mem pool
    void* p = malloc(size);
    return p;
}

void Command::operator delete(void* memory)
{
    //TODO mem pool
    free(memory);
}

} //namespace renderer
} //namespace v3d
