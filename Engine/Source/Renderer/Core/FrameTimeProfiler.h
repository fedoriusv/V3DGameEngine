#pragma once

#include "Common.h"
#include "Utils/Profiler.h"

namespace v3d
{
namespace renderer
{

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief FrameTimeProfiler class
    */
    class FrameTimeProfiler : public utils::Profiler
    {
    public:

        FrameTimeProfiler() noexcept;
        ~FrameTimeProfiler() = default;

        void start() override;
        void stop()override;

        void update(f32 dt) override;
        void reset() override;

        u32 getAverageFPS() const;
        f32 getFrameTime() const;

        void printToLog() const override;

    private:

        u32 m_counterFPS;
        f32 m_frameTime;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace renderer
} //namespace v3d