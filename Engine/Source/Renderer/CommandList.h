#pragma once

#include "Context.h"

namespace v3d
{
namespace renderer
{
    class CommandList final //no cloneable
    {
    public:

        enum class CommandListType
        {
            ImmediateCommandList,
            ThreadCommandList
        };

        CommandList(Context* context, CommandListType type);
        ~CommandList();

        void cmdBeginFrame();
        void cmdEndFrame();
        void cmdPresentFrame();

        Context* getContext() const;
        bool     isThreaded() const;

        //....

    private:

        Context* m_context;
        CommandListType m_commandListType;
    };
} //namespace renderer
} //namespace v3d
