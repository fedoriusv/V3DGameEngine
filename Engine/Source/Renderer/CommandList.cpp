#include "CommandList.h"
#include "Texture.h"
#include "RenderTargetState.h"
#include "PipelineState.h"
#include "QueryRequest.h"
#include "ShaderProgram.h"
#include "DeviceCaps.h"
#include "Core/Context.h"
#include "Utils/Logger.h"
#include "Utils/MemoryPool.h"


namespace v3d
{
namespace renderer
{

utils::MemoryPool g_commandMemoryPool(2048, 64, 2048, utils::MemoryPool::getDefaultMemoryPoolAllocator());

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

//may calls from another thread
void Command::operator delete(void* memory) noexcept
{
    free(memory);
    //g_commandMemoryPool.freeMemory(memory);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////

    /*CommandBeginFrame*/
class CommandBeginFrame final : public Command
{
public:

    CommandBeginFrame(CommandBeginFrame&) = delete;
    CommandBeginFrame() noexcept
    {
#if DEBUG_COMMAND_LIST
        LOG_DEBUG("CommandBeginFrame constructor");
#endif //DEBUG_COMMAND_LIST
    };

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

    CommandEndFrame(CommandEndFrame&) = delete;
    CommandEndFrame() noexcept
    {
#if DEBUG_COMMAND_LIST
        LOG_DEBUG("CommandEndFrame constructor");
#endif //DEBUG_COMMAND_LIST
    };

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

    CommandPresentFrame(CommandPresentFrame&) = delete;
    CommandPresentFrame() noexcept
    {
#if DEBUG_COMMAND_LIST
        LOG_DEBUG("CommandPresentFrame constructor");
#endif //DEBUG_COMMAND_LIST
    };

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

    CommandSetContextState() = delete;
    CommandSetContextState(CommandSetContextState&) = delete;
    explicit CommandSetContextState(const CommandList::ContextStates& pendingStates) noexcept
        : m_pendingStates(pendingStates)
    {
#if DEBUG_COMMAND_LIST
        LOG_DEBUG("CommandSetContextState constructor");
#endif //DEBUG_COMMAND_LIST
    };

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

    CommandSetRenderTarget() = delete;
    CommandSetRenderTarget(CommandSetRenderTarget&) = delete;
    explicit CommandSetRenderTarget(const RenderPass::RenderPassInfo& renderpassInfo, const Framebuffer::FramebufferInfo& framebufferInfo) noexcept
        : m_renderpassInfo(renderpassInfo)
        , m_framebufferInfo(framebufferInfo)
    {
#if DEBUG_COMMAND_LIST
        LOG_DEBUG("CommandSetRenderTarget constructor");
#endif //DEBUG_COMMAND_LIST
    };

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
        cmdList.getContext()->setRenderTarget(&m_renderpassInfo, &m_framebufferInfo);
    }

private:

    RenderPass::RenderPassInfo  m_renderpassInfo;
    Framebuffer::FramebufferInfo m_framebufferInfo;
};

    /*CommandSetGraphicPipeline*/
class CommandSetGraphicPipeline final : public Command
{
public:

    CommandSetGraphicPipeline() = delete;
    CommandSetGraphicPipeline(CommandSetGraphicPipeline&) = delete;
    explicit CommandSetGraphicPipeline(const RenderPassDescription& renderpassInfo, const ShaderProgramDescription& shaderProgramInfo, const GraphicsPipelineStateDescription& pipelineInfo, ObjectTracker<Pipeline>* tracker) noexcept
        : m_pipelineDesc(pipelineInfo)
        , m_programDesc(shaderProgramInfo)
        , m_renderpassDesc(renderpassInfo)
        , m_tracker(tracker)
    {
#if DEBUG_COMMAND_LIST
        LOG_DEBUG("CommandSetGraphicPipeline constructor");
#endif //DEBUG_COMMAND_LIST
    };

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
        pipelineGraphicInfo._tracker = m_tracker;

        cmdList.getContext()->setPipeline(&pipelineGraphicInfo);
    }

private:

    GraphicsPipelineStateDescription         m_pipelineDesc;
    ShaderProgramDescription                 m_programDesc;
    RenderPassDescription                    m_renderpassDesc;
    ObjectTracker<Pipeline>*                 m_tracker;
};

    /*CommandSetComputePipeline*/
class CommandSetComputePipeline final : public Command
{
public:

    CommandSetComputePipeline() = delete;
    CommandSetComputePipeline(CommandSetComputePipeline&) = delete;
    explicit CommandSetComputePipeline(const ShaderProgramDescription& shaderProgramInfo, ObjectTracker<Pipeline>* tracker) noexcept
        : m_programDesc(shaderProgramInfo)
        , m_tracker(tracker)
    {
#if DEBUG_COMMAND_LIST
        LOG_DEBUG("CommandSetComputePipeline constructor");
#endif //DEBUG_COMMAND_LIST
    };

    ~CommandSetComputePipeline()
    {
#if DEBUG_COMMAND_LIST
        LOG_DEBUG("CommandSetComputePipeline destructor");
#endif //DEBUG_COMMAND_LIST
    };

    void execute(const CommandList& cmdList)
    {
#if DEBUG_COMMAND_LIST
        LOG_DEBUG("CommandSetComputePipeline execute");
#endif //DEBUG_COMMAND_LIST
        Pipeline::PipelineComputeInfo pipelineComputeInfo;
        pipelineComputeInfo._programDesc = m_programDesc;
        pipelineComputeInfo._tracker = m_tracker;

        cmdList.getContext()->setPipeline(&pipelineComputeInfo);
    }

private:

    ShaderProgramDescription m_programDesc;
    ObjectTracker<Pipeline>* m_tracker;
};

    /*CommandSubmit*/
class CommandSubmit : public Command
{
public:

    CommandSubmit() = delete;
    CommandSubmit(CommandSubmit&) = delete;
    explicit CommandSubmit(bool wait, u64 timeout) noexcept
        : m_timeout(timeout)
        , m_wait(wait)
    {
#if DEBUG_COMMAND_LIST
        LOG_DEBUG("CommandSubmit constructor");
#endif //DEBUG_COMMAND_LIST
    }

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

    [[maybe_unused]] u64    m_timeout;
    bool                    m_wait;
};

    /*CommandDraw*/
class CommandDraw : public Command
{
public:

    CommandDraw() = delete;
    CommandDraw(CommandDraw&) = delete;
    explicit CommandDraw(const StreamBufferDescription& desc, u32 firstVertex, u32 countVertex, u32 firtsInstance, u32 instanceCount) noexcept
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

    CommandDrawIndexed() = delete;
    CommandDrawIndexed(CommandDrawIndexed&) = delete;
    explicit CommandDrawIndexed(const StreamBufferDescription& desc, u32 firstIndex, u32 countIndex, u32 firtsInstance, u32 instanceCount) noexcept
        : m_buffersDesc(desc)
        , m_firtsInstance(firtsInstance)
        , m_instanceCount(instanceCount)
        , m_firstIndex(firstIndex)
        , m_countIndex(countIndex)
    {
#if DEBUG_COMMAND_LIST
        LOG_DEBUG("CommandDrawIndexed constructor");
#endif //DEBUG_COMMAND_LIST
        ASSERT(!m_buffersDesc._vertices.empty() && m_buffersDesc._indices, "buffers is empty");
    }

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

    /*CommandDispatchCompute*/
class CommandDispatchCompute : public Command
{
public:

    CommandDispatchCompute() = delete;
    CommandDispatchCompute(CommandDispatchCompute&) = delete;

    explicit CommandDispatchCompute(const math::Dimension3D& groups) noexcept
        : m_groups(groups)
    {
#if DEBUG_COMMAND_LIST
        LOG_DEBUG("CommandDispatchCompute constructor");
#endif //DEBUG_COMMAND_LIST
    }

    ~CommandDispatchCompute()
    {
#if DEBUG_COMMAND_LIST
        LOG_DEBUG("CommandDispatchCompute constructor");
#endif //DEBUG_COMMAND_LIST
    }

    void execute(const CommandList& cmdList)
    {
#if DEBUG_COMMAND_LIST
        LOG_DEBUG("CommandDispatchCompute execute");
#endif //DEBUG_COMMAND_LIST
        cmdList.getContext()->dispatchCompute(m_groups);
    }

private:

    math::Dimension3D m_groups;
};

    /*CommandInvalidateRenderTarget*/
class CommandInvalidateRenderTarget : public Command
{
public:

    CommandInvalidateRenderTarget(CommandInvalidateRenderTarget&) = delete;
    CommandInvalidateRenderTarget() noexcept
    {
#if DEBUG_COMMAND_LIST
        LOG_DEBUG("CommandInvalidateRenderTarget constructor");
#endif //DEBUG_COMMAND_LIST
    }

    ~CommandInvalidateRenderTarget()
    {
#if DEBUG_COMMAND_LIST
        LOG_DEBUG("CommandInvalidateRenderTarget constructor");
#endif //DEBUG_COMMAND_LIST
    }

    void execute(const CommandList& cmdList)
    {
#if DEBUG_COMMAND_LIST
        LOG_DEBUG("CommandInvalidateRenderTarget execute");
#endif //DEBUG_COMMAND_LIST
        cmdList.getContext()->invalidateRenderTarget();
    }
};

/////////////////////////////////////////////////////////////////////////////////////////////////////

#if CMD_LIST_PROFILER_ENABLE
CmdListProfiler CommandList::s_commandListProfiler(
    {
        CmdListProfiler::CounterName::CmdAllCommands,
        CmdListProfiler::CounterName::CmdPushList,
        CmdListProfiler::CounterName::CmdExecuteList,
        CmdListProfiler::CounterName::CmdPendingFlush,
        CmdListProfiler::CounterName::CmdFrame,
        CmdListProfiler::CounterName::CmdSetState,
        CmdListProfiler::CounterName::CmdDraw,
        CmdListProfiler::CounterName::CmdBind,
    });
#endif //CMD_LIST_PROFILER_ENABLE

CommandList::CommandList(Context* context, CommandListType type) noexcept
    : m_context(context)
    , m_commandListType(type)
    , m_thread(nullptr)
    , m_writeSemaphore(1)

    , m_pendingFlushMask(0)
{
    if (CommandList::isThreaded())
    {
        m_thread = new utils::Thread();
        m_thread->setName("RenderThread");
        m_thread->run(std::bind(CommandList::threadedCommandsCallback, this), this);
    }

    m_swapchainTexture = createObject<Backbuffer>();
}

CommandList::~CommandList()
{
    CommandList::flushCommands();
    if (CommandList::isThreaded())
    {
        ASSERT(m_thread, "must be not nullptr");
        m_thread->terminate(true);
    }
    ASSERT(m_commandList.empty(), "not empty");
}

void CommandList::flushCommands()
{
    if (CommandList::isImmediate())
    {
        return;
    }

    m_pendingFlushMask = CommandList::flushPendingCommands(m_pendingFlushMask);
    if (!CommandList::isThreaded())
    {
        CommandList::executeCommands();
    }
}

void CommandList::submitCommands(bool wait)
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
            if (!CommandList::isThreaded())
            {
                CommandList::executeCommands();
            }
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

void CommandList::draw(const StreamBufferDescription& desc, u32 firstVertex, u32 countVertex, u32 countInstance)
    {
#if CMD_LIST_PROFILER_ENABLE
    CmdListProfiler::StackProfiler stackAllProfiler(&s_commandListProfiler, CmdListProfiler::CounterName::CmdAllCommands);
    CmdListProfiler::StackProfiler stackProfiler(&s_commandListProfiler, CmdListProfiler::CounterName::CmdDraw);
#endif //CMD_LIST_PROFILER_ENABLE

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

void CommandList::drawIndexed(const StreamBufferDescription& desc, u32 firstIndex, u32 countIndex, u32 countInstance)
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

void CommandList::dispatchCompute(const math::Dimension3D& groups)
{
    if (CommandList::isImmediate())
    {
        m_context->dispatchCompute(groups);
    }
    else
    {
        m_pendingFlushMask = CommandList::flushPendingCommands(m_pendingFlushMask);
        CommandList::pushCommand(new CommandDispatchCompute(groups));
    }
}

void CommandList::clearBackbuffer(const math::Vector4D& color)
{
    if (!CommandList::isImmediate())
    {
        m_pendingFlushMask = CommandList::flushPendingCommands(m_pendingFlushMask);
    }
    m_swapchainTexture->clear(color);
}

void CommandList::setRenderTarget(RenderTargetState* rendertarget)
{
    if (!rendertarget)
    {
        if (CommandList::isImmediate())
        {
            m_context->invalidateRenderTarget();
        }
        else
        {
            m_pendingFlushMask = CommandList::flushPendingCommands(m_pendingFlushMask);
            CommandList::pushCommand(new CommandInvalidateRenderTarget());
        }

    }
    else
    {
        RenderTargetPendingState renderTargetInfo;
        RenderPass::RenderPassInfo& renderPassInfo = renderTargetInfo._renderpassInfo;
        renderPassInfo._tracker = &rendertarget->m_trackerRenderpass;
        renderTargetInfo._framebufferInfo._tracker = &rendertarget->m_trackerFramebuffer;
        rendertarget->extractRenderTargetInfo(renderPassInfo._desc, renderTargetInfo._framebufferInfo._images, renderTargetInfo._framebufferInfo._clearInfo);

        if (CommandList::isImmediate())
        {
            m_context->setRenderTarget(&renderTargetInfo._renderpassInfo, &renderTargetInfo._framebufferInfo);
        }
        else
        {
            m_pendingRenderTargetInfo._framebufferInfo = std::move(renderTargetInfo._framebufferInfo);
            m_pendingRenderTargetInfo._renderpassInfo = std::move(renderTargetInfo._renderpassInfo);

            m_pendingFlushMask |= PendingFlush_UpdateRenderTarget;
        }
    }
}

void CommandList::setPipelineState(GraphicsPipelineState* pipeline)
{
    if (!pipeline || !pipeline->m_renderTaget || !pipeline->m_program)
    {
        ASSERT(false, "setPipelineState is nullptr");
        return;
    }

    Framebuffer::FramebufferInfo framebufferInfo;
    RenderPassDescription renderPassDescription;
    pipeline->m_renderTaget->extractRenderTargetInfo(renderPassDescription, framebufferInfo._images, framebufferInfo._clearInfo);

    Pipeline::PipelineGraphicInfo pipelineGraphicInfo;
    pipelineGraphicInfo._pipelineDesc = pipeline->getGraphicsPipelineStateDesc();
    pipelineGraphicInfo._programDesc = pipeline->m_program->getShaderDesc();
    pipelineGraphicInfo._renderpassDesc = renderPassDescription;
    pipelineGraphicInfo._tracker = &pipeline->m_tracker;
    pipelineGraphicInfo._name = pipeline->m_name;

    if (CommandList::isImmediate())
    {
        m_context->setPipeline(&pipelineGraphicInfo);
    }
    else
    {
        m_pendingPipelineStateInfo._pipelineGraphicInfo = std::move(pipelineGraphicInfo);
        m_pendingFlushMask |= PendingFlush_UpdateGraphicsPipeline;
    }
}

void CommandList::setPipelineState(ComputePipelineState* pipeline)
{
    if (!pipeline || !pipeline->m_program)
    {
        ASSERT(false, "setPipelineState is nullptr");
        return;
    }

    Pipeline::PipelineComputeInfo pipelineComputeInfo;
    pipelineComputeInfo._programDesc = pipeline->m_program->getShaderDesc();
    pipelineComputeInfo._tracker = &pipeline->m_tracker;
    pipelineComputeInfo._name = pipeline->m_name;

    if (CommandList::isImmediate())
    {
        m_context->setPipeline(&pipelineComputeInfo);
    }
    else
    {
        m_pendingPipelineStateInfo._pipelineComputeInfo = std::move(pipelineComputeInfo);
        m_pendingFlushMask |= PendingFlush_UpdateComputePipeline;
    }
}

void CommandList::setViewport(const math::Rect32& viewport, const math::Vector2D& depth)
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

void CommandList::setScissor(const math::Rect32& scissor)
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
    if (CommandList::isThreaded())
    {
        m_writeSemaphore.acquire();
    }
    m_commandList.push(cmd);

    if (CommandList::isThreaded())
    {
        m_writeSemaphore.release();
    }
    }
}

void CommandList::executeCommands()
{
    if (CommandList::isThreaded())
    {
        m_writeSemaphore.acquire();
    }

    while (!m_commandList.empty())
    {
        Command* cmd = m_commandList.front();
        m_commandList.pop();
        
        cmd->execute(*this);
        delete cmd;
    }

    if (CommandList::isThreaded())
    {
        m_writeSemaphore.release();
    }
}

void CommandList::threadedCommandsCallback(CommandList* cmdList)
{
    ASSERT(cmdList->isThreaded(), "Must be threaded");
    while (cmdList->m_thread->isRunning())
    {
        cmdList->executeCommands();

        std::this_thread::yield();
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
        CommandList::pushCommand(new CommandSetRenderTarget(m_pendingRenderTargetInfo._renderpassInfo, m_pendingRenderTargetInfo._framebufferInfo));
        pendingFlushMask &= ~PendingFlush_UpdateRenderTarget;
    }

    if (pendingFlushMask & PendingFlush_UpdateContextState)
    {
        CommandList::pushCommand(new CommandSetContextState(m_pendingStates));
        pendingFlushMask &= ~PendingFlush_UpdateContextState;
    }

    if (pendingFlushMask & PendingFlush_UpdateGraphicsPipeline)
    {
        Pipeline::PipelineGraphicInfo& info = m_pendingPipelineStateInfo._pipelineGraphicInfo;
        CommandList::pushCommand(new CommandSetGraphicPipeline(info._renderpassDesc, info._programDesc, info._pipelineDesc, info._tracker));

        pendingFlushMask &= ~PendingFlush_UpdateGraphicsPipeline;
    }

    if (pendingFlushMask & PendingFlush_UpdateComputePipeline)
    {
        Pipeline::PipelineComputeInfo& info = m_pendingPipelineStateInfo._pipelineComputeInfo;
        CommandList::pushCommand(new CommandSetComputePipeline(info._programDesc, info._tracker));

        pendingFlushMask &= ~PendingFlush_UpdateComputePipeline;
    }

    if (pendingFlushMask & PendingFlush_UpdateTransitions)
    {
        /*CommandTransitionImage*/
        class CommandTransitionImage : public Command
        {
        public:

            explicit CommandTransitionImage(const std::vector<std::tuple<const Image*, Image::Subresource>>& images, TransitionOp state) noexcept
                : m_images(images)
                , m_state(state)
            {
#if DEBUG_COMMAND_LIST
                LOG_DEBUG("CommandTransitionImage constructor");
#endif //DEBUG_COMMAND_LIST
            }

            CommandTransitionImage() = delete;
            CommandTransitionImage(CommandTransitionImage&) = delete;

            ~CommandTransitionImage()
            {
#if DEBUG_COMMAND_LIST
                LOG_DEBUG("CommandTransitionImage constructor");
#endif //DEBUG_COMMAND_LIST
            }

            void execute(const CommandList& cmdList)
            {
#if DEBUG_COMMAND_LIST
                LOG_DEBUG("CommandTransitionImage execute");
#endif //DEBUG_COMMAND_LIST
                cmdList.getContext()->transitionImages(m_images, m_state);
            }

        private:

            std::vector<std::tuple<const Image*, Image::Subresource>> m_images;
            TransitionOp m_state;
        };

        auto state = m_pendingTransitions._transitions.cbegin()->first;
        std::vector<std::tuple<const Image*, Image::Subresource>> images;
        for (auto& st : m_pendingTransitions._transitions)
        {
            if (state != st.first)
            {
                CommandList::pushCommand(new CommandTransitionImage(images, state));

                images.clear();
                state = st.first;
            }

            images.emplace_back(st.second._texture->getImage(), Image::makeImageSubresource(st.second._baseLayer, st.second._layers, st.second._baseMip, st.second._mips));
        }

        if (!images.empty())
        {
            CommandList::pushCommand(new CommandTransitionImage(images, state));
        }

        m_pendingTransitions._transitions.clear();
        pendingFlushMask &= ~PendingFlush_UpdateTransitions;
    }

    return pendingFlushMask;
}

void CommandList::transition(const TextureView& texture, TransitionOp state)
{
     ASSERT(texture._texture, "nullptr");
    if (CommandList::isImmediate())
    {
        std::vector<std::tuple<const Image*, Image::Subresource>> images(1, { texture._texture->getImage(), Image::makeImageSubresource(texture._baseLayer, texture._layers, texture._baseMip, texture._mips) });
        m_context->transitionImages(images, state);
    }
    else
    {
        m_pendingTransitions._transitions.insert({ state, texture });
        m_pendingFlushMask |= PendingFlush_UpdateTransitions;
    }
}

void CommandList::generateMipmaps(Texture2D* texute, TransitionOp state)
{
    if (!texute)
    {
        return;
    }

    ASSERT(m_context->getDeviceCaps()->supportBlitImage, "Must be true");
    ASSERT(texute->getLayersCount() == 1, "must be 1");
    if (CommandList::isImmediate())
    {
        m_context->generateMipmaps(texute->getImage(), 0, state);
    }
    else
    {
        /*CommandGenerateMipmaps*/
        class CommandGenerateMipmaps : public Command
        {
        public:

            explicit CommandGenerateMipmaps(Image* image, u32 layer, TransitionOp state)
                : m_image(image)
                , m_layer(layer)
                , m_state(state)
            {
#if DEBUG_COMMAND_LIST
                LOG_DEBUG("CommandGenerateMipmaps constructor");
#endif //DEBUG_COMMAND_LIST
            }

            CommandGenerateMipmaps() = delete;
            CommandGenerateMipmaps(CommandGenerateMipmaps&) = delete;

            ~CommandGenerateMipmaps()
            {
#if DEBUG_COMMAND_LIST
                LOG_DEBUG("CommandGenerateMipmaps constructor");
#endif //DEBUG_COMMAND_LIST
            }

            void execute(const CommandList& cmdList)
            {
#if DEBUG_COMMAND_LIST
                LOG_DEBUG("CommandGenerateMipmaps execute");
#endif //DEBUG_COMMAND_LIST
                cmdList.getContext()->generateMipmaps(m_image, m_layer, m_state);
            }

        private:

            Image* m_image;
            u32 m_layer;
            TransitionOp m_state;
        };

        CommandList::pushCommand(new CommandGenerateMipmaps(texute->getImage(), 0, state));
    }
}

} //namespace renderer
} //namespace v3d
