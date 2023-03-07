#include "SceneProfiler.h"
#include "Utils/Timer.h"
#include "Utils/Logger.h"

namespace v3d
{
namespace scene
{

SceneProfiler::SceneProfiler() noexcept
{
}

void SceneProfiler::start(const std::string& name)
{
    m_active = true;

    u64 currTime = utils::Timer::getCurrentTime<utils::Timer::Duration::Duration_NanoSeconds>();
    auto found = m_collect.emplace(name, currTime);
    if (!found.second)
    {
        found.first->second = currTime;
    }
}

void SceneProfiler::stop(const std::string& name)
{
    m_active = false;

    auto found = m_collect.find(name);
    if (found != m_collect.end())
    {
        u64 currTime = utils::Timer::getCurrentTime<utils::Timer::Duration::Duration_NanoSeconds>();
        ASSERT(currTime >= found->second, "must be greater");
        found->second = currTime - found->second;
    }
}

void SceneProfiler::update(f32 dt)
{
    static f32 collectedDeltaTime = 0;
    collectedDeltaTime += dt;

    //Print every sec
    if (collectedDeltaTime >= 1.0f)
    {
        collectedDeltaTime = 0;

        SceneProfiler::collect();
        m_stat.print();
    }
}

void SceneProfiler::reset()
{
}

void SceneProfiler::collect()
{
    m_stat._statistic.resize(m_collect.size());
    u32 index = 0;
    for (auto& stat : m_collect)
    {
        utils::Profiler::CommonMetric& metric = m_stat._statistic[index];
        metric._type = 0;
        metric._index = index;
        metric._name = stat.first;
        metric._desctiption = "";
        metric._unit = "ms";
        metric._value = (f32)stat.second / 1'000'000.f;

        ++index;
    }
}

const utils::ProfilerStaticstics* SceneProfiler::statistic() const
{
    return &m_stat;
}


void SceneStatistic::print() const
{
    for (auto& stat : _statistic)
    {
        LOG("SceneStatistic:: CPU Scene Time %s: %.1f %s", stat._name.c_str(), stat._value, stat._unit.c_str());
    }
}

} //namespace scene
} //namespace v3d