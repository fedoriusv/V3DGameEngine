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

class CommandClearColor : public Command
{
public:
    CommandClearColor(const core::Vector4D& color)
        : m_clearColor(color)
    {
        LOG_DEBUG("CommandClearColor constructor");
    };
    ~CommandClearColor()
    {
        LOG_DEBUG("CommandClearColor destructor");
    };

    void execute(const CommandList& cmdList)
    {
        cmdList.getContext()->clearColor(m_clearColor);
    }

private:

    core::Vector4D m_clearColor;
};

class CommandSetContextState : public Command
{
public:
    CommandSetContextState(const ContextStates& pendingStates)
        : m_pendingStates(pendingStates)
    {
        LOG_DEBUG("CommandSetContextState constructor");
    };
    ~CommandSetContextState()
    {
        LOG_DEBUG("CommandSetContextState destructor");
    };

    void execute(const CommandList& cmdList)
    {
        cmdList.getContext()->setViewport(m_pendingStates._viewport);
    }

private:
    ContextStates m_pendingStates;
};





CommandList::CommandList(Context* context, CommandListType type)
    : m_context(context)
    , m_commandListType(type)
    , m_statesNeedUpdate(false)
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

    if (m_statesNeedUpdate)
    {
        CommandList::cmdSetContextStates(m_pendingStates);
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

void CommandList::cmdClearColor(const core::Vector4D & color)
{
    if (m_commandListType == CommandListType::ImmediateCommandList)
    {
        m_context->clearColor(color);
    }
    else
    {
        CommandList::pushCommand(new CommandClearColor(color));
    }
}

void CommandList::setViewport(const core::Rect32& viewport)
{
    if (m_commandListType == CommandListType::ImmediateCommandList)
    {
        m_context->setViewport(viewport);
    }
    else
    {
        m_pendingStates._viewport = viewport;
        m_statesNeedUpdate = true;
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

void CommandList::cmdSetContextStates(const ContextStates & pendingStates)
{
    if (m_commandListType == CommandListType::ImmediateCommandList)
    {
        return;
    }

    CommandList::pushCommand(new CommandSetContextState(pendingStates));
    m_statesNeedUpdate = false;

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
