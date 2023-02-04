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
    m_staticstic._timer.resize(activeTimers.size(), {});
    m_staticstic._counter.resize(activeCounters.size(), {});

    for (auto activeCounter : activeCounters)
    {
        m_metrics.emplace(activeCounter, Counter{ 0U, 0U, 0U, 0x1, false });
    }

    for (auto activeTimer : activeTimers)
    {
        auto found = m_metrics.emplace(activeTimer, Counter{ 0U, 0U, 0U, 0x2, false });
        if (!found.second)
        {
            found.first->second._collectFlags |= 0x2;
        }
    }
}

void RenderFrameProfiler::start(FrameCounter counter)
{
    m_active = true;

    auto found = m_metrics.find(counter);
    if (found != m_metrics.end())
    {
        ++found->second._calls;
        if (found->second._collectFlags & 0x2)
        {
            if (found->second._recording)
            {
                return;
            }

            u64 startTime = utils::Timer::getCurrentTime<utils::Timer::Duration_NanoSeconds>();
            found->second._startTime = startTime;
            found->second._recording = true;
        }
    }
}

void RenderFrameProfiler::stop(FrameCounter counter)
{
    m_active = false;

    auto found = m_metrics.find(counter);
    if (found != m_metrics.end())
    {
        if (found->second._collectFlags & 0x2)
        {
            if (!found->second._recording)
            {
                return;
            }

            u64 endTime = utils::Timer::getCurrentTime<utils::Timer::Duration_NanoSeconds>();
            ASSERT(found->second._startTime <= endTime, "must be the greater");
            found->second._duration += endTime - found->second._startTime;
            found->second._recording = false;
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

    u32 indexCounters = 0;
    u32 indexTimers = 0;
    for (auto iter = m_metrics.begin(); iter != m_metrics.end(); ++iter)
    {
        if (iter->second._collectFlags & 0x1)
        {
            utils::Profiler::CommonMetric& counterMetric = m_staticstic._counter[indexCounters];
            counterMetric._type = 1;
            counterMetric._index = toEnumType(iter->first);
            counterMetric._name = s_frameCounterNames[toEnumType(iter->first)];
            counterMetric._desctiption = "";
            counterMetric._unit = "calls";
            counterMetric._value = static_cast<f32>(iter->second._calls);
            ++indexCounters;
        }

        if (iter->second._collectFlags & 0x2)
        {
            ASSERT(!iter->second._recording, "must be inactive");
            utils::Profiler::CommonMetric& timerMetric = m_staticstic._timer[indexTimers];
            timerMetric._type = 0;
            timerMetric._index = toEnumType(iter->first);
            timerMetric._name = s_frameCounterNames[toEnumType(iter->first)];
            timerMetric._desctiption = "";
            timerMetric._unit = "ms";
            timerMetric._value = ((f32)iter->second._duration) / 1'000'000.f;
            ++indexTimers;
        }

        iter->second.reset();
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
        LOG("FrameTimeStaticstics:: CPU Render Time %s: %.1f %s", time._name.c_str(), time._value, time._unit.c_str());
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