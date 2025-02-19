#pragma once

#include "Common.h"
#include "Utils/Profiler.h"

namespace v3d
{
namespace renderer
{

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief RenderFrameStaticstics struct
    */
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
            DrawCalls,
            ComputeCalls,
            Submit,
            GPUWait,
            Present,
            SetTarget,
            SetPipeline,
            SetStates,
            BindResources,
            Transitions,
            UpdateSubmitResorces,
            CreateResources,
            RemoveResources,
            QueryCommands,
            Custom,

            MaxValue
        };

        class StackProfiler final
        {
        public:

            StackProfiler(RenderFrameProfiler* profiler, u32 slot, FrameCounter counter) noexcept;
            ~StackProfiler();

        private:

            RenderFrameProfiler* m_profiler;
            FrameCounter         m_counter;
            u32                  m_slot;
        };

        RenderFrameProfiler(u32 slots, const std::vector<FrameCounter>& activeTimers, const std::vector<FrameCounter>& activeCounters) noexcept;
        ~RenderFrameProfiler() = default;

        void start(u32 slot, FrameCounter counter);
        void stop(u32 slot, FrameCounter counter);

    private:

        void update(f32 dt) override;
        void reset() override;

        void collect() override;
        const utils::ProfilerStaticstics* statistic() const override;

        u32 getAverageFPS() const;
        f32 getFrameTime() const;

        struct Metric
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

        std::vector<std::array<Metric, toEnumType(FrameCounter::MaxValue)>> m_metrics;
        u32 m_counterFPS;
        f32 m_frameTime;

        RenderFrameStaticstics m_staticstic;

        static const std::string s_frameCounterNames[toEnumType(FrameCounter::MaxValue)];
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace renderer
} //namespace v3d