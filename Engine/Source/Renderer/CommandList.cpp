#include "CommandList.h"
#include "Utils/Logger.h"
#include "Utils/MemoryPool.h"

#include "Context.h"
#include "Object/Texture.h"
#include "Object/RenderTarget.h"
#include "Object/PipelineState.h"
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
        LOG_DEBUG("CommandBeginFrame constructor");
    };
    CommandBeginFrame(CommandBeginFrame&) = delete;

    ~CommandBeginFrame() 
    {
        LOG_DEBUG("CommandBeginFrame destructor");
    };

    void execute(const CommandList& cmdList)
    {
        LOG_DEBUG("CommandBeginFrame execute");
        cmdList.getContext()->beginFrame();
    }
};

    /*CommandEndFrame*/
class CommandEndFrame final : public Command
{
public:
    CommandEndFrame() noexcept
    {
        LOG_DEBUG("CommandEndFrame constructor");
    };
    CommandEndFrame(CommandEndFrame&) = delete;

    ~CommandEndFrame() 
    {
        LOG_DEBUG("CommandEndFrame destructor");
    };

    void execute(const CommandList& cmdList)
    {
        LOG_DEBUG("CommandEndFrame execute");
        cmdList.getContext()->endFrame();
    }
};

    /*CommandPresentFrame*/
class CommandPresentFrame final : public Command
{
public:
    CommandPresentFrame() noexcept
    {
        LOG_DEBUG("CommandPresentFrame constructor");
    };
    CommandPresentFrame(CommandPresentFrame&) = delete;

    ~CommandPresentFrame()
    {
        LOG_DEBUG("CommandPresentFrame destructor");
    };

    void execute(const CommandList& cmdList)
    {
        LOG_DEBUG("CommandPresentFrame execute");
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
        LOG_DEBUG("CommandSetContextState constructor");
    };
    CommandSetContextState() = delete;
    CommandSetContextState(CommandSetContextState&) = delete;

    ~CommandSetContextState()
    {
        LOG_DEBUG("CommandSetContextState destructor");
    };

    void execute(const CommandList& cmdList)
    {
        LOG_DEBUG("CommandSetContextState execute");
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
        LOG_DEBUG("CommandSetRenderTarget constructor");
    };
    CommandSetRenderTarget() = delete;
    CommandSetRenderTarget(CommandSetRenderTarget&) = delete;

    ~CommandSetRenderTarget()
    {
        LOG_DEBUG("CommandSetRenderTarget destructor");
    };

    void execute(const CommandList& cmdList)
    {
        LOG_DEBUG("CommandSetRenderTarget execute");
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
        LOG_DEBUG("CommandSetGraphicPipeline constructor");
    };
    CommandSetGraphicPipeline() = delete;
    CommandSetGraphicPipeline(CommandSetGraphicPipeline&) = delete;

    ~CommandSetGraphicPipeline()
    {
        LOG_DEBUG("CommandSetGraphicPipeline destructor");
    };

    void execute(const CommandList& cmdList)
    {
        LOG_DEBUG("CommandSetGraphicPipeline execute");
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
        LOG_DEBUG("CommandSubmit constructor");
    }
    CommandSubmit() = delete;
    CommandSubmit(CommandSubmit&) = delete;

    ~CommandSubmit()
    {
        LOG_DEBUG("CommandSubmit constructor");
    }

    void execute(const CommandList& cmdList)
    {
        LOG_DEBUG("CommandSubmit execute");
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
        LOG_DEBUG("CommandDraw constructor");
    }
    CommandDraw() = delete;
    CommandDraw(CommandDraw&) = delete;

    ~CommandDraw()
    {
        LOG_DEBUG("CommandDraw constructor");
    }

    void execute(const CommandList& cmdList)
    {
        LOG_DEBUG("CommandDraw execute");
        cmdList.getContext()->draw(m_buffersDesc, m_firstVertex, m_vertexCount, m_firtsInstance, m_instanceCount);
    }

private:
    StreamBufferDescription m_buffersDesc;
    u32 m_firtsInstance;
    u32 m_instanceCount;
    u32 m_firstVertex;
    u32 m_vertexCount;
};

    /*CommandInvalidateRenderPass*/
class CommandInvalidateRenderPass : public Command
{
public:
    CommandInvalidateRenderPass() noexcept
    {
        LOG_DEBUG("CommandInvalidateRenderPass constructor");
    }
    CommandInvalidateRenderPass(CommandInvalidateRenderPass&) = delete;

    ~CommandInvalidateRenderPass()
    {
        LOG_DEBUG("CommandInvalidateRenderPass constructor");
    }

    void execute(const CommandList& cmdList)
    {
        LOG_DEBUG("CommandInvalidateRenderPass execute");
        cmdList.getContext()->invalidateRenderPass();
    }
};

/////////////////////////////////////////////////////////////////////////////////////////////////////

Command::Command() noexcept
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

/////////////////////////////////////////////////////////////////////////////////////////////////////

CommandList::CommandList(Context* context, CommandListType type) noexcept
    : m_context(context)
    , m_commandListType(type)

    , m_pendingFlushMask(0)
{
    m_swapchainTexture = createObject<BackbufferTexture>();
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

BackbufferTexture* CommandList::getBackbuffer() const
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
