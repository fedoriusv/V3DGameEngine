#include "CommandList.h"
#include "Utils/Logger.h"
#include "Utils/MemoryPool.h"

#include "Context.h"
#include "Object/Texture.h"
#include "Object/RenderTarget.h"
#include "RenderPass.h"
#include "Framebuffer.h"

namespace v3d
{
namespace renderer
{

utils::MemoryPool g_commandMemoryPool(1024, 64, 1024 * 2, utils::MemoryPool::getDefaultMemoryPoolAllocator());

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
        LOG_DEBUG("CommandBeginFrame destructor");
    };

    void execute(const CommandList& cmdList)
    {
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
        cmdList.getContext()->presentFrame();
    }
};

    /*CommandSetContextState*/
class CommandSetContextState final : public Command
{
public:
    explicit CommandSetContextState(const ContextStates& pendingStates) noexcept
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
        cmdList.getContext()->setViewport(m_pendingStates._viewport);
    }

private:
    ContextStates m_pendingStates;
};

    /*CommandSetRenderTarget*/
class CommandSetRenderTarget final : public Command
{
public:
    CommandSetRenderTarget(const RenderPassInfo& renderpassInfo, const std::vector<Image*>& attachments, const ClearValueInfo& clearInfo) noexcept
        : m_renderpassInfo(renderpassInfo)
        , m_attachments(attachments)
        , m_clearInfo(clearInfo)
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
        cmdList.getContext()->setRenderTarget(&m_renderpassInfo, m_attachments, &m_clearInfo);
    }

private:
    RenderPassInfo      m_renderpassInfo;
    std::vector<Image*> m_attachments;
    ClearValueInfo      m_clearInfo;
};

    /*CommandRemoveRenderTarget*/
class CommandRemoveRenderTarget final : public Command
{
public:
    CommandRemoveRenderTarget(const RenderPassInfo& renderpassInfo, const std::vector<Image*>& attachments, const ClearValueInfo& clearInfo) noexcept
        : m_renderpassInfo(renderpassInfo)
        , m_attachments(attachments)
        , m_clearInfo(clearInfo)
    {
        LOG_DEBUG("CommandRemoveRenderTarget constructor");
    };
    CommandRemoveRenderTarget() = delete;
    CommandRemoveRenderTarget(CommandRemoveRenderTarget&) = delete;

    ~CommandRemoveRenderTarget()
    {
        LOG_DEBUG("CommandRemoveRenderTarget destructor");
    };

    void execute(const CommandList& cmdList)
    {
        cmdList.getContext()->removeRenderTarget(&m_renderpassInfo, m_attachments, &m_clearInfo);
    }

private:
    RenderPassInfo      m_renderpassInfo;
    std::vector<Image*> m_attachments;
    ClearValueInfo      m_clearInfo;
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
{
    m_swapchainTexture = createObject<SwapchainTexture>();
    m_backbuffer = createObject<Backbuffer>(m_swapchainTexture);
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

    CommandList::flushPendingCommands(m_pendingFlushMask);
    CommandList::executeCommands();
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

void CommandList::clearBackbuffer(const core::Vector4D & color)
{
    m_swapchainTexture->clear(color);
}

void CommandList::setRenderTarget(RenderTarget* rendertarget)
{
    std::vector<renderer::Image*> images;
    images.reserve(rendertarget->getColorTextureCount() + (rendertarget->hasDepthStencilTexture()) ? 1 : 0);

    ClearValueInfo clearValuesInfo;
    clearValuesInfo._size = rendertarget->m_size;
    clearValuesInfo._color.reserve(images.size());

    RenderPassInfo renderPassInfo;

    renderPassInfo._countColorAttachments = rendertarget->getColorTextureCount();
    for (u32 index = 0; index < renderPassInfo._countColorAttachments; ++index)
    {
        auto attachment = rendertarget->m_colorTextures[index];

        images.push_back(std::get<0>(attachment)->getImage());
        renderPassInfo._attachments[index] = std::get<1>(attachment);

        clearValuesInfo._color.push_back(std::get<2>(attachment));
    }

    renderPassInfo._hasDepthStencilAttahment = rendertarget->hasDepthStencilTexture();
    if (renderPassInfo._hasDepthStencilAttahment)
    {
        images.push_back(rendertarget->getDepthStencilTexture()->getImage());
        renderPassInfo._attachments.back() = std::get<1>(rendertarget->m_depthStencilTexture);

        clearValuesInfo._depth = std::get<2>(rendertarget->m_depthStencilTexture);
        clearValuesInfo._stencil = std::get<3>(rendertarget->m_depthStencilTexture);
    }


    if (CommandList::isImmediate())
    {
        m_context->setRenderTarget(&renderPassInfo, images, &clearValuesInfo);
    }
    else
    {
        m_pendingRenderTargetInfo._attachments = std::move(images);
        m_pendingRenderTargetInfo._clearInfo = std::move(clearValuesInfo);
        m_pendingRenderTargetInfo._renderpassInfo = std::move(renderPassInfo);
        m_pendingFlushMask |= PendingFlush_UpdateRenderTarget;
    }
}

void CommandList::setPipelineState(PipelineState * pipeline)
{
    //TODO:
}

void CommandList::setViewport(const core::Rect32& viewport)
{
    if (CommandList::isImmediate())
    {
        m_context->setViewport(viewport);
    }
    else
    {
        m_pendingStates._viewport = viewport;
        m_pendingFlushMask |= PendingFlush_UpdateContextState;
    }
}

void CommandList::removeRenderTarget(RenderTarget* rendertarget)
{
    std::vector<renderer::Image*> images;
    images.reserve(rendertarget->getColorTextureCount() + (rendertarget->hasDepthStencilTexture()) ? 1 : 0);

    ClearValueInfo clearValuesInfo;
    clearValuesInfo._size = rendertarget->m_size;
    clearValuesInfo._color.reserve(images.size());

    RenderPassInfo renderPassInfo;

    renderPassInfo._countColorAttachments = rendertarget->getColorTextureCount();
    for (u32 index = 0; index < renderPassInfo._countColorAttachments; ++index)
    {
        auto attachment = rendertarget->m_colorTextures[index];

        images.push_back(std::get<0>(attachment)->getImage());
        renderPassInfo._attachments[index] = std::get<1>(attachment);

        clearValuesInfo._color.push_back(std::get<2>(attachment));
    }

    renderPassInfo._hasDepthStencilAttahment = rendertarget->hasDepthStencilTexture();
    if (renderPassInfo._hasDepthStencilAttahment)
    {
        images.push_back(rendertarget->getDepthStencilTexture()->getImage());
        renderPassInfo._attachments.back() = std::get<1>(rendertarget->m_depthStencilTexture);

        clearValuesInfo._depth = std::get<2>(rendertarget->m_depthStencilTexture);
        clearValuesInfo._stencil = std::get<3>(rendertarget->m_depthStencilTexture);
    }

    if (CommandList::isImmediate())
    {
        m_context->removeRenderTarget(&renderPassInfo, images, &clearValuesInfo);
    }
    else
    {
        CommandList::flushPendingCommands(PendingFlush_UpdateRenderTarget);
        CommandList::pushCommand(new CommandRemoveRenderTarget(renderPassInfo, images, clearValuesInfo));
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

void CommandList::flushPendingCommands(u32 pendingFlushMask)
{
    if (CommandList::isImmediate())
    {
        return;
    }

    if (pendingFlushMask & PendingFlush_UpdateRenderTarget)
    {
        CommandList::pushCommand(
            new CommandSetRenderTarget(m_pendingRenderTargetInfo._renderpassInfo, m_pendingRenderTargetInfo._attachments, m_pendingRenderTargetInfo._clearInfo));
        pendingFlushMask |= ~PendingFlush_UpdateRenderTarget;
    }

    if (pendingFlushMask & PendingFlush_UpdateContextState)
    {
        CommandList::pushCommand(new CommandSetContextState(m_pendingStates));
        pendingFlushMask |= ~PendingFlush_UpdateContextState;
    }
}

} //namespace renderer
} //namespace v3d
