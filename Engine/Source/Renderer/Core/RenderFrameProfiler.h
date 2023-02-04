#pragma once

#include "Common.h"
#include "Utils/Profiler.h"

namespace v3d
{
namespace renderer
{

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    struct RenderFrameStaticstics : utils::ProfilerStaticstics
    {
        RenderFrameStaticstics() noexcept = default;
        ~RenderFrameStaticstics() = default;

        std::vector<utils::Profiler::CommonMetric> _timer;
        std::vector<utils::Profiler::CommonMetric> _counter;
        u32 _avgFPS = 0U;
        f32 _frameTime = 0.f;

        void print() const override;

        void printTimers() const;
        void printCounters() const;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief RenderFrameProfiler class
    */
    class RenderFrameProfiler : public utils::Profiler
    {
    public:

        enum class FrameCounter : u32
        {
            FrameTime = 0,
            DrawCallCommands,
            ComputeCallCommands,
            SubmitCommand,
            PresentCommand,
            SetTargetCommands,
            SetPipelineCommands,
            SetStateCommands,
            BindResourceCommands,
            TransitionsCommands,
            UpdateSubmitResorces,
            CreateResources,
            RemoveResources,
            QueryCommands,

            MaxValue
        };

        class StackProfiler final
        {
        public:

            StackProfiler(RenderFrameProfiler* profiler, FrameCounter counter) noexcept;
            ~StackProfiler();

        private:

            RenderFrameProfiler* m_profiler;
            FrameCounter m_counter;
        };

        RenderFrameProfiler(const std::vector<FrameCounter>& activeTimers, const std::vector<FrameCounter>& activeCounters) noexcept;
        ~RenderFrameProfiler() = default;

        void start(FrameCounter counter);
        void stop(FrameCounter counter);

    private:

        void update(f32 dt) override;
        void reset() override;

        void collect() override;
        const utils::ProfilerStaticstics* statistic() const override;

        u32 getAverageFPS() const;
        f32 getFrameTime() const;

        struct Counter
        {
            u64 _startTime = 0U;
            u64 _duration = 0U;
            u64 _calls = 0U;

            u8 _collectFlags = 0x0;
            bool _recording = false;

            void reset()
            {
                _startTime = 0U;
                _duration = 0U;
                _calls = 0U;
            }
        };

        std::map<FrameCounter, Counter> m_metrics;
        u32 m_counterFPS;
        f32 m_frameTime;

        RenderFrameStaticstics m_staticstic;

        static const std::string s_frameCounterNames[toEnumType(FrameCounter::MaxValue)];
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace renderer
} //namespace v3d