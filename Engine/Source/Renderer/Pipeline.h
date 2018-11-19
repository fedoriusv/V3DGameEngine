#pragma once

#include "Common.h"
#include "Context.h"
#include "Utils/Observable.h"

namespace v3d
{
namespace renderer
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    class Pipeline : public utils::Observable
    {
    public:
        Pipeline() noexcept {};
        virtual ~Pipeline() {};

        virtual bool create() = 0;
        virtual void destroy() = 0;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    class PipelineManager final
    {
    public:
        PipelineManager(Context* context)
            : m_context(context)
        {
        }
        
        ~PipelineManager()
        {
        }

        Pipeline* acquireFramebuffer()
        {
            //TODO:
        }

    private:

        Context* m_context;
        std::map<u32, Pipeline*> m_pipelineList;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace renderer
} //namespace v3d
