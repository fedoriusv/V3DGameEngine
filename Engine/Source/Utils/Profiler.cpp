#include "Profiler.h"

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
    freeAllProfilers();
}

void ProfileManager::attach(Profiler* profiler)
{
    ASSERT(profiler, "nullptr");
    m_profilers.insert(profiler);
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

void ProfileManager::start()
{
    for (auto& profiler : m_profilers)
    {
        if (!profiler->isActive())
        {
            profiler->start();
        }
    }
}

void ProfileManager::stop()
{
    for (auto& profiler : m_profilers)
    {
        if (profiler->isActive())
        {
            profiler->stop();
        }
    }
}

void ProfileManager::update()
{
    if (m_timer.isStopped())
    {
        m_timer.start();
    }
    static u64 s_prevTime = 0;
    const u64 currentTime = m_timer.getTime<utils::Timer::Duration_MilliSeconds>();

    const f32 deltaTime = static_cast<f32>(std::max<s64>(static_cast<s64>(currentTime) - static_cast<s64>(s_prevTime), 0)) / 1'000;
    for (auto& profiler : m_profilers)
    {
        profiler->update(deltaTime);
    }

    if (currentTime >= k_resetTime)
    {
        m_timer.reset();
        for (auto& profiler : m_profilers)
        {
#if PRINT_TO_LOG
            profiler->printToLog();
#endif
            profiler->reset();
        }
    }

    s_prevTime = currentTime;
}

void ProfileManager::freeAllProfilers()
{
    for (auto& profiler : m_profilers)
    {
        ASSERT(!profiler->isActive(), "still active");
        delete profiler;
    }
    m_profilers.clear();
}

} //namespace utils
} //namespace v3d