#include "Profiler.h"

#define PRINT_TO_LOG 1

namespace v3d
{
namespace utils
{

bool Profiler::isActive() const
{
    return m_active;
}

ProfileManager::~ProfileManager()
{
    freeProfilers();
}

void ProfileManager::attach(Profiler* profiler)
{
    ASSERT(profiler, "nullptr");
    m_profilers.insert(profiler);
    
    if (profiler->statistic())
    {
        m_statistic.push_back(profiler->statistic());
    }
}

bool ProfileManager::dettach(Profiler* profiler)
{
    auto found = m_profilers.find(profiler);
    if (found != m_profilers.end())
    {
        m_profilers.erase(found);
        return true;
    }

    return false;
}

void ProfileManager::update()
{
    const u32 k_second = 1'000;

    if (m_profilers.empty())
    {
        return;
    }

    if (m_timer.isStopped())
    {
        m_timer.start();
    }
    static u64 s_prevTime = 0;
    const u64 spendingTime = m_timer.getTime<utils::Timer::Duration_MilliSeconds>();

    const f32 deltaTime = static_cast<f32>(std::max<s64>(static_cast<s64>(spendingTime) - static_cast<s64>(s_prevTime), 0)) / k_second;
    for (auto profiler : m_profilers)
    {
        profiler->update(deltaTime);
        //every frame
        if (profiler->m_factor == Profiler::IntervalFactor::PerFrame)
        {
            profiler->collect();
        }
    }

    if (spendingTime >= k_second)
    {
        m_timer.reset();
        for (auto profiler : m_profilers)
        {
            //every second
            if (profiler->m_factor == Profiler::IntervalFactor::PerSecond)
            {
                profiler->collect();
            }

#if PRINT_TO_LOG
            const ProfilerStaticstics* stat = profiler->statistic();
            if (stat)
            {
                stat->print();
            }
#endif
            profiler->reset();
        }
    }

    s_prevTime = spendingTime;
}

void ProfileManager::freeProfilers()
{
    m_statistic.clear();
    for (auto profiler : m_profilers)
    {
        ASSERT(!profiler->isActive(), "still active");
        delete profiler;
    }
    m_profilers.clear();
}

} //namespace utils
} //namespace v3d