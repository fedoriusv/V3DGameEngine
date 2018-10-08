#include "CommandList.h"
#include "Utils/Logger.h"
#include "Utils/MemoryPool.h"

#include "Context.h"
#include "Object/Texture.h"
#include "Object/RenderTarget.h"

namespace v3d
{
namespace renderer
{

utils::MemoryPool g_commandMemoryPool(1024, 64, 1024 * 2, utils::MemoryPool::getDefaultMemoryPoolAllocator());

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


//class CommandBindRenderTarget : public Command
//{
//public:
//    CommandBindRenderTarget(const std::vector<RenderTarget::AttachmentDesc>& attachments)
//        : m_attachments(attachments)
//    {
//        LOG_DEBUG("CommandBindRenderTarget constructor");
//    };
//    ~CommandBindRenderTarget()
//    {
//        LOG_DEBUG("CommandBindRenderTarget destructor");
//    };
//
//    void execute(const CommandList& cmdList)
//    {
//        cmdList.getContext()->
//    }
//
//private:
//    std::vector<RenderTarget::AttachmentDesc> m_attachments;
//};

/////////////////////////////////////////////////////////////////////////////////////////////////////

CommandList::CommandList(Context* context, CommandListType type)
    : m_context(context)
    , m_commandListType(type)
    , m_statesNeedUpdate(false)
{
    m_swapchainTexture = createObject<SwapchainTexture>();
    m_backbuffer = createObject<Backbuffer>(m_swapchainTexture);



    m_currentRenderTarget = nullptr;
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

void CommandList::beginFrame()
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

void CommandList::endFrame()
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

void CommandList::presentFrame()
{
    if (m_commandListType == CommandListType::ImmediateCommandList)
    {
        m_context->presentFrame();
    }
    else
    {
        CommandList::pushCommand(new CommandPresentFrame());
    }
}

void CommandList::clearBackbuffer(const core::Vector4D & color)
{
    m_swapchainTexture->clear(color);
}

void CommandList::setRenderTarget(RenderTarget * rendertarget)
{
    rendertarget->makeRenderTarget();

    if (m_commandListType == CommandListType::ImmediateCommandList)
    {
        //bind
    }
    else
    {
        //pendingState
    }

    m_currentRenderTarget = rendertarget;
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

bool CommandList::isImmediate() const
{
    return m_commandListType == CommandListType::ImmediateCommandList;
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

void* Command::operator new(size_t size) noexcept
{
#ifdef _DEBUG
    static size_t s_sizeMax = 0;
    s_sizeMax = std::max(s_sizeMax, size);
    LOG_DEBUG("Command new allocate size %u, maxSize %u", size, s_sizeMax);
#endif
    void* ptr = g_commandMemoryPool.getMemory(size);
    return ptr;
}

void Command::operator delete(void* memory) noexcept
{
    g_commandMemoryPool.freeMemory(memory);
}

} //namespace renderer
} //namespace v3d
