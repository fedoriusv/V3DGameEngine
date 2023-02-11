#include "RenderFrameProfiler.h"
#include "Utils/Logger.h"
#include "Utils/Timer.h"

namespace v3d
{
namespace renderer
{

const std::string RenderFrameProfiler::s_frameCounterNames[] =
{
    "Frame",
    "DrawCalls",
    "ComputerCalls",
    "Submit",
    "Present",
    "SetRenderTarget",
    "SetPipeline",
    "SetRenderStates",
    "ResourceBinding",
    "Transitions",
    "UpdateSubmit",
    "CreateResources",
    "RemoveResources",
    "Query",
    "Custom"
};

RenderFrameProfiler::StackProfiler::StackProfiler(RenderFrameProfiler* profiler, FrameCounter counter) noexcept
    : m_profiler(profiler)
    , m_counter(counter)
{
    m_profiler->start(m_counter);
}

RenderFrameProfiler::StackProfiler::~StackProfiler()
{
    m_profiler->stop(m_counter);
}

RenderFrameProfiler::RenderFrameProfiler(const std::vector<FrameCounter>& activeTimers, const std::vector<FrameCounter>& activeCounters) noexcept
    : m_counterFPS(0)
    , m_frameTime(0.f)
{
    m_factor = IntervalFactor::PerSecond;
    m_staticstic._timer.resize(activeTimers.size(), {});
    m_staticstic._counter.resize(activeCounters.size(), {});

    for (u32 index = 0; index < m_metrics.size(); ++index)
    {
        Metric& metric = m_metrics[index];
        metric.reset();
        metric._recording = false;

        auto foundCounter = std::find(activeCounters.cbegin(), activeCounters.cend(), FrameCounter(index));
        if (foundCounter != activeCounters.cend())
        {
            metric._collectFlags = 0x1;
        }

        auto foundTimer = std::find(activeTimers.cbegin(), activeTimers.cend(), FrameCounter(index));
        if (foundTimer != activeTimers.cend())
        {
            metric._collectFlags |= 0x2;
        }
    }
}

void RenderFrameProfiler::start(FrameCounter counter)
{
    m_active = true;

    Metric& metric = m_metrics[toEnumType(counter)];
    if (metric._collectFlags)
    {
        ++metric._calls;
        if (metric._collectFlags & 0x2)
        {
            if (metric._recording)
            {
                return;
            }

            u64 startTime = utils::Timer::getCurrentTime<utils::Timer::Duration_NanoSeconds>();
            metric._startTime = startTime;
            metric._recording = true;
        }
    }
}

void RenderFrameProfiler::stop(FrameCounter counter)
{
    m_active = false;

    Metric& metric = m_metrics[toEnumType(counter)];
    if (metric._collectFlags)
    {
        if (metric._collectFlags & 0x2)
        {
            if (!metric._recording)
            {
                return;
            }

            u64 endTime = utils::Timer::getCurrentTime<utils::Timer::Duration_NanoSeconds>();
            ASSERT(metric._startTime <= endTime, "must be the greater");
            metric._duration += endTime - metric._startTime;
            metric._recording = false;
        }
    }
}

void RenderFrameProfiler::update(f32 dt)
{
    m_frameTime += dt;
    ++m_counterFPS;
}

void RenderFrameProfiler::reset()
{
    m_frameTime = 0;
    m_counterFPS = 0;
}

u32 RenderFrameProfiler::getAverageFPS() const
{
    return m_counterFPS;
}

f32 RenderFrameProfiler::getFrameTime() const
{
    return m_frameTime / m_counterFPS;
}

void RenderFrameProfiler::collect()
{
    m_staticstic._avgFPS = RenderFrameProfiler::getAverageFPS();
    m_staticstic._frameTime = RenderFrameProfiler::getFrameTime();

    const f32 koeff = (m_factor == IntervalFactor::PerSecond) ? (1.f / (f32)m_counterFPS) : 1.f;

    u32 indexCounters = 0;
    u32 indexTimers = 0;
    for (u32 index = 0; index < m_metrics.size(); ++index)
    {
        Metric& metric = m_metrics[index];
        if (metric._collectFlags & 0x1)
        {
            utils::Profiler::CommonMetric& counterMetric = m_staticstic._counter[indexCounters];
            counterMetric._type = 1;
            counterMetric._index = index;
            counterMetric._name = s_frameCounterNames[index];
            counterMetric._desctiption = "";
            counterMetric._unit = "calls";
            counterMetric._value = static_cast<f32>(metric._calls) * koeff;
            ++indexCounters;
        }

        if (metric._collectFlags & 0x2)
        {
            ASSERT(!metric._recording, "must be inactive");
            utils::Profiler::CommonMetric& timerMetric = m_staticstic._timer[indexTimers];
            timerMetric._type = 0;
            timerMetric._index = index;
            timerMetric._name = s_frameCounterNames[index];
            timerMetric._desctiption = "";
            timerMetric._unit = "ms";
            timerMetric._value = ((f32)metric._duration) * koeff / 1'000'000.f;
            ++indexTimers;
        }

        metric.reset();
    }
}

const utils::ProfilerStaticstics* RenderFrameProfiler::statistic() const
{
    return &m_staticstic;
}


void RenderFrameStaticstics::print() const
{
    LOG("FrameTimeStaticstics: FPS: %u (%.1f ms)", _avgFPS, _frameTime * 1'000.0f);

    printTimers();
    printCounters();
}

void RenderFrameStaticstics::printTimers() const
{
    for (auto& time : _timer)
    {
        LOG("FrameTimeStaticstics:: CPU Render Time %s: %.2f %s", time._name.c_str(), time._value, time._unit.c_str());
    }
}

void RenderFrameStaticstics::printCounters() const
{
    for (auto& counter : _counter)
    {
        LOG("FrameTimeStaticstics:: CPU Calls %s: %d %s", counter._name.c_str(), (u32)counter._value, counter._unit.c_str());
    }
}

} //namespace renderer
} //namespace v3d