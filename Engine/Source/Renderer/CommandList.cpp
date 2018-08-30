#include "CommandList.h"

namespace v3d
{
namespace renderer
{

CommandList::CommandList(Context* context, CommandListType type)
    : m_context(context)
    , m_commandListType(type)
{
}

CommandList::~CommandList()
{
}

void CommandList::cmdBeginFrame()
{
    if (isThreaded())
    {
        //create command
    }
    else
    {
        m_context->beginFrame();
    }
}

void CommandList::cmdEndFrame()
{
    if (isThreaded())
    {
        //create command
    }
    else
    {
        m_context->endFrame();
    }
 }

void CommandList::cmdPresentFrame()
{
    if (isThreaded())
    {
        //create command
    }
    else
    {
        m_context->presentFrame();
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

} //namespace renderer
} //namespace v3d
