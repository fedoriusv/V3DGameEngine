#include "CommandList.h"
#include "Utils/Logger.h"
#include "Utils/MemoryPool.h"

#include "Context.h"
#include "Object/Texture.h"
#include "Object/RenderTarget.h"
#include "Object/PipelineState.h"
#include "Object/ShaderProgram.h"
#include "RenderPass.h"
#include "Framebuffer.h"


namespace v3d
{
namespace renderer
{

utils::MemoryPool g_commandMemoryPool(2048, 64, 2048, utils::MemoryPool::getDefaultMemoryPoolAllocator());

/////////////////////////////////////////////////////////////////////////////////////////////////////

    /*CommandBeginFrame*/
class CommandBeginFrame final : public Command
{
public:
    CommandBeginFrame() noexcept
    {
#if DEBUG_COMMAND_LIST
        LOG_DEBUG("CommandBeginFrame constructor");
#endif //DEBUG_COMMAND_LIST
    };
    CommandBeginFrame(CommandBeginFrame&) = delete;

    ~CommandBeginFrame() 
    {
#if DEBUG_COMMAND_LIST
        LOG_DEBUG("CommandBeginFrame destructor");
#endif //DEBUG_COMMAND_LIST
    };

    void execute(const CommandList& cmdList)
    {
#if DEBUG_COMMAND_LIST
        LOG_DEBUG("CommandBeginFrame execute");
#endif //DEBUG_COMMAND_LIST
        cmdList.getContext()->beginFrame();
    }
};

    /*CommandEndFrame*/
class CommandEndFrame final : public Command
{
public:
    CommandEndFrame() noexcept
    {
#if DEBUG_COMMAND_LIST
        LOG_DEBUG("CommandEndFrame constructor");
#endif //DEBUG_COMMAND_LIST
    };
    CommandEndFrame(CommandEndFrame&) = delete;

    ~CommandEndFrame() 
    {
#if DEBUG_COMMAND_LIST
        LOG_DEBUG("CommandEndFrame destructor");
#endif //DEBUG_COMMAND_LIST
    };

    void execute(const CommandList& cmdList)
    {
#if DEBUG_COMMAND_LIST
        LOG_DEBUG("CommandEndFrame execute");
#endif //DEBUG_COMMAND_LIST
        cmdList.getContext()->endFrame();
    }
};

    /*CommandPresentFrame*/
class CommandPresentFrame final : public Command
{
public:
    CommandPresentFrame() noexcept
    {
#if DEBUG_COMMAND_LIST
        LOG_DEBUG("CommandPresentFrame constructor");
#endif //DEBUG_COMMAND_LIST
    };
    CommandPresentFrame(CommandPresentFrame&) = delete;

    ~CommandPresentFrame()
    {
#if DEBUG_COMMAND_LIST
        LOG_DEBUG("CommandPresentFrame destructor");
#endif //DEBUG_COMMAND_LIST
    };

    void execute(const CommandList& cmdList)
    {
#if DEBUG_COMMAND_LIST
        LOG_DEBUG("CommandPresentFrame execute");
#endif //DEBUG_COMMAND_LIST
        cmdList.getContext()->presentFrame();
    }
};

    /*CommandSetContextState*/
class CommandSetContextState final : public Command
{
public:
    explicit CommandSetContextState(const CommandList::ContextStates& pendingStates) noexcept
        : m_pendingStates(pendingStates)
    {
#if DEBUG_COMMAND_LIST
        LOG_DEBUG("CommandSetContextState constructor");
#endif //DEBUG_COMMAND_LIST
    };
    CommandSetContextState() = delete;
    CommandSetContextState(CommandSetContextState&) = delete;

    ~CommandSetContextState()
    {
#if DEBUG_COMMAND_LIST
        LOG_DEBUG("CommandSetContextState destructor");
#endif //DEBUG_COMMAND_LIST
    };

    void execute(const CommandList& cmdList)
    {
#if DEBUG_COMMAND_LIST
        LOG_DEBUG("CommandSetContextState execute");
#endif //DEBUG_COMMAND_LIST
        cmdList.getContext()->setViewport(m_pendingStates._viewportColor, m_pendingStates._viewportDepth);
        cmdList.getContext()->setScissor(m_pendingStates._scissor);
    }

private:
    CommandList::ContextStates m_pendingStates;
};

    /*CommandSetRenderTarget*/
class CommandSetRenderTarget final : public Command
{
public:
    CommandSetRenderTarget(const RenderPass::RenderPassInfo& renderpassInfo, const std::vector<Image*>& attachments, const RenderPass::ClearValueInfo& clearInfo, const std::tuple<ObjectTracker<RenderPass>*, ObjectTracker<Framebuffer>*>& trackers) noexcept
        : m_renderpassInfo(renderpassInfo)
        , m_attachments(attachments)
        , m_clearInfo(clearInfo)
        , m_trackers(trackers)
    {
#if DEBUG_COMMAND_LIST
        LOG_DEBUG("CommandSetRenderTarget constructor");
#endif //DEBUG_COMMAND_LIST
    };
    CommandSetRenderTarget() = delete;
    CommandSetRenderTarget(CommandSetRenderTarget&) = delete;

    ~CommandSetRenderTarget()
    {
#if DEBUG_COMMAND_LIST
        LOG_DEBUG("CommandSetRenderTarget destructor");
#endif //DEBUG_COMMAND_LIST
    };

    void execute(const CommandList& cmdList)
    {
#if DEBUG_COMMAND_LIST
        LOG_DEBUG("CommandSetRenderTarget execute");
#endif //DEBUG_COMMAND_LIST
        cmdList.getContext()->setRenderTarget(&m_renderpassInfo, m_attachments, &m_clearInfo, m_trackers);
    }

private:
    RenderPass::RenderPassInfo  m_renderpassInfo;
    std::vector<Image*>         m_attachments;
    RenderPass::ClearValueInfo  m_clearInfo;
    std::tuple<ObjectTracker<RenderPass>*, ObjectTracker<Framebuffer>*> m_trackers;
};

    /*CommandSetRenderTarget*/
class CommandSetGraphicPipeline final : public Command
{
public:
    CommandSetGraphicPipeline(const RenderPass::RenderPassInfo& renderpassInfo, const ShaderProgramDescription& shaderProgramInfo, const GraphicsPipelineStateDescription& pipelineInfo, ObjectTracker<Pipeline>* tracker) noexcept
        : m_pipelineDesc(pipelineInfo)
        , m_programDesc(shaderProgramInfo)
        , m_renderpassDesc(renderpassInfo)
        , m_tracker(tracker)
    {
#if DEBUG_COMMAND_LIST
        LOG_DEBUG("CommandSetGraphicPipeline constructor");
#endif //DEBUG_COMMAND_LIST
    };
    CommandSetGraphicPipeline() = delete;
    CommandSetGraphicPipeline(CommandSetGraphicPipeline&) = delete;

    ~CommandSetGraphicPipeline()
    {
#if DEBUG_COMMAND_LIST
        LOG_DEBUG("CommandSetGraphicPipeline destructor");
#endif //DEBUG_COMMAND_LIST
    };

    void execute(const CommandList& cmdList)
    {
#if DEBUG_COMMAND_LIST
        LOG_DEBUG("CommandSetGraphicPipeline execute");
#endif //DEBUG_COMMAND_LIST
        Pipeline::PipelineGraphicInfo pipelineGraphicInfo;
        pipelineGraphicInfo._renderpassDesc = m_renderpassDesc;
        pipelineGraphicInfo._programDesc = m_programDesc;
        pipelineGraphicInfo._pipelineDesc = std::move(m_pipelineDesc);

        cmdList.getContext()->setPipeline(&pipelineGraphicInfo, m_tracker);
    }

private:
    GraphicsPipelineStateDescription         m_pipelineDesc;
    ShaderProgramDescription                 m_programDesc;
    RenderPass::RenderPassInfo               m_renderpassDesc;
    ObjectTracker<Pipeline>*                 m_tracker;
};

    /*CommandSubmit*/
class CommandSubmit : public Command
{
public:
    CommandSubmit(bool wait, u64 timeout) noexcept
        : m_timeout(timeout)
        , m_wait(wait)
    {
#if DEBUG_COMMAND_LIST
        LOG_DEBUG("CommandSubmit constructor");
#endif //DEBUG_COMMAND_LIST
    }
    CommandSubmit() = delete;
    CommandSubmit(CommandSubmit&) = delete;

    ~CommandSubmit()
    {
#if DEBUG_COMMAND_LIST
        LOG_DEBUG("CommandSubmit constructor");
#endif //DEBUG_COMMAND_LIST
    }

    void execute(const CommandList& cmdList)
    {
#if DEBUG_COMMAND_LIST
        LOG_DEBUG("CommandSubmit execute");
#endif //DEBUG_COMMAND_LIST
        cmdList.getContext()->submit(m_wait);
    }

private:
    u64     m_timeout;
    bool    m_wait;
};

    /*CommandDraw*/
class CommandDraw : public Command
{
public:
    CommandDraw(const StreamBufferDescription& desc, u32 firstVertex, u32 countVertex, u32 firtsInstance, u32 instanceCount) noexcept
        : m_buffersDesc(desc)
        , m_firtsInstance(firtsInstance)
        , m_instanceCount(instanceCount)
        , m_firstVertex(firstVertex)
        , m_vertexCount(countVertex)
    {
#if DEBUG_COMMAND_LIST
        LOG_DEBUG("CommandDraw constructor");
#endif //DEBUG_COMMAND_LIST
    }
    CommandDraw() = delete;
    CommandDraw(CommandDraw&) = delete;

    ~CommandDraw()
    {
#if DEBUG_COMMAND_LIST
        LOG_DEBUG("CommandDraw constructor");
#endif //DEBUG_COMMAND_LIST
    }

    void execute(const CommandList& cmdList)
    {
#if DEBUG_COMMAND_LIST
        LOG_DEBUG("CommandDraw execute");
#endif //DEBUG_COMMAND_LIST
        cmdList.getContext()->draw(m_buffersDesc, m_firstVertex, m_vertexCount, m_firtsInstance, m_instanceCount);
    }

private:
    StreamBufferDescription m_buffersDesc;
    u32 m_firtsInstance;
    u32 m_instanceCount;
    u32 m_firstVertex;
    u32 m_vertexCount;
};

    /*CommandDrawIndexed*/
class CommandDrawIndexed : public Command
{
public:
    CommandDrawIndexed(const StreamBufferDescription& desc, u32 firstIndex, u32 countIndex, u32 firtsInstance, u32 instanceCount) noexcept
        : m_buffersDesc(desc)
        , m_firtsInstance(firtsInstance)
        , m_instanceCount(instanceCount)
        , m_firstIndex(firstIndex)
        , m_countIndex(countIndex)
    {
#if DEBUG_COMMAND_LIST
        LOG_DEBUG("CommandDrawIndexed constructor");
#endif //DEBUG_COMMAND_LIST
    }
    CommandDrawIndexed() = delete;
    CommandDrawIndexed(CommandDrawIndexed&) = delete;

    ~CommandDrawIndexed()
    {
#if DEBUG_COMMAND_LIST
        LOG_DEBUG("CommandDrawIndexed constructor");
#endif //DEBUG_COMMAND_LIST
    }

    void execute(const CommandList& cmdList)
    {
#if DEBUG_COMMAND_LIST
        LOG_DEBUG("CommandDrawIndexed execute");
#endif //DEBUG_COMMAND_LIST
        cmdList.getContext()->drawIndexed(m_buffersDesc, m_firstIndex, m_countIndex, m_firtsInstance, m_instanceCount);
    }

private:
    StreamBufferDescription m_buffersDesc;
    u32 m_firtsInstance;
    u32 m_instanceCount;
    u32 m_firstIndex;
    u32 m_countIndex;
};

    /*CommandInvalidateRenderPass*/
class CommandInvalidateRenderPass : public Command
{
public:
    CommandInvalidateRenderPass() noexcept
    {
#if DEBUG_COMMAND_LIST
        LOG_DEBUG("CommandInvalidateRenderPass constructor");
#endif //DEBUG_COMMAND_LIST
    }
    CommandInvalidateRenderPass(CommandInvalidateRenderPass&) = delete;

    ~CommandInvalidateRenderPass()
    {
#if DEBUG_COMMAND_LIST
        LOG_DEBUG("CommandInvalidateRenderPass constructor");
#endif //DEBUG_COMMAND_LIST
    }

    void execute(const CommandList& cmdList)
    {
#if DEBUG_COMMAND_LIST
        LOG_DEBUG("CommandInvalidateRenderPass execute");
#endif //DEBUG_COMMAND_LIST
        cmdList.getContext()->invalidateRenderPass();
    }
};

/////////////////////////////////////////////////////////////////////////////////////////////////////

Command::Command() noexcept
{
#if DEBUG_COMMAND_LIST
    LOG_DEBUG("Command constructor");
#endif //DEBUG_COMMAND_LIST

}

Command::~Command()
{
#if DEBUG_COMMAND_LIST
    LOG_DEBUG("Command destructor");
#endif //DEBUG_COMMAND_LIST
}

void* Command::operator new(size_t size) noexcept
{
#if DEBUG_COMMAND_LIST
    static size_t s_sizeMax = 0;
    s_sizeMax = std::max(s_sizeMax, size);
    LOG_DEBUG("Command new allocate size %u, maxSize %u", size, s_sizeMax);
#endif //DEBUG_COMMAND_LIST

    //void* ptr = g_commandMemoryPool.getMemory(size);
    void* ptr = malloc(size);
    return ptr;


}

void Command::operator delete(void* memory) noexcept
{
    free(memory);
    //g_commandMemoryPool.freeMemory(memory);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////

CommandList::CommandList(Context* context, CommandListType type) noexcept
    : m_context(context)
    , m_commandListType(type)

    , m_pendingFlushMask(0)
{
    m_swapchainTexture = createObject<Backbuffer>();
}

CommandList::~CommandList()
{
}

void CommandList::flushCommands()
{
    if (CommandList::isImmediate())
    {
        return;
    }

    m_pendingFlushMask = CommandList::flushPendingCommands(m_pendingFlushMask);
    CommandList::executeCommands();
}

void CommandList::sumitCommands(bool wait)
{
    if (CommandList::isImmediate())
    {
        m_context->submit(wait);
    }
    else
    {
        m_pendingFlushMask = CommandList::flushPendingCommands(m_pendingFlushMask);
        CommandList::pushCommand(new CommandSubmit(wait, 0));
        if (wait)
        {
            CommandList::executeCommands();
        }
    }
}

void CommandList::beginFrame()
{
    if (CommandList::isImmediate())
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
    if (CommandList::isImmediate())
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
    if (CommandList::isImmediate())
    {
        m_context->presentFrame();
    }
    else
    {
        CommandList::pushCommand(new CommandPresentFrame());
    }
}

void CommandList::draw(StreamBufferDescription& desc, u32 firstVertex, u32 countVertex, u32 countInstance)
{
    if (CommandList::isImmediate())
    {
        m_context->draw(desc, firstVertex, countVertex, 0, countInstance);
    }
    else
    {
        m_pendingFlushMask = CommandList::flushPendingCommands(m_pendingFlushMask);
        CommandList::pushCommand(new CommandDraw(desc, firstVertex, countVertex, 0, countInstance));
    }
}

void CommandList::drawIndexed(StreamBufferDescription& desc, u32 firstIndex, u32 countIndex, u32 countInstance)
{
    if (CommandList::isImmediate())
    {
        m_context->drawIndexed(desc, firstIndex, countIndex, 0, countInstance);
    }
    else
    {
       m_pendingFlushMask = CommandList::flushPendingCommands(m_pendingFlushMask);
       CommandList::pushCommand(new CommandDrawIndexed(desc, firstIndex, countIndex, 0, countInstance));
    }
}

void CommandList::clearBackbuffer(const core::Vector4D & color)
{
    m_swapchainTexture->clear(color);
}

void CommandList::setRenderTarget(RenderTarget* rendertarget)
{
    if (!rendertarget)
    {
        if (CommandList::isImmediate())
        {
            m_context->invalidateRenderPass();
        }
        else
        {
            m_pendingFlushMask = CommandList::flushPendingCommands(m_pendingFlushMask);
            CommandList::pushCommand(new CommandInvalidateRenderPass());
        }
    }

    RenderTargetInfo renderTargetInfo;
    rendertarget->extractRenderTargetInfo(renderTargetInfo._renderpassInfo, renderTargetInfo._attachments, renderTargetInfo._clearInfo);

    if (CommandList::isImmediate())
    {
        m_context->setRenderTarget(&renderTargetInfo._renderpassInfo, renderTargetInfo._attachments, &renderTargetInfo._clearInfo, { &rendertarget->m_trackerRenderpass, &rendertarget->m_trackerFramebuffer });
    }
    else
    {
        m_pendingRenderTargetInfo._attachments = std::move(renderTargetInfo._attachments);
        m_pendingRenderTargetInfo._clearInfo = renderTargetInfo._clearInfo;
        m_pendingRenderTargetInfo._renderpassInfo = renderTargetInfo._renderpassInfo;
        m_pendingRenderTargetInfo._trackerFramebuffer = &rendertarget->m_trackerFramebuffer;
        m_pendingRenderTargetInfo._trackerRenderpass = &rendertarget->m_trackerRenderpass;

        m_pendingFlushMask |= PendingFlush_UpdateRenderTarget;
    }
}

void CommandList::setPipelineState(GraphicsPipelineState * pipeline)
{
    if (!pipeline || !pipeline->m_renderTaget || !pipeline->m_program)
    {
        ASSERT(false, "nullptr");
        return;
    }

    RenderTargetInfo renderTargetInfo;
    pipeline->m_renderTaget->extractRenderTargetInfo(renderTargetInfo._renderpassInfo, renderTargetInfo._attachments, renderTargetInfo._clearInfo);

    Pipeline::PipelineGraphicInfo pipelineGraphicInfo;
    pipelineGraphicInfo._pipelineDesc = pipeline->getGraphicsPipelineStateDesc();
    pipelineGraphicInfo._programDesc = pipeline->m_program->getShaderDesc();
    pipelineGraphicInfo._renderpassDesc = renderTargetInfo._renderpassInfo;

    if (CommandList::isImmediate())
    {
        m_context->setPipeline(&pipelineGraphicInfo, &pipeline->m_tracker);
    }
    else
    {
        m_pendingPipelineStateInfo._pipelineInfo = pipelineGraphicInfo;
        m_pendingPipelineStateInfo._tracker = &pipeline->m_tracker;
        m_pendingFlushMask |= PendingFlush_UpdateGraphicsPipeline;
    }
}

void CommandList::setViewport(const core::Rect32& viewport, const core::Vector2D& depth)
{
    if (CommandList::isImmediate())
    {
        m_context->setViewport(viewport);
    }
    else
    {
        m_pendingStates._viewportColor = viewport;
        m_pendingStates._viewportDepth = depth;
        m_pendingFlushMask |= PendingFlush_UpdateContextState;
    }
}

void CommandList::setScissor(const core::Rect32 & scissor)
{
    if (CommandList::isImmediate())
    {
        m_context->setScissor(scissor);
    }
    else
    {
        m_pendingStates._scissor = scissor;
        m_pendingFlushMask |= PendingFlush_UpdateContextState;
    }
}

Context* CommandList::getContext() const
{
    return m_context;
}

Backbuffer* CommandList::getBackbuffer() const
{
    return m_swapchainTexture;
}

bool CommandList::isThreaded() const
{
    return m_commandListType == CommandListType::ThreadCommandList;
}

bool CommandList::isImmediate() const
{
    return m_commandListType == CommandListType::ImmediateCommandList;
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

CommandList::PendingFlushMaskFlags CommandList::flushPendingCommands(PendingFlushMaskFlags pendingFlushMask)
{
    if (CommandList::isImmediate())
    {
        return 0;
    }

    if (pendingFlushMask & PendingFlush_UpdateRenderTarget)
    {
        CommandList::pushCommand(new CommandSetRenderTarget(m_pendingRenderTargetInfo._renderpassInfo, m_pendingRenderTargetInfo._attachments, m_pendingRenderTargetInfo._clearInfo, 
                { m_pendingRenderTargetInfo._trackerRenderpass, m_pendingRenderTargetInfo._trackerFramebuffer }));
        pendingFlushMask &= ~PendingFlush_UpdateRenderTarget;
    }

    if (pendingFlushMask & PendingFlush_UpdateContextState)
    {
        CommandList::pushCommand(new CommandSetContextState(m_pendingStates));
        pendingFlushMask &= ~PendingFlush_UpdateContextState;
    }

    if (pendingFlushMask & PendingFlush_UpdateGraphicsPipeline)
    {
        Pipeline::PipelineGraphicInfo& info = m_pendingPipelineStateInfo._pipelineInfo;
        CommandList::pushCommand(new CommandSetGraphicPipeline(info._renderpassDesc, info._programDesc, info._pipelineDesc, m_pendingPipelineStateInfo._tracker));

        pendingFlushMask &= ~PendingFlush_UpdateGraphicsPipeline;
    }

    return pendingFlushMask;
}

} //namespace renderer
} //namespace v3d
