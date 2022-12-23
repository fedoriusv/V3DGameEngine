#pragma once

#include "Common.h"
#include "Utils/Profiler.h"
#include "Query.h"

namespace v3d
{
namespace renderer
{

    /*class GPUTimeProfiler : public utils::Profiler
    {
    public:

        GPUTimeProfiler() noexcept;
        ~GPUTimeProfiler();

        void start() override;
        void stop()override;

        void update(f32 dt) override;
        void reset() override;

        void printToLog() const override;

        void attachBeginFrame(RenderQuery& query);
        void attachEndFrame(RenderQuery& query);

    private:

        void responseFrametime(const RenderQuery* query, const void* data);

        Query* m_frameStart;
        Query* m_frameEnd;
    };*/

} //namespace renderer
} //namespace v3d